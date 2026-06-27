param(
    [Parameter(Mandatory = $true)]
    [string]$RunRoot
)

$ErrorActionPreference = "Stop"
$resolvedRunRoot = [IO.Path]::GetFullPath($RunRoot)
$historyPath = Join-Path $resolvedRunRoot "data\diagnostics\health-history.jsonl"
$reportPath = Join-Path $resolvedRunRoot "soak-report.json"

if (-not (Test-Path $historyPath -PathType Leaf)) {
    throw "Health history not found: $historyPath"
}

$samples = @(
    Get-Content $historyPath |
        Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
        ForEach-Object { $_ | ConvertFrom-Json }
)
if ($samples.Count -eq 0) {
    throw "Health history contains no samples: $historyPath"
}

$reloadEvents = @()
if (Test-Path $reportPath -PathType Leaf) {
    $report = Get-Content $reportPath -Raw | ConvertFrom-Json
    $reloadEvents = @($report.reloadEvents)
}

$previous = $null
$dropEvents = [System.Collections.Generic.List[object]]::new()
$discontinuityEvents = [System.Collections.Generic.List[object]]::new()
$deadlineEvents = [System.Collections.Generic.List[object]]::new()

foreach ($sample in $samples) {
    if ($previous) {
        [long]$dropDelta = $sample.Dropped - $previous.Dropped
        [long]$discontinuityDelta = $sample.Discontinuities - $previous.Discontinuities
        [long]$deadlineDelta = $sample.DspDeadlineMisses - $previous.DspDeadlineMisses
        if ($dropDelta -gt 0) {
            $dropEvents.Add([ordered]@{
                timestamp = $sample.TimestampUtc
                dropDelta = $dropDelta
                dropped = $sample.Dropped
                discontinuities = $sample.Discontinuities
                deadlineMisses = $sample.DspDeadlineMisses
                paddingMaximum = $sample.PaddingMaximum
                frames = $sample.Frames
                packets = $sample.Packets
            })
        }
        if ($discontinuityDelta -gt 0) {
            $discontinuityEvents.Add([ordered]@{
                timestamp = $sample.TimestampUtc
                discontinuityDelta = $discontinuityDelta
                discontinuities = $sample.Discontinuities
                dropped = $sample.Dropped
                paddingMaximum = $sample.PaddingMaximum
            })
        }
        if ($deadlineDelta -gt 0) {
            $deadlineEvents.Add([ordered]@{
                timestamp = $sample.TimestampUtc
                deadlineMissDelta = $deadlineDelta
                deadlineMisses = $sample.DspDeadlineMisses
                dropped = $sample.Dropped
                paddingMaximum = $sample.PaddingMaximum
                maximumDspUs = $sample.DspMaximumUs
            })
        }
    }
    $previous = $sample
}

$first = $samples[0]
$last = $samples[$samples.Count - 1]
$summary = [ordered]@{
    runRoot = $resolvedRunRoot
    sampleCount = $samples.Count
    firstTimestampUtc = $first.TimestampUtc
    lastTimestampUtc = $last.TimestampUtc
    final = [ordered]@{
        frames = $last.Frames
        packets = $last.Packets
        dropped = $last.Dropped
        discontinuities = $last.Discontinuities
        conversionErrors = $last.ConversionErrors
        renderStarvations = $last.RenderStarvations
        renderErrors = $last.RenderErrors
        dspDeadlineMisses = $last.DspDeadlineMisses
        dspCriticalStalls = $last.DspCriticalStalls
        paddingMaximum = $last.PaddingMaximum
        bufferFrames = $last.BufferFrames
    }
    reloadCount = $reloadEvents.Count
    reloadEvents = @($reloadEvents | Select-Object timestamp, value, survived)
    dropEvents = @($dropEvents)
    discontinuityEvents = @($discontinuityEvents)
    deadlineEvents = @($deadlineEvents)
}

$summary | ConvertTo-Json -Depth 8

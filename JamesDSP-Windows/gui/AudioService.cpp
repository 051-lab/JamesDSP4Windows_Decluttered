#include "AudioService.h"
#include "DspController.h" // DspManager is defined here
#include <QDebug>
#include <QElapsedTimer>
#include <functiondiscoverykeys_devpkey.h>
#include <mmreg.h>
#include <ksmedia.h>
#include <vector>
#include <stdint.h>

// SAFE_RELEASE macro
template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// SEH Helper to trap driver crashes
HRESULT safeInitialize(IAudioClient* client, DWORD flags, WAVEFORMATEX* format) {
    __try {
        return client->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, 1000000, 0, format, NULL);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return E_FAIL; // Detected Crash
    }
}

AudioService& AudioService::instance() {
    static AudioService _instance;
    return _instance;
}

AudioService::AudioService() : m_running(0) {
    CoInitialize(NULL); // Init COM for the creating thread just in case
}

AudioService::~AudioService() {
    stopProcessing();
    wait();
    CoUninitialize();
}

void AudioService::setDevices(const QString& captureId, const QString& renderId) {
    QMutexLocker locker(&m_mutex);
    
    // Prevent redundant updates / restarts
    if (m_captureId == captureId && m_renderId == renderId) {
        return;
    }

    m_captureId = captureId;
    m_renderId = renderId;
    
    // If running, restart to apply changes could be an option, 
    // but for now let's rely on user toggling or explicit restart.
    if(m_running) {
        // ideally signal a restart or just log
        qDebug() << "Devices changed while running. Restart required.";
    }
}

void AudioService::startProcessing() {
    if(m_running) return;
    m_running = 1;
    emit stateChanged(true);
    start(QThread::TimeCriticalPriority);
}

void AudioService::stopProcessing() {
    m_running = 0;
    emit stateChanged(false);
    // Thread will exit loop and finish
}

HRESULT AudioService::initClient(IMMDevice* device, IAudioClient** client, WAVEFORMATEX** format) {
    HRESULT hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)client);
    if(FAILED(hr)) {
        qCritical() << "[AudioService] Failed to activate audio client. HR:" << QString::number(hr, 16);
        return hr;
    }

    hr = (*client)->GetMixFormat(format);
    if(FAILED(hr)) {
        qCritical() << "[AudioService] Failed to get mix format. HR:" << QString::number(hr, 16);
        return hr;
    }

    // Determine if Loopback is required
    // Loopback is needed if we are capturing from a Render device.
    DWORD flags = 0;
    IMMEndpoint* endpoint = NULL;
    hr = device->QueryInterface(__uuidof(IMMEndpoint), (void**)&endpoint);
    if(SUCCEEDED(hr)) {
        EDataFlow flow;
        hr = endpoint->GetDataFlow(&flow);
        if(SUCCEEDED(hr) && flow == eRender) {
             flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
             qDebug() << "[AudioService] Detected Render Device. Using Loopback Mode.";
        } else {
             qDebug() << "[AudioService] Detected Capture Device. Using Standard Capture Mode.";
        }
        SafeRelease(&endpoint);
    }

    // flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM; // SRC (Optional, might fail on some Win vers)

    qDebug() << "[AudioService] Pre-Init Check: Client=" << (void*)(*client) << " Format=" << (void*)(*format);
    if(*format) {
        qDebug() << "[AudioService] Format: Tag=" << (*format)->wFormatTag 
                 << " Rate=" << (*format)->nSamplesPerSec 
                 << " Channels=" << (*format)->nChannels 
                 << " Bits=" << (*format)->wBitsPerSample;
    }



    // Buffer Duration: 100ms (1000000 reftimes).
    // Use SEH wrapper
    hr = safeInitialize((*client), flags, *format);
    
    if(FAILED(hr)) {
         qCritical() << "[AudioService] Initialize failed (or Crashed). HR:" << QString::number(hr, 16) << " Flags:" << flags;
    }
    return hr;
}

void AudioService::run() {
    try {
        qDebug() << "[AudioService] Thread Started";
        CoInitialize(NULL); // Init COM for this thread
    
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pCaptureDevice = NULL;
    IMMDevice* pRenderDevice = NULL;
    IAudioClient* pCaptureClient = NULL;
    IAudioClient* pRenderClient = NULL;
    IAudioCaptureClient* pCaptureService = NULL;
    IAudioRenderClient* pRenderService = NULL;
    WAVEFORMATEX* pwfx = NULL; // Capture format (Authoritative)
    WAVEFORMATEX* pwfxRender = NULL; // Render format
    WAVEFORMATEX dspFormat; // Persistent struct for Render Config
    memset(&dspFormat, 0, sizeof(WAVEFORMATEX));

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    
    qDebug() << "[AudioService] Getting Default Render Device (for Loopback)...";
    if(SUCCEEDED(hr)) {
        if(m_captureId.isEmpty() || m_captureId == "Default") 
            hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pCaptureDevice); 
        else 
            hr = pEnumerator->GetDevice(m_captureId.toStdWString().c_str(), &pCaptureDevice);
    }
    if(FAILED(hr)) qCritical() << "[AudioService] Failed to get Capture Device. HR:" << QString::number(hr, 16);

    if(SUCCEEDED(hr)) {
         qDebug() << "[AudioService] Initializing Capture Client...";
         hr = initClient(pCaptureDevice, &pCaptureClient, &pwfx);
         if(FAILED(hr)) qCritical() << "[AudioService] Capture Client Init Failed. HR:" << QString::number(hr, 16);
         else qDebug() << "[AudioService] Capture Client Init SUCCESS";
    }

    if(SUCCEEDED(hr)) {
        qDebug() << "[AudioService] Getting Render Device...";
        // Get Render Device (Where we send the processed audio)
        // CAUTION: If Render Device == Capture Device, feedback loop!
        if(m_renderId.isEmpty() || m_renderId == "Default")
             hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pRenderDevice);
        else 
             hr = pEnumerator->GetDevice(m_renderId.toStdWString().c_str(), &pRenderDevice);
        
        if(FAILED(hr)) qCritical() << "[AudioService] Failed to get Render Device. HR:" << QString::number(hr, 16);
        else qDebug() << "[AudioService] Got Render Device.";
    }

    if(SUCCEEDED(hr)) {
        qDebug() << "[AudioService] Creating DSP Float Format...";
        // Create DSP Format (IEEE Float)
        dspFormat = *pwfx;
        dspFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        dspFormat.wBitsPerSample = 32;
        dspFormat.nBlockAlign = dspFormat.nChannels * 4;
        dspFormat.nAvgBytesPerSec = dspFormat.nSamplesPerSec * dspFormat.nBlockAlign;
        dspFormat.cbSize = 0; // Simple WAVEFORMATEX
        
        // ALWAYS initialize pwfxRender to dspFormat to prevent NULL dereference
        pwfxRender = &dspFormat;
        
        qDebug() << "[AudioService] Activating Render Client...";
        // Init Render Client with DSP Format (Float)
        // We use AUTOCONVERTPCM to let Windows convert Float -> Device Format
        DWORD flags = AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM; 
        
        hr = pRenderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pRenderClient);
        if(FAILED(hr)) {
            qCritical() << "[AudioService] Failed to activate Render Client. HR:" << QString::number(hr, 16);
        } else {
            qDebug() << "[AudioService] Render Client Activated. Initializing with Float format...";
            // Try initializing with DSP Format (Float32)
            hr = safeInitialize(pRenderClient, flags, &dspFormat);
            if(FAILED(hr)) {
                qDebug() << "[AudioService] Float init failed, trying capture format...";
                hr = safeInitialize(pRenderClient, flags, pwfx);
                if(SUCCEEDED(hr)) {
                    dspFormat = *pwfx;
                    pwfxRender = &dspFormat;
                    if(dspFormat.wFormatTag != WAVE_FORMAT_IEEE_FLOAT && dspFormat.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
                         qDebug() << "[AudioService] Warning: Render device rejected Float. Audio might be distorted.";
                    }
                }
            } else {
                qDebug() << "[AudioService] Render Client Init SUCCESS with Float format.";
            }
        }
    }

    // Init Services
    if(SUCCEEDED(hr)) {
        qDebug() << "[AudioService] Getting CaptureService...";
        hr = pCaptureClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureService);
        if(FAILED(hr)) qCritical() << "[AudioService] Failed to get CaptureService. HR:" << QString::number(hr, 16);
        else qDebug() << "[AudioService] Got CaptureService.";
    }
    if(SUCCEEDED(hr)) {
        qDebug() << "[AudioService] Getting RenderService...";
        hr = pRenderClient->GetService(__uuidof(IAudioRenderClient), (void**)&pRenderService);
        if(FAILED(hr)) qCritical() << "[AudioService] Failed to get RenderService. HR:" << QString::number(hr, 16);
        else qDebug() << "[AudioService] Got RenderService.";
    }

    // Init DSP
    if(SUCCEEDED(hr)) {
        qDebug() << "[AudioService] Initializing DspManager...";
        DspManager::instance().init(pwfx->nSamplesPerSec, 4096);
        qDebug() << "[AudioService] DspManager Initialized. Starting audio streams...";
        
        pCaptureClient->Start();
        pRenderClient->Start();
        qDebug() << "[AudioService] Audio streams started. Entering processing loop...";
        
        UINT32 packetLength = 0;
        UINT32 numFramesPadding = 0;
        BYTE* pData = NULL;
        BYTE* pRenderData = NULL;
        DWORD flags = 0;

        // Buffer for conversion
        std::vector<float> fBuffer;
        
        uint64_t totalFrames = 0;
        QElapsedTimer timer;
        timer.start();

        while(m_running) {
            hr = pCaptureService->GetNextPacketSize(&packetLength);
            
            while(packetLength != 0) {
                hr = pCaptureService->GetBuffer(&pData, &packetLength, &flags, NULL, NULL);
                
                if(SUCCEEDED(hr)) {
                    if(packetLength > 0 && pData) {
                         // Conversion Logic: Capture(pwfx) -> Float(fBuffer)
                         size_t numSamples = packetLength * pwfx->nChannels;
                         fBuffer.resize(numSamples);
                         
                         bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT);
                         
                         if(silent) {
                             memset(fBuffer.data(), 0, numSamples * sizeof(float));
                         } else {
                             // Convert
                             // Check Capture Format
                             if(pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
                               (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
                                 // Already Float
                                 memcpy(fBuffer.data(), pData, numSamples * sizeof(float));
                             }
                             else if(pwfx->wBitsPerSample == 16) {
                                 int16_t* inputs = (int16_t*)pData;
                                 for(size_t i=0; i<numSamples; ++i) fBuffer[i] = inputs[i] / 32768.0f;
                             }
                             else if(pwfx->wBitsPerSample == 32) {
                                 // Int32
                                 int32_t* inputs = (int32_t*)pData;
                                 for(size_t i=0; i<numSamples; ++i) fBuffer[i] = inputs[i] / 2147483648.0f;
                             }
                             else if(pwfx->wBitsPerSample == 24) {
                                 // 24-bit packed
                                 uint8_t* ptr = pData;
                                 for(size_t i=0; i<numSamples; ++i) {
                                     int32_t val = (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
                                     if (val & 0x800000) val |= 0xFF000000; // Sign extend
                                     fBuffer[i] = val / 8388608.0f;
                                     ptr += 3;
                                 }
                             }
                         }

                         // DSP Process (In-place on fBuffer)
                         if(!silent) {
                             QElapsedTimer stallTimer; stallTimer.start();
                             DspManager::instance().process(fBuffer.data(), packetLength);
                             if(stallTimer.elapsed() > 100) {
                                 qWarning() << "[AudioService] DSP Stalled for" << stallTimer.elapsed() << "ms at frame" << totalFrames;
                             }
                         }

                         // Write to Render (Converted to RenderFormat if needed)
                         // RenderFormat is dspFormat (Float) usually.
                         
                         hr = pRenderClient->GetCurrentPadding(&numFramesPadding);
                         UINT32 bufferSize = 0;
                         pRenderClient->GetBufferSize(&bufferSize);
                         
                         if(bufferSize - numFramesPadding >= packetLength) {
                             hr = pRenderService->GetBuffer(packetLength, &pRenderData);
                             if(SUCCEEDED(hr)) {
                                 if(pwfxRender->wFormatTag == WAVE_FORMAT_IEEE_FLOAT || 
                                    (pwfxRender->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)pwfxRender)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
                                      // Render expects Float
                                      memcpy(pRenderData, fBuffer.data(), numSamples * sizeof(float));
                                 } else {
                                      size_t renderBytes = packetLength * pwfxRender->nBlockAlign;
                                      size_t floatBytes = numSamples * sizeof(float);
                                      if(renderBytes >= floatBytes) {
                                           memcpy(pRenderData, fBuffer.data(), floatBytes);
                                      } else {
                                           memset(pRenderData, 0, renderBytes);
                                      }
                                 }
                                 pRenderService->ReleaseBuffer(packetLength, 0);
                             }
                         }
                    }
                    
                    pCaptureService->ReleaseBuffer(packetLength);
                }
                
                hr = pCaptureService->GetNextPacketSize(&packetLength);
                totalFrames++;
                if(totalFrames % 500 == 0) {
                    qDebug() << "[AudioService] Heartbeat. Total Frames:" << totalFrames << ". Uptime:" << timer.elapsed()/1000 << "s";
                }
            }
            
            Sleep(1); 
        }
        
        pCaptureClient->Stop();
        pRenderClient->Stop();
    } else {
        emit errorOccurred("Failed to initialize Audio Engine (WASAPI). HR: " + QString::number(hr, 16));
    }

    // Cleanup
    SafeRelease(&pEnumerator);
    SafeRelease(&pCaptureDevice);
    SafeRelease(&pRenderDevice);
    SafeRelease(&pCaptureClient);
    SafeRelease(&pRenderClient);
    SafeRelease(&pCaptureService);
    SafeRelease(&pRenderService);
    CoTaskMemFree(pwfx); 
    // pwfxRender points to dspFormat (stack) or pwfx (heap). We freed pwfx. Do not free pwfxRender.
    qDebug() << "[AudioService] Thread Cleanup Done";
    
    CoUninitialize();
    } catch (...) {
        qCritical() << "[AudioService] CRITICAL EXCEPTION in Thread Run!";
    }
}

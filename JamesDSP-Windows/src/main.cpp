/**
 * JamesDSP for Windows
 * 
 * Full-featured audio DSP processor using WASAPI loopback.
 * Replicates Linux JamesDSP functionality.
 * 
 * Usage:
 *   JamesDSP-Console.exe [-o <device_index>] [-c <config_file>]
 * 
 * Keys:
 *   R - Reload configuration from file
 *   S - Show current DSP status
 *   Q - Quit (or Ctrl+C)
 */

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <conio.h>

#include "DspConfig.h"

// Reference time units
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

// Global state
static std::atomic<bool> g_running{true};
static std::atomic<bool> g_reloadConfig{false};
static std::atomic<bool> g_showStatus{false};
static JamesDSPLib* g_dsp = nullptr;
static DspController* g_controller = nullptr;
static DspConfig g_config;
static std::string g_configFile;

// Console handler for Ctrl+C
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT) {
        std::cout << "\n[INFO] Stopping..." << std::endl;
        g_running = false;
        return TRUE;
    }
    return FALSE;
}

// Keyboard input thread
void keyboardThread() {
    while (g_running) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'r' || ch == 'R') {
                g_reloadConfig = true;
            } else if (ch == 's' || ch == 'S') {
                g_showStatus = true;
            } else if (ch == 'q' || ch == 'Q') {
                g_running = false;
            }
        }
        Sleep(50);
    }
}

// Initialize JamesDSP library
bool initDsp(int sampleRate, int blockSize) {
    g_dsp = new JamesDSPLib();
    if (!g_dsp) {
        std::cerr << "[ERROR] Failed to allocate JamesDSPLib" << std::endl;
        return false;
    }
    
    memset(g_dsp, 0, sizeof(JamesDSPLib));
    JamesDSPGlobalMemoryAllocation();
    JamesDSPInit(g_dsp, blockSize, static_cast<float>(sampleRate));
    
    g_controller = new DspController(g_dsp, sampleRate);
    
    std::cout << "[INFO] JamesDSP initialized @ " << sampleRate << " Hz, block size: " << blockSize << std::endl;
    return true;
}

// Cleanup DSP
void cleanupDsp() {
    if (g_controller) {
        delete g_controller;
        g_controller = nullptr;
    }
    if (g_dsp) {
        JamesDSPFree(g_dsp);
        JamesDSPGlobalMemoryDeallocation();
        delete g_dsp;
        g_dsp = nullptr;
    }
}

// Get default audio endpoint device
IMMDevice* GetDefaultDevice(EDataFlow flow, ERole role) {
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&enumerator
    );
    
    if (FAILED(hr)) {
        std::cerr << "[ERROR] Failed to create device enumerator" << std::endl;
        return nullptr;
    }
    
    hr = enumerator->GetDefaultAudioEndpoint(flow, role, &device);
    enumerator->Release();
    
    if (FAILED(hr)) {
        std::cerr << "[ERROR] Failed to get default audio endpoint" << std::endl;
        return nullptr;
    }
    
    return device;
}

// List audio devices
void ListDevices(EDataFlow flow) {
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* collection = nullptr;
    
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    
    if (!enumerator) return;
    
    enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &collection);
    
    if (!collection) {
        enumerator->Release();
        return;
    }
    
    UINT count;
    collection->GetCount(&count);
    
    std::wcout << L"\n" << (flow == eRender ? L"OUTPUT" : L"INPUT") << L" DEVICES:" << std::endl;
    std::wcout << L"----------------------------------------" << std::endl;
    
    for (UINT i = 0; i < count; i++) {
        IMMDevice* device = nullptr;
        collection->Item(i, &device);
        
        if (device) {
            IPropertyStore* props = nullptr;
            device->OpenPropertyStore(STGM_READ, &props);
            
            if (props) {
                PROPVARIANT name;
                PropVariantInit(&name);
                props->GetValue(PKEY_Device_FriendlyName, &name);
                
                std::wcout << L"[" << i << L"] " << name.pwszVal << std::endl;
                
                PropVariantClear(&name);
                props->Release();
            }
            
            device->Release();
        }
    }
    
    collection->Release();
    enumerator->Release();
}

// Print usage
void PrintUsage() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   JamesDSP for Windows" << std::endl;
    std::cout << "========================================\n" << std::endl;
    std::cout << "Usage: JamesDSP-Console.exe [-o <device>] [-c <config>]\n" << std::endl;
    std::cout << "Setup:" << std::endl;
    std::cout << "  1. Set VB-Cable as Windows default output" << std::endl;
    std::cout << "  2. Run this program with -o to select real output" << std::endl;
    std::cout << "\nKeys:" << std::endl;
    std::cout << "  R - Reload config.ini" << std::endl;
    std::cout << "  S - Show DSP status" << std::endl;
    std::cout << "  Q - Quit" << std::endl;
}

// Main audio processing loop
void AudioProcessingLoop(IMMDevice* captureDevice, IMMDevice* renderDevice) {
    IAudioClient* captureClient = nullptr;
    IAudioClient* renderClient = nullptr;
    IAudioCaptureClient* captureService = nullptr;
    IAudioRenderClient* renderService = nullptr;
    WAVEFORMATEX* captureFormat = nullptr;
    WAVEFORMATEX* renderFormat = nullptr;
    HANDLE captureEvent = nullptr;
    std::thread* kbThread = nullptr;
    
    HRESULT hr;
    
    // Create event for capture notifications
    captureEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!captureEvent) {
        std::cerr << "[ERROR] Failed to create capture event" << std::endl;
        goto cleanup;
    }
    
    // Initialize capture
    hr = captureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&captureClient);
    if (FAILED(hr)) {
        std::cerr << "[ERROR] Failed to activate capture device" << std::endl;
        goto cleanup;
    }
    
    hr = captureClient->GetMixFormat(&captureFormat);
    if (FAILED(hr)) {
        std::cerr << "[ERROR] Failed to get capture format" << std::endl;
        goto cleanup;
    }
    
    std::cout << "[INFO] Capture: " << captureFormat->nSamplesPerSec << " Hz, " 
              << captureFormat->nChannels << " ch" << std::endl;
    
    {
        REFERENCE_TIME bufferDuration = REFTIMES_PER_SEC / 10; // 100ms
        hr = captureClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            bufferDuration, 0, captureFormat, nullptr
        );
        
        if (FAILED(hr)) {
            std::cerr << "[ERROR] Capture init failed: 0x" << std::hex << hr << std::endl;
            goto cleanup;
        }
        
        captureClient->SetEventHandle(captureEvent);
    }
    
    hr = captureClient->GetService(__uuidof(IAudioCaptureClient), (void**)&captureService);
    if (FAILED(hr)) goto cleanup;
    
    // Initialize render
    hr = renderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&renderClient);
    if (FAILED(hr)) goto cleanup;
    
    hr = renderClient->GetMixFormat(&renderFormat);
    if (FAILED(hr)) goto cleanup;
    
    std::cout << "[INFO] Render: " << renderFormat->nSamplesPerSec << " Hz, "
              << renderFormat->nChannels << " ch" << std::endl;
    
    {
        REFERENCE_TIME bufferDuration = REFTIMES_PER_SEC / 10;
        hr = renderClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0,
            bufferDuration, 0, renderFormat, nullptr);
        if (FAILED(hr)) goto cleanup;
    }
    
    hr = renderClient->GetService(__uuidof(IAudioRenderClient), (void**)&renderService);
    if (FAILED(hr)) goto cleanup;
    
    // Initialize DSP
    {
        int dspBlockSize = 960;
        if (!initDsp(captureFormat->nSamplesPerSec, dspBlockSize)) goto cleanup;
        
        // Load and apply config
        ConfigFile::load(g_configFile, g_config);
        g_controller->applyConfig(g_config);
        g_controller->printStatus();
    }
    
    // Pre-fill render buffer
    {
        UINT32 bufferFrames;
        renderClient->GetBufferSize(&bufferFrames);
        BYTE* data;
        renderService->GetBuffer(bufferFrames / 2, &data);
        if (data) {
            memset(data, 0, (bufferFrames / 2) * renderFormat->nBlockAlign);
            renderService->ReleaseBuffer(bufferFrames / 2, AUDCLNT_BUFFERFLAGS_SILENT);
        }
    }
    
    // Start keyboard thread
    kbThread = new std::thread(keyboardThread);
    
    // Start streams
    captureClient->Start();
    renderClient->Start();
    
    std::cout << "\n[INFO] Processing started. Press R=reload, S=status, Q=quit\n" << std::endl;
    
    // Main loop
    while (g_running) {
        // Check for config reload request
        if (g_reloadConfig.exchange(false)) {
            std::cout << "\n[INFO] Reloading config..." << std::endl;
            ConfigFile::load(g_configFile, g_config);
            g_controller->applyConfig(g_config);
            g_controller->printStatus();
        }
        
        // Check for status request
        if (g_showStatus.exchange(false)) {
            g_controller->printStatus();
        }
        
        // Wait for capture event
        DWORD waitResult = WaitForSingleObject(captureEvent, 100);
        if (waitResult == WAIT_TIMEOUT) continue;
        if (waitResult != WAIT_OBJECT_0) break;
        
        // Process packets
        UINT32 packetLength = 0;
        hr = captureService->GetNextPacketSize(&packetLength);
        
        while (SUCCEEDED(hr) && packetLength > 0 && g_running) {
            BYTE* captureData = nullptr;
            UINT32 numFrames = 0;
            DWORD flags = 0;
            
            hr = captureService->GetBuffer(&captureData, &numFrames, &flags, nullptr, nullptr);
            if (FAILED(hr)) break;
            
            if (numFrames > 0) {
                UINT32 padding = 0;
                UINT32 renderBufSize = 0;
                renderClient->GetBufferSize(&renderBufSize);
                renderClient->GetCurrentPadding(&padding);
                
                UINT32 available = renderBufSize - padding;
                UINT32 toWrite = min(numFrames, available);
                
                if (toWrite > 0) {
                    BYTE* renderData = nullptr;
                    hr = renderService->GetBuffer(toWrite, &renderData);
                    
                    if (SUCCEEDED(hr) && renderData) {
                        bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0;
                        
                        if (silent) {
                            memset(renderData, 0, toWrite * renderFormat->nBlockAlign);
                        } else {
                            if (g_dsp && g_dsp->processFloatMultiplexd) {
                                g_dsp->processFloatMultiplexd(g_dsp, 
                                    (float*)captureData, (float*)renderData, toWrite);
                            } else {
                                memcpy(renderData, captureData, toWrite * captureFormat->nBlockAlign);
                            }
                        }
                        
                        renderService->ReleaseBuffer(toWrite, 0);
                    }
                }
            }
            
            captureService->ReleaseBuffer(numFrames);
            hr = captureService->GetNextPacketSize(&packetLength);
        }
    }
    
    std::cout << "\n[INFO] Stopping..." << std::endl;
    
    captureClient->Stop();
    renderClient->Stop();
    
    // Wait for keyboard thread
    if (kbThread && kbThread->joinable()) kbThread->join();
    delete kbThread;
    
cleanup:
    cleanupDsp();
    if (captureEvent) CloseHandle(captureEvent);
    if (renderService) renderService->Release();
    if (captureService) captureService->Release();
    if (renderFormat) CoTaskMemFree(renderFormat);
    if (captureFormat) CoTaskMemFree(captureFormat);
    if (renderClient) renderClient->Release();
    if (captureClient) captureClient->Release();
}

int main(int argc, char* argv[]) {
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    
    // Parse args
    int selectedOutput = -1;
    g_configFile = ConfigFile::getDefaultPath();
    
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
            selectedOutput = atoi(argv[++i]);
        }
        else if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) && i + 1 < argc) {
            g_configFile = argv[++i];
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "JamesDSP for Windows\n" << std::endl;
            std::cout << "Usage: JamesDSP-Console.exe [options]\n" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -o, --output <idx>   Output device index" << std::endl;
            std::cout << "  -c, --config <file>  Config file path" << std::endl;
            std::cout << "  -h, --help           Show this help" << std::endl;
            return 0;
        }
    }
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "[ERROR] COM init failed" << std::endl;
        return 1;
    }
    
    PrintUsage();
    ListDevices(eRender);
    
    // Get capture device (default = VB-Cable)
    IMMDevice* captureDevice = GetDefaultDevice(eRender, eConsole);
    if (!captureDevice) {
        std::cerr << "[ERROR] No capture device" << std::endl;
        CoUninitialize();
        return 1;
    }
    
    // Print capture device name
    {
        IPropertyStore* props;
        captureDevice->OpenPropertyStore(STGM_READ, &props);
        PROPVARIANT name;
        PropVariantInit(&name);
        props->GetValue(PKEY_Device_FriendlyName, &name);
        std::wcout << L"\n[INFO] Capture: " << name.pwszVal << std::endl;
        PropVariantClear(&name);
        props->Release();
    }
    
    // Get output device
    IMMDevice* renderDevice = nullptr;
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* collection = nullptr;
    
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    
    if (enumerator) {
        enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
        
        if (collection) {
            UINT count;
            collection->GetCount(&count);
            
            LPWSTR captureId;
            captureDevice->GetId(&captureId);
            
            if (selectedOutput >= 0 && (UINT)selectedOutput < count) {
                collection->Item(selectedOutput, &renderDevice);
                
                LPWSTR id;
                renderDevice->GetId(&id);
                if (wcscmp(id, captureId) == 0) {
                    std::cerr << "[ERROR] Output same as capture" << std::endl;
                    renderDevice->Release();
                    renderDevice = nullptr;
                }
                CoTaskMemFree(id);
            } else {
                // Auto-select first non-capture device
                for (UINT i = 0; i < count; i++) {
                    IMMDevice* dev;
                    collection->Item(i, &dev);
                    LPWSTR id;
                    dev->GetId(&id);
                    if (wcscmp(id, captureId) != 0) {
                        renderDevice = dev;
                        CoTaskMemFree(id);
                        break;
                    }
                    CoTaskMemFree(id);
                    dev->Release();
                }
            }
            
            CoTaskMemFree(captureId);
            collection->Release();
        }
        enumerator->Release();
    }
    
    if (!renderDevice) {
        std::cerr << "[ERROR] No output device. Use -o <index>" << std::endl;
        captureDevice->Release();
        CoUninitialize();
        return 1;
    }
    
    // Print output device name
    {
        IPropertyStore* props;
        renderDevice->OpenPropertyStore(STGM_READ, &props);
        PROPVARIANT name;
        PropVariantInit(&name);
        props->GetValue(PKEY_Device_FriendlyName, &name);
        std::wcout << L"[INFO] Output: " << name.pwszVal << std::endl;
        PropVariantClear(&name);
        props->Release();
    }
    
    std::cout << "[INFO] Config: " << g_configFile << std::endl;
    
    // Run
    AudioProcessingLoop(captureDevice, renderDevice);
    
    captureDevice->Release();
    renderDevice->Release();
    CoUninitialize();
    
    std::cout << "[INFO] Done." << std::endl;
    return 0;
}

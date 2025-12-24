#ifndef DEVICE_ENUMERATOR_H
#define DEVICE_ENUMERATOR_H

#include <QList>
#include <QPair>
#include <QString>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

class DeviceEnumerator {
public:
    struct DeviceInfo {
        QString id;
        QString name;
    };

    static QList<DeviceInfo> getPlaybackDevices() {
        return enumerate(eRender);
    }

    static QList<DeviceInfo> getRecordingDevices() {
        return enumerate(eCapture);
    }

private:
    static QList<DeviceInfo> enumerate(EDataFlow dataFlow) {
        QList<DeviceInfo> devices;
        HRESULT hr;
        IMMDeviceEnumerator *pEnumerator = NULL;
        IMMDeviceCollection *pCollection = NULL;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                              CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                              (void**)&pEnumerator);
        if (FAILED(hr)) return devices;

        hr = pEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pCollection);
        if (FAILED(hr)) {
            pEnumerator->Release();
            return devices;
        }

        UINT count;
        pCollection->GetCount(&count);

        for (UINT i = 0; i < count; i++) {
            IMMDevice *pEndpoint = NULL;
            hr = pCollection->Item(i, &pEndpoint);
            if (FAILED(hr)) continue;

            LPWSTR pwszID = NULL;
            hr = pEndpoint->GetId(&pwszID);
            if (FAILED(hr)) {
                pEndpoint->Release();
                continue;
            }

            IPropertyStore *pProps = NULL;
            hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
            if (FAILED(hr)) {
                CoTaskMemFree(pwszID);
                pEndpoint->Release();
                continue;
            }

            PROPVARIANT varName;
            PropVariantInit(&varName);
            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
            if (FAILED(hr)) {
                pProps->Release();
                CoTaskMemFree(pwszID);
                pEndpoint->Release();
                continue;
            }

            QString name = QString::fromWCharArray(varName.pwszVal);
            QString id = QString::fromWCharArray(pwszID);
            devices.append({id, name});

            PropVariantClear(&varName);
            pProps->Release();
            CoTaskMemFree(pwszID);
            pEndpoint->Release();
        }

        pCollection->Release();
        pEnumerator->Release();
        return devices;
    }
};

#endif // DEVICE_ENUMERATOR_H

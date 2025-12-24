#ifndef AUDIOSERVICE_H
#define AUDIOSERVICE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QAtomicInt>
#include <QString>
#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

class AudioService : public QThread {
    Q_OBJECT
public:
    static AudioService& instance();

    void setDevices(const QString& captureId, const QString& renderId);
    void startProcessing();
    void stopProcessing();
    bool isRunning() { return m_running; }
    QString getCaptureId() { QMutexLocker l(&m_mutex); return m_captureId; }
    QString getRenderId() { QMutexLocker l(&m_mutex); return m_renderId; }

signals:
    void errorOccurred(const QString& msg);
    void stateChanged(bool running);

protected:
    void run() override;

private:
    AudioService();
    ~AudioService();

    QString m_captureId;
    QString m_renderId;
    
    QAtomicInt m_running;
    QMutex m_mutex;
    
    // WASAPI Helpers
    HRESULT initClient(IMMDevice* device, IAudioClient** client, WAVEFORMATEX** format);
};

#endif // AUDIOSERVICE_H

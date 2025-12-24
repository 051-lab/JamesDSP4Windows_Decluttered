#ifndef DSPCONTROLLER_H
#define DSPCONTROLLER_H

#include <QObject>
#include <QMutex>

// Forward declaration of JamesDSPLib struct
extern "C" {
    #include "jdsp_header.h"
}

class DspManager : public QObject {
    Q_OBJECT
public:
    static DspManager& instance() {
        static DspManager _instance;
        return _instance;
    }

    void init(int sampleRate, int blockSize);
    void process(float* buffer, int frames); // Simple stereo interleaved for now

    // Effect Control Methods
    void setBassBoost(bool enabled, int maxGain);
    void setToneControl(bool enabled, float drive); // Tube
    void setReverb(bool enabled, int preset);
    void setLimiter(double threshold, double release, double postGain);
    void setConvolver(bool enabled, const QString& irFile, int optimization = 0, const QString& advParams = "-80;-100;0;1;0;0");
    void setLiveprog(bool enabled, const QString& eelScript);
    void setLiveProgContent(bool enabled, const QString& content);
    
    // Missing effects verified
    void setEqualizer(bool enabled, int mode, int interp, const std::vector<double>& gains);
    // Overload or helper for changing single band? 
    // Easier to simplified setter:
    void setEqualizerParams(bool enabled, int mode, int interp);
    void setEqualizerBand(int index, double gain);
    
    void setCrossfeed(bool enabled, int mode);
    void setStereoWide(bool enabled, int level);
    void setDdc(bool enabled, const QString& vdcFile);
    void setCompander(bool enabled, bool modeA, double timeConst, int granularity, int tfresolution = 0); // Basic set
    void setCompanderPoints(const std::vector<double>& gains); // Updates the 7-band curve
    
    // New Setters
    void setArbitraryEq(bool enabled, const QString& curveString);
    void setLiveProgParam(const QString& name, double value);

    JamesDSPLib* getLib() { return &m_jdsp; }
    
    // Visualization Helper
    bool getImpulseResponse(std::vector<float>& out, int& channels);

private:
    DspManager();
    ~DspManager();

    JamesDSPLib m_jdsp;
    QMutex m_mutex;
    bool m_initialized = false;
    
    // Cache for visualization
    std::vector<float> m_cachedIrData;
    int m_cachedIrChannels = 0;
};

#endif // DSPMANAGER_H

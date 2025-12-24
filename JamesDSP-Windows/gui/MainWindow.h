/**
 * JamesDSP for Windows - Main Window Header
 * Tabbed Layout with Android-style Cards
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTabWidget>
#include <QGroupBox>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QScrollArea>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QMap>

class GraphicEqPage;
class CompanderPage;
class EffectCard; // Defined in widgets.h
class FrequencyResponseWidget;
class CompanderCurveWidget;
class LiveprogEditorWidget;
class LiveprogParamsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void toggleProcessing(bool enabled);
    void updateStatus();
    
    // Output Control
    void onLimiterThresholdChanged(int value);
    void onLimiterReleaseChanged(int value);
    void onPostGainChanged(int value);
    
    // Bass - Aligned names
    void onBassBoostEnabledChanged(bool enabled);
    void onBassBoostGainChanged(int value);
    
    // Tube - Aligned names
    void onTubeEnabledChanged(bool enabled);
    void onTubeDriveChanged(int value);
    
    // Compander - Aligned names
    void onCompanderEnabledChanged(bool enabled);
    void onCompanderTimeChanged(int value);
    void onCompanderGranularityChanged(int index);
    void onCompanderTFChanged(int index);
    
    // Stereo Wide - Aligned names
    void onStereoEnabledChanged(bool enabled);
    void onStereoLevelChanged(int value);
    
    // Equalizer - Aligned names
    // Equalizer
    void onEqEnabledChanged(bool enabled);
    void onEqBandChanged(int bandIndex, int value);
    void onEqModeChanged(int index);
    void onEqInterpChanged(int index);
    
    // Reverb
    void onReverbEnabledChanged(bool enabled);
    void onReverbPresetChanged(int index);
    
    // Crossfeed
    void onCrossfeedEnabledChanged(bool enabled);
    void onCrossfeedPresetChanged(int index);

    // Convolver/DDC/Liveprog
    void onConvolverEnabledChanged(bool enabled);
    void onConvolverFileSelected(const QString& path);
    void onConvolverOptimizationChanged(int index);
    void onDdcEnabledChanged(bool enabled);
    void onDdcFileSelected(const QString& path);
    void onLiveprogEnabledChanged(bool enabled);
    void onLiveprogFileSelected(const QString& path);
    
    // Graphic EQ
    void onGraphicEqEnabledChanged(bool enabled);
    
    // Settings
    void savePreset();
    void loadPreset();
    void resetDefaults();

private:
    void setupTheme();
    void setupUi();
    void createTabs(); 
    void createTrayIcon();
    void connectSignals();
    void populateDevices();
    void loadSettings();
    void saveSettings();
    
    // Tab creation
    QWidget* createOutputTab();
    QWidget* createSoundstageTab();
    QWidget* createEqualizersTab();
    QWidget* createConvolverTab();
    QWidget* createLiveprogTab();
    QWidget* createSettingsTab();
    
    // Helpers
    QGroupBox* createEffectGroup(const QString& title, QCheckBox** enableCheck);
    QSlider* createSlider(int min, int max, int value, Qt::Orientation orient = Qt::Horizontal);
    QWidget* createLabeledSlider(const QString& label, QSlider* slider, QLabel** valueLabel);
    QWidget* createLabeledSliderWithSpinBox(const QString& label, QSlider* slider, QLabel** valueLabel, double scaleFactor, const QString& suffix, int decimals);
    
    // UI Core
    QTabWidget* m_tabs;
    QStackedWidget* m_mainStack;
    GraphicEqPage* m_geqPage;
    CompanderPage* m_compPage;
    QCheckBox* m_enabledCheck;
    QLabel* m_engineStatusLabel;
    QSystemTrayIcon* m_trayIcon;
    QTimer* m_statusTimer;
    bool m_processing;
    
    // Widgets Pointer Storage
    FrequencyResponseWidget* m_inlineGeq;
    CompanderCurveWidget* m_inlineComp;
    
    // Output
    QSlider* m_limThresholdSlider; QLabel* m_limThresholdValue;
    QSlider* m_limReleaseSlider; QLabel* m_limReleaseValue;
    QSlider* m_postGainSlider; QLabel* m_postGainValue;
    
    // Bass
    QCheckBox* m_bassEnable;
    QSlider* m_bassGainSlider; QLabel* m_bassGainValue;
    
    // Tube
    QCheckBox* m_tubeEnable;
    QSlider* m_tubeDriveSlider; QLabel* m_tubeDriveValue;
    
    // Compander
    QCheckBox* m_companderEnable;
    QSlider* m_companderTimeSlider; QLabel* m_companderTimeValue;
    QComboBox* m_companderGranularity;
    QComboBox* m_companderTF;
    QPushButton* m_companderCurveBtn;
    
    // Crossfeed
    QCheckBox* m_crossfeedEnable;
    QComboBox* m_crossfeedPreset;
    
    // DDC
    QCheckBox* m_ddcEnable;
    QLineEdit* m_ddcFileEdit;
    QPushButton* m_ddcBrowseBtn;
    
    // Liveprog
    QCheckBox* m_liveprogEnable;
    // Integrated Liveprog Widgets
    QComboBox* m_liveprogCombo;
    QLabel* m_liveprogInfoLabel;
    LiveprogEditorWidget* m_liveEditor;
    LiveprogParamsWidget* m_liveParams;
    QStackedWidget* m_liveStack;
    
    // LiveProg State
    QString m_currentLiveProgScript;
    QMap<QString, float> m_currentLiveProgParams;
    
    // Settings
    QComboBox* m_captureDeviceCombo;
    QComboBox* m_outputDeviceCombo;
    QCheckBox* m_startMinimizedCheck;
    QCheckBox* m_autoStartCheck;
    
    // Stereo
    QCheckBox* m_stereoEnable;
    QSlider* m_stereoLevelSlider; QLabel* m_stereoLevelValue;
    
    // Reverb
    QCheckBox* m_reverbEnable;
    QComboBox* m_reverbPreset;
    
    // EQ
    QCheckBox* m_eqEnable;
    QComboBox* m_eqFilterType;
    QComboBox* m_eqInterpolator;
    QSlider* m_eqSliders[15];
    QLabel* m_eqLabels[15];
    
    // Graphic EQ
    QCheckBox* m_geqEnable;
    
    QPushButton* m_geqEditBtn;
    
    // Convolver
    QCheckBox* m_convolverEnable;
    QLineEdit* m_convolverFileEdit; // Kept for logic
    QPushButton* m_convolverBrowseBtn; // Kept via layout
    QLabel* m_convolverInfoLabel;
    QComboBox* m_convolverOptimization;
    QString m_currentConvolverFile; // Stores currently selected IR path
    QString m_currentAdvParams; // Stores "-80;-100..." string
    
    // Config storage
    QString m_savedCaptureId;
    QString m_savedOutputId;
    
    // Persistence Helpers
    QComboBox* m_convolverCombo;
    QComboBox* m_ddcCombo;
    QString m_currentDdcFile;
    
    QString m_savedConvolverPath;
    QString m_savedDdcPath;
    QString m_savedLiveprogPath;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H


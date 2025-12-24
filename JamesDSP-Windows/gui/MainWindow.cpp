/**
 * JamesDSP for Windows - Main Window Implementation
 * Complete Android port with dark purple theme and all 12 DSP effects
 */

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QScrollArea>
#include <QFrame>
#include <QWheelEvent>
#include <QFileInfo>
#include <QSlider>
#include "MainWindow.h"
#include "AudioService.h"
#include "DeviceEnumerator.h"
#include "DspController.h"
#include "widgets.h"
#include "curve_widgets.h"
#include "../src/DspConfig.h"
#include <fstream>

static std::ofstream g_log("debug_log.txt", std::ios::out | std::ios::app);
#define LOG(x) g_log << x << std::endl; g_log.flush()

// EQ frequencies (Hz)
static const char* EQ_FREQ_LABELS[] = {
    "25", "40", "63", "100", "160", "250", "400", "630",
    "1k", "1.6k", "2.5k", "4k", "6.3k", "10k", "16k"
};

// NoWheelSlider is defined in widgets.h

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabs(nullptr)
    , m_trayIcon(nullptr)
    , m_statusTimer(nullptr)
    , m_processing(false)
{
    LOG("[DEBUG] MainWindow Ctor: Start");
    setWindowTitle("JamesDSP for Windows");
    setMinimumSize(800, 600);
    
    LOG("[DEBUG] Calling setupTheme...");
    setupTheme();
    LOG("[DEBUG] Calling setupUi...");
    setupUi();
    // createTabs() is called inside setupUi, do not call again
    
    LOG("[DEBUG] Calling createTrayIcon...");
    createTrayIcon();
    LOG("[DEBUG] Calling connectSignals...");
    connectSignals();
    LOG("[DEBUG] Calling populateDevices...");
    populateDevices();
    LOG("[DEBUG] Calling loadSettings...");
    loadSettings();
    
    // Status timer
    LOG("[DEBUG] Starting Timer...");
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    m_statusTimer->start(1000);
    LOG("[DEBUG] MainWindow Ctor: Done");
}
MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::setupTheme()
{
    // Premium Dark Theme (Material Design inspired)
    qApp->setStyle("Fusion");
    QPalette p;
    p.setColor(QPalette::Window, QColor(18, 18, 18));
    p.setColor(QPalette::WindowText, QColor(224, 224, 224));
    p.setColor(QPalette::Base, QColor(30, 30, 30));
    p.setColor(QPalette::AlternateBase, QColor(42, 42, 42));
    p.setColor(QPalette::ToolTipBase, Qt::black);
    p.setColor(QPalette::ToolTipText, Qt::white);
    p.setColor(QPalette::Text, QColor(224, 224, 224));
    p.setColor(QPalette::Button, QColor(45, 45, 45));
    p.setColor(QPalette::ButtonText, QColor(224, 224, 224));
    p.setColor(QPalette::BrightText, Qt::red);
    p.setColor(QPalette::Link, QColor(187, 134, 252));
    p.setColor(QPalette::Highlight, QColor(187, 134, 252));
    p.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(p);

    setStyleSheet(R"(
        QMainWindow { background-color: #121212; }
        QTabWidget::pane { border: 1px solid #333; background: #1e1e1e; border-radius: 8px; margin-top: -1px; }
        QTabBar::tab { background: #2c2c2c; color: #aaa; padding: 10px 24px; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 4px; font-weight: bold; }
        QTabBar::tab:selected { background: #383838; color: #bb86fc; border-bottom: 2px solid #bb86fc; }
        QTabBar::tab:hover:!selected { background: #333; }
        QGroupBox { background: #1e1e1e; border: 1px solid #333; border-radius: 12px; margin-top: 1.5em; padding: 20px; font-family: "Segoe UI", sans-serif; }
        QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; color: #bb86fc; font-weight: bold; font-size: 14px; }
        QCheckBox { color: #e0e0e0; spacing: 10px; font-size: 13px; }
        QCheckBox::indicator { width: 22px; height: 22px; border: 2px solid #555; border-radius: 6px; background: #2c2c2c; }
        QCheckBox::indicator:checked { background: #bb86fc; border-color: #bb86fc; image: url(:/icons/check.png); }
        QSlider::groove:horizontal { border: none; height: 6px; background: #333; border-radius: 3px; }
        QSlider::handle:horizontal { background: #bb86fc; width: 20px; height: 20px; margin: -7px 0; border-radius: 10px; border: 2px solid #121212; }
        QSlider::handle:horizontal:hover { background: #d4a5ff; }
        QSlider::sub-page:horizontal { background: #bb86fc; border-radius: 3px; }
        QPushButton { background-color: #333; border: 1px solid #444; border-radius: 6px; padding: 10px 20px; color: #eee; font-weight: 500; }
        QPushButton:hover { background-color: #444; border-color: #777; }
        QPushButton:pressed { background-color: #bb86fc; color: #000; }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { background: #2c2c2c; border: 1px solid #333; padding: 10px; border-radius: 6px; color: #fff; font-size: 13px; }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #bb86fc; }
        QComboBox::drop-down { border: none; }
        QLabel { color: #ccc; }
        QScrollArea { border: none; background: transparent; }
        QStatusBar { background: #1a1a20; color: #888; border-top: 1px solid #222; }
    )");
}

void MainWindow::setupUi()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header (Title + Master Switch)
    QWidget* topBar = new QWidget();
    topBar->setStyleSheet("background-color: #1a1a1a; border-bottom: 1px solid #333;");
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(16, 12, 16, 12);
    
    QLabel* title = new QLabel("JamesDSP");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #fff;");
    topLayout->addWidget(title);
    topLayout->addStretch();
    
    m_enabledCheck = new QCheckBox("Master Power");
    m_enabledCheck->setChecked(false); // Disabled by default
    m_enabledCheck->setStyleSheet("color: #bb86fc; font-weight: bold;");
    connect(m_enabledCheck, &QCheckBox::toggled, this, &MainWindow::toggleProcessing);
    topLayout->addWidget(m_enabledCheck);

    // Auto-start disabled for safety
    
    // Motor status indicator
    m_engineStatusLabel = new QLabel("Motor: STOPPED");
    m_engineStatusLabel->setStyleSheet("color: #888; font-weight: bold; margin-left: 16px;");
    topLayout->addWidget(m_engineStatusLabel);
    
    // Connect to AudioService state changes
    connect(&AudioService::instance(), &AudioService::stateChanged, this, [this](bool running){
        if(running) {
            m_engineStatusLabel->setText("Motor: ACTIVE");
            m_engineStatusLabel->setStyleSheet("color: #bb86fc; font-weight: bold; margin-left: 16px;");
        } else {
            m_engineStatusLabel->setText("Motor: STOPPED");
            m_engineStatusLabel->setStyleSheet("color: #888; font-weight: bold; margin-left: 16px;");
        }
    });
    
    mainLayout->addWidget(topBar);
    
    // Stack for Navigation (Tabs vs Pages)
    m_mainStack = new QStackedWidget(this);
    mainLayout->addWidget(m_mainStack);
    
    LOG("[DEBUG] setupUi calling createTabs...");
    createTabs(); // Restore this call
    LOG("[DEBUG] setupUi createTabs done.");
    
    statusBar()->showMessage("JamesDSP Ready");
}

void MainWindow::createTabs()
{
    LOG("[DEBUG] createTabs start");
    // Main Tab Widget
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(R"(
        QTabWidget::pane { border: none; background: #121212; }
        QTabBar::tab { background: #1e1e1e; color: #888; padding: 12px 16px; border: none; font-weight: bold; }
        QTabBar::tab:selected { color: #bb86fc; border-bottom: 2px solid #bb86fc; background: #252525; }
        QTabBar::tab:hover:!selected { background: #222; }
    )");
    
    m_tabs->addTab(createOutputTab(), "Output & Effects");
    m_tabs->addTab(createSoundstageTab(), "Soundstage");
    m_tabs->addTab(createEqualizersTab(), "Equalizers");
    m_tabs->addTab(createConvolverTab(), "Convolver & DDC");
    m_tabs->addTab(createLiveprogTab(), "Liveprog");
    m_tabs->addTab(createSettingsTab(), "Settings");
    
    m_mainStack->addWidget(m_tabs);
    
    m_mainStack->addWidget(m_tabs);
    
    // Sub-Pages
    m_geqPage = new GraphicEqPage(this);
    m_geqPage->onBack = [this](){ m_mainStack->setCurrentIndex(0); };
    // Sync GEQ
    m_geqPage->curve()->onCurveChanged = [this](){
        if(m_inlineGeq) m_inlineGeq->setNodes(m_geqPage->curve()->nodes());
        // Real-time update
        if(m_geqEnable->isChecked()) {
             DspManager::instance().setArbitraryEq(true, m_geqPage->curve()->toGraphicEqString());
        }
    };
    m_geqPage->onApply = [this](const QString& str) {
        if(m_geqEnable->isChecked()) {
             DspManager::instance().setArbitraryEq(true, str);
        }
    };
    m_mainStack->addWidget(m_geqPage);
    
    m_compPage = new CompanderPage(this);
    m_compPage->onBack = [this](){ m_mainStack->setCurrentIndex(0); };
    // Sync Compander
    m_compPage->curve()->onCurveChanged = [this](){
        if(m_inlineComp) m_inlineComp->setGains(m_compPage->curve()->gains());
        // PROACTIVE FIX: Update DSP immediately
        if(m_companderEnable->isChecked()) {
             QList<double> qg = m_compPage->curve()->gains();
             std::vector<double> stdg(qg.begin(), qg.end());
             DspManager::instance().setCompanderPoints(stdg);
        }
    };
    m_mainStack->addWidget(m_compPage);
}

QGroupBox* MainWindow::createEffectGroup(const QString& title, QCheckBox** enableCheck)
{
    QGroupBox* group = new QGroupBox(title);
    *enableCheck = new QCheckBox("Enable");
    (*enableCheck)->setChecked(false);
    return group;
}

QSlider* MainWindow::createSlider(int min, int max, int value, Qt::Orientation orient)
{
    NoWheelSlider* slider = new NoWheelSlider(orient);
    slider->setMinimum(min);
    slider->setMaximum(max);
    slider->setValue(value);
    return slider;
}

QWidget* MainWindow::createLabeledSlider(const QString& label, QSlider* slider, QLabel** valueLabel)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 4, 0, 4);
    
    QLabel* nameLabel = new QLabel(label);
    nameLabel->setMinimumWidth(130);
    layout->addWidget(nameLabel);
    
    layout->addWidget(slider, 1);
    
    *valueLabel = new QLabel("0");
    (*valueLabel)->setMinimumWidth(70);
    (*valueLabel)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    (*valueLabel)->setStyleSheet("color: #bb86fc; font-weight: bold;");
    layout->addWidget(*valueLabel);
    
    return widget;
}

// Helper: Slider with SpinBox for precise input (scaled values)
// scaleFactor: how to convert slider int to display value (e.g., 100.0 means slider 22 -> 0.22)
// suffix: unit suffix (e.g., " s", " dB", " ms")
// decimals: number of decimal places for SpinBox
QWidget* MainWindow::createLabeledSliderWithSpinBox(const QString& label, QSlider* slider, 
    QLabel** valueLabel, double scaleFactor, const QString& suffix, int decimals)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 4, 0, 4);
    
    QLabel* nameLabel = new QLabel(label);
    nameLabel->setMinimumWidth(130);
    layout->addWidget(nameLabel);
    
    layout->addWidget(slider, 1);
    
    // SpinBox with hidden buttons
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(slider->minimum() / scaleFactor, slider->maximum() / scaleFactor);
    spinBox->setSingleStep(1.0 / scaleFactor);
    spinBox->setDecimals(decimals);
    spinBox->setValue(slider->value() / scaleFactor);
    spinBox->setSuffix(suffix);
    spinBox->setFixedWidth(90);
    spinBox->setStyleSheet(
        "QDoubleSpinBox { background: #2c2c2c; color: #bb86fc; border: 1px solid #444; "
        "border-radius: 4px; padding: 6px 8px; font-weight: bold; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 0; height: 0; }"
    );
    layout->addWidget(spinBox);
    
    // Hidden label for compatibility with existing code
    *valueLabel = new QLabel();
    (*valueLabel)->hide();
    
    // Slider -> SpinBox sync
    connect(slider, &QSlider::valueChanged, [spinBox, scaleFactor](int v) {
        spinBox->blockSignals(true);
        spinBox->setValue(v / scaleFactor);
        spinBox->blockSignals(false);
    });
    
    // SpinBox -> Slider sync
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [slider, scaleFactor](double v) {
        slider->blockSignals(true);
        slider->setValue((int)(v * scaleFactor));
        slider->blockSignals(false);
        // Emit valueChanged so DSP updates
        emit slider->valueChanged(slider->value());
    });
    
    return widget;
}

QWidget* MainWindow::createOutputTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(0); // Spacing handled by card margins
    layout->setContentsMargins(12, 12, 12, 12);
    
    // 1. Output Control (Special Card - Non-collapsible)
    EffectCard* outputCard = new EffectCard("Output control", QIcon(":/icons/tune.png"));
    outputCard->enableSwitch->setVisible(false); 
    outputCard->contentArea->setEnabled(true);
    // Force expand and remove toggle connection to ensure it stays open
    outputCard->updateState(true);
    outputCard->enableSwitch->disconnect(); // Disconnect default handler
    m_limThresholdSlider = createSlider(-6000, -10, -10); // Max -0.10dB (-10), Min -60.00dB (-6000)
    outputCard->addWidget(createLabeledSliderWithSpinBox("Limiter threshold", m_limThresholdSlider, &m_limThresholdValue, 100.0, " dB", 2));
    m_limReleaseSlider = createSlider(2, 500, 60);
    outputCard->addWidget(createLabeledSliderWithSpinBox("Limiter release", m_limReleaseSlider, &m_limReleaseValue, 1.0, " ms", 0));
    m_postGainSlider = createSlider(-150, 150, 0);
    outputCard->addWidget(createLabeledSliderWithSpinBox("Post gain", m_postGainSlider, &m_postGainValue, 10.0, " dB", 1));
    layout->addWidget(outputCard);
    
    // 2. Dynamic Bass Boost
    EffectCard* bassCard = new EffectCard("Dynamic bass boost");
    m_bassEnable = bassCard->enableSwitch;
    m_bassGainSlider = createSlider(30, 150, 50);
    bassCard->addWidget(createLabeledSliderWithSpinBox("Maximum gain", m_bassGainSlider, &m_bassGainValue, 10.0, " dB", 1));
    layout->addWidget(bassCard);
    
    // 3. Vacuum Tube
    EffectCard* tubeCard = new EffectCard("Vacuum tube");
    m_tubeEnable = tubeCard->enableSwitch;
    m_tubeDriveSlider = createSlider(20, 120, 20);
    tubeCard->addWidget(createLabeledSliderWithSpinBox("Tube drive", m_tubeDriveSlider, &m_tubeDriveValue, 10.0, " dB", 1));
    layout->addWidget(tubeCard);
    
    // 4. Compander
    EffectCard* compCard = new EffectCard("Dynamic range compander");
    m_companderEnable = compCard->enableSwitch;
    
    // Inline Graph for Compander (Small view)
    m_inlineComp = new CompanderCurveWidget();
    m_inlineComp->setMinimumHeight(180);
    m_inlineComp->setReadOnly(true);
    compCard->addWidget(m_inlineComp);
    
    QHBoxLayout* btnRow = new QHBoxLayout();
    m_companderCurveBtn = new QPushButton("Fullscreen Editor");
    connect(m_companderCurveBtn, &QPushButton::clicked, [this](){ 
        m_compPage->show(); 
        m_mainStack->setCurrentWidget(m_compPage); 
    });
    btnRow->addWidget(m_companderCurveBtn);
    btnRow->addStretch();
    compCard->addLayout(btnRow);
    
    // Command from User: "Granularity" first?
    // Android layout typically: Enable -> Graph -> Editor Btn -> Options
    
    // Granularity
    // "Granularity: Very low, Low, Medium, High, Extreme"
    // Granularity
    // "Granularity: Very low, Low, Medium, High, Extreme"
    compCard->addWidget(new QLabel("Granularity:"));
    m_companderGranularity = new NoWheelComboBox();
    m_companderGranularity->addItems({"Very low", "Low", "Medium", "High", "Extreme"});
    
    // Connect Granularity
    connect(m_companderGranularity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCompanderGranularityChanged);
    
    compCard->addWidget(m_companderGranularity);
    
    // Transformation
    compCard->addWidget(new QLabel("Transformation:"));
    m_companderTF = new NoWheelComboBox();
    m_companderTF->addItem("Uniform (Short-time Fourier)", 0);
    m_companderTF->addItem("Multiresolution (Incomplete dual frame)", 1);
    m_companderTF->addItem("Pseudo multiresolution (Subsampling frame)", 2);
    m_companderTF->addItem("Pseudo multiresolution (Time domain)", 3);
    
    // Connect Transformation
    connect(m_companderTF, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCompanderTFChanged);
    
    compCard->addWidget(m_companderTF);
    
    // Time Constant
    // User requested range: 0.06s to 0.30s
    // Slider: 6 to 30. Scale factor 100.0 (Allocator division).
    m_companderTimeSlider = createSlider(6, 30, 22); // 0.06s to 0.30s
    QWidget* timeW = createLabeledSliderWithSpinBox("Time constant", m_companderTimeSlider, &m_companderTimeValue, 100.0, " s", 2);
    // CONNECT SLIDER MANUALLY to new slot
    connect(m_companderTimeSlider, &QSlider::valueChanged, this, &MainWindow::onCompanderTimeChanged);
    
    compCard->addWidget(timeW);
    
    layout->addWidget(compCard);
    
    layout->addStretch();
    scroll->setWidget(page);
    return scroll;
}

QWidget* MainWindow::createSoundstageTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 12, 12, 12); layout->setSpacing(0);
    
    // Crossfeed
    EffectCard* crossCard = new EffectCard("Crossfeed");
    m_crossfeedEnable = crossCard->enableSwitch;
    m_crossfeedPreset = new QComboBox();
    m_crossfeedPreset->addItems({"BS2B Weak", "BS2B Strong", "Out of head", "Realistic surround", "Surround 1", "Surround 2"});
    crossCard->addWidget(new QLabel("Preset:"));
    crossCard->addWidget(m_crossfeedPreset);
    layout->addWidget(crossCard);
    
    // Soundstage
    EffectCard* stereoCard = new EffectCard("Stereo wideness");
    m_stereoEnable = stereoCard->enableSwitch;
    m_stereoLevelSlider = createSlider(0, 100, 60);
    stereoCard->addWidget(createLabeledSliderWithSpinBox("Wideness", m_stereoLevelSlider, &m_stereoLevelValue, 1.0, "", 0));
    layout->addWidget(stereoCard);
    
    // Reverb
    EffectCard* reverbCard = new EffectCard("Virtual room effect");
    m_reverbEnable = reverbCard->enableSwitch;
    m_reverbPreset = new QComboBox();
    m_reverbPreset->addItems({"Default", "Small hall 1", "Small hall 2", "Medium hall 1", "Medium hall 2", "Large hall 1", "Large hall 2", "Small room 1", "Small room 2", "Medium room 1", "Medium room 2", "Large room 1", "Large room 2", "Plate"});
    reverbCard->addWidget(new QLabel("Room preset:"));
    reverbCard->addWidget(m_reverbPreset);
    layout->addWidget(reverbCard);
    
    layout->addStretch();
    scroll->setWidget(page);
    return scroll;
}

QWidget* MainWindow::createEqualizersTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 12, 12, 12); layout->setSpacing(0);
    
    // Multimodal
    EffectCard* eqCard = new EffectCard("Multimodal equalizer");
    m_eqEnable = eqCard->enableSwitch;
    
    QHBoxLayout* eqTop = new QHBoxLayout();
    m_eqFilterType = new QComboBox(); m_eqFilterType->addItems({"FIR Minimum phase", "IIR 4th order", "IIR 6th order", "IIR 8th order", "IIR 12th order"});
    
    m_eqInterpolator = new QComboBox(); m_eqInterpolator->addItems({"PCHIP (Hermite)", "Modified Akima"});
    
    eqTop->addWidget(new QLabel("Type:")); eqTop->addWidget(m_eqFilterType);
    eqTop->addWidget(new QLabel("Interp:")); eqTop->addWidget(m_eqInterpolator);
    eqCard->addLayout(eqTop);
    
    // 15-band sliders
    QFrame* sliderFrame = new QFrame();
    sliderFrame->setFixedHeight(200);
    QHBoxLayout* sliderLayout = new QHBoxLayout(sliderFrame);
    sliderLayout->setSpacing(2);
    for(int i=0; i<15; i++) {
        QVBoxLayout* band = new QVBoxLayout();
        m_eqSliders[i] = new NoWheelSlider(Qt::Vertical);
        m_eqSliders[i]->setRange(-120, 120); m_eqSliders[i]->setValue(0);
        band->addWidget(m_eqSliders[i], 1, Qt::AlignHCenter);
        m_eqLabels[i] = new QLabel("0"); m_eqLabels[i]->setStyleSheet("font-size: 10px; color: #888;");
        band->addWidget(m_eqLabels[i], 0, Qt::AlignHCenter);
        
        QLabel* freqLabel = new QLabel(EQ_FREQ_LABELS[i]);
        freqLabel->setStyleSheet("font-size: 9px; color: #666;");
        band->addWidget(freqLabel, 0, Qt::AlignHCenter);
        
        sliderLayout->addLayout(band);
    }
    eqCard->addWidget(sliderFrame);
    layout->addWidget(eqCard);
    
    // Graphic EQ
    EffectCard* geqCard = new EffectCard("Arbitrary response equalizer");
    m_geqEnable = geqCard->enableSwitch;
    
    m_inlineGeq = new FrequencyResponseWidget();
    m_inlineGeq->setMinimumHeight(180);
    m_inlineGeq->setReadOnly(true);
    geqCard->addWidget(m_inlineGeq);
    
    // Arbitrary EQ connection
    m_geqEditBtn = new QPushButton("Fullscreen Editor");
    connect(m_geqEditBtn, &QPushButton::clicked, [this](){ 
        m_mainStack->setCurrentWidget(m_geqPage);
    });
    geqCard->addWidget(m_geqEditBtn);
    layout->addWidget(geqCard);
    
    layout->addStretch();
    scroll->setWidget(page);
    return scroll;
}

QWidget* MainWindow::createConvolverTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    
    // ============ Convolver ============
    EffectCard* convCard = new EffectCard("Convolver");
    m_convolverEnable = convCard->enableSwitch;
    
    // Convolver File Row (Liveprog Style)
    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(new QLabel("Impulse response:"));
    
    QComboBox* convCombo = new QComboBox();
    convCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_convolverCombo = convCombo;
    fileRow->addWidget(convCombo, 1);
    
    QPushButton* convLoadBtn = new QPushButton("Load..."); // Shorter text
    // convLoadBtn->setCursor(Qt::PointingHandCursor); 
    // Revert to standard style (remove setStyleSheet)
     connect(convLoadBtn, &QPushButton::clicked, [this, convCombo](){
        QString file = QFileDialog::getOpenFileName(this, "Select IR", "", "Impulse Responses (*.wav *.irs)");
        if(!file.isEmpty()) {
            int idx = convCombo->findData(file);
            if(idx == -1) {
                convCombo->addItem(QFileInfo(file).fileName(), file);
                idx = convCombo->count() - 1;
            }
            convCombo->setCurrentIndex(idx);
            onConvolverFileSelected(file);
        }
    });
    fileRow->addWidget(convLoadBtn);

    // CONNECT SIGNAL FOR HOT-SWAPPING (Restored)
    connect(convCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, convCombo](int index){
         QString data = convCombo->itemData(index).toString();
         if(!data.isEmpty()) onConvolverFileSelected(data);
    });

    convCard->addLayout(fileRow);

    m_convolverInfoLabel = new QLabel("Select an IR...");
    m_convolverInfoLabel->setStyleSheet("color: #888;");
    convCard->addWidget(m_convolverInfoLabel);

    // Waveform Editor Debugging
    QHBoxLayout* btnRow = new QHBoxLayout(); // Restored
    // Waveform Controls
    // Waveform Controls
    
    // 1. See Waveform (Visualizer)
    QPushButton* seeBtn = new QPushButton("See Waveform");
    seeBtn->setToolTip("Visualize the processed Impulse Response");
    connect(seeBtn, &QPushButton::clicked, [this](){
        ConvolverWaveformDialog dlg("", this);
        // Load data from DspManager (this will now get the PROCESSED data)
        if(DspManager::instance().getImpulseResponse(dlg.m_irData, dlg.m_channels)) {
             dlg.exec();
        } else {
             QMessageBox::warning(this, "Waveform", "No Impulse Response loaded.");
        }
    });
    btnRow->addWidget(seeBtn);

    // 2. Edit Params (Android Feature)
    QPushButton* editBtn = new QPushButton("Edit Params");
    editBtn->setToolTip("Advanced processing: Trim Silence, Channel Routing, Phase.");
    connect(editBtn, &QPushButton::clicked, [this](){
        QDialog dlg(this);
        dlg.setWindowTitle("Advanced IR Processing");
        dlg.resize(350, 250);
        
        QVBoxLayout* layout = new QVBoxLayout(&dlg);
        layout->setSpacing(10);
        
        // Parse current params or defaults
        // Default: -80;-100;0;0;0;0
        QStringList p = m_currentAdvParams.split(';');
        double startTrim = (p.size()>0) ? p[0].toDouble() : -80;
        double endTrim   = (p.size()>1) ? p[1].toDouble() : -100;
        int lSrc = (p.size()>2) ? p[2].toInt() : 0;
        int rSrc = (p.size()>3) ? p[3].toInt() : 1; // Default to 1 (Right) for Stereo behavior 
                                                    // Let's assume our Router logic: 0=Ch0, 1=Ch1
                                                    // Warning: if mono file, 1 = Silence. 
        int lPhase = (p.size()>4) ? p[4].toInt() : 0;
        int rPhase = (p.size()>5) ? p[5].toInt() : 0;

        // Group 1: Silence Trimming
        QGroupBox* gbTrim = new QGroupBox("Silence Trimming (Auto-Crop)");
        QFormLayout* flTrim = new QFormLayout(gbTrim);
        QDoubleSpinBox* sbStart = new QDoubleSpinBox(); sbStart->setRange(-200, 0); sbStart->setSuffix(" dB"); sbStart->setValue(startTrim);
        QDoubleSpinBox* sbEnd   = new QDoubleSpinBox(); sbEnd->setRange(-200, 0); sbEnd->setSuffix(" dB"); sbEnd->setValue(endTrim);
        flTrim->addRow("Start Threshold:", sbStart);
        flTrim->addRow("End Threshold:", sbEnd);
        layout->addWidget(gbTrim);

        // Group 2: Channel Routing
        QGroupBox* gbRoute = new QGroupBox("Channel Routing & Phase");
        QGridLayout* glRoute = new QGridLayout(gbRoute);
        
        glRoute->addWidget(new QLabel("Left Output:"), 0, 0);
        QComboBox* cbL = new QComboBox(); 
        cbL->addItem("Input Left (Ch0)", 0); cbL->addItem("Input Right (Ch1)", 1); cbL->addItem("Silence", 2);
        cbL->setCurrentIndex(lSrc > 2 ? 0 : lSrc);
        glRoute->addWidget(cbL, 0, 1);
        
        QCheckBox* chkL = new QCheckBox("Invert Phase");
        chkL->setChecked(lPhase == 1);
        glRoute->addWidget(chkL, 0, 2);

        glRoute->addWidget(new QLabel("Right Output:"), 1, 0);
        QComboBox* cbR = new QComboBox(); 
        cbR->addItem("Input Right (Ch1)", 0); // WAIT: The integer is the SOURCE index. 
                                              // If user wants Normal Stereo: L=Source0, R=Source1.
                                              // If user wants Mono->Stereo: L=Source0, R=Source0.
                                              // Our ComboBox logic needs to map strictly to index.
        // Re-do Combo Items for clarity
        cbR->clear();
        cbR->addItem("Input Left (Ch0)", 0); cbR->addItem("Input Right (Ch1)", 1); cbR->addItem("Silence", 2);
        
        // This is confusing. Android Default is "0;0". Does that mean Mono? Or does Android Engine handle "0" as "Pass-Through"?
        // Just in case, let's stick to strict Index. 
        // If file is Stereo, L=0, R=1 used in Process logic.
        // If current default is 0 for R, it effectively copies L to R?
        // Let's check logic: "if(rSrcIdx < oldChannels) rSample = impulse[i*oldChannels + rSrcIdx]"
        // If Stereo (2ch), oldChannels=2. 
        //   rSrc=0 -> rSample = impulse[i*2 + 0] (Left) !!
        //   rSrc=1 -> rSample = impulse[i*2 + 1] (Right) !!
        // SO DEFAULT "0;0" CONVERTS STEREO TO MONO LEFT!?
        // Or assumes Mono Input?
        // WE SHOULD DEFAULT RIGHT TO 1 on creation if not set.
        // But here we just read params.
        
        cbR->setCurrentIndex(rSrc > 2 ? 0 : rSrc);
        glRoute->addWidget(cbR, 1, 1);
        
        QCheckBox* chkR = new QCheckBox("Invert Phase");
        chkR->setChecked(rPhase == 1);
        glRoute->addWidget(chkR, 1, 2);
        
        layout->addWidget(gbRoute);

        // Buttons
        QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        layout->addWidget(bbox);

        if(dlg.exec() == QDialog::Accepted) {
            // Build String
            QString newParams = QString("%1;%2;%3;%4;%5;%6")
                .arg(sbStart->value()).arg(sbEnd->value())
                .arg(cbL->currentData().toInt())
                .arg(cbR->currentData().toInt())
                .arg(chkL->isChecked() ? 1 : 0)
                .arg(chkR->isChecked() ? 1 : 0);
                
            m_currentAdvParams = newParams;
            qDebug() << "[UI] New Adv Params:" << newParams;
            
            // Apply immediately (Reload)
            if(!m_currentConvolverFile.isEmpty()) {
                 onConvolverFileSelected(m_currentConvolverFile);
            }
        }
    });
    btnRow->addWidget(editBtn);
    
    btnRow->addStretch();
    convCard->addLayout(btnRow);
    
    // Optimization Options
    convCard->addWidget(new QLabel("Optimization:"));
    QComboBox* optCombo = new QComboBox();
    optCombo->addItem("Original (Standard)", 0);
    optCombo->addItem("Reduced (Partitioned)", 1); 
    optCombo->addItem("Transformation & Minimal Reduction", 2);
    convCard->addWidget(optCombo);
    
    // Connect Optimization (Placeholder)
    // Connect Optimization
    connect(optCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onConvolverOptimizationChanged);
    m_convolverOptimization = optCombo; // Keep reference consistent!
    // Wait, m_convolverOptimization was declared somewhat lower in original code but convCard used optCombo?
    // I need to ensure m_convolverOptimization IS optCombo.
    // In previous code I see I declared 'QComboBox* optCombo' then used 'm_convolverOptimization' later?
    // Let's unify.
    // The previous code had DUPLICATE sections. I removed one.
    // Ensure m_convolverOptimization is assigned.
    
    // Populate list at end
    // Use QTimer to delay scanning to avoid blocking startup if many files
    QTimer::singleShot(100, [this, convCombo](){
        // Scan assets/Convolver
        QDir irDir(QCoreApplication::applicationDirPath() + "/assets/Convolver");
        QStringList irFiles = irDir.entryList(QStringList() << "*.wav" << "*.irs", QDir::Files);
        
        bool wasEmpty = (convCombo->count() == 0);
        
        for(const QString& f : irFiles) {
            QString fullPath = irDir.absoluteFilePath(f);
            if(convCombo->findData(fullPath) == -1)
                convCombo->addItem(f, fullPath);
        }
        
        // If we have items and nothing selected (or default), select first
        if(convCombo->count() > 0) {
             // If we have a saved path, try to find it
             if(!m_savedConvolverPath.isEmpty()) {
                 int idx = convCombo->findData(m_savedConvolverPath);
                 if(idx >= 0) convCombo->setCurrentIndex(idx);
                 else {
                     // Not in list, add it or just set it?
                     // If it's a custom path not in assets, it might be separate.
                     // For now, if valid file, trigger.
                     onConvolverFileSelected(m_savedConvolverPath);
                 }
             } else {
                 // No saved path, select first by default (Linear Phase or similar)
                 convCombo->setCurrentIndex(0);
                 onConvolverFileSelected(convCombo->itemData(0).toString());
             }
        }
    });
    
    // Add the card to the layout!
    layout->addWidget(convCard);
    


    // FIX: Force initial selection check AFTER UI elements are created (Moved to prevent NPE)
    if(convCombo->count() > 0) {
        onConvolverFileSelected(convCombo->itemData(convCombo->currentIndex()).toString());
    }
    
    // ============ ViPER-DDC ============
    EffectCard* ddcCard = new EffectCard("ViPER-DDC");
    m_ddcEnable = ddcCard->enableSwitch;
    
    // DDC file (ComboBox)
    QHBoxLayout* ddcFileRow = new QHBoxLayout();
    ddcFileRow->addWidget(new QLabel("DDC file:"));
    
    QComboBox* ddcCombo = new QComboBox();
    m_ddcCombo = ddcCombo; // Store member
    QString ddcResPath = QCoreApplication::applicationDirPath() + "/resources";
    QDirIterator itD(ddcResPath + "/DDC", QDirIterator::Subdirectories);
    while (itD.hasNext()) {
        QString f = itD.next();
        if(f.endsWith(".vdc")) ddcCombo->addItem(QFileInfo(f).fileName(), f);
    }
    // ddcCombo->addItem("Load Custom...", "CUSTOM");
    
    // Scan assets/DDC
    QDir ddcDir(QCoreApplication::applicationDirPath() + "/assets/DDC");
    QStringList vdcFiles = ddcDir.entryList(QStringList() << "*.vdc", QDir::Files);
    for(const QString& f : vdcFiles) {
        ddcCombo->addItem(f, ddcDir.absoluteFilePath(f));
    }
    
    QPushButton* loadDdcBtn = new QPushButton("Load...");
    connect(loadDdcBtn, &QPushButton::clicked, [this, ddcCombo](){
        QString file = QFileDialog::getOpenFileName(this, "Select VDC", "", "ViPER DDC files (*.vdc)");
        if(!file.isEmpty()) {
            // Add to combo
            if(ddcCombo->findData(file) == -1) ddcCombo->addItem(QFileInfo(file).fileName(), file);
            ddcCombo->setCurrentIndex(ddcCombo->findData(file));
            onDdcFileSelected(file);
        }
    });

    connect(ddcCombo, &QComboBox::currentIndexChanged, [this, ddcCombo](int index){
         QString data = ddcCombo->itemData(index).toString();
         if(!data.isEmpty()) onDdcFileSelected(data);
    });
    
    // FIX: Force initial selection
    if(ddcCombo->count() > 0) {
        onDdcFileSelected(ddcCombo->itemData(ddcCombo->currentIndex()).toString());
    }

    ddcFileRow->addWidget(ddcCombo, 1);
    ddcFileRow->addWidget(loadDdcBtn);
    ddcCard->addLayout(ddcFileRow);
    
    layout->addWidget(ddcCard);
    
    layout->addStretch();
    
    scroll->setWidget(page);
    return scroll;
}

QWidget* MainWindow::createLiveprogTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    
    // ============ Live Programmable DSP ============
    EffectCard* liveprogCard = new EffectCard("Live programmable DSP");
    m_liveprogEnable = liveprogCard->enableSwitch;
    
    // Script file
    QHBoxLayout* fileRow = new QHBoxLayout();
    fileRow->addWidget(new QLabel("Liveprog script:"));
    
    QComboBox* liveCombo = new QComboBox();
    QString resourcePath = QCoreApplication::applicationDirPath() + "/resources";
    QDirIterator itL(resourcePath + "/liveprog", QDirIterator::Subdirectories);
    while (itL.hasNext()) {
        QString f = itL.next();
        if(f.endsWith(".eel")) liveCombo->addItem(QFileInfo(f).fileName(), f);
    }
    // liveCombo->addItem("Load Custom...", "CUSTOM");
    
    // Scan assets/Liveprog
    QDir liveDir(QCoreApplication::applicationDirPath() + "/assets/Liveprog");
    QStringList eelFiles = liveDir.entryList(QStringList() << "*.eel", QDir::Files);
    for(const QString& f : eelFiles) {
        liveCombo->addItem(f, liveDir.absoluteFilePath(f));
    }
    
    QPushButton* loadLiveBtn = new QPushButton("Load...");
    connect(loadLiveBtn, &QPushButton::clicked, [this, liveCombo](){
        QString file = QFileDialog::getOpenFileName(this, "Select EEL", "", "EEL Scripts (*.eel)");
        if(!file.isEmpty()) {
            m_liveprogInfoLabel->setText(QFileInfo(file).fileName());
            if(liveCombo->findData(file) == -1) liveCombo->addItem(QFileInfo(file).fileName(), file);
            liveCombo->setCurrentIndex(liveCombo->findData(file));
            onLiveprogFileSelected(file);
        }
    });

    connect(liveCombo, &QComboBox::currentIndexChanged, [this, liveCombo](int index){
         QString data = liveCombo->itemData(index).toString();
         if(!data.isEmpty()) {
             m_liveprogInfoLabel->setText(liveCombo->currentText());
             onLiveprogFileSelected(data);
         }
    });


    
    fileRow->addWidget(liveCombo, 1);
    fileRow->addWidget(loadLiveBtn);
    liveprogCard->addLayout(fileRow);
    
    m_liveprogInfoLabel = new QLabel("Select a script...");
    m_liveprogInfoLabel->setStyleSheet("color: #888;");
    liveprogCard->addWidget(m_liveprogInfoLabel);
    
    // Integrated UI: Toggle Buttons
    QHBoxLayout* toggleRow = new QHBoxLayout();
    toggleRow->setSpacing(10);
    
    QCheckBox* showEditorBtn = new QCheckBox("Show Editor");
    showEditorBtn->setStyleSheet("QCheckBox::indicator { width: 0; height: 0; } QCheckBox { background: #333; color: #aaa; padding: 6px 12px; border-radius: 4px; font-weight: bold; } QCheckBox:checked { background: #bb86fc; color: black; }");
    
    QCheckBox* showParamsBtn = new QCheckBox("Show Parameters");
    showParamsBtn->setStyleSheet("QCheckBox::indicator { width: 0; height: 0; } QCheckBox { background: #333; color: #aaa; padding: 6px 12px; border-radius: 4px; font-weight: bold; } QCheckBox:checked { background: #bb86fc; color: black; }");
    
    // Behavior: Mutually exclusive-ish or Stacked
    // Stack: 0=Msg/Blank, 1=Editor, 2=Params
    m_liveStack = new QStackedWidget();
    
    connect(showEditorBtn, &QCheckBox::clicked, [this, showEditorBtn, showParamsBtn](){
        if(showEditorBtn->isChecked()) {
            showParamsBtn->setChecked(false);
            m_liveStack->setCurrentWidget(m_liveEditor);
            m_liveStack->setVisible(true);
        } else {
            m_liveStack->setVisible(false);
        }
    });

    connect(showParamsBtn, &QCheckBox::clicked, [this, showEditorBtn, showParamsBtn](){
        if(showParamsBtn->isChecked()) {
            showEditorBtn->setChecked(false);
            m_liveStack->setCurrentWidget(m_liveParams);
            m_liveStack->setVisible(true);
        } else {
            m_liveStack->setVisible(false);
        }
    });
    
    toggleRow->addWidget(showParamsBtn);
    toggleRow->addWidget(showEditorBtn); // Params first as it's most used
    toggleRow->addStretch();
    
    liveprogCard->addLayout(toggleRow);
    
    // Stacked Content
    m_liveEditor = new LiveprogEditorWidget(this);
    m_liveParams = new LiveprogParamsWidget(this);
    
    m_liveStack->addWidget(m_liveEditor);
    m_liveStack->addWidget(m_liveParams);
    m_liveStack->setVisible(false); // Hidden by default until toggled? Or Show Params by default?
    
    // Auto-select Params if loaded
    showParamsBtn->setChecked(true);
    m_liveStack->setCurrentWidget(m_liveParams);
    m_liveStack->setVisible(true);
    
    liveprogCard->addWidget(m_liveStack);
    
    // Wire Logic
    m_liveEditor->onApply = [this](QString code) {
        m_currentLiveProgScript = code;
        DspManager::instance().setLiveProgContent(true, code);
        // Also refresh params
        auto params = LiveProgHelper::parseParams(code);
        // Merge values?
        m_liveParams->rebuild(code, m_currentLiveProgParams);
    };
    m_liveEditor->onSave = [this](QString code) {
         QString savePath = QFileDialog::getSaveFileName(this, "Save Script As", "", "EEL Scripts (*.eel)");
         if(!savePath.isEmpty()) {
             QFile f(savePath);
             if(f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                 QTextStream out(&f); out << code;
                 // Add to combo...
             }
         }
    };
    
    m_liveParams->onParamChanged = [this](QString var, float val) {
        m_currentLiveProgParams[var] = val;
        // Inject and Update
        QString processed = LiveProgHelper::injectValues(m_currentLiveProgScript, m_currentLiveProgParams);
        DspManager::instance().setLiveProgContent(true, processed);
        DspManager::instance().setLiveProgParam(var, val);
    };
    
    layout->addWidget(liveprogCard);
    
    layout->addStretch();
    
    scroll->setWidget(page);

    // FIX: Force initial selection check AFTER UI elements are created
    if(m_liveprogCombo && m_liveprogCombo->count() > 0) {
         m_liveprogInfoLabel->setText(m_liveprogCombo->currentText());
         onLiveprogFileSelected(m_liveprogCombo->itemData(m_liveprogCombo->currentIndex()).toString());
    }

    return scroll;
}

QWidget* MainWindow::createSettingsTab()
{
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(16);
    
    // Audio devices
    QGroupBox* devicesGroup = new QGroupBox("Audio devices");
    QFormLayout* devLayout = new QFormLayout(devicesGroup);
    
    m_captureDeviceCombo = new QComboBox();
    m_outputDeviceCombo = new QComboBox();
    
    QPushButton* refreshBtn = new QPushButton("Refresh Device List");
    refreshBtn->setStyleSheet("background-color: #3d3d3d; padding: 5px 10px; border-radius: 4px;");
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::populateDevices);
    
    // Auto-update service when changed
    connect(m_captureDeviceCombo, &QComboBox::currentIndexChanged, [this]{
        QString newCap = m_captureDeviceCombo->currentData().toString();
        QString newRen = m_outputDeviceCombo->currentData().toString();
        QString oldCap = AudioService::instance().getCaptureId();
        QString oldRen = AudioService::instance().getRenderId();

        // DIAGNOSTIC: Log to file
        {
            QFile f("C:/Users/gmaym/.gemini/antigravity/playground/blazing-comet/JamesDSP-Windows/build-final/interp_debug.txt");
            if(f.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&f);
                out << "[COMBO-CAP] m_processing=" << m_processing << " newCap=" << newCap << " oldCap=" << oldCap << "\n";
            }
        }

        if(m_processing && (newCap != oldCap || newRen != oldRen)) {
             qDebug() << "[UI] Device Selection Changed. Restarting Service. Old:" << oldCap << " New:" << newCap;
             AudioService::instance().setDevices(newCap, newRen);
             AudioService::instance().stopProcessing();
             AudioService::instance().startProcessing();
        } else {
             qDebug() << "[UI] Device Selection Skipped (Same Device). Old:" << oldCap << " New:" << newCap;
        }
    });

    connect(m_outputDeviceCombo, &QComboBox::currentIndexChanged, [this]{
        QString newCap = m_captureDeviceCombo->currentData().toString();
        QString newRen = m_outputDeviceCombo->currentData().toString();
        QString oldCap = AudioService::instance().getCaptureId();
        QString oldRen = AudioService::instance().getRenderId();

        if(m_processing && (newCap != oldCap || newRen != oldRen)) {
             qDebug() << "[UI] Device Selection Changed. Restarting Service. Old:" << oldCap << " New:" << newCap;
             AudioService::instance().setDevices(newCap, newRen);
             AudioService::instance().stopProcessing();
             AudioService::instance().startProcessing();
        } else {
             qDebug() << "[UI] Device Selection Skipped (Same Device). Old:" << oldCap << " New:" << newCap;
        }
    });
    
    devLayout->addRow("Input Source (Loopback):", m_captureDeviceCombo);
    devLayout->addRow("Output Destination:", m_outputDeviceCombo);
    devLayout->addRow("", refreshBtn);
    
    layout->addWidget(devicesGroup);

    populateDevices(); // Initial population
    
    // Startup options
    QGroupBox* startupGroup = new QGroupBox("Startup");
    QVBoxLayout* startupLayout = new QVBoxLayout(startupGroup);
    
    m_startMinimizedCheck = new QCheckBox("Start minimized to tray");
    m_autoStartCheck = new QCheckBox("Start with Windows");
    
    startupLayout->addWidget(m_startMinimizedCheck);
    startupLayout->addWidget(m_autoStartCheck);
    
    layout->addWidget(startupGroup);
    layout->addWidget(startupGroup);

    // Presets
    QGroupBox* presetGroup = new QGroupBox("Presets");
    QHBoxLayout* presetLayout = new QHBoxLayout(presetGroup);
    
    QPushButton* savePresetBtn = new QPushButton("Save preset...");
    connect(savePresetBtn, &QPushButton::clicked, this, &MainWindow::savePreset);
    presetLayout->addWidget(savePresetBtn);
    
    QPushButton* loadPresetBtn = new QPushButton("Load preset...");
    connect(loadPresetBtn, &QPushButton::clicked, this, &MainWindow::loadPreset);
    presetLayout->addWidget(loadPresetBtn);
    
    QPushButton* resetBtn = new QPushButton("Reset to defaults");
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetDefaults);
    presetLayout->addWidget(resetBtn);
    
    presetLayout->addStretch();
    
    layout->addWidget(presetGroup);
    
    layout->addStretch();
    
    scroll->setWidget(page);
    return scroll;
}

void MainWindow::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_trayIcon->setToolTip("JamesDSP for Windows");
    
    QMenu* trayMenu = new QMenu(this);
    trayMenu->addAction("Show", this, &QMainWindow::show);
    trayMenu->addAction("Toggle Processing", [this]() {
        m_enabledCheck->toggle();
    });
    trayMenu->addSeparator();
    trayMenu->addAction("Exit", qApp, &QApplication::quit);
    
    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            show();
            activateWindow();
        }
    });
}

void MainWindow::connectSignals()
{
    // Master enable - ALREADY CONNECTED in constructor (line ~160), DO NOT duplicate!
    // connect(m_enabledCheck, &QCheckBox::toggled, this, &MainWindow::toggleProcessing);
    
    // Output Control
    connect(m_limThresholdSlider, &QSlider::valueChanged, this, &MainWindow::onLimiterThresholdChanged);
    onLimiterThresholdChanged(m_limThresholdSlider->value());
    
    connect(m_limReleaseSlider, &QSlider::valueChanged, this, &MainWindow::onLimiterReleaseChanged);
    onLimiterReleaseChanged(m_limReleaseSlider->value());
    
    connect(m_postGainSlider, &QSlider::valueChanged, this, &MainWindow::onPostGainChanged);
    onPostGainChanged(m_postGainSlider->value());
    
    LOG("[DEBUG] connectSignals: Output done");
    
    // Dynamic Bass
    connect(m_bassEnable, &QCheckBox::toggled, this, &MainWindow::onBassBoostEnabledChanged);
    connect(m_bassGainSlider, &QSlider::valueChanged, this, &MainWindow::onBassBoostGainChanged);
    onBassBoostGainChanged(m_bassGainSlider->value());
    
    LOG("[DEBUG] connectSignals: Bass done");
    
    // Tube
    connect(m_tubeEnable, &QCheckBox::toggled, this, &MainWindow::onTubeEnabledChanged);
    connect(m_tubeDriveSlider, &QSlider::valueChanged, this, &MainWindow::onTubeDriveChanged);
    onTubeDriveChanged(m_tubeDriveSlider->value());
    
    // Compander
    connect(m_companderEnable, &QCheckBox::toggled, this, &MainWindow::onCompanderEnabledChanged);
    
    auto compUpdate = [this](){
        m_companderTimeValue->setText(QString("%1 s").arg(m_companderTimeSlider->value()/100.0));
        if(m_companderEnable->isChecked()) {
            int tf = m_companderTF->currentData().toInt();
            DspManager::instance().setCompander(true, false, m_companderTimeSlider->value()/100.0, m_companderGranularity->currentIndex(), tf);
        }
    };
    connect(m_companderTimeSlider, &QSlider::valueChanged, compUpdate);
    connect(m_companderGranularity, QOverload<int>::of(&QComboBox::currentIndexChanged), compUpdate);
    connect(m_companderTF, QOverload<int>::of(&QComboBox::currentIndexChanged), compUpdate);
    
    // Initial Compander Label Update
    compUpdate();
    
    // Graph Sync: Fullscreen -> Inline is handled in createTabs via lambdas
    
    // Equalizer
    // Equalizer
    connect(m_eqEnable, &QCheckBox::toggled, this, &MainWindow::onEqEnabledChanged);
    connect(m_eqFilterType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEqModeChanged);
    connect(m_eqInterpolator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEqInterpChanged);
    
    for(int i=0; i<15; i++) {
        connect(m_eqSliders[i], &QSlider::valueChanged, [this, i](int val){
            onEqBandChanged(i, val);
        });
        // Only update DSP on release to avoid audio starvation/cutouts during drag
        connect(m_eqSliders[i], &QSlider::sliderReleased, [this](){
            if(m_eqEnable->isChecked()) {
                std::vector<double> gains;
                for(auto* s : m_eqSliders) gains.push_back(s->value() / 10.0);
                DspManager::instance().setEqualizer(true, m_eqFilterType->currentIndex(), m_eqInterpolator->currentIndex(), gains);
            }
        });
    }
    
    // Graphic EQ
    connect(m_geqEnable, &QCheckBox::toggled, this, &MainWindow::onGraphicEqEnabledChanged);
    
    // Stereo
    connect(m_stereoEnable, &QCheckBox::toggled, this, &MainWindow::onStereoEnabledChanged);
    connect(m_stereoLevelSlider, &QSlider::valueChanged, this, &MainWindow::onStereoLevelChanged);
    onStereoLevelChanged(m_stereoLevelSlider->value()); // Always update label on init

    // Crossfeed
    connect(m_crossfeedEnable, &QCheckBox::toggled, this, &MainWindow::onCrossfeedEnabledChanged);
    connect(m_crossfeedPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onCrossfeedPresetChanged);

    // Reverb
    connect(m_reverbEnable, &QCheckBox::toggled, this, &MainWindow::onReverbEnabledChanged);
    connect(m_reverbPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onReverbPresetChanged);

    // Convolver
    connect(m_convolverEnable, &QCheckBox::toggled, this, &MainWindow::onConvolverEnabledChanged);
    
    // DDC
    connect(m_ddcEnable, &QCheckBox::toggled, this, &MainWindow::onDdcEnabledChanged);
    
    // Liveprog
    // Liveprog
    connect(m_liveprogEnable, &QCheckBox::toggled, this, &MainWindow::onLiveprogEnabledChanged);
    
    // Audio Service Errors
    connect(&AudioService::instance(), &AudioService::errorOccurred, this, [this](const QString& msg){
        // Ensure UI updates happen on main thread (signal is cross-thread)
        QMetaObject::invokeMethod(this, [this, msg]{
             QMessageBox::critical(this, "Audio Engine Error", msg);
             // Stop processing?
             m_enabledCheck->setChecked(false);
        }, Qt::QueuedConnection);
    });
}



#include <QSettings>

void MainWindow::saveSettings()
{
    QSettings settings("JamesDSP", "JamesDSP-Windows");
    
    // Window state
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    
    // Audio Devices (Store IDs preferably)
    settings.setValue("captureDeviceId", m_captureDeviceCombo->currentData().toString());
    settings.setValue("outputDeviceId", m_outputDeviceCombo->currentData().toString());
    
    // Startup
    settings.setValue("startMinimized", m_startMinimizedCheck->isChecked());
    settings.setValue("autoStart", m_autoStartCheck->isChecked());

    // --- DSP Effects ---
    settings.beginGroup("DSP");
    
    // Limiter
    settings.setValue("limiterThreshold", m_limThresholdSlider->value());
    settings.setValue("limiterRelease", m_limReleaseSlider->value());
    settings.setValue("postGain", m_postGainSlider->value());
    
    // Bass
    settings.setValue("bassEnabled", m_bassEnable->isChecked());
    settings.setValue("bassGain", m_bassGainSlider->value());
    
    // Tube
    settings.setValue("tubeEnabled", m_tubeEnable->isChecked());
    settings.setValue("tubeDrive", m_tubeDriveSlider->value());
    
    // Stereo
    settings.setValue("stereoEnabled", m_stereoEnable->isChecked());
    settings.setValue("stereoLevel", m_stereoLevelSlider->value());
    
    // Equalizer
    settings.setValue("eqEnabled", m_eqEnable->isChecked());
    QList<QVariant> eqGains;
    for(auto* slider : m_eqSliders) eqGains << slider->value();
    settings.setValue("eqGains", eqGains);
    
    // Reverb
    settings.setValue("reverbEnabled", m_reverbEnable->isChecked());
    settings.setValue("reverbPreset", m_reverbPreset->currentIndex());
    
    settings.setValue("crossfeedEnabled", m_crossfeedEnable->isChecked());
    settings.setValue("crossfeedPreset", m_crossfeedPreset->currentIndex());
    
    // Compander
    settings.setValue("companderEnabled", m_companderEnable->isChecked());
    settings.setValue("companderTime", m_companderTimeSlider->value());
    settings.setValue("companderGranularity", m_companderGranularity->currentIndex());
    settings.setValue("companderTF", m_companderTF->currentIndex());

    // File Effects
    settings.setValue("convolverEnabled", m_convolverEnable->isChecked());
    settings.setValue("convolverPath", m_currentConvolverFile);
    settings.setValue("convolverAdvParams", m_currentAdvParams);
    settings.setValue("convolverOpt", m_convolverOptimization->currentIndex());
    
    settings.setValue("ddcEnabled", m_ddcEnable->isChecked());
    settings.setValue("ddcPath", m_currentDdcFile); 
    
    settings.setValue("liveprogEnabled", m_liveprogEnable->isChecked());
    settings.setValue("liveprogPath", m_currentLiveProgScript);
    
    settings.endGroup();
    
    LOG("Settings saved");
}

void MainWindow::loadSettings()
{
    QSettings settings("JamesDSP", "JamesDSP-Windows");
    
    // Window State
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // Startup
    m_startMinimizedCheck->setChecked(settings.value("startMinimized", false).toBool());
    m_autoStartCheck->setChecked(settings.value("autoStart", false).toBool());
    
    // DSP (Block signals to prevent DSP spam during load, or just let them fire?)
    // Better to let them fire so DSP gets updated.
    
    settings.beginGroup("DSP");
    
    if(settings.contains("limiterThreshold")) {
        m_limThresholdSlider->setValue(settings.value("limiterThreshold").toInt());
        m_limReleaseSlider->setValue(settings.value("limiterRelease").toInt());
        m_postGainSlider->setValue(settings.value("postGain").toInt());
        
        m_bassEnable->setChecked(settings.value("bassEnabled").toBool());
        m_bassGainSlider->setValue(settings.value("bassGain").toInt());
        
        m_tubeEnable->setChecked(settings.value("tubeEnabled").toBool());
        m_tubeDriveSlider->setValue(settings.value("tubeDrive").toInt());
        
        m_stereoEnable->setChecked(settings.value("stereoEnabled").toBool());
        m_stereoLevelSlider->setValue(settings.value("stereoLevel").toInt());
        
    m_companderEnable->setChecked(settings.value("companderEnabled").toBool());
    m_companderTimeSlider->setValue(settings.value("companderTime", 22).toInt()); // Default to 22 (0.22s) if missing
    // Granularity
    m_companderGranularity->setCurrentIndex(settings.value("companderGranularity").toInt());
    // TF
    m_companderTF->setCurrentIndex(settings.value("companderTF").toInt());
        
        m_eqEnable->setChecked(settings.value("eqEnabled").toBool());
        QList<QVariant> eqGains = settings.value("eqGains").toList();
        for(int i=0; i < qMin((int)eqGains.size(), 15); ++i) {
            m_eqSliders[i]->setValue(eqGains[i].toInt());
        }
        
        m_reverbEnable->setChecked(settings.value("reverbEnabled").toBool());
        m_reverbPreset->setCurrentIndex(settings.value("reverbPreset").toInt());
        
        m_crossfeedEnable->setChecked(settings.value("crossfeedEnabled").toBool());
        m_crossfeedPreset->setCurrentIndex(settings.value("crossfeedPreset").toInt());

        // File Effects Restore
        m_convolverEnable->setChecked(settings.value("convolverEnabled").toBool());
        m_savedConvolverPath = settings.value("convolverPath").toString();
        m_currentAdvParams = settings.value("convolverAdvParams", "-80;-100;0;1;0;0").toString();
        // MIGRATION FIX: If user has old broken default (0;0), force update to Stereo (0;1)
        if(m_currentAdvParams == "-80;-100;0;0;0;0") {
            m_currentAdvParams = "-80;-100;0;1;0;0";
            qDebug() << "[UI] Migrated old mono params to stereo default.";
        }
        m_convolverOptimization->setCurrentIndex(settings.value("convolverOpt").toInt());
        
        m_ddcEnable->setChecked(settings.value("ddcEnabled").toBool());
        m_savedDdcPath = settings.value("ddcPath").toString();
        
        m_liveprogEnable->setChecked(settings.value("liveprogEnabled").toBool());
        m_savedLiveprogPath = settings.value("liveprogPath").toString();
        
        // Timer to set combos after they are populated? They are populated in constructor.
        // We need to set them here.
        QTimer::singleShot(100, this, [this]{
             if(!m_savedConvolverPath.isEmpty() && m_convolverCombo) {
                 int idx = m_convolverCombo->findData(m_savedConvolverPath);
                 if(idx >= 0) m_convolverCombo->setCurrentIndex(idx);
                 else onConvolverFileSelected(m_savedConvolverPath); 
             }
             if(!m_savedDdcPath.isEmpty() && m_ddcCombo) {
                 int idx = m_ddcCombo->findData(m_savedDdcPath);
                 if(idx >= 0) m_ddcCombo->setCurrentIndex(idx);
                 else onDdcFileSelected(m_savedDdcPath);
             }
             if(!m_savedLiveprogPath.isEmpty() && m_liveprogCombo) {
                 int idx = m_liveprogCombo->findData(m_savedLiveprogPath);
                 if(idx >= 0) m_liveprogCombo->setCurrentIndex(idx);
                 else {
                      // Liveprog might need content load if custom
                      m_currentLiveProgScript = m_savedLiveprogPath;
                      // Try to load?
                      onLiveprogFileSelected(m_savedLiveprogPath); 
                 }
             }
        });
    } else {
        // Defaults if no config exists
        m_stereoLevelSlider->setValue(60); 
        m_companderTimeSlider->setValue(22);
    }
    
    settings.endGroup();
    
    // Devices
    // We can't set them directly yet because populateDevices might not have run or IDs might changed.
    // They are updated in populateDevices() if we store the desired ID.
    m_savedCaptureId = settings.value("captureDeviceId").toString();
    m_savedOutputId = settings.value("outputDeviceId").toString();
    
    // Trigger deferred device selection
    QTimer::singleShot(500, this, [this]{
        int idx = m_captureDeviceCombo->findData(m_savedCaptureId);
        if(idx >= 0) m_captureDeviceCombo->setCurrentIndex(idx);
        
        idx = m_outputDeviceCombo->findData(m_savedOutputId);
        if(idx >= 0) m_outputDeviceCombo->setCurrentIndex(idx);
        
        // Auto-start processing if it was running? Maybe not for safety.
    });
}



void MainWindow::toggleProcessing(bool enabled)
{
    // DIAGNOSTIC: Log to file with caller info
    {
        QFile f("C:/Users/gmaym/.gemini/antigravity/playground/blazing-comet/JamesDSP-Windows/build-final/interp_debug.txt");
        if(f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&f);
            out << "[TOGGLE] enabled=" << enabled << " m_processing=" << m_processing << "\n";
        }
    }
    LOG("Processing toggled: " << enabled);
    if(enabled) {
        // Update device selection before starting
        QString capId = m_captureDeviceCombo->currentData().toString();
        QString rendId = m_outputDeviceCombo->currentData().toString();
        AudioService::instance().setDevices(capId, rendId);
        
        AudioService::instance().startProcessing();
        m_statusTimer->start(1000); 
    } else {
        AudioService::instance().stopProcessing();
        m_statusTimer->stop();
    }
}

// ... 

// In populateDevices, we probably don't need to do anything immediately unless we want hot-swapping.
// But we should probably connect the ComboBox changed signals to update AudioService if running.

// We need to add connections for dev combos in createSettingsTab.


#include "DspController.h"

// ... 

// ... Include DspManager.h is already DspController.h
// Wait, I didn't rename the FILE DspController.h, only the class.

void MainWindow::onLimiterThresholdChanged(int val)
{
    m_limThresholdValue->setText(QString("%1 dB").arg(val / 100.0));
    DspManager::instance().setLimiter(val / 100.0, m_limReleaseSlider->value(), m_postGainSlider->value() / 10.0);
}
void MainWindow::onLimiterReleaseChanged(int val)
{
    m_limReleaseValue->setText(QString("%1 ms").arg(val));
    DspManager::instance().setLimiter(m_limThresholdSlider->value() / 100.0, val, m_postGainSlider->value() / 10.0);
}
void MainWindow::onPostGainChanged(int val)
{
    m_postGainValue->setText(QString("%1 dB").arg(val / 10.0));
    DspManager::instance().setLimiter(m_limThresholdSlider->value() / 100.0, m_limReleaseSlider->value(), val / 10.0);
}

void MainWindow::onBassBoostEnabledChanged(bool e) {
    DspManager::instance().setBassBoost(e, m_bassGainSlider->value());
}
void MainWindow::onBassBoostGainChanged(int val) { 
    m_bassGainValue->setText(QString("%1 dB").arg(val / 10.0));
    DspManager::instance().setBassBoost(m_bassEnable->isChecked(), val);
}

void MainWindow::onTubeEnabledChanged(bool e) {
    DspManager::instance().setToneControl(e, m_tubeDriveSlider->value() / 10.0f);
}
void MainWindow::onTubeDriveChanged(int val) { 
    m_tubeDriveValue->setText(QString("%1 dB").arg(val / 10.0));
    DspManager::instance().setToneControl(m_tubeEnable->isChecked(), val / 10.0f);
}

void MainWindow::onCompanderEnabledChanged(bool e) {
    DspManager::instance().setCompander(e, false, m_companderTimeSlider->value()/100.0, m_companderGranularity->currentIndex(), m_companderTF->currentIndex()); 
}

void MainWindow::onCompanderTimeChanged(int val) {
    m_companderTimeValue->setText(QString::number(val / 100.0, 'f', 2) + " s");
    if(m_companderEnable->isChecked())
        DspManager::instance().setCompander(true, false, val/100.0, m_companderGranularity->currentIndex(), m_companderTF->currentIndex());
}

void MainWindow::onCompanderGranularityChanged(int index) {
    if(m_companderEnable->isChecked())
        DspManager::instance().setCompander(true, false, m_companderTimeSlider->value()/100.0, index, m_companderTF->currentIndex());
}

void MainWindow::onCompanderTFChanged(int index) {
    if(m_companderEnable->isChecked())
        DspManager::instance().setCompander(true, false, m_companderTimeSlider->value()/100.0, m_companderGranularity->currentIndex(), index);
}

void MainWindow::onConvolverOptimizationChanged(int index) {
    qDebug() << "[UI] Convolver Optimization Changed:" << index;
    if(m_convolverEnable->isChecked()) {
        // Reload with optimization
        if(!m_currentConvolverFile.isEmpty()) {
            DspManager::instance().setConvolver(true, m_currentConvolverFile, index, m_currentAdvParams);
        }
    }
}

void MainWindow::onStereoEnabledChanged(bool e) {
    DspManager::instance().setStereoWide(e, m_stereoLevelSlider->value());
}
void MainWindow::onStereoLevelChanged(int val) { 
    m_stereoLevelValue->setText(QString::number(val)); 
    if(m_stereoEnable->isChecked()) DspManager::instance().setStereoWide(true, val);
}

// Equalizer
void MainWindow::onEqEnabledChanged(bool e) {
    std::vector<double> gains;
    for(auto* s : m_eqSliders) gains.push_back(s->value() / 10.0);
    DspManager::instance().setEqualizer(e, m_eqFilterType->currentIndex(), m_eqInterpolator->currentIndex(), gains);
}

void MainWindow::onEqBandChanged(int bandIndex, int value) {
    m_eqLabels[bandIndex]->setText(QString::number(value / 10.0, 'f', 1));
    // DSP update moved to sliderReleased to prevent audio dropouts during drag
}

void MainWindow::onEqModeChanged(int index) {
   if(m_eqEnable->isChecked()) onEqEnabledChanged(true);
}
void MainWindow::onEqInterpChanged(int index) {
    {
        QFile f("C:/Users/gmaym/.gemini/antigravity/playground/blazing-comet/JamesDSP-Windows/jamesdsp_debug.log");
        if(f.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&f);
            out << "[UI] Interpolator Changed to: " << index << "\n";
        }
    }
    if(m_eqEnable->isChecked()) onEqEnabledChanged(true);
}

void MainWindow::onGraphicEqEnabledChanged(bool e) {
    // Graphic EQ (Arbitrary) not yet fully wired to DspManager in this MVP
    // Leaving as stub or could link to same Equalizer function if compatible
}

// Reverb
void MainWindow::onReverbEnabledChanged(bool enabled) {
    DspManager::instance().setReverb(enabled, m_reverbPreset->currentIndex());
}
void MainWindow::onReverbPresetChanged(int index) {
    if(m_reverbEnable->isChecked()) DspManager::instance().setReverb(true, index);
}

// Crossfeed
void MainWindow::onCrossfeedEnabledChanged(bool enabled) {
    DspManager::instance().setCrossfeed(enabled, m_crossfeedPreset->currentIndex());
}
void MainWindow::onCrossfeedPresetChanged(int index) {
    if(m_crossfeedEnable->isChecked()) DspManager::instance().setCrossfeed(true, index);
}

// File Effects
void MainWindow::onConvolverEnabledChanged(bool enabled) {
    qDebug() << "[UI] Convolver Enable Changed:" << enabled;
    if(enabled) {
        // Use stored file path (set by onConvolverFileSelected)
        if(!m_currentConvolverFile.isEmpty()) {
            DspManager::instance().setConvolver(true, m_currentConvolverFile, m_convolverOptimization ? m_convolverOptimization->currentIndex() : 0, m_currentAdvParams);
        } else {
            qDebug() << "[UI] Convolver enabled but no file selected.";
        }
    } else {
        DspManager::instance().setConvolver(false, "");
    }
}
void MainWindow::onConvolverFileSelected(const QString& path) {
    qDebug() << "[UI] Convolver File Selected:" << path;
    m_currentConvolverFile = path; // Store for later use by enable toggle
    m_convolverInfoLabel->setText(QFileInfo(path).fileName());
    if(m_convolverEnable->isChecked()) DspManager::instance().setConvolver(true, path, m_convolverOptimization ? m_convolverOptimization->currentIndex() : 0, m_currentAdvParams);
}

void MainWindow::onDdcEnabledChanged(bool enabled) {
     if(!enabled) DspManager::instance().setDdc(false, "");
     // If enabled, we need the file. The UI flow usually selects file THEN enables.
}
void MainWindow::onDdcFileSelected(const QString& path) {
    m_currentDdcFile = path;
    if(m_ddcEnable->isChecked()) DspManager::instance().setDdc(true, path);
}

void MainWindow::onLiveprogEnabledChanged(bool enabled) {
    if(!enabled) DspManager::instance().setLiveprog(false, "");
}
void MainWindow::onLiveprogFileSelected(const QString& path) {
    if (path.isEmpty()) {
        if(m_liveprogEnable->isChecked()) DspManager::instance().setLiveProgContent(true, "");
        return;
    }

    QFile f(path);
    if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_currentLiveProgScript = f.readAll();
        f.close();
        
        // Parse params and initialize defaults
        auto params = LiveProgHelper::parseParams(m_currentLiveProgScript);
        m_currentLiveProgParams.clear();
        for(const auto& p : params) {
            m_currentLiveProgParams[p.var] = p.def;
        }
        
        // Update label
        if(m_liveprogInfoLabel) m_liveprogInfoLabel->setText(QFileInfo(path).fileName());
        
        // Update Embedded Widgets ALWAYS (so user can see them before enabling)
        if(m_liveEditor) m_liveEditor->setContent(m_currentLiveProgScript);
        if(m_liveParams) m_liveParams->rebuild(m_currentLiveProgScript, m_currentLiveProgParams);

        // Update DSP ONLY if enabled
        if(m_liveprogEnable->isChecked()) {
            QString processed = LiveProgHelper::injectValues(m_currentLiveProgScript, m_currentLiveProgParams);
            DspManager::instance().setLiveProgContent(true, processed);
            
            // Force update of all parameters to Ensure engine state matches
            for(auto it = m_currentLiveProgParams.begin(); it != m_currentLiveProgParams.end(); ++it) {
                DspManager::instance().setLiveProgParam(it.key(), it.value());
            }
        }
    } else {
        LOG("[ERROR] Failed to read LiveProg file");
    }
}

void MainWindow::savePreset() { QMessageBox::information(this, "Preset", "Save Preset clicked"); }
void MainWindow::loadPreset() { QMessageBox::information(this, "Preset", "Load Preset clicked"); }
void MainWindow::resetDefaults() { QMessageBox::information(this, "Reset", "Reset to defaults clicked"); }

void MainWindow::updateStatus()
{
    // Update labels if processing active
}

void MainWindow::populateDevices()
{
    m_captureDeviceCombo->clear();
    m_outputDeviceCombo->clear();

    // User requested Loopback workflow: Capture from Render devices (e.g. VB Cable Input)
    auto inputs = DeviceEnumerator::getPlaybackDevices();
    for (int i = 0; i < inputs.size(); ++i) {
        m_captureDeviceCombo->addItem(inputs[i].name + " [Loopback]", inputs[i].id);
    }

    auto outputs = DeviceEnumerator::getPlaybackDevices();
    for (int i = 0; i < outputs.size(); ++i) {
        m_outputDeviceCombo->addItem(outputs[i].name, outputs[i].id);
    }
    
    // Restore selection if saved
    if(!m_savedCaptureId.isEmpty()) {
        int idx = m_captureDeviceCombo->findData(m_savedCaptureId);
        if(idx >= 0) m_captureDeviceCombo->setCurrentIndex(idx);
    }
    if(!m_savedOutputId.isEmpty()) {
        int idx = m_outputDeviceCombo->findData(m_savedOutputId);
        if(idx >= 0) m_outputDeviceCombo->setCurrentIndex(idx);
    }
}



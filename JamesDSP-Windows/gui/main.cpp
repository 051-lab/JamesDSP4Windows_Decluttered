#include "MainWindow.h"
extern "C" void JamesDSP_Initialize();
extern "C" void JamesDSP_Destruction();
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <iostream>
#include <windows.h>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QSettings>

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    
    QFile file("debug_log.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    
    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }
    
    out << "[" << time << "] [" << level << "] " << msg << "\n";
    
    // Also print to console if attached
    std::cout << "[" << level.toStdString() << "] " << msg.toStdString() << "\n";
}

int main(int argc, char *argv[])
{
    // Attach console for debugging
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    
    // Install Log Handler
    qInstallMessageHandler(messageHandler);

    JamesDSP_Initialize();
    atexit(JamesDSP_Destruction);
    
    qDebug() << "JamesDSP Starting...";
    std::cout << "[DEBUG] Starting JamesDSP..." << std::endl;

    QApplication app(argc, argv);
    
    app.setApplicationName("JamesDSP");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("JamesDSP");
    
    // Reset Settings as requested for clean debugging
    QSettings settings("JamesDSP", "JamesDSP-Windows");
    settings.clear(); 
    qDebug() << "Settings cleared for JamesDSP-Windows.";
    
    std::cout << "[DEBUG] Creating MainWindow..." << std::endl;
    MainWindow window;
    
    // Center on screen
    window.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            window.size(),
            QGuiApplication::primaryScreen()->geometry()
        )
    );
    
    std::cout << "[DEBUG] Showing Window..." << std::endl;
    window.show();
    
    std::cout << "[DEBUG] Entering Event Loop..." << std::endl;
    return app.exec();
}

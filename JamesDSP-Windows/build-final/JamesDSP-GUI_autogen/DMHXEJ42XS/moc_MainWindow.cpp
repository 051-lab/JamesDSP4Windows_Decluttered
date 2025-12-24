/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../gui/MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "toggleProcessing",
        "",
        "enabled",
        "updateStatus",
        "onLimiterThresholdChanged",
        "value",
        "onLimiterReleaseChanged",
        "onPostGainChanged",
        "onBassBoostEnabledChanged",
        "onBassBoostGainChanged",
        "onTubeEnabledChanged",
        "onTubeDriveChanged",
        "onCompanderEnabledChanged",
        "onCompanderTimeChanged",
        "onCompanderGranularityChanged",
        "index",
        "onCompanderTFChanged",
        "onStereoEnabledChanged",
        "onStereoLevelChanged",
        "onEqEnabledChanged",
        "onEqBandChanged",
        "bandIndex",
        "onEqModeChanged",
        "onEqInterpChanged",
        "onReverbEnabledChanged",
        "onReverbPresetChanged",
        "onCrossfeedEnabledChanged",
        "onCrossfeedPresetChanged",
        "onConvolverEnabledChanged",
        "onConvolverFileSelected",
        "path",
        "onConvolverOptimizationChanged",
        "onDdcEnabledChanged",
        "onDdcFileSelected",
        "onLiveprogEnabledChanged",
        "onLiveprogFileSelected",
        "onGraphicEqEnabledChanged",
        "savePreset",
        "loadPreset",
        "resetDefaults"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'toggleProcessing'
        QtMocHelpers::SlotData<void(bool)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'updateStatus'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLimiterThresholdChanged'
        QtMocHelpers::SlotData<void(int)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onLimiterReleaseChanged'
        QtMocHelpers::SlotData<void(int)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onPostGainChanged'
        QtMocHelpers::SlotData<void(int)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onBassBoostEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onBassBoostGainChanged'
        QtMocHelpers::SlotData<void(int)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onTubeEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onTubeDriveChanged'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onCompanderEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onCompanderTimeChanged'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onCompanderGranularityChanged'
        QtMocHelpers::SlotData<void(int)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onCompanderTFChanged'
        QtMocHelpers::SlotData<void(int)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onStereoEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onStereoLevelChanged'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onEqEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onEqBandChanged'
        QtMocHelpers::SlotData<void(int, int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 }, { QMetaType::Int, 6 },
        }}),
        // Slot 'onEqModeChanged'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onEqInterpChanged'
        QtMocHelpers::SlotData<void(int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onReverbEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onReverbPresetChanged'
        QtMocHelpers::SlotData<void(int)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onCrossfeedEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onCrossfeedPresetChanged'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onConvolverEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onConvolverFileSelected'
        QtMocHelpers::SlotData<void(const QString &)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Slot 'onConvolverOptimizationChanged'
        QtMocHelpers::SlotData<void(int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
        // Slot 'onDdcEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onDdcFileSelected'
        QtMocHelpers::SlotData<void(const QString &)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Slot 'onLiveprogEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'onLiveprogFileSelected'
        QtMocHelpers::SlotData<void(const QString &)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 31 },
        }}),
        // Slot 'onGraphicEqEnabledChanged'
        QtMocHelpers::SlotData<void(bool)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'savePreset'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'loadPreset'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'resetDefaults'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->toggleProcessing((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->updateStatus(); break;
        case 2: _t->onLimiterThresholdChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->onLimiterReleaseChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onPostGainChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onBassBoostEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->onBassBoostGainChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onTubeEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->onTubeDriveChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->onCompanderEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->onCompanderTimeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onCompanderGranularityChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->onCompanderTFChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->onStereoEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->onStereoLevelChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->onEqEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->onEqBandChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 17: _t->onEqModeChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->onEqInterpChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->onReverbEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->onReverbPresetChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 21: _t->onCrossfeedEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 22: _t->onCrossfeedPresetChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->onConvolverEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 24: _t->onConvolverFileSelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 25: _t->onConvolverOptimizationChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->onDdcEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->onDdcFileSelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: _t->onLiveprogEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 29: _t->onLiveprogFileSelected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->onGraphicEqEnabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 31: _t->savePreset(); break;
        case 32: _t->loadPreset(); break;
        case 33: _t->resetDefaults(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 34)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 34;
    }
    return _id;
}
QT_WARNING_POP

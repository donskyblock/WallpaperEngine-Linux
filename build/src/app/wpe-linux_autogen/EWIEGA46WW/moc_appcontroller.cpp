/****************************************************************************
** Meta object code from reading C++ file 'appcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/app/appcontroller.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'appcontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
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
struct qt_meta_tag_ZN3wpe13AppControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto wpe::AppController::qt_create_metaobjectdata<qt_meta_tag_ZN3wpe13AppControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "wpe::AppController",
        "windowVisibleChanged",
        "",
        "onboardingChanged",
        "environmentChanged",
        "displaysChanged",
        "selectedOutputChanged",
        "activeChanged",
        "toolsChanged",
        "settingsChanged",
        "loadingChanged",
        "downloadsChanged",
        "applyingChanged",
        "toast",
        "message",
        "kind",
        "guardCodeRequired",
        "loginFinished",
        "ok",
        "quitRequested",
        "search",
        "query",
        "loadMore",
        "setSort",
        "sort",
        "setTypeFilter",
        "type",
        "filterLibrary",
        "text",
        "refreshLibrary",
        "applyLibraryItem",
        "row",
        "applyWorkshopId",
        "workshopId",
        "applyPath",
        "pathOrUrl",
        "stopWallpaper",
        "download",
        "itemId",
        "saveSettings",
        "QVariantMap",
        "map",
        "steamLogin",
        "user",
        "password",
        "submitGuardCode",
        "code",
        "installAssets",
        "installGnomeExtension",
        "installPlasmaPlugin",
        "refreshTools",
        "openExternal",
        "url",
        "quitApp",
        "formatBytes",
        "b",
        "formatCount",
        "n",
        "workshop",
        "wpe::WorkshopModel*",
        "library",
        "wpe::LibraryModel*",
        "windowVisible",
        "onboarding",
        "environmentSummary",
        "caveats",
        "desktopId",
        "displays",
        "QVariantList",
        "selectedOutput",
        "active",
        "tools",
        "settings",
        "loadingWorkshop",
        "loadingMore",
        "workshopError",
        "workshopTotal",
        "downloads",
        "applying"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'windowVisibleChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'onboardingChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'environmentChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'displaysChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedOutputChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'toolsChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'settingsChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loadingChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'downloadsChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'applyingChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'toast'
        QtMocHelpers::SignalData<void(QString, QString)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 }, { QMetaType::QString, 15 },
        }}),
        // Signal 'guardCodeRequired'
        QtMocHelpers::SignalData<void(QString)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
        // Signal 'loginFinished'
        QtMocHelpers::SignalData<void(bool, QString)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 18 }, { QMetaType::QString, 14 },
        }}),
        // Signal 'quitRequested'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'search'
        QtMocHelpers::MethodData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 },
        }}),
        // Method 'loadMore'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'setSort'
        QtMocHelpers::MethodData<void(const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 24 },
        }}),
        // Method 'setTypeFilter'
        QtMocHelpers::MethodData<void(const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 26 },
        }}),
        // Method 'filterLibrary'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 28 }, { QMetaType::QString, 26 },
        }}),
        // Method 'refreshLibrary'
        QtMocHelpers::MethodData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'applyLibraryItem'
        QtMocHelpers::MethodData<void(int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
        // Method 'applyWorkshopId'
        QtMocHelpers::MethodData<void(const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 33 },
        }}),
        // Method 'applyPath'
        QtMocHelpers::MethodData<void(const QString &)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 35 },
        }}),
        // Method 'stopWallpaper'
        QtMocHelpers::MethodData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'download'
        QtMocHelpers::MethodData<void(const QString &)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 38 },
        }}),
        // Method 'saveSettings'
        QtMocHelpers::MethodData<void(const QVariantMap &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 40, 41 },
        }}),
        // Method 'steamLogin'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 43 }, { QMetaType::QString, 44 },
        }}),
        // Method 'submitGuardCode'
        QtMocHelpers::MethodData<void(const QString &)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 46 },
        }}),
        // Method 'installAssets'
        QtMocHelpers::MethodData<void()>(47, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'installGnomeExtension'
        QtMocHelpers::MethodData<void()>(48, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'installPlasmaPlugin'
        QtMocHelpers::MethodData<void()>(49, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshTools'
        QtMocHelpers::MethodData<void()>(50, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openExternal'
        QtMocHelpers::MethodData<void(const QString &)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 52 },
        }}),
        // Method 'quitApp'
        QtMocHelpers::MethodData<void()>(53, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'formatBytes'
        QtMocHelpers::MethodData<QString(qulonglong) const>(54, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::ULongLong, 55 },
        }}),
        // Method 'formatCount'
        QtMocHelpers::MethodData<QString(qulonglong) const>(56, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::ULongLong, 57 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'workshop'
        QtMocHelpers::PropertyData<wpe::WorkshopModel*>(58, 0x80000000 | 59, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'library'
        QtMocHelpers::PropertyData<wpe::LibraryModel*>(60, 0x80000000 | 61, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'windowVisible'
        QtMocHelpers::PropertyData<bool>(62, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'onboarding'
        QtMocHelpers::PropertyData<bool>(63, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'environmentSummary'
        QtMocHelpers::PropertyData<QString>(64, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'caveats'
        QtMocHelpers::PropertyData<QStringList>(65, QMetaType::QStringList, QMC::DefaultPropertyFlags, 2),
        // property 'desktopId'
        QtMocHelpers::PropertyData<QString>(66, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'displays'
        QtMocHelpers::PropertyData<QVariantList>(67, 0x80000000 | 68, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'selectedOutput'
        QtMocHelpers::PropertyData<QString>(69, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'active'
        QtMocHelpers::PropertyData<QVariantList>(70, 0x80000000 | 68, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 5),
        // property 'tools'
        QtMocHelpers::PropertyData<QVariantMap>(71, 0x80000000 | 40, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 6),
        // property 'settings'
        QtMocHelpers::PropertyData<QVariantMap>(72, 0x80000000 | 40, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 7),
        // property 'loadingWorkshop'
        QtMocHelpers::PropertyData<bool>(73, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
        // property 'loadingMore'
        QtMocHelpers::PropertyData<bool>(74, QMetaType::Bool, QMC::DefaultPropertyFlags, 8),
        // property 'workshopError'
        QtMocHelpers::PropertyData<QString>(75, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'workshopTotal'
        QtMocHelpers::PropertyData<quint64>(76, QMetaType::ULongLong, QMC::DefaultPropertyFlags, 8),
        // property 'downloads'
        QtMocHelpers::PropertyData<QVariantMap>(77, 0x80000000 | 40, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 9),
        // property 'applying'
        QtMocHelpers::PropertyData<bool>(78, QMetaType::Bool, QMC::DefaultPropertyFlags, 10),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AppController, qt_meta_tag_ZN3wpe13AppControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject wpe::AppController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe13AppControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe13AppControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3wpe13AppControllerE_t>.metaTypes,
    nullptr
} };

void wpe::AppController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AppController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->windowVisibleChanged(); break;
        case 1: _t->onboardingChanged(); break;
        case 2: _t->environmentChanged(); break;
        case 3: _t->displaysChanged(); break;
        case 4: _t->selectedOutputChanged(); break;
        case 5: _t->activeChanged(); break;
        case 6: _t->toolsChanged(); break;
        case 7: _t->settingsChanged(); break;
        case 8: _t->loadingChanged(); break;
        case 9: _t->downloadsChanged(); break;
        case 10: _t->applyingChanged(); break;
        case 11: _t->toast((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->guardCodeRequired((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->loginFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->quitRequested(); break;
        case 15: _t->search((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->loadMore(); break;
        case 17: _t->setSort((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->setTypeFilter((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->filterLibrary((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 20: _t->refreshLibrary(); break;
        case 21: _t->applyLibraryItem((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->applyWorkshopId((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: _t->applyPath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 24: _t->stopWallpaper(); break;
        case 25: _t->download((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->saveSettings((*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 27: _t->steamLogin((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 28: _t->submitGuardCode((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 29: _t->installAssets(); break;
        case 30: _t->installGnomeExtension(); break;
        case 31: _t->installPlasmaPlugin(); break;
        case 32: _t->refreshTools(); break;
        case 33: _t->openExternal((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 34: _t->quitApp(); break;
        case 35: { QString _r = _t->formatBytes((*reinterpret_cast<std::add_pointer_t<qulonglong>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 36: { QString _r = _t->formatCount((*reinterpret_cast<std::add_pointer_t<qulonglong>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::windowVisibleChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::onboardingChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::environmentChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::displaysChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::selectedOutputChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::activeChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::toolsChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::settingsChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::loadingChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::downloadsChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::applyingChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(QString , QString )>(_a, &AppController::toast, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(QString )>(_a, &AppController::guardCodeRequired, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(bool , QString )>(_a, &AppController::loginFinished, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::quitRequested, 14))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< wpe::LibraryModel* >(); break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< wpe::WorkshopModel* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<wpe::WorkshopModel**>(_v) = _t->workshopModel(); break;
        case 1: *reinterpret_cast<wpe::LibraryModel**>(_v) = _t->libraryModel(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->windowVisible(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->onboarding(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->environmentSummary(); break;
        case 5: *reinterpret_cast<QStringList*>(_v) = _t->caveats(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->desktopId(); break;
        case 7: *reinterpret_cast<QVariantList*>(_v) = _t->displays(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->selectedOutput(); break;
        case 9: *reinterpret_cast<QVariantList*>(_v) = _t->active(); break;
        case 10: *reinterpret_cast<QVariantMap*>(_v) = _t->toolStatus(); break;
        case 11: *reinterpret_cast<QVariantMap*>(_v) = _t->settingsMap(); break;
        case 12: *reinterpret_cast<bool*>(_v) = _t->loadingWorkshop(); break;
        case 13: *reinterpret_cast<bool*>(_v) = _t->loadingMore(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->workshopError(); break;
        case 15: *reinterpret_cast<quint64*>(_v) = _t->workshopTotal(); break;
        case 16: *reinterpret_cast<QVariantMap*>(_v) = _t->downloads(); break;
        case 17: *reinterpret_cast<bool*>(_v) = _t->applying(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 2: _t->setWindowVisible(*reinterpret_cast<bool*>(_v)); break;
        case 3: _t->setOnboarding(*reinterpret_cast<bool*>(_v)); break;
        case 8: _t->setSelectedOutput(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *wpe::AppController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *wpe::AppController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe13AppControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int wpe::AppController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 37)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 37;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 37)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 37;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void wpe::AppController::windowVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void wpe::AppController::onboardingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void wpe::AppController::environmentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void wpe::AppController::displaysChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void wpe::AppController::selectedOutputChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void wpe::AppController::activeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void wpe::AppController::toolsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void wpe::AppController::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void wpe::AppController::loadingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void wpe::AppController::downloadsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void wpe::AppController::applyingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void wpe::AppController::toast(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}

// SIGNAL 12
void wpe::AppController::guardCodeRequired(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void wpe::AppController::loginFinished(bool _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1, _t2);
}

// SIGNAL 14
void wpe::AppController::quitRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}
QT_WARNING_POP

/****************************************************************************
** Meta object code from reading C++ file 'dbusapi.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/app/dbusapi.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dbusapi.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3wpe7DBusApiE_t {};
} // unnamed namespace

template <> constexpr inline auto wpe::DBusApi::qt_create_metaobjectdata<qt_meta_tag_ZN3wpe7DBusApiE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "wpe::DBusApi",
        "D-Bus Interface",
        "org.wpelinux.WallpaperEngine1",
        "SetWallpaper",
        "",
        "path",
        "type",
        "output",
        "Random",
        "Stop",
        "Restore",
        "Outputs",
        "Status",
        "Environment",
        "Show",
        "Quit"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'SetWallpaper'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(3, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 6 }, { QMetaType::QString, 7 },
        }}),
        // Slot 'Random'
        QtMocHelpers::SlotData<void(const QString &)>(8, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Slot 'Stop'
        QtMocHelpers::SlotData<void(const QString &)>(9, 4, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Slot 'Restore'
        QtMocHelpers::SlotData<void()>(10, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Outputs'
        QtMocHelpers::SlotData<QString()>(11, 4, QMC::AccessPublic, QMetaType::QString),
        // Slot 'Status'
        QtMocHelpers::SlotData<QString()>(12, 4, QMC::AccessPublic, QMetaType::QString),
        // Slot 'Environment'
        QtMocHelpers::SlotData<QString()>(13, 4, QMC::AccessPublic, QMetaType::QString),
        // Slot 'Show'
        QtMocHelpers::SlotData<void()>(14, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Quit'
        QtMocHelpers::SlotData<void()>(15, 4, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<DBusApi, qt_meta_tag_ZN3wpe7DBusApiE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject wpe::DBusApi::staticMetaObject = { {
    QMetaObject::SuperData::link<QDBusAbstractAdaptor::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe7DBusApiE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe7DBusApiE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3wpe7DBusApiE_t>.metaTypes,
    nullptr
} };

void wpe::DBusApi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DBusApi *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->SetWallpaper((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 1: _t->Random((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->Stop((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->Restore(); break;
        case 4: { QString _r = _t->Outputs();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 5: { QString _r = _t->Status();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 6: { QString _r = _t->Environment();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 7: _t->Show(); break;
        case 8: _t->Quit(); break;
        default: ;
        }
    }
}

const QMetaObject *wpe::DBusApi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *wpe::DBusApi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe7DBusApiE_t>.strings))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int wpe::DBusApi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP

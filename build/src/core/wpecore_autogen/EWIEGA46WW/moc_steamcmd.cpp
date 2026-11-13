/****************************************************************************
** Meta object code from reading C++ file 'steamcmd.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/steamcmd.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'steamcmd.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3wpe8SteamCmdE_t {};
} // unnamed namespace

template <> constexpr inline auto wpe::SteamCmd::qt_create_metaobjectdata<qt_meta_tag_ZN3wpe8SteamCmdE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "wpe::SteamCmd",
        "progress",
        "",
        "itemId",
        "line",
        "downloadFinished",
        "ok",
        "pathOrError",
        "guardCodeRequired",
        "kind",
        "loginFinished",
        "message",
        "assetsFinished"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'progress'
        QtMocHelpers::SignalData<void(QString, QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'downloadFinished'
        QtMocHelpers::SignalData<void(QString, bool, QString)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 6 }, { QMetaType::QString, 7 },
        }}),
        // Signal 'guardCodeRequired'
        QtMocHelpers::SignalData<void(QString)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'loginFinished'
        QtMocHelpers::SignalData<void(bool, QString)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 }, { QMetaType::QString, 11 },
        }}),
        // Signal 'assetsFinished'
        QtMocHelpers::SignalData<void(bool, QString)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 }, { QMetaType::QString, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SteamCmd, qt_meta_tag_ZN3wpe8SteamCmdE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject wpe::SteamCmd::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe8SteamCmdE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe8SteamCmdE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3wpe8SteamCmdE_t>.metaTypes,
    nullptr
} };

void wpe::SteamCmd::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SteamCmd *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->progress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->downloadFinished((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->guardCodeRequired((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->loginFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->assetsFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SteamCmd::*)(QString , QString )>(_a, &SteamCmd::progress, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SteamCmd::*)(QString , bool , QString )>(_a, &SteamCmd::downloadFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SteamCmd::*)(QString )>(_a, &SteamCmd::guardCodeRequired, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SteamCmd::*)(bool , QString )>(_a, &SteamCmd::loginFinished, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SteamCmd::*)(bool , QString )>(_a, &SteamCmd::assetsFinished, 4))
            return;
    }
}

const QMetaObject *wpe::SteamCmd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *wpe::SteamCmd::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe8SteamCmdE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int wpe::SteamCmd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void wpe::SteamCmd::progress(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void wpe::SteamCmd::downloadFinished(QString _t1, bool _t2, QString _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void wpe::SteamCmd::guardCodeRequired(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void wpe::SteamCmd::loginFinished(bool _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void wpe::SteamCmd::assetsFinished(bool _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}
QT_WARNING_POP

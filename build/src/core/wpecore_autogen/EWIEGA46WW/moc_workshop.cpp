/****************************************************************************
** Meta object code from reading C++ file 'workshop.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/core/workshop.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'workshop.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN3wpe14WorkshopClientE_t {};
} // unnamed namespace

template <> constexpr inline auto wpe::WorkshopClient::qt_create_metaobjectdata<qt_meta_tag_ZN3wpe14WorkshopClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "wpe::WorkshopClient",
        "searchFinished",
        "",
        "page",
        "QList<wpe::WorkshopItem>",
        "items",
        "total",
        "detailsFinished",
        "wpe::WorkshopItem",
        "item",
        "failed",
        "message"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'searchFinished'
        QtMocHelpers::SignalData<void(int, QList<wpe::WorkshopItem>, quint64)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 4, 5 }, { QMetaType::ULongLong, 6 },
        }}),
        // Signal 'detailsFinished'
        QtMocHelpers::SignalData<void(wpe::WorkshopItem)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'failed'
        QtMocHelpers::SignalData<void(QString)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WorkshopClient, qt_meta_tag_ZN3wpe14WorkshopClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject wpe::WorkshopClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe14WorkshopClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe14WorkshopClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3wpe14WorkshopClientE_t>.metaTypes,
    nullptr
} };

void wpe::WorkshopClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkshopClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->searchFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<wpe::WorkshopItem>>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<quint64>>(_a[3]))); break;
        case 1: _t->detailsFinished((*reinterpret_cast<std::add_pointer_t<wpe::WorkshopItem>>(_a[1]))); break;
        case 2: _t->failed((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<wpe::WorkshopItem> >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< wpe::WorkshopItem >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WorkshopClient::*)(int , QList<wpe::WorkshopItem> , quint64 )>(_a, &WorkshopClient::searchFinished, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WorkshopClient::*)(wpe::WorkshopItem )>(_a, &WorkshopClient::detailsFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WorkshopClient::*)(QString )>(_a, &WorkshopClient::failed, 2))
            return;
    }
}

const QMetaObject *wpe::WorkshopClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *wpe::WorkshopClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3wpe14WorkshopClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int wpe::WorkshopClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void wpe::WorkshopClient::searchFinished(int _t1, QList<wpe::WorkshopItem> _t2, quint64 _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void wpe::WorkshopClient::detailsFinished(wpe::WorkshopItem _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void wpe::WorkshopClient::failed(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP

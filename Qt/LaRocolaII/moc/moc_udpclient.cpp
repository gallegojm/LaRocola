/****************************************************************************
** Meta object code from reading C++ file 'udpclient.h'
**
** Created: Thu 19. Sep 16:42:09 2013
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../udpclient.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'udpclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_UdpClient[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       3,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      26,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   42,   10,   10, 0x0a,
      71,   67,   59,   10, 0x0a,
      86,   10,   10,   10, 0x0a,
     104,   10,   10,   10, 0x08,

 // properties: name, type, flags
     126,  114, 0x0b495001,
     132,   59, 0x0a495001,
     140,   59, 0x0a495001,

 // properties: notify_signal_id
       0,
       1,
       1,

       0        // eod
};

static const char qt_meta_stringdata_UdpClient[] = {
    "UdpClient\0\0linesChanged()\0statusChanged()\0"
    "cmd\0sendCmd(int)\0QString\0ind\0"
    "colorFile(int)\0sendCmdListRoot()\0"
    "getData()\0QStringList\0lines\0status1\0"
    "status2\0"
};

const QMetaObject UdpClient::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_UdpClient,
      qt_meta_data_UdpClient, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UdpClient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UdpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UdpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UdpClient))
        return static_cast<void*>(const_cast< UdpClient*>(this));
    return QObject::qt_metacast(_clname);
}

int UdpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: linesChanged(); break;
        case 1: statusChanged(); break;
        case 2: sendCmd((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: { QString _r = colorFile((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 4: sendCmdListRoot(); break;
        case 5: getData(); break;
        default: ;
        }
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QStringList*>(_v) = lines(); break;
        case 1: *reinterpret_cast< QString*>(_v) = status1(); break;
        case 2: *reinterpret_cast< QString*>(_v) = status2(); break;
        }
        _id -= 3;
    } else if (_c == QMetaObject::WriteProperty) {
        _id -= 3;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void UdpClient::linesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void UdpClient::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE

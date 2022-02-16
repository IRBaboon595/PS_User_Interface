#ifndef HARDWAREDEVICE_H
#define HARDWAREDEVICE_H

#include <QObject>
#include <QString>
#include <stdint-gcc.h>
#include <stdlib.h>

class HardwareDevice : public QObject
{
    Q_OBJECT
public:
    explicit HardwareDevice(QObject *parent = nullptr);
    explicit HardwareDevice(HardwareDevice *device);
    explicit HardwareDevice(QString name);
    ~HardwareDevice();

    QString getName();
    uint8_t getID();
    QString getStatus();

    void setName(QString name);
    void setID(uint8_t id);
    void setStatus(QString stat);

    void operator=(HardwareDevice *other);
    void operator=(HardwareDevice other);

private:
    QString         devName;
    uint8_t         devID;
    QString         devStatus;
};

#endif // HARDWAREDEVICE_H

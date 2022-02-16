#include "hardwaredevice.h"

HardwareDevice::HardwareDevice(QObject *parent) : QObject(parent)
{
    devName = "Blank";
    devID = 0xFF;
    devStatus = "Pre-initialized";
}

HardwareDevice::HardwareDevice(HardwareDevice *device)
{
    devName = device->devName;
    devID = device->devID;
    devStatus = device->devStatus;
}

HardwareDevice::HardwareDevice(QString name)
{
   devName = name;
}

QString HardwareDevice::getName()
{
    return this->devName;
}

uint8_t HardwareDevice::getID()
{
    return this->devID;
}

QString HardwareDevice::getStatus()
{
    return this->devStatus;
}

void HardwareDevice::setName(QString name)
{
    devName = name;
}

void HardwareDevice::setID(uint8_t id)
{
    devID = id;
}

void HardwareDevice::setStatus(QString stat)
{
    devStatus = stat;
}

void HardwareDevice::operator=(HardwareDevice *other)
{
    devName = other->devName;
    devID = other->devID;
    devStatus = other->devStatus;
}

void HardwareDevice::operator=(HardwareDevice other)
{
    devName = other.devName;
    devID = other.devID;
    devStatus = other.devStatus;
}

HardwareDevice::~HardwareDevice()
{

}

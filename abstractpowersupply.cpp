#include "abstractpowersupply.h"

abstractPowerSupply::abstractPowerSupply()
{
    mode = MODE_STANDBY;
    mubpTemp = 23;
    vent_1 = false;
    vent_2 = false;
    vent_1_rot = 0;
    vent_2_rot = 0;
    psState = STATE_ALARM;
    powerSourcesQu = 0;
}

abstractPowerSupply::abstractPowerSupply(abstractPowerSupply *a)
{
    mode = a->mode;
    mubpTemp = a->mubpTemp;
    vent_1 = a->vent_1;
    vent_2 = a->vent_2;
    vent_1_rot = a->vent_1_rot;
    vent_2_rot = a->vent_2_rot;
    psState = a->psState;
    powerSourcesQu = a->powerSourcesQu;
}

void abstractPowerSupply::setMode(uint8_t mode)
{
    this->mode = mode;
}

void abstractPowerSupply::setMubpTemp(float temp)
{
    this->mubpTemp = temp;
}

void abstractPowerSupply::setVentState(int ventNum, bool ventState)
{
    switch (ventNum) {
    case VENT_1:
        this->vent_1 = ventState;
    break;
    case VENT_2:
        this->vent_2 = ventState;
    break;
    default:
        qDebug() << "Wrong Vent number!";
    break;
    }
}

void abstractPowerSupply::setVentRot(int ventNum, uint16_t ventRot)
{
    switch (ventNum) {
    case VENT_1:
        this->vent_1_rot = ventRot;
    break;
    case VENT_2:
        this->vent_2_rot = ventRot;
    break;
    default:
        qDebug() << "Wrong Vent number!";
    break;
    }
}

void abstractPowerSupply::setPsState(bool state)
{
    this->psState = state;
}

void abstractPowerSupply::setPsQuantity(uint8_t quant)
{
    this->powerSourcesQu = quant;
}

uint8_t abstractPowerSupply::getMode()
{
    return mode;
}

float abstractPowerSupply::getMubpTemp()
{
    return mubpTemp;
}

bool abstractPowerSupply::getVentState(int ventNum)
{
    bool result = false;
    switch (ventNum) {
    case VENT_1:
        result = vent_1;
    break;
    case VENT_2:
        result = vent_2;
    break;
    default:
        qDebug() << "Wrong Vent number!";
    break;
    }
    return result;
}

uint16_t abstractPowerSupply::getVentRot(int ventNum)
{
    uint16_t result = 0xFFFF;
    switch (ventNum) {
    case VENT_1:
        result = vent_1_rot;
    break;
    case VENT_2:
        result = vent_2_rot;
    break;
    default:
        qDebug() << "Wrong Vent number!";
    break;
    }
    return result;
}

bool abstractPowerSupply::getPsState()
{
    return psState;
}
uint8_t abstractPowerSupply::getPsQuantity()
{
    return powerSourcesQu;
}

abstractPowerSupply::~abstractPowerSupply()
{

}

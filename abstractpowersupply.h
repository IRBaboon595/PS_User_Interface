#ifndef ABSTRACTPOWERSUPPLY_H
#define ABSTRACTPOWERSUPPLY_H

#include <QObject>
#include <QDebug>
#include <hardwaredevice.h>

typedef enum{
    MODE_STANDBY = 0,
    MODE_WORK,
    MODE_STAND
}psModes;

typedef enum{
    SOURCE_ON = 0,
    SOURCE_OFF
}powerSourceStates;

typedef enum{
    STATE_NORM = 0,
    STATE_ALARM
}psStates;

typedef enum{
    VENT_1 = 0,
    VENT_2
}ventState;

typedef struct{
    bool powerSourceState;
    bool tempSensState;
    bool tempSensState_reserved;
    float temperature;
    float voltage;
    float current;
}psStruct;

class abstractPowerSupply : public HardwareDevice
{
    Q_OBJECT
public:
    explicit abstractPowerSupply();
    abstractPowerSupply(abstractPowerSupply *a);
    ~abstractPowerSupply();

    void setMode(uint8_t mode);
    void setMubpTemp(float temp);
    void setVentState(int ventNum, bool ventState);
    void setVentRot(int ventNum, uint16_t ventRot);
    void setPsState(bool state);
    void setPsQuantity(uint8_t quant);

    uint8_t getMode();
    float getMubpTemp();
    bool getVentState(int ventNum);
    uint16_t getVentRot(int ventNum);
    bool getPsState();
    uint8_t getPsQuantity();

    psStruct *psStructMass;

private:
    uint8_t mode;
    float mubpTemp;
    bool vent_1;
    bool vent_2;
    uint16_t vent_1_rot;
    uint16_t vent_2_rot;
    bool psState;
    uint8_t powerSourcesQu;
};

#endif // ABSTRACTPOWERSUPPLY_H

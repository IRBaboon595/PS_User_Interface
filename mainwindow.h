#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QTextEdit>
#include <iostream>
#include <iostream>
#include <math.h>
#include <QtMath>
#include <time.h>
#include <locale.h>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cstring>
#include <string>
#include <iomanip>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QDialog>
#include <QTabWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QDataStream>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>
#include <abstractpowersupply.h>
#include <QThread>
#include <QFileDialog>
#include <QFrame>

#define	SYNCHRO                                 0x02
#define UART_ADDR                               0x0A
#define SERVICE_BITS_LEN                        0x06

#define ECHO                                    0x00
#define SUPPLY_CTRL                             0x01
#define ATT_CTRL                                0x02
#define DAC_REVISION                            0x03
#define	DAC_CTRL                                0x04
#define POWER_AMP_EN                            0x05
#define UM78_SUPPLY_CTRL                        0x06
#define UM78_AMP_EN                             0x07
#define UM78_DAC_CTRL                           0x08
#define STROBE_CTRL                             0x09
#define GET_POWER                               0x0A
#define CAL_POWER                               0x0B
#define COMP_DAC                                0x0C
#define STOP_CAL                                0x0D

#define ON                                      0x01
#define OFF                                     0x00

typedef enum{
    POWER_UNIT_SECONDARY_ADDRESS = 0x1E,
    POWER_UNIT_PRIMARY_RX_ADDRESS = 0x1E,
    POWER_UNIT_PRIMARY_TX_4_ADDRESS = 0x13,
    POWER_UNIT_PRIMARY_TX_3_ADDRESS,
    POWER_UNIT_PRIMARY_TX_2_ADDRESS,
    POWER_UNIT_PRIMARY_TX_1_ADDRESS
}powerUnitsAdd;

typedef enum{
    POWER_UNIT_SECONDARY = 0x01,
    POWER_UNIT_PRIMARY_RX,
    POWER_UNIT_PRIMARY_TX_1,
    POWER_UNIT_PRIMARY_TX_2,
    POWER_UNIT_PRIMARY_TX_3,
    POWER_UNIT_PRIMARY_TX_4
}powerUnitsID;

typedef enum{
    DO_READ = 1,
    DI_READ,
    AO_READ,
    AI_READ,
    DO_SINGLE_WRITE,
    AO_SINGLE_WRITE,
    DO_MULT_WRITE = 0x0F,
    AO_MULT_WRITE
}func;

typedef enum{
    CMD_1_H = 0x3000,
    CMD_1_L,
    CMD_2_H,
    CMD_2_L,
    ADDR_TEMP_SENSOR_1,
    ADDR_TEMP_SENSOR_2,
    ADDR_TEMP_SENSOR_3,
    OFFSET_CURR_SENSOR_1,
    OFFSET_CURR_SENSOR_2,
    OFFSET_CURR_SENSOR_3
}modbus_INPUT_adresses;

typedef enum{
    STATE_1_H = 0x3080,
    STATE_1_L,
    TEMPERATURE_1 = 0x3086,
    TEMPERATURE_2,
    TEMPERATURE_3,
    TEMPERATURE_4,
    FAN_RPM_1 = 0x308B,
    FAN_RPM_2,
    VOLTAGE_1 = 0x3090,
    VOLTAGE_2,
    VOLTAGE_3,
    CURRENT_1,
    CURRENT_2,
    CURRENT_3
}modbus_OUTPUT_adresses;

typedef enum{
    WORK_MODE = 0x0001,
    STANDBY_MODE,
    TEST_MODE = 0x0004
}modbus_CMD_1_commands;

typedef enum{
    PS_1_ON = 0x0001,
    PS_1_OFF,
    PS_2_ON = 0x0004,
    PS_2_OFF = 0x0008,
    PS_3_ON = 0x0010,
    PS_3_OFF = 0x0020
}modbus_CMD_2_commands;

/*typedef enum{
    OK_STATE = 0,
    ERROR_SLAVE_ADDRESS,
    ERROR_CRC,
    ERROR_LEN,
    ERROR_COMMAND,
    ERROR_CRC_MOD
}modbus_status;*/

typedef union{
    uint16_t    istd;
    uint8_t 	cstd[2];
}std_union;

typedef union{
    uint32_t 	listd;
    uint16_t 	istd[2];
    uint8_t 	cstd[4];
}long_std_union;

typedef union{
    uint64_t 	llistd;
    uint32_t 	listd[2];
    uint16_t 	istd[4];
    uint8_t 	cstd[8];
}long_long_std_union;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    uint16_t crc(char *buff, uint16_t len);
    char* modbusMessage(char devAddress, char modbusFunc, uint16_t modbusRegAddress, uint16_t modbusRegNum, char *data);

    void clearDataBank();

    QSerialPort      *COM;
    QFile           *File;

private slots:
    void getEcho();
    void connectCOM();
    void readData();
    void on_com_refresh_button_clicked();
    void timer_echo_timeout();
    void puChanged(int index);
    void modeChanged(int index);
    void onPuOnClicked();
    void onPsOnClicked();
    void onTimerFeedbackElapsed();
    void onStopPollClicked();

private:
    abstractPowerSupply     puMass[6];
    abstractPowerSupply     *puSec;
    abstractPowerSupply     *puPriRX;
    abstractPowerSupply     *puPriTX_1;
    abstractPowerSupply     *puPriTX_2;
    abstractPowerSupply     *puPriTX_3;
    abstractPowerSupply     *puPriTX_4;

    QTimer                  *timer_echo;
    uint8_t                 com_mode;
    uint8_t                 uart_command;
    int                     currentPowerUnit;
    uint16_t                lastAccessedReg;
    uint16_t                lastWrittenValue;
    uint16_t                lastNumberRegWritten;
    uint32_t                puState;

    uint64_t                time;
    QTimer                  *timer_feedback;
    QElapsedTimer           *timer_2;
    QTimer                  *timer_3;
    QByteArray              ba;
    char                    CRC = 0;
    char                    modbusAddresses[6] = {POWER_UNIT_SECONDARY_ADDRESS, POWER_UNIT_PRIMARY_RX_ADDRESS, POWER_UNIT_PRIMARY_TX_1_ADDRESS,
                                                 POWER_UNIT_PRIMARY_TX_2_ADDRESS, POWER_UNIT_PRIMARY_TX_3_ADDRESS, POWER_UNIT_PRIMARY_TX_4_ADDRESS};

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

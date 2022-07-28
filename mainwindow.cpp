#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Power Units initialization

    for (uint8_t i = 0; i < 6; i++)
    {
        switch (i + 1) {
            case POWER_UNIT_SECONDARY:
                puMass[i].setName("Secondary Channel Power Unit");
                puMass[i].setPsQuantity(3);
            break;
            case POWER_UNIT_PRIMARY_RX:
                puMass[i].setName("Primary Channel RX Rack Power Unit");
                puMass[i].setPsQuantity(2);
            break;
            case POWER_UNIT_PRIMARY_TX_1:
                puMass[i].setName("Primary Channel TX Rack 1 Power Unit");
                puMass[i].setPsQuantity(1);
            break;
            case POWER_UNIT_PRIMARY_TX_2:
                puMass[i].setName("Primary Channel TX Rack 2 Power Unit");
                puMass[i].setPsQuantity(1);
            break;
            case POWER_UNIT_PRIMARY_TX_3:
                puMass[i].setName("Primary Channel TX Rack 3 Power Unit");
                puMass[i].setPsQuantity(1);
            break;
            case POWER_UNIT_PRIMARY_TX_4:
                puMass[i].setName("Primary Channel TX Rack 4 Power Unit");
                puMass[i].setPsQuantity(1);
            break;
            default:
                qDebug() << "Enumenator ERROR!";
            break;
        }
        puMass[i].psStructMass = new psStruct[puMass[i].getPsQuantity()];
        puMass[i].setID(i + 1);
        puMass[i].setStatus("Inactive");
        puMass[i].setPsState(STATE_NORM);
        puMass[i].setMode(MODE_STANDBY);
    }

    timer_echo = new QTimer;
    timer_feedback = new QTimer;
    timer_feedback->setInterval(3000);
    currentPowerUnit = 0;

    COM = new QSerialPort(this);
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            ui->com_combobox->addItem(info.portName());

    connect(ui->deviceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(puChanged(int)));
    connect(ui->modeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
    connect(ui->powerSupplyOnOffPushButton, SIGNAL(clicked()), this, SLOT(onPuOnClicked()));
    connect(ui->powerSourceOnOffPushButton, SIGNAL(clicked()), this, SLOT(onPsOnClicked()));
    connect(COM, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(ui->com_button, SIGNAL(clicked(bool)), this, SLOT(connectCOM()));
    connect(ui->echo_button, SIGNAL(clicked(bool)), this, SLOT(getEcho()));
    connect(timer_echo, SIGNAL(timeout()), this, SLOT(timer_echo_timeout()));
    connect(timer_feedback, SIGNAL(timeout()), this, SLOT(onTimerFeedbackElapsed()));
    connect(ui->stopPollPushButton, SIGNAL(clicked(bool)), this, SLOT(onStopPollClicked()));
}

/**************************************** Service Functions ****************************************/

void MainWindow::connectCOM(void)
{
    if(COM->portName() != ui->com_combobox->currentText())
    {
        COM->close();
        COM->setPortName(ui->com_combobox->currentText());
    }

    COM->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
    COM->setDataBits(QSerialPort::Data8);
    COM->setParity(QSerialPort::NoParity);
    COM->setStopBits(QSerialPort::OneStop);
    COM->setFlowControl(QSerialPort::NoFlowControl);
    COM->setReadBufferSize(0);
    if(COM->open(QSerialPort::ReadWrite))
    {
        ui->comstate_label->setText("<FONT COLOR=#008000>Opened</FONT>");
        com_mode = 1;
        ui->com_button->setText("Close port");
    }
    else
    {
        COM->close();
        ui->comstate_label->setText("<FONT COLOR=red>Closed</FONT>");
        ui->com_button->setText("Open port");
        com_mode = 0;
    }
}

/***************************************** Control Functions *********************************************/

void MainWindow::getEcho(void)
{
    QByteArray ba_1;
    char len = 0x06;
    char crc = 0;
    char echo = ECHO;
    ba_1.append(SYNCHRO);
    ba_1.append(char(0x00));
    ba_1.append(len);
    ba_1.append(UART_ADDR);
    ba_1.append(echo);
    for(int i = 0; i < ba_1.size(); i++)
    {
        crc ^= ba_1.at(i);
    }
    ba_1.append(crc);
    if(COM->write(ba_1) != -1)
    {
        COM->waitForBytesWritten(200);
        COM->waitForReadyRead(300);
    }
    //timer_echo->start(2000);
    //ui->powerSupplyStateFrame->setStyleSheet("background-color: rgb(255, 0, 0)");
    //ui->tempSensFrame_3->setStyleSheet("background-color: rgb(255, 0, 0)");
}

void MainWindow::timer_echo_timeout()
{
    ui->service_message->setText("Empty");
}

void MainWindow::on_com_refresh_button_clicked()
{
    ui->com_combobox->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            ui->com_combobox->addItem(info.portName());
}

void MainWindow::readData(void)
{
    //QByteArray ba;
    bool trigger = true;
    std_union len;
    len.istd = 0;
    std_union powTemp;
    powTemp.istd = 0;
    long_std_union temp_long;
    temp_long.listd = 0;
    char *buff;

    qDebug() << "ReadyRead_Got";
    ba.append(COM->readAll());
    int idx = ba.indexOf(modbusAddresses[currentPowerUnit], 0);
    if(idx >= 0)
    {
        qDebug() << "Slave Address found";
        if(idx > 0)
        {
            ba.remove(0,idx);
            idx = 0;
        }
        while((ba.size() >= 8) && (trigger))
        {
            qDebug() << "Potential parcel len achieved";
            switch (ba.at(1)) {
                case AO_READ:
                    qDebug() << "Modbus Read";
                    len.istd = uint16_t(ba.at(2));
                    if(ba.size() == 61)
                    {
                        buff = new char[len.istd];
                        for (int i = 0; i < (len.istd + 5); i++)
                        {
                            buff[i] = ba.at(i);
                        }
                        if(crc(buff, (len.istd + 5)) == 0x0000)
                        {
                            qDebug() << "CRC16 is correct - the PARCEL is OK";
                            temp_long.cstd[3] = uint8_t(ba.at(5));
                            temp_long.cstd[2] = uint8_t(ba.at(6));
                            temp_long.cstd[1] = uint8_t(ba.at(3));
                            temp_long.cstd[0] = uint8_t(ba.at(4));
                            puState = temp_long.listd;
                            if((puState & 0x00000003) == puMass[currentPowerUnit].getMode())
                            {
                                qDebug() << "Returned mode is correct";
                            }
                            else
                            {
                                qDebug() << "Returned mode is incorrect";
                            }
                            if(puState & 0x00000004)
                            {
                                ui->powerSupplyStateFrame->setStyleSheet("background-color: rgb(255, 0, 0)");
                            }
                            else
                            {
                                ui->powerSupplyStateFrame->setStyleSheet("background-color: rgb(0, 255, 0)");
                            }

                            powTemp.cstd[1] = uint8_t(ba.at(15));
                            powTemp.cstd[0] = uint8_t(ba.at(16));
                            puMass[currentPowerUnit].setMubpTemp(float(powTemp.istd));
                            ui->tempMUBP->setText(QString::number(double(puMass[currentPowerUnit].getMubpTemp())));
                            if(puState & 0x00000008)
                            {
                                ui->tempMUBPFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                            }
                            else
                            {
                                ui->tempMUBPFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                            }

                            for (int i = 0; i < puMass[currentPowerUnit].getPsQuantity(); i++)
                            {
                                powTemp.cstd[1] = uint8_t(ba.at(17 + 2 * i));       //ПРОВЕРИТЬ ПЕРЕД ДАЛЬНЕЙШЕЙ ПРАВКОЙ!
                                powTemp.cstd[0] = uint8_t(ba.at(18 + 2 * i));
                                puMass[currentPowerUnit].psStructMass[i].temperature = float(powTemp.istd);
                                if(i == 0)
                                {
                                    ui->psTempDataLabel_1->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].temperature)));
                                    if(puState & 0x00000010)
                                    {
                                        ui->tempFrame_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempFrame_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                    if(currentPowerUnit > 1)                                            //Second temp sensor for SHPPK PU
                                    {
                                        if(puState & 0x00000100)
                                        {
                                            ui->tempSensFrame_1_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                                        }
                                        else
                                        {
                                            ui->tempSensFrame_1_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                                        }
                                    }
                                    if(puState & 0x00000080)
                                    {
                                        ui->tempSensFrame_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempSensFrame_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 1)
                                {
                                    ui->psTempDataLabel_2->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].temperature)));
                                    if(puState & 0x00000020)
                                    {
                                        ui->tempFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                    if(puState & 0x00000100)
                                    {
                                        ui->tempSensFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempSensFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 2)
                                {
                                    ui->psTempDataLabel_3->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].temperature)));
                                    if(puState & 0x00000040)
                                    {
                                        ui->tempFrame_3->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempFrame_3->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                    if(puState & 0x00000200)
                                    {
                                        ui->tempSensFrame_3->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->tempSensFrame_3->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                            }

                            powTemp.cstd[1] = uint8_t(ba.at(25));
                            powTemp.cstd[0] = uint8_t(ba.at(26));
                            puMass[currentPowerUnit].setVentRot(0, powTemp.istd);
                            ui->ventRotLabel_1->setText(QString::number(puMass[currentPowerUnit].getVentRot(0)));
                            if(puState & 0x00000400)
                            {
                                ui->ventRotStateFrame_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                            }
                            else
                            {
                                ui->ventRotStateFrame_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                            }
                            powTemp.cstd[1] = uint8_t(ba.at(27));
                            powTemp.cstd[0] = uint8_t(ba.at(28));
                            puMass[currentPowerUnit].setVentRot(1, powTemp.istd);
                            ui->ventRotLabel_2->setText(QString::number(puMass[currentPowerUnit].getVentRot(1)));
                            if(puState & 0x00000800)
                            {
                                ui->ventRotStateFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                            }
                            else
                            {
                                ui->ventRotStateFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                            }

                            for (int i = 0; i < puMass[currentPowerUnit].getPsQuantity(); i++)
                            {
                                powTemp.cstd[1] = uint8_t(ba.at(35 + 2 * i));
                                powTemp.cstd[0] = uint8_t(ba.at(36 + 2 * i));
                                puMass[currentPowerUnit].psStructMass[i].voltage = float(powTemp.istd);
                                if(i == 0)
                                {
                                    ui->psVoltDataLabel_1->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].voltage) * 0.01976));
                                    if(puState & 0x00001000)
                                    {
                                        ui->outVoltFrame_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outVoltFrame_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 1)
                                {
                                    ui->psVoltDataLabel_2->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].voltage) * 0.01976));
                                    if(puState & 0x00002000)
                                    {
                                        ui->outVoltFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outVoltFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 2)
                                {
                                    ui->psVoltDataLabel_3->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].voltage) * 0.01976));
                                    if(puState & 0x00004000)
                                    {
                                        ui->outVoltFrame_3->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outVoltFrame_3->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                            }

                            for (int i = 0; i < puMass[currentPowerUnit].getPsQuantity(); i++)
                            {
                                powTemp.cstd[1] = uint8_t(ba.at(45 + 2 * i));
                                powTemp.cstd[0] = uint8_t(ba.at(46 + 2 * i));
                                puMass[currentPowerUnit].psStructMass[i].current = float(powTemp.istd);
                                if(i == 0)
                                {
                                    ui->psCurDataLabel_1->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].current) * 0.0734));
                                    if(puState & 0x00010000)
                                    {
                                        ui->outCurFrame_1->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outCurFrame_1->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 1)
                                {
                                    ui->psCurDataLabel_2->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].current) * 0.0734));
                                    if(puState & 0x00020000)
                                    {
                                        ui->outCurFrame_2->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outCurFrame_2->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                                else if(i == 2)
                                {
                                    ui->psCurDataLabel_3->setText(QString::number(double(puMass[currentPowerUnit].psStructMass[i].current) * 0.0734));
                                    if(puState & 0x00040000)
                                    {
                                        ui->outCurFrame_3->setStyleSheet("background-color: rgb(255, 0, 0)");
                                    }
                                    else
                                    {
                                        ui->outCurFrame_3->setStyleSheet("background-color: rgb(0, 255, 0)");
                                    }
                                }
                            }

                            powTemp.cstd[1] = uint8_t(ba.at(51));
                            powTemp.cstd[0] = uint8_t(ba.at(52));
                            if(powTemp.istd == 0x5555)
                            {
                                ui->currentSensorOffsetStateFrame->setStyleSheet("background-color: rgb(0, 255, 0)");
                            }
                            else
                            {
                                 ui->currentSensorOffsetStateFrame->setStyleSheet("background-color: rgb(177, 177, 177)");
                            }

                            for (int i = 0; i < puMass[currentPowerUnit].getPsQuantity(); i++)
                            {
                                powTemp.cstd[1] = uint8_t(ba.at(53 + 2 * i));
                                powTemp.cstd[0] = uint8_t(ba.at(54 + 2 * i));

                                if(i == 0)
                                {
                                    ui->currentSensorOffsetLabel_1->setText(QString::number(double(powTemp.istd)));
                                }
                                else if(i == 1)
                                {
                                    ui->currentSensorOffsetLabel_2->setText(QString::number(double(powTemp.istd)));
                                }
                                else if(i == 2)
                                {
                                    ui->currentSensorOffsetLabel_3->setText(QString::number(double(powTemp.istd)));
                                }
                            }
                            ba.clear();
                        }
                        else
                        {
                            qDebug() << "CRC16 is incorrect!";
                        }
                    }
                break;
                case AO_SINGLE_WRITE:
                    qDebug() << "Modbus Single Write Response";
                    len.istd = 8;
                    powTemp.cstd[1] = uint8_t(ba.at(2));
                    powTemp.cstd[0] = uint8_t(ba.at(3));
                    if(powTemp.istd == lastAccessedReg)
                    {
                        qDebug() << "Last accessed register equals";
                        powTemp.cstd[1] = uint8_t(ba.at(4));
                        powTemp.cstd[0] = uint8_t(ba.at(5));
                        if(powTemp.istd == lastWrittenValue)
                        {
                            qDebug() << "Last written value equals";
                            buff = new char[len.istd];
                            for (int i = 0; i < len.istd; i++)
                            {
                                buff[i] = ba.at(i);
                            }
                            if(crc(buff, len.istd) == 0x0000)
                            {
                                qDebug() << "CRC16 is correct - the PARCEL is OK";
                                ba.clear();
                            }
                            else
                            {
                                qDebug() << "CRC16 is incorrect!";
                            }
                        }
                        else
                        {
                            qDebug() << "The last written value differs";
                        }
                    }
                    else
                    {
                        qDebug() << "The last accessed address differs";
                    }
                break;
                case AO_MULT_WRITE:
                    qDebug() << "Modbus Multiple Write Response";
                    len.istd = 8;
                    powTemp.cstd[1] = uint8_t(ba.at(2));
                    powTemp.cstd[0] = uint8_t(ba.at(3));
                    if(powTemp.istd == lastAccessedReg)
                    {
                        qDebug() << "Last accessed register equals";
                        powTemp.cstd[1] = uint8_t(ba.at(4));
                        powTemp.cstd[0] = uint8_t(ba.at(5));
                        if(powTemp.istd == lastNumberRegWritten)
                        {
                            qDebug() << "Last written registers number equals";
                            buff = new char[len.istd];
                            for (int i = 0; i < len.istd; i++)
                            {
                                buff[i] = ba.at(i);
                            }
                            if(crc(buff, len.istd) == 0x0000)
                            {
                                qDebug() << "CRC16 is correct - the PARCEL is OK";
                                ba.clear();
                            }
                            else
                            {
                                qDebug() << "CRC16 is incorrect!";
                            }
                        }
                        else
                        {
                            qDebug() << "The last written value differs";
                        }
                    }
                    else
                    {
                        qDebug() << "The last accessed address differs";
                    }
                break;
                default:

                break;
            }
            trigger = true;
            ba.clear();

        }
    }
}

void MainWindow::onTimerFeedbackElapsed()
{
    qDebug() << puMass[currentPowerUnit].getMode();
    if(puMass[currentPowerUnit].getMode() != MODE_STANDBY)
    {
        QByteArray ba_1;
        ba_1.clear();
        char data[2];
        ba_1.append(modbusMessage(modbusAddresses[currentPowerUnit], AO_READ, 0x3080, 28, data), 8);
        if(COM->write(ba_1) != -1)
        {
            COM->waitForBytesWritten(200);
            COM->waitForReadyRead(300);
        }
    }
    else
    {
        timer_feedback->stop();
    }
}

void MainWindow::puChanged(int index)
{
    qDebug() << index;
    currentPowerUnit = index;
    for (int i = 0; i < 6; i++)
    {
        puMass[i].setStatus("Inactive");
    }
    puMass[currentPowerUnit].setStatus("Active");
    if(currentPowerUnit == 0)
    {
        ui->powerSupplyGroupBox_1->setEnabled(true);
        ui->powerSupplyGroupBox_2->setEnabled(true);
        ui->powerSupplyGroupBox_3->setEnabled(true);
        ui->tempSensLabel_4->setEnabled(false);
    }
    else if(currentPowerUnit == 1)
    {
        ui->powerSupplyGroupBox_1->setEnabled(true);
        ui->powerSupplyGroupBox_2->setEnabled(true);
        ui->powerSupplyGroupBox_3->setEnabled(false);
        ui->tempSensLabel_4->setEnabled(false);
    }
    else
    {
        ui->powerSupplyGroupBox_1->setEnabled(true);
        ui->powerSupplyGroupBox_2->setEnabled(false);
        ui->powerSupplyGroupBox_3->setEnabled(false);
        ui->tempSensLabel_4->setEnabled(true);
    }
}

void MainWindow::modeChanged(int index)
{
    timer_feedback->stop();
    if(index == 0)
    {
        puMass[currentPowerUnit].setMode(MODE_WORK);
        ui->standModeGroubBox->setDisabled(true);
        ui->modeGroupBox->setDisabled(false);
        ui->currentSensorLineEdit_1->clear();
        ui->currentSensorLineEdit_2->clear();
        ui->currentSensorLineEdit_3->clear();
        ui->currentSensorOffsetLabel_1->setText("Смещение ДТ 1");
        ui->currentSensorOffsetLabel_2->setText("Смещение ДТ 2");
        ui->currentSensorOffsetLabel_3->setText("Смещение ДТ 3");
        ui->psCheckBox_1->setCheckState(Qt::Unchecked);
        ui->psCheckBox_2->setCheckState(Qt::Unchecked);
        ui->psCheckBox_3->setCheckState(Qt::Unchecked);
        ui->powerSourceOnOffCheckBox->setCheckState(Qt::Unchecked);
        ui->currentSensorOffsetStateFrame->setStyleSheet("background-color: rgb(177, 177, 177)");
        clearDataBank();
    }
    else if(index == 1)
    {
        puMass[currentPowerUnit].setMode(MODE_STAND);
        ui->standModeGroubBox->setDisabled(false);
        ui->currentSensorLineEdit_1->setEnabled(true);
        ui->currentSensorLineEdit_2->setEnabled(true);
        ui->currentSensorLineEdit_3->setEnabled(true);
        ui->modeGroupBox->setDisabled(true);
        ui->powerSupplyOnOffCheckBox->setCheckState(Qt::Unchecked);
        clearDataBank();
    }
    else
    {
        puMass[currentPowerUnit].setMode(MODE_STANDBY);
        ui->standModeGroubBox->setDisabled(false);
        ui->modeGroupBox->setDisabled(false);
        ui->currentSensorLineEdit_1->clear();
        ui->currentSensorLineEdit_2->clear();
        ui->currentSensorLineEdit_3->clear();
        ui->currentSensorOffsetLabel_1->setText("Смещение ДТ 1");
        ui->currentSensorOffsetLabel_2->setText("Смещение ДТ 2");
        ui->currentSensorOffsetLabel_3->setText("Смещение ДТ 3");
        ui->psCheckBox_1->setCheckState(Qt::Unchecked);
        ui->psCheckBox_2->setCheckState(Qt::Unchecked);
        ui->psCheckBox_3->setCheckState(Qt::Unchecked);
        ui->powerSourceOnOffCheckBox->setCheckState(Qt::Unchecked);
        ui->currentSensorOffsetStateFrame->setStyleSheet("background-color: rgb(177, 177, 177)");
        ui->powerSupplyOnOffCheckBox->setCheckState(Qt::Unchecked);
        clearDataBank();
    }
    COM->waitForReadyRead(200);
}

void MainWindow::onPuOnClicked()
{
    QByteArray ba_1;
    ba_1.clear();
    char data[4];
    std_union temp;
    temp.istd = 0;

    COM->waitForReadyRead(200);
    if(ui->powerSupplyOnOffCheckBox->isChecked())
    {
        temp.istd = WORK_MODE;
        puMass[currentPowerUnit].setMode(MODE_WORK);
        data[0] = char(0x00);
        data[1] = char(0x00);
        data[2] = char(temp.cstd[1]);
        data[3] = char(temp.cstd[0]);
        ba_1.append(modbusMessage(modbusAddresses[currentPowerUnit], AO_MULT_WRITE, 0x3000, 2, data), 13);
    }
    else
    {
        temp.istd = STANDBY_MODE;
        puMass[currentPowerUnit].setMode(MODE_STANDBY);
        data[0] = char(0x00);
        data[1] = char(0x00);
        data[2] = char(temp.cstd[1]);
        data[3] = char(temp.cstd[0]);
        ba_1.append(modbusMessage(modbusAddresses[currentPowerUnit], AO_MULT_WRITE, 0x3000, 2, data), 13);
    }
    lastAccessedReg = 0x3000;
    lastNumberRegWritten = 0x0002;
    if(COM->write(ba_1) != -1)
    {
        COM->waitForBytesWritten(200);
        COM->waitForReadyRead(300);
    }
    timer_feedback->start();
}

void MainWindow::onPsOnClicked()
{
    QByteArray ba_1;
    ba_1.clear();
    char data[20];
    std_union temp;
    temp.istd = 0;

    COM->waitForReadyRead(200);
    if(ui->psCheckBox_1->isChecked())
    {
        temp.istd |= 0x0001;
        temp.istd &=~ 0x0002;
    }
    else
    {
        temp.istd |= 0x0002;
        temp.istd &=~ 0x0001;
    }

    if(ui->psCheckBox_2->isChecked())
    {
        temp.istd |= 0x0004;
        temp.istd &=~ 0x0008;
    }
    else
    {
        temp.istd |= 0x0008;
        temp.istd &=~ 0x0004;
    }

    if(ui->psCheckBox_3->isChecked())
    {
        temp.istd |= 0x0010;
        temp.istd &=~ 0x0020;
    }
    else
    {
        temp.istd |= 0x0020;
        temp.istd &=~ 0x0010;
    }

    if(ui->powerSourceOnOffCheckBox->isChecked())
    {
        temp.istd |= 0x0040;
    }
    else
    {
        temp.istd &=~ 0x0040;
    }

    data[0] = char(0x00);
    data[1] = char(0x00);
    data[2] = char(0x00);
    data[3] = char(TEST_MODE);
    data[4] = char(0x00);
    data[5] = char(0x00);
    data[6] = char(temp.cstd[1]);
    data[7] = char(temp.cstd[0]);
    data[8] = char(0x00);
    data[9] = char(0x00);
    data[10] = char(0x00);
    data[11] = char(0x00);
    data[12] = char(0x00);
    data[13] = char(0x00);

    temp.istd = uint16_t(ui->currentSensorLineEdit_1->text().toInt());
    data[14] = char(temp.cstd[1]);
    data[15] = char(temp.cstd[0]);

    temp.istd = uint16_t(ui->currentSensorLineEdit_2->text().toInt());
    data[16] = char(temp.cstd[1]);
    data[17] = char(temp.cstd[0]);

    temp.istd = uint16_t(ui->currentSensorLineEdit_3->text().toInt());
    data[18] = char(temp.cstd[1]);
    data[19] = char(temp.cstd[0]);
    ba_1.append(modbusMessage(modbusAddresses[currentPowerUnit], AO_MULT_WRITE, 0x3000, 10, data), 29);
    lastAccessedReg = 0x3000;
    lastNumberRegWritten = 0x000A;
    if(COM->write(ba_1) != -1)
    {
        COM->waitForBytesWritten(200);
        COM->waitForReadyRead(300);
    }
    for(int a; a < 30; a++)
    {
        qDebug() << data[a];
    }
    timer_feedback->start();
}

/**************************************** Modbus Functions ****************************************/

uint16_t MainWindow::crc(char *buff, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    std_union temp;
    temp.istd = 0;

    for(int i = 0; i < len; i++)
    {
        temp.cstd[0] = uint8_t(buff[i]);
        crc ^= temp.istd;
        for(int n = 8; n != 0; n--)
        {
            if((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>=1;
            }
        }
        temp.istd = 0;
    }

    return crc;
}

char* MainWindow::modbusMessage(char devAddress, char modbusFunc, uint16_t regAddress, uint16_t modbusRegNum, char *data)
{
    char *buff;
    std_union temp;
    temp.istd = regAddress;
    uint16_t len = 0;

    if((modbusFunc == AO_SINGLE_WRITE) || (modbusFunc == DO_SINGLE_WRITE))
    {
        len = 8;
        buff = new char[len];
        buff[0] = devAddress;
        buff[1]	= modbusFunc;
        buff[2] = char(temp.cstd[1]);
        buff[3] = char(temp.cstd[0]);
        buff[4] = data[0];
        buff[5] = data[1];
        temp.istd = crc(buff, len - 2);
        buff[6] = char(temp.cstd[0]);
        buff[7] = char(temp.cstd[1]);
        return buff;
    }
    else if((modbusFunc == AO_MULT_WRITE) || (modbusFunc == DO_MULT_WRITE))
    {
        len = (9 + modbusRegNum * 2);
        buff = new char[len];
        buff[0] = devAddress;
        buff[1]	= modbusFunc;
        buff[2] = char(temp.cstd[1]);
        buff[3] = char(temp.cstd[0]);
        temp.istd = modbusRegNum;
        buff[4] = char(temp.cstd[1]);
        buff[5] = char(temp.cstd[0]);
        buff[6] = char(modbusRegNum * 2);
        memcpy(buff + 7, data, modbusRegNum * 2);
        temp.istd = crc(buff, len - 2);
        buff[len - 1] = char(temp.cstd[1]);
        buff[len - 2] = char(temp.cstd[0]);
        return buff;
    }
    else if(modbusFunc == AO_READ)
    {
        len = 8;
        buff = new char[len];
        buff[0] = devAddress;
        buff[1]	= modbusFunc;
        buff[2] = char(temp.cstd[1]);
        buff[3] = char(temp.cstd[0]);
        temp.istd = modbusRegNum;
        buff[4] = char(temp.cstd[1]);
        buff[5] = char(temp.cstd[0]);
        temp.istd = crc(buff, len - 2);
        buff[6] = char(temp.cstd[0]);
        buff[7] = char(temp.cstd[1]);
        return buff;
    }
    else
    {
        qDebug() << "Wrong MODBUS command!";
        buff = new char[8];
        memset(buff, 0, 8);
        return buff;
    }
}

void MainWindow::onStopPollClicked()
{
    timer_feedback->stop();
}
void MainWindow::clearDataBank()
{
    ui->tempFrame_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->tempFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->tempFrame_3->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->outVoltFrame_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->outVoltFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->outVoltFrame_3->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->outCurFrame_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->outCurFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->outCurFrame_3->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->tempSensFrame_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->tempSensFrame_1_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->tempSensFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->tempSensFrame_3->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->tempMUBPFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->ventRotStateFrame_1->setStyleSheet("background-color: rgb(177, 177, 177)");
    ui->ventRotStateFrame_2->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->powerSupplyStateFrame->setStyleSheet("background-color: rgb(177, 177, 177)");

    ui->psTempDataLabel_1->clear();
    ui->psTempDataLabel_2->clear();
    ui->psTempDataLabel_3->clear();

    ui->psVoltDataLabel_1->clear();
    ui->psVoltDataLabel_2->clear();
    ui->psVoltDataLabel_3->clear();

    ui->psCurDataLabel_1->clear();
    ui->psCurDataLabel_2->clear();
    ui->psCurDataLabel_3->clear();

    ui->ventRotLabel_1->clear();
    ui->ventRotLabel_2->clear();

    ui->tempMUBP->clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}




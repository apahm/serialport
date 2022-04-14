#ifndef QPRANGEMODULE_H
#define QPRANGEMODULE_H

#include <QObject>
#include "SerialPort.h"

#pragma pack(push, 1)

#define REQ_TEMP_DS18B20 0x18
#define ANS_TEMP_DS18B20 0x19

#define REQ_INIT_DS18B20 0x20
#define ANS_INIT_DS18B20 0x21

#define REQ_ADC_BUFFER 0x22
#define ANS_ADC_BUFFER 0x23

#define REQ_INIT_ADC_SOUND 0x24
#define ANS_INIT_ADC_SOUND 0x25

#define REQ_INIT_ADC_LIGHT 0x26
#define ANS_INIT_ADC_LIGHT 0x27

#define REQ_INIT_HX711 0x30
#define ANS_INIT_HX711 0x31

#define REQ_WEIGHT_HX711 0x32
#define ANS_WEIGHT_HX711 0x33

class QPRangeDevice: public QObject
{
    Q_OBJECT

protected:
    CBaseSerialPort* dev;

    unsigned int FTimeout;

    unsigned short makeCRC16(unsigned char * data, unsigned short len, bool check = false) const;

    int extendedSendRecvData(unsigned char *src, int src_len, unsigned char *dst, int dst_len, uint32_t timeout);

public:
    
    int sendData(void* data, int length = -1);
    int recvData(void* data, int length);
    
    explicit QPRangeDevice(QObject *parent = 0);
    ~QPRangeDevice();

    bool isActive() const { if(dev) return dev->isActive();}

    void setTimeout(unsigned int to);

   
    int reqInitDs18b20();
    int reqInitAdcSound();
    int reqInitAdcLight();
    int reqInitHX711();

    int reqAdcBuffer128(int16_t* buffer);
    int reqTempDs18b20(std::vector<double>& temp);
    int reqWeightHX711(uint32_t& weight);


    double seachMaxFreq(int16_t* buffer, uint8_t buffer_len);


signals:
    void activated();
    void deactivated();
    void received(const QString &);
    void received(int);
    void transmitted(const QString &);
    void transmitted(int);
    void crashed(int);
    void crashed(const QString &);
    void memoryProcess(int process);

public slots:
    bool activate();
    bool activate(const QString &a);
    bool deactivate();

};

#endif // QPRANGEMODULE_H

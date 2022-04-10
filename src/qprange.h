#ifndef QPRANGEMODULE_H
#define QPRANGEMODULE_H

#include <QObject>
#include "SerialPort.h"

#pragma pack(push, 1)

#define REQ_TEMP_DS18B20 0x18
#define ANS_TEMP_DS18B20 0x19

#define REQ_SEACH_DS18B20 0x20
#define ANS_SEACH_DS18B20 0x21

#define REQ_ADC_BUFFER 0x22
#define ANS_ADC_BUFFER 0x23



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

    int reqTempDs18b20(std::vector<double>& temp);
    
    int reqAdcBuffer128(int16_t* buffer);

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

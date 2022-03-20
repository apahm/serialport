#ifndef QPRANGEMODULE_H
#define QPRANGEMODULE_H

#include <QObject>
#include "SerialPort.h"

// Protocol
#define PRANGE_PRINT_LENGTH     16

#define PRANGE_BROADCAST        0xFF
#define PRANGE_ADDR_CONTROL     0xAC
#define PRANGE_ADDR_PREAMP      0x00
#define PRANGE_ADDR_VERT        0x00
#define PRANGE_ADDR_HORZ        0x10
#define PRANGE_ADDR_PPM_BC      0x0F

#define PRANGE_PRE_THECH        0xB5
#define PRANGE_CMD_SETWORKMODE      0x00
#define PRANGE_CMD_SETTECHSTATE     0x01
#define PRANGE_CMD_GETTECHSTATE     0x02
#define PRANGE_CMD_SETFUNC          0x03
#define PRANGE_CMD_GETFUNC          0x04
#define PRANGE_CMD_SETDEFAULT       0x05
#define PRANGE_CMD_SETSTROBES       0x06
#define PRANGE_CMD_MEMWRITE         0x07
#define PRANGE_CMD_MEMREAD          0x08
#define PRANGE_CMD_GETVERSION       0x09
#define PRANGE_CMD_GETSTATE         0x0A
#define PRANGE_CMD_PHAWRITE         0x0B
#define PRANGE_CMD_PHAREAD          0x0C

// Work modes
#define PRANGE_MODE_RESET       0x00
#define PRANGE_MODE_NORMAL      0x01
#define PRANGE_MODE_BLOADER     0x02

//-----------------------------------------------------------------------------
#pragma pack(push, 1)

typedef struct {
    unsigned char applyAdj; // Always 0

    unsigned char digitalPhaseTX;
    unsigned short analogPhaseTX;
    unsigned char attenuatorTX;
    unsigned char enabledTX;

    unsigned char digitalPhaseRX;
    unsigned short analogPhaseRX;
    unsigned char attenuatorRX;
    unsigned char enabledRX;
} PRANGE_FUNCCTRL;

//-----------------------------------------------------------------------------
typedef struct {
    unsigned char applyAdj;
    unsigned char phaseTX;
    unsigned char phaseRX;
} PRANGE_PHASES;

//-----------------------------------------------------------------------------
typedef struct {
    unsigned char mode;
    unsigned char pol;
    unsigned int lengthTX;
    unsigned int pauseTX;
    unsigned int offsetRX;
    unsigned int lengthRX;
    unsigned char psp;
} PRANGE_STROBES;

typedef struct {
    unsigned short temper;
    unsigned short powerTX;
    unsigned short powerRX;
} PRANGE_STATE;

//-----------------------------------------------------------------------------
typedef struct {
    unsigned char workMode;
    union {
        unsigned char techState;
        struct {
            unsigned char secPower:1;
            unsigned char heat:1;
            unsigned char rsvTechState:6;
        };
    };
} PRANGE_STATUS;

//-----------------------------------------------------------------------------
typedef struct {
    unsigned char  PPM_Count;

    unsigned char  BPU_Att_tx;
    unsigned char  BPU_Att_rx;
    unsigned short BPU_Int_Pow_Min;
    unsigned short BPU_Int_Pow_Max;
    unsigned short BPU_Ext_Pow_Min;
    unsigned short BPU_Ext_Pow_Max;

    unsigned char  PPM1_Att_tx;
    unsigned char  PPM1_Att_rx;
    unsigned short PPM1_Int_Pow_Min;
    unsigned short PPM1_Int_Pow_Max;
    unsigned short PPM1_Ext_Pow_Min;
    unsigned short PPM1_Ext_Pow_Max;

    unsigned char  PPM2_Att_tx;
    unsigned char  PPM2_Att_rx;
    unsigned short PPM2_Int_Pow_Min;
    unsigned short PPM2_Int_Pow_Max;
    unsigned short PPM2_Ext_Pow_Min;
    unsigned short PPM2_Ext_Pow_Max;

    unsigned short PPM1_Rx_Pwr_Min;
    unsigned short PPM1_Rx_Pwr_Max;
    unsigned short PPM2_Rx_Pwr_Min;
    unsigned short PPM2_Rx_Pwr_Max;

} PRANGE_MEMSETTINGS;

//-----------------------------------------------------------------------------
typedef struct {
/*0x00*/unsigned short second;

/*0x02*/unsigned short bpu_temp;
/*0x04*/unsigned short bpu_power_tx;

/*0x06*/unsigned short ppm1_temp;
/*0x08*/unsigned short ppm1_power_tx;
/*0x0A*/unsigned short ppm1_power_rx;

/*0x0C*/unsigned short ppm2_temp;
/*0x0E*/unsigned short ppm2_power_tx;
/*0x10*/unsigned short ppm2_power_rx;

/*0x12*/unsigned short dummy1;
/*0x14*/unsigned int dummy2;
/*0x18*/unsigned int dummy3;
/*0x1C*/unsigned int dummy4;
/*0x20*/
} PRANGE_STATISTIC;

#pragma pack (pop)

//-----------------------------------------------------------------------------
const struct {
    unsigned char index;
    unsigned char address;
    std::wstring description;
} devDesc[] = {
    {0, 0xFE, L""},
    {1, 0xAC, L"БУ"},
    {2, 0x00, L"БПУ ВП"},
    {3, 0x10, L"БПУ ГП"}
};

//-----------------------------------------------------------------------------
class QPRangeDevice: public QObject
{
    Q_OBJECT

protected:
    CBaseSerialPort* dev;

    bool FSimulation;
    unsigned int FTimeout;

    unsigned short makeCRC16(unsigned char * data, unsigned short len, bool check = false) const;
    //int sendData(void * data, int length = -1);
    //int recvData(void * data, int length);

    int sendSim(unsigned char * data, int length);
    int recvSim(unsigned char * data, int length);

    int extendedSendRecvData(unsigned char *src, int src_len, unsigned char *dst, int dst_len);

public:
    
    int sendData(void* data, int length = -1);
    int recvData(void* data, int length);
    explicit QPRangeDevice(QObject *parent = 0);
    ~QPRangeDevice();

    bool isActive() const { if(dev) return dev->isActive(); return FSimulation; }
    void setTimeout(unsigned int to);

    bool setWorkMode(unsigned char mode);
    bool setTechState(unsigned char addr, bool secpow, bool heat);
    bool getTechState(unsigned char addr, bool *secpow, bool *heat);

    bool setFuncState(unsigned char addr, const PRANGE_FUNCCTRL &state);
    bool getFuncState(unsigned char addr,       PRANGE_FUNCCTRL &state);

    bool setDefault(unsigned char addr);

    bool setStrobes(const PRANGE_STROBES &strobes);

    bool writeMemoryPortion(unsigned char addr, unsigned int offset, unsigned char length, const void * data);
    bool readMemoryPortion(unsigned char addr, unsigned int offset, unsigned char length, void * data);

    bool getVersion(unsigned char addr, unsigned char * minor, unsigned char * major);

    bool getState(unsigned char addr, PRANGE_STATE &state);

    bool writePhases(unsigned char addr, const PRANGE_PHASES &phases);
    bool readPhases(unsigned char addr, PRANGE_PHASES &phases);

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

//-----------------------------------------------------------------------------

#endif // QPRANGEMODULE_H

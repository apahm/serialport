#include "qprange.h"

QString getVersion(const unsigned char val[]) {
    return QString("%1.%2.%3.%4").arg(val[0]).arg(val[1]).arg(val[2]).arg(val[3]);
}

//-----------------------------------------------------------------------------
QPRangeDevice::QPRangeDevice(QObject *parent) : QObject(parent)
{
    dev = NULL;
    FSimulation = false;
    FTimeout = 200;
}

//-----------------------------------------------------------------------------
QPRangeDevice::~QPRangeDevice()
{
    if(dev) {
        delete dev;
        dev = NULL;
    }
}

//-----------------------------------------------------------------------------
unsigned short QPRangeDevice::makeCRC16(unsigned char * data, unsigned short len, bool check) const
{
    const unsigned short poly = 0x1021;
    unsigned short crc16 = 0x1D0F;
    unsigned int t_len;
    if(!check) { // Transmit
        t_len = len - 2;
        data[len - 2] = 0;
        data[len - 1] = 0;
    } else { // Receive
        t_len = len;
        // unsigned char tmp = data[len - 2];
        // data[len - 2] = data[len - 1];
        // data[len - 1] = tmp;
    }

    for(unsigned int j = 0; j < t_len; j++) {
        crc16 ^= data[j] << 8;
        for(int i = 0; i < 8; i++)
            crc16 = crc16 & 0x8000 ? (crc16 << 1) ^ poly : (crc16 << 1);
    }

    // crc16 = ((crc16 & 0xFF) << 8) | ((crc16 & 0xFF00) >> 8);

    if(!check) { // Transmit
        data[len - 1] = (crc16 >> 0) & 0xFF;
        data[len - 2] = (crc16 >> 8) & 0xFF;
    }

    return crc16;
}

//-----------------------------------------------------------------------------
void QPRangeDevice::setTimeout(unsigned int to)
{
    FTimeout = to;
}

//-----------------------------------------------------------------------------
int QPRangeDevice::sendData(void * data, int length)
{
    int ans = -1;

    unsigned char * uc_data = (unsigned char *) data;
    if(length == -1) length = uc_data[1];

    makeCRC16(uc_data, length);

    if(FSimulation) return sendSim(uc_data, length);

    if(dev) {
        QString tt = "DEVICE << ";
        for(int i = 0; i < length; i++) {
#if (PRANGE_PRINT_LENGTH != 0)
            if((i % PRANGE_PRINT_LENGTH) == 0) tt += "\r\n";
#endif
            tt += QString::number(uc_data[i], 16).rightJustified(2, '0').toUpper() + " ";
        }
        emit transmitted(length);
        emit transmitted(tt.trimmed());
        ans = dev->writeData(data, length);

        if(ans < 0) {
            int e_code = dev->getLastError();
            QString e_text = dev->getErrorStringA(e_code);
            emit crashed(e_code);
            emit crashed(e_text.trimmed());
        }
    } else {
#if defined (_WIN32)
        int e_code = ERROR_INVALID_HANDLE;
        QString e_text = QString::fromWCharArray(getWindowsErrorW(e_code));
#elif defined (__linux__)
        int e_code = ENXIO;
        QString e_text = getLinuxError(e_code);
#endif
        emit crashed(e_code);
        emit crashed(e_text.trimmed());
    }

    return ans;
}

//-----------------------------------------------------------------------------
int QPRangeDevice::recvData(void * data, int length)
{
    int ans = -1;
    unsigned char * uc_data = (unsigned char *) data;

    if(FSimulation) return recvSim(uc_data, length);

    if(dev) {
        ans = dev->readDataTimeout(uc_data, length, FTimeout);

        if(ans < 0) {
            int e_code = dev->getLastError();
            QString e_text = dev->getErrorStringA(e_code);
            emit crashed(e_code);
            emit crashed(e_text.trimmed());
        } else {
            if(ans != uc_data[1]) {
                emit crashed(-1);
                emit crashed("Wrong length!");
            }
            if(makeCRC16(uc_data, ans, true)) {
                emit crashed(-2);
                emit crashed("Wrong checksumm!");
            }
            QString tt = "DEVICE >> ";
            for(int i = 0; i < ans; i++) {
#if (PRANGE_PRINT_LENGTH != 0)
                if((i % PRANGE_PRINT_LENGTH) == 0) tt += "\r\n";
#endif
                tt += QString::number(uc_data[i], 16).rightJustified(2, '0').toUpper() + " ";
            }
            emit received(ans);
            emit received(tt.trimmed());
        }
    } else {
#if defined (_WIN32)
        int e_code = ERROR_INVALID_HANDLE;
        QString e_text = getWindowsErrorA(e_code);
#elif defined (__linux__)
        int e_code = ENXIO;
        QString e_text = getLinuxError(e_code);
#endif
        emit crashed(e_code);
        emit crashed(e_text.trimmed());
    }

    return ans;
}

//-----------------------------------------------------------------------------
int QPRangeDevice::extendedSendRecvData(unsigned char * src, int src_len, unsigned char * dst, int dst_len)
{
    if(sendData(src, src_len) < 0) return -1;
    int ans = recvData(dst, dst_len);
    if(ans < 0) return -1;
    else if(ans == 0) { // No first answer data immidately
        Sleep(FTimeout);
        ans = recvData(dst, dst_len);
        if(ans < 0) return -1;
        else if(ans == 0) { // No first answer data timeout
            src[0] = (src[0] & 0xF0) | 0x09;
            if(sendData(src, src_len) < 0) return -1;
            ans = recvData(dst, dst_len);
            if(ans < 0) return -1;
            else if(ans == 0) { // No second answer data immidately
                Sleep(250);
                ans = recvData(dst, dst_len);
                if(ans < 0) return -1;
                else if(ans == 0) return -1; // // No second answer data timeout
            }
        }
    }

    if(ans > dst_len) { emit crashed("Extra bytes received!"); }
    if(ans < dst_len) { emit crashed("Insufficient bytes received!"); return -2; }
    if((unsigned int) ans != dst[1]) { emit crashed("Wrong number bytes received!"); return -3; }
    if(makeCRC16(dst, ans, true) != 0) { emit crashed("Wrong checksumm received!"); return -4; }

    return ans;
}

//-----------------------------------------------------------------------------
int QPRangeDevice::sendSim(unsigned char * data, int length)
{
    QString tt = "DEVICE << ";
    for(int i = 0; i < length; i++) {
#if (PRANGE_PRINT_LENGTH != 0)
        if((i % PRANGE_PRINT_LENGTH) == 0) tt += "\r\n";
#endif
        tt += QString::number(data[i], 16).rightJustified(2, '0').toUpper() + " ";
    }
    emit transmitted(length);
    emit transmitted(tt.trimmed());
    return length;
}

//-----------------------------------------------------------------------------
int QPRangeDevice::recvSim(unsigned char * data, int length)
{
    QString tt = "DEVICE >> ";

    data[0] = 0xFF;
    data[1] = length;
    for(int i = 2; i < length; i++) {
        data[i] = rand();
    }
    makeCRC16(data, length);

    for(int i = 0; i < length; i++) {
#if (PRANGE_PRINT_LENGTH != 0)
        if((i % PRANGE_PRINT_LENGTH) == 0) tt += "\r\n";
#endif
        tt += QString::number(data[i], 16).rightJustified(2, '0').toUpper() + " ";
    }
    emit received(length);
    emit received(tt.trimmed());
    return length;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::activate()
{
    return activate("");
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::activate(const QString &a)
{
    deactivate();

    if(a.toUpper() == "SIMULATION") {
        FSimulation = true;
        return true;
    }

    dev = new CSerialPort();
    if(dev->ConnectA(a.toLatin1().data())) {
        dev->setBaudrate(9600);
        emit activated();
        return true;
    }
    
    delete dev;  dev = NULL;

#if defined (_WIN32)
    emit crashed(ERROR_FILE_NOT_FOUND);
    emit crashed(QString(getWindowsErrorA(ERROR_FILE_NOT_FOUND)).trimmed());
#elif defined (__linux__)
    emit crashed(ENOENT);
    emit crashed(QString(getLinuxError(ENOENT)).trimmed();
#endif

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::deactivate()
{
    bool ans = false;
    if(FSimulation) {
        ans = true;
        FSimulation = false;
    }

    if(dev) {
        if(dev->isActive()) {
            dev->Disconnect();
            emit deactivated();
            ans = true;
        }
        delete dev;
        dev = NULL;
    }
    return ans;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::setWorkMode(unsigned char mode)
{
    const unsigned char cnt_out = 7;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_SETWORKMODE;
    cmd[3] = PRANGE_BROADCAST;
    cmd[4] = mode;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::setTechState(unsigned char addr, bool secpow, bool heat)
{
    const unsigned char cnt_out = 7;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;
    if((addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_SETTECHSTATE;
    cmd[3] = addr;
    cmd[4] = (secpow ? 0x01 : 0) | (heat ? 0x02 : 0);

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::getTechState(unsigned char addr, bool *secpow, bool *heat)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 7;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;
    if((addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_HORZ))) return false;

    if((addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_GETTECHSTATE;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?
        if(secpow)  *secpow  = ans[4] & 0x01;
        if(heat)    *heat    = ans[4] & 0x02;

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::setFuncState(unsigned char addr, const PRANGE_FUNCCTRL &state)
{
    const unsigned char cnt_out = 17;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_SETFUNC;
    cmd[3] = addr;
    memcpy(cmd + 4, &state, sizeof(PRANGE_FUNCCTRL));

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::getFuncState(unsigned char addr, PRANGE_FUNCCTRL &state)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 17;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_GETFUNC;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?
        memcpy(&state, ans + 4, sizeof(PRANGE_FUNCCTRL));

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::setDefault(unsigned char addr)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_SETDEFAULT;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::setStrobes(const PRANGE_STROBES &strobes)
{
    const unsigned char cnt_out = 4 + 2 + sizeof(PRANGE_STROBES);
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_SETSTROBES;
    cmd[3] = PRANGE_BROADCAST; // PRANGE_ADDR_CONTROL;

    memcpy(cmd + 4, &strobes, sizeof(PRANGE_STROBES));

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::writeMemoryPortion(unsigned char addr, unsigned int offset, unsigned char length, const void * data)
{
    const unsigned char cnt_out = 138;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_MEMWRITE;
    cmd[3] = addr;

    cmd[4] = offset & 0xFF;
    cmd[5] = (offset >> 8) & 0xFF;
    cmd[6] = (offset >> 16) & 0xFF;
    cmd[7] = length;
    memcpy(cmd + 8, data, length);
    memset(cmd + 8 + length, 0, 128 - length);

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::readMemoryPortion(unsigned char addr, unsigned int offset, unsigned char length, void * data)
{
    const unsigned char cnt_out = 10;
    const unsigned char cnt_in  = 138;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if((addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_MEMREAD;
    cmd[3] = addr;

    cmd[4] = offset & 0xFF;
    cmd[5] = (offset >> 8) & 0xFF;
    cmd[6] = (offset >> 16) & 0xFF;
    cmd[7] = length;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        memcpy(data, ans + 8, length);

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::getVersion(unsigned char addr, unsigned char * minor, unsigned char * major)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if((addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_GETVERSION;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        if(minor) *minor = ans[4];
        if(major) *major = ans[5];

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::getState(unsigned char addr, PRANGE_STATE &state)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 12;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;
    if((addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_GETSTATE;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?
        memcpy(&state, ans + 4, sizeof(PRANGE_STATE));

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::writePhases(unsigned char addr, const PRANGE_PHASES &phases)
{
    const unsigned char cnt_out = 9;
    const unsigned char cnt_in  = 8;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;
    if((addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PREAMP | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_PHAWRITE;
    cmd[3] = addr;
    memcpy(cmd + 4, &phases, sizeof(PRANGE_PHASES));

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool QPRangeDevice::readPhases(unsigned char addr, PRANGE_PHASES &phases)
{
    const unsigned char cnt_out = 6;
    const unsigned char cnt_in  = 9;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    if(addr == PRANGE_BROADCAST) return false;
    if(addr == PRANGE_ADDR_CONTROL) return false;
    if((addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_VERT)) ||
       (addr == (PRANGE_ADDR_PPM_BC | PRANGE_ADDR_HORZ))) return false;

    cmd[0] = PRANGE_PRE_THECH;
    cmd[1] = cnt_out;
    cmd[2] = PRANGE_CMD_PHAREAD;
    cmd[3] = addr;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in) > 0) {
        // Save data?
        // Receive ok?
        memcpy(&phases, ans + 4, sizeof(PRANGE_PHASES));

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------


#include "qprange.h"
#include <iostream>
#include <fftw3.h>
#include <algorithm>
#include <vector>
#include <cmath>

QPRangeDevice::QPRangeDevice(QObject *parent) : QObject(parent)
{
    dev = NULL;
    FTimeout = 200;
}

QPRangeDevice::~QPRangeDevice()
{
    if(dev) {
        delete dev;
        dev = NULL;
    }
}

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

void QPRangeDevice::setTimeout(unsigned int to)
{
    FTimeout = to;
}

int QPRangeDevice::sendData(void * data, int length)
{
    int ans = -1;

    unsigned char * uc_data = (unsigned char *) data;
    if(length == -1) length = uc_data[1];

    makeCRC16(uc_data, length);

    if(dev) 
    {
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
    } 
    else 
    {
        int e_code = ERROR_INVALID_HANDLE;
        QString e_text = QString::fromWCharArray(getWindowsErrorW(e_code));
        emit crashed(e_code);
        emit crashed(e_text.trimmed());
    }

    return ans;
}

int QPRangeDevice::recvData(void * data, int length)
{
    int ans = -1;
    unsigned char * uc_data = (unsigned char *) data;

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

int QPRangeDevice::extendedSendRecvData(unsigned char * src, int src_len, unsigned char * dst, int dst_len, uint32_t timeout)
{
    if(sendData(src, src_len) < 0) return -1;
    Sleep(timeout);
    int ans = recvData(dst, dst_len);
    if(ans < 0) return -1;
    else if(ans == 0) 
    { 
        Sleep(FTimeout);
        ans = recvData(dst, dst_len);
        if(ans < 0) return -1;
        else if(ans == 0) 
        { 
            if(sendData(src, src_len) < 0) return -1;
            ans = recvData(dst, dst_len);
            if(ans < 0) return -1;
            else if(ans == 0) 
            { 
                Sleep(250);
                ans = recvData(dst, dst_len);
                if(ans < 0) return -1;
                else if(ans == 0) return -1; // // No second answer data timeout
            }
        }
    }

    if(ans > dst_len) 
    { 
        std::cout << "Extra bytes received!" << std::endl;
    }
    if(ans < dst_len) 
    { 
        std::cout << "Insufficient bytes received!" << std::endl;
        return -2; 
    }
    if((unsigned int) ans != dst[1]) 
    { 
        std::cout << "Wrong number bytes received!" << std::endl;
        return -3; 
    }
    if(makeCRC16(dst, ans, true) != 0) 
    { 
        std::cout << "Wrong checksumm received!" << std::endl;
        return -4; 
    }

    return ans;
}

bool QPRangeDevice::activate()
{
    return activate("");
}

bool QPRangeDevice::activate(const QString &a)
{
    deactivate();


    dev = new CSerialPort();
    if(dev->ConnectA(a.toLatin1().data())) {
        dev->setBaudrate(9600);
        emit activated();
        return true;
    }
    
    delete dev;  dev = NULL;

    return false;
}

bool QPRangeDevice::deactivate()
{
    bool ans = false;

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

/*
 *   Функция запроса температуры с первого и второго датчика
 *   
 *   nubmer_ds18b20 = 0 - для первого датчика
 *   nubmer_ds18b20 = 1 - для второго датчика
*/

double QPRangeDevice::reqTempDs18b20(uint8_t nubmer_ds18b20)
{

    uint32_t timeout = 1000; // 1000ms

    double tempOneDs18b20 = 0.0;
    double tempTwoDs18b20 = 0.0;

    const unsigned char cnt_out = 5;
    const unsigned char cnt_in  = 9;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = cnt_out;
    cmd[2] = REQ_TEMP_DS18B20;

    if(extendedSendRecvData(cmd, -1, ans, cnt_in, timeout) > 0) {
        // Save data?
        // Receive ok?
        
        if (ans[2] == ANS_TEMP_DS18B20)
        {
            //std::cout << "0x" << std::hex << (int)ans[3] << std::endl;
            //std::cout << "0x" << std::hex << (int)ans[4] << std::endl;
            tempOneDs18b20 = ((ans[3] << 8) + ans[4]) * 0.0625;
            //std::cout << "T0 = " << tempOneDs18b20 << std::endl;

            //std::cout << "0x" << std::hex << (int)ans[5] << std::endl;
            //std::cout << "0x" << std::hex << (int)ans[6] << std::endl;
            tempTwoDs18b20 = ((ans[5] << 8) + ans[6]) * 0.0625;
            //std::cout << "T1 = " << tempTwoDs18b20 << std::endl;
            
            if (!nubmer_ds18b20)
                return tempOneDs18b20;
            else
                return tempTwoDs18b20;
        }
    }

    return -1;
}

int QPRangeDevice::reqAdcBuffer128(int16_t* buffer)
{
    if (buffer == nullptr)
        return -2;
    
    uint32_t timeout = 1000; // 1000ms

    const unsigned char cnt_out = 6;
    const unsigned char cnt_in = 261;  // 256 + 5

    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = cnt_out;
    cmd[2] = REQ_ADC_BUFFER;
    cmd[3] = 128; // Numbers of sample

    if (extendedSendRecvData(cmd, -1, ans, cnt_in, timeout) > 0) {
        // Save data?
        // Receive ok?

        if (ans[2] == ANS_ADC_BUFFER)
        {
            memcpy(buffer, &ans[3], 256);
            return 0;
        }
    }

    return -1;
}

double QPRangeDevice::seachMaxFreq(int16_t* buffer, uint8_t buffer_len)
{
    
    if (buffer_len == 0)
        return -1;
    if (buffer == nullptr)
        return -2;

    fftw_complex* in = new fftw_complex[buffer_len]; 
    fftw_complex* out = new fftw_complex[buffer_len];

    if (in == nullptr)
        return -2;
    if (out == nullptr)
        return -2;

    fftw_plan p;

    for (size_t i = 0; i < buffer_len; i++)
    {
        in[i][0] = (double)buffer[i];
        in[i][1] = 0;
    }

    p = fftw_plan_dft_1d(buffer_len, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    std::vector<double> mag;

    for (size_t i = 0; i < buffer_len; i++)
    {
        mag.push_back(
            std::sqrt(
                std::pow(out[i][0],2) + std::pow(out[i][1],2)
            )
        );
    }

    std::vector<double>::iterator result;

    result = std::max_element(mag.begin(), mag.end());

    fftw_destroy_plan(p);

    delete[] in;
    delete[] out;

    return (9600.0 / (double)buffer_len) * (double)std::distance(mag.begin(), result);
}








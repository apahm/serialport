#include <iostream>
#include "qprange.h"
#include <QString>
#include <fftw3.h>
#include <vector>
#include <fstream>

int main()
{
    QPRangeDevice prange;
    bool dev_connected;

    QString port("COM3");

    dev_connected = prange.activate(port);

    uint32_t prange_timeout = 250;
    prange.setTimeout(prange_timeout);
    

    if(dev_connected) {
        std::cout << "Connect to port: " << std::endl;
    } else {
        std::cout << "Error connect to port: " << std::endl;
    }
    
    std::cout << "T0 = " << prange.reqTempDs18b20(0) << std::endl;
    std::cout << "T1 = " << prange.reqTempDs18b20(1) << std::endl;

    uint16_t buffer_len = 256;
    std::vector<int16_t> v(buffer_len,0);

    int ret = prange.reqAdcBuffer128(v.data());

    std::ofstream sample("sample.txt");

    if (sample.is_open())
    {
        for (size_t i = 0; i < v.size(); i++)
        {
            sample << v.at(i) << std::endl;
        }
    }

    //double freq = prange.seachMaxFreq(v.data(), buffer_len);

    //std::cout << freq << std::endl;

    return 0;
}

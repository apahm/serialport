#include <iostream>
#include "qprange.h"
#include <QString>

int test_min_length(QPRangeDevice& prange)
{
    const unsigned char cnt_out = 12;
    const unsigned char cnt_in = 20;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = 4;
    cmd[2] = PRANGE_CMD_SETWORKMODE;
    cmd[3] = PRANGE_BROADCAST;
    cmd[4] = 0x56;
    cmd[5] = 0x96;
    cmd[6] = 0xFF;
    cmd[7] = 0x01;
    cmd[8] = 0x02;
    cmd[9] = 0x89;


    int ret = prange.sendData(cmd, cnt_out);

    ret = prange.recvData(ans, cnt_in);

    return ret;
}

int test_min_normal_length(QPRangeDevice &prange)
{
    const unsigned char cnt_out = 12;
    const unsigned char cnt_in = 20;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = 10;
    cmd[2] = PRANGE_CMD_SETWORKMODE;
    cmd[3] = PRANGE_BROADCAST;
    cmd[4] = 0x56;
    cmd[5] = 0x96;
    cmd[6] = 0xFF;
    cmd[7] = 0x01;
    cmd[8] = 0x02;
    cmd[9] = 0x89;


    int ret = prange.sendData(cmd, cnt_out);

    ret = prange.recvData(ans, cnt_in);

    return ret;
}

int test_normal_length(QPRangeDevice& prange)
{
    const unsigned char cnt_out = 12;
    const unsigned char cnt_in = 20;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = 12;
    cmd[2] = PRANGE_CMD_SETWORKMODE;
    cmd[3] = PRANGE_BROADCAST;
    cmd[4] = 0x56;
    cmd[5] = 0x96;
    cmd[6] = 0xFF;
    cmd[7] = 0x01;
    cmd[8] = 0x02;
    cmd[9] = 0x89;


    int ret = prange.sendData(cmd, cnt_out);

    ret = prange.recvData(ans, cnt_in);

    return ret;
}

int test_large_normal_length(QPRangeDevice& prange)
{
    const unsigned char cnt_out = 12;
    const unsigned char cnt_in = 20;
    unsigned char cmd[cnt_out];
    unsigned char ans[cnt_in];

    cmd[0] = 0x01;
    cmd[1] = 18;
    cmd[2] = PRANGE_CMD_SETWORKMODE;
    cmd[3] = PRANGE_BROADCAST;
    cmd[4] = 0x56;
    cmd[5] = 0x96;
    cmd[6] = 0xFF;
    cmd[7] = 0x01;
    cmd[8] = 0x02;
    cmd[9] = 0x89;


    int ret = prange.sendData(cmd, cnt_out);

    ret = prange.recvData(ans, cnt_in);

    return ret;
}

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

    for (size_t i = 0; i < 25; i++)
    {

        std::cout << i << std::endl;

        int ret = test_min_normal_length(prange);

        if (ret == 0)
        {
            std::cout << "Test min normal length successfully " << std::endl;
        }

        ret = test_normal_length(prange);

        if (ret == 12)
        {
            std::cout << "Test normal length successfully " << std::endl;
        }

        ret = test_large_normal_length(prange);

        if (ret == 0)
        {
            std::cout << "Test large length successfully " << std::endl;
        }

        ret = test_min_length(prange);

        if (ret == 0)
        {
            std::cout << "Test minumum length successfully " << std::endl;
        }

        std::cout << std::endl;
    }


    return 0;
}

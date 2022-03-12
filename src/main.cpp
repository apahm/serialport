#include <iostream>
#include "qprange.h"

int main()
{
    QPRangeDevice prange;
    bool dev_connected;

    dev_connected = prange.activate();

    uint32_t prange_timeout = 250;
    prange.setTimeout(prange_timeout);

    if(dev_connected) {
        std::cout << L"Подключено" << std::endl;
    } else {
        std::cout << "Ошибка подключения" << std::endl;
    }
    
    return 0;
}

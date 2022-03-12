//***************************************************************************
//** SerialPort.cpp                                                        **
//** Automated COM/TTY component                                           **
//** Based on Win API, IOCNTL                                              **
//**   OS: Windows / Linux                                                 **
//**   Version: 2.0 CrossLib                                               **
//**   Autor: Pavel Brain                                                  **
//**   Last modified: 27-04-2017                                           **
//***************************************************************************

#include <SerialPort.h>

//-----------------------------------------------------------------------------
//************************ Start CBaseSerialPort ******************************
//-----------------------------------------------------------------------------
CBaseSerialPort::CBaseSerialPort(): CBaseDevice("CBaseSerialPort")
{
#if defined (_WIN32)
	handle = INVALID_HANDLE_VALUE;
#elif defined (__linux__)
	handle = -1;
#endif

	memset(&FParameters, 0, sizeof(SERIAL_PORT_PARAMETERS));
	FParameters.uSize = sizeof(SERIAL_PORT_PARAMETERS);
	FParameters.uBaudRate = 115200;
	//FParameters.ucParity = 0;
	FParameters.ucDataBits = 8;
	//FParameters.ucStopBits = 0;
	//FParameters.ucRTSCtrl = 0;
	//FParameters.ucDTRCtrl = 0;
	//FParameters.ucDriver = 0;

#if defined(_WIN32)
	FCt.ReadIntervalTimeout = 15; //10000 / FParameters.uBaudRate + 1;
	FCt.ReadTotalTimeoutMultiplier = 0; // 10000 / FParameters.uBaudRate + 1;
	FCt.ReadTotalTimeoutConstant = 150;
	FCt.WriteTotalTimeoutMultiplier = 0;
	FCt.WriteTotalTimeoutConstant = 0;
#endif
}

//-----------------------------------------------------------------------------
CBaseSerialPort::~CBaseSerialPort()
{
	Disconnect();
}

//---- CBaseSerialPort static section -----------------------------------------
//-----------------------------------------------------------------------------
/* inline */ unsigned int CBaseSerialPort::IntToBaud(unsigned int baudrate) const
{
#if defined (_WIN32)
	return baudrate;
#elif defined (__linux__)
	switch(baudrate)
	{
		case 50:        return B50;
		case 75:        return B75;
		case 110:       return B110;
		case 134:       return B134;
		case 150:       return B150;
		case 200:       return B200;
		case 300:       return B300;
		case 600:       return B600;
		case 1200:      return B1200;
		case 1800:      return B1800;
		case 2400:      return B2400;
        case 4800:      return B4800;
		case 9600:      return B9600;
		case 19200:     return B19200;
		case 38400:     return B38400;
		case 57600:     return B57600;
		case 115200:    return B115200;
        case 921600:    return B921600;
	}
	return 0;
#else
#	error CBaseSerialPort::IntToBaud not supported
#endif
}

//-----------------------------------------------------------------------------
/* virtual */ const char * CBaseSerialPort::getErrorStringA(int e_code) const
{
#if defined (_WIN32)
	return getWindowsErrorA(e_code);
#elif defined (__linux__)
	return getLinuxError(e_code);
#else
#	error CBaseSerialPort::getErrorStringA not supported
#endif
}

//-----------------------------------------------------------------------------
/* virtual */ const wchar_t * CBaseSerialPort::getErrorStringW(int e_code) const
{
#if defined (_WIN32)
	return getWindowsErrorW(e_code);
#else
#	warning CBaseSerialPort::getErrorStringW not supported
    (void)e_code;
	return NULL;
#endif
}

//-----------------------------------------------------------------------------
/* static */ int CBaseSerialPort::findDevicesA(SVECTOR & dst)
{
#if defined (_WIN32)
	//Check windows version
	OSVERSIONINFOA osvinfo;
	memset(&osvinfo, 0, sizeof(osvinfo));
	osvinfo.dwOSVersionInfoSize = sizeof(osvinfo);
	GetVersionExA(&osvinfo);

	//Registry variables
	HKEY hKey = NULL;
	char lpValueName[64], lpPortName[64];
	DWORD dwError, cbValueName, cbPortName, dwIndex = 0;

	if(osvinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        //Old windows NT versions
		if(osvinfo.dwMajorVersion < 4) return 0;

        //Windows NT 4.0 version
        //Win 2K, XP, Vista, Seven and so on
        //Reg key = "HARDWARE\DEVICEMAP\SERIALCOMM"
		dwError = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey);
        if(dwError == ERROR_SUCCESS)
			do {
				cbValueName = 64;
				cbPortName = 64;
				dwError = RegEnumValueA(hKey, dwIndex++, lpValueName, &cbValueName, NULL, NULL, (LPBYTE)lpPortName, &cbPortName);
				if(dwError == ERROR_SUCCESS)
					dst.push_back(std::string(lpPortName/*, cbPortName*/));
			} while(dwError != ERROR_NO_MORE_ITEMS);

		if(hKey) RegCloseKey(hKey);
	}
#elif defined(__linux__)
//    #warning CBaseSerialPort::findPorts under construction
	DIR *dp;
	struct dirent *dent;
	dp = opendir("/dev");
	if(dp) {
		std::string cur_name;
        dent = readdir(dp);
        while(dent) {
            cur_name.assign(dent->d_name);
            if(cur_name.find("ttyS") != std::string::npos) dst.push_back(cur_name);
            if(cur_name.find("ttyUSB") != std::string::npos) dst.push_back(cur_name);
            dent = readdir(dp);
        }
        closedir(dp);
    }
#else
#	error CBaseSerialPort::findPortsA not supported
#endif
	std::sort(dst.begin(), dst.end());
	return dst.size();
}

////-----------------------------------------------------------------------------
/* static */ int CBaseSerialPort::findDevicesW(WSVECTOR & dst)
{
#if defined (_WIN32)
    //Check windows version
	OSVERSIONINFOW osvinfo;
    memset(&osvinfo, 0, sizeof(osvinfo));
    osvinfo.dwOSVersionInfoSize = sizeof(osvinfo);
	GetVersionExW(&osvinfo);

    //Registry variables
	HKEY hKey = NULL;
	wchar_t lpValueName[64], lpPortName[64];
	DWORD dwError, cbValueName, cbPortName, dwIndex = 0;

	if(osvinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		//Old windows NT versions
		if(osvinfo.dwMajorVersion < 4) return 0;

        //Windows NT 4.0 version
        //Win 2K, XP, Vista, Seven and so on
        //Reg key = "HARDWARE\DEVICEMAP\SERIALCOMM"
        dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey);
        if(dwError == ERROR_SUCCESS)
        do
        {
            cbValueName = 64;
            cbPortName = 64;
            dwError = RegEnumValueW(hKey, dwIndex++, lpValueName, &cbValueName, NULL, NULL, (LPBYTE)lpPortName, &cbPortName);
            if(dwError == ERROR_SUCCESS)
                dst.push_back(std::wstring(lpPortName, cbPortName));
        }
        while(dwError != ERROR_NO_MORE_ITEMS);

        if(hKey) RegCloseKey(hKey);
    }
#else
#	warning CBaseSerialPort::findPortsW not supported
#endif
	std::sort(dst.begin(), dst.end());
    return dst.size();
}

//-----------------------------------------------------------------------------
/* static */ int CBaseSerialPort::findDevicesVXIA(SVECTOR & dst)
{
    SVECTOR tmp;
    findDevicesA(tmp);
    for(SVECTOR::iterator it = tmp.begin(); it != tmp.end(); it++)
        dst.push_back(*it + "::INSTR");
    return dst.size();
}

//-----------------------------------------------------------------------------
/* static */ int CBaseSerialPort::findDevicesVXIW(WSVECTOR & dst)
{
    WSVECTOR tmp;
    findDevicesW(tmp);
    for(WSVECTOR::iterator it = tmp.begin(); it != tmp.end(); it++)
        dst.push_back(*it + L"::INSTR");
    return dst.size();
}

//------- CBaseSerialPort protected section -----------------------------------
//-----------------------------------------------------------------------------
/* Virtual */ bool CBaseSerialPort::DoError(int ECode)
{
#if defined (_WIN32)
	if((ECode == ERROR_ACCESS_DENIED) || (ECode == ERROR_BAD_COMMAND))
		Disconnect();
#elif defined (__linux__)
	if(ECode == 13) Disconnect();
#endif

	return CBaseDevice::DoError(ECode);
}

//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::setupPort(bool setup_dcb, bool setup_cto)
{
    FLastError = 0;

	if(FActive) {
#if defined (_WIN32)
		if(setup_dcb) {
			FDcb.DCBlength = sizeof(DCB);
			if(!GetCommState(handle, &FDcb)) return DoError(GetLastError());
			FDcb.DCBlength = sizeof(DCB);
			FDcb.BaudRate = FParameters.uBaudRate;
			FDcb.StopBits = FParameters.ucStopBits;
			FDcb.ByteSize = FParameters.ucDataBits;
			FDcb.Parity   = FParameters.ucParity;
			FDcb.fRtsControl = FParameters.ucRTSCtrl;
			FDcb.fDtrControl = FParameters.ucDTRCtrl;
			FDcb.fNull = false;
			if(!SetCommState(handle, &FDcb))   return DoError(GetLastError());
		}

		if(setup_cto) {
			//FCt.ReadIntervalTimeout = 10000 / FParameters.uBaudRate + 1;
			//FCt.ReadTotalTimeoutMultiplier = 10000 / FParameters.uBaudRate + 1;
			// if(!GetCommTimeouts(handle, &FCt)) return DoError(GetLastError());
			if(!SetCommTimeouts(handle, &FCt)) return DoError(GetLastError());
		}

#elif defined (__linux__)
		if(setup_dcb) {
            (void)setup_cto;

			if(tcgetattr(handle, &opts) == -1) return DoError(errno);

			cfsetispeed(&opts, IntToBaud(FParameters.uBaudRate));
			cfsetospeed(&opts, IntToBaud(FParameters.uBaudRate));

			opts.c_cflag &= ~CSIZE; // default 5 bits

			if(FParameters.ucDataBits == 6) opts.c_cflag |= CS6;
			if(FParameters.ucDataBits == 7) opts.c_cflag |= CS7;
			if(FParameters.ucDataBits == 8) opts.c_cflag |= CS8;

			if(FParameters.ucStopBits == 2)  opts.c_cflag |= CSTOPB; else opts.c_cflag &=~CSTOPB;

			if(FParameters.ucParity)     opts.c_cflag |= PARENB; else opts.c_cflag &=~PARENB;
			if(FParameters.ucParity & 1) opts.c_cflag |= PARODD; else opts.c_cflag &=~PARODD;

			opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			opts.c_cflag |= (CLOCAL | CREAD);

			if(tcsetattr(handle, TCSANOW, &opts) == -1) return DoError(errno);
		}
#else
#	error CBaseSerialPort::setupPort not supported
#endif
	}
	return true;
}

//------- CBaseSerialPort public section --------------------------------------
//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::isCorrectAddressA(const char * addr, void * dst) const
{
	if(!addr) return false;
	SERIAL_PORT_PARAMETERS * rcv_params = (SERIAL_PORT_PARAMETERS *) dst;
	if(rcv_params->uSize != sizeof(SERIAL_PORT_PARAMETERS)) rcv_params = NULL;

	int cnt = -1, port_num = -1, drv = 0, baud = 115200;
	char port[16], tmp_port[16];

	std::string con(addr);
    std::transform<std::string::iterator, std::string::iterator, int (*) (int)>(con.begin(), con.end(), con.begin(), ::toupper);

	// Check VXI Address standart

#if defined (_WIN32)
	if(cnt == -1) sscanf(con.c_str(), "%[^:]::DRV%i::%i::INSTR%n", tmp_port, &drv, &baud, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "%[^:]::%i::DRV%i::INSTR%n", tmp_port, &baud, &drv, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "%[^:]::DRV%i::INSTR%n", tmp_port, &drv, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "%[^:]::%i::INSTR%n", tmp_port, &baud, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "%[^:]::INSTR%n", tmp_port, &cnt);
#elif defined (__linux__)
	if(cnt == -1) sscanf(con.c_str(), "TTY%[^:]::DRV%i::%i::INSTR%n", tmp_port, &drv, &baud, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "TTY%[^:]::%i::DRV%i::INSTR%n", tmp_port, &baud, &drv, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "TTY%[^:]::DRV%i::INSTR%n", tmp_port, &drv, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "TTY%[^:]::%i::INSTR%n", tmp_port, &baud, &cnt);
	if(cnt == -1) sscanf(con.c_str(), "TTY%[^:]::INSTR%n", tmp_port, &cnt);
#endif

    if(cnt != -1) {
#if defined (__WIN32)
        port_num = -1;
        sscanf(tmp_port, "COM%i", &port_num);
        if(port_num <= 8) sprintf(port, "%s", tmp_port);
        else sprintf(port, "\\\\.\\%s", tmp_port);
#elif defined (__linux__)
        (void)port_num;
        sprintf(port, "/dev/tty%s", tmp_port);
#endif
		if(rcv_params) {
			strcpy(rcv_params->szPort, port);
			rcv_params->uBaudRate = baud;
			rcv_params->ucParity = 0; // default
            rcv_params->ucDataBits = 8; // default
			rcv_params->ucStopBits = 0; // default
			rcv_params->ucRTSCtrl = 0; // default
			rcv_params->ucDTRCtrl = 0; // default
			rcv_params->ucDriver = drv;
		}
		return true;

	} else { // Check more human standart as port, baud, parity, data, stop
		int b, d; double s; char p;
    #if defined (_WIN32)
		cnt = sscanf(con.c_str(), "%*[ \\.]%3s%i,%i,%c,%i,%lf", tmp_port, &port_num, &b, &p, &d, &s);
		if(cnt <= 0) cnt = sscanf(con.c_str(), "%3s%i,%i,%c,%i,%lf", tmp_port, &port_num, &b, &p, &d, &s);
	#elif defined (__linux__)
		cnt = sscanf(con.c_str(), "TTY%s,%i,%c,%i,%lf", tmp_port, &b, &p, &d, &s);
		if(cnt <= 0) cnt = sscanf(con.c_str(), "/DEV/TTY%s,%i,%c,%i,%lf", tmp_port, &b, &p, &d, &s);
	#endif
		if(cnt > 0) {
        #if defined (_WIN32)
			--cnt;
            if(port_num <= 8) sprintf(port, "%s%i", tmp_port, port_num);
			else sprintf(port, "\\\\.\\%s%i", tmp_port, port_num);
		#elif defined (__linux__)
			sprintf(port, "/dev/tty%s", tmp_port);
		#endif
			if(rcv_params) {
				strcpy(rcv_params->szPort, port);
				rcv_params->uBaudRate = (cnt >= 2) ? b : 115200;
				rcv_params->ucParity = 0; // default
				rcv_params->ucDataBits = (cnt >= 4) ? d : 8;
				rcv_params->ucStopBits = (cnt >= 5) ? (s * 2 - 2) : 0; // default
				rcv_params->ucRTSCtrl = 0; // default
				rcv_params->ucDTRCtrl = 0; // default
				rcv_params->ucDriver = 0;

				if(cnt >= 3) {
					switch(p) {
						default:
						case 'N': rcv_params->ucParity = 0; break; // None
						case 'S': rcv_params->ucParity = 4; break; // Space
						case 'M': rcv_params->ucParity = 3; break; // Mark
						case 'E': rcv_params->ucParity = 2; break; // Even
						case 'O': rcv_params->ucParity = 1; break; // Odd
					}
				}
			}
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::isCorrectAddressW(const wchar_t * addr, void * dst) const
{
	if(!addr) return false;
	SERIAL_PORT_PARAMETERS * rcv_params = (SERIAL_PORT_PARAMETERS *) dst;
	if(rcv_params->uSize != sizeof(SERIAL_PORT_PARAMETERS)) rcv_params = NULL;

	int cnt = -1, port_num = -1, drv = 0, baud = 115200;
	wchar_t port[16], tmp_port[16];

	std::wstring con(addr);
	// <std::wstring::iterator, std::wstring::iterator, int (*) (int)>
	std::transform(con.begin(), con.end(), con.begin(), ::towupper);

	// Check VXI Address standart

#if defined (_WIN32)
	if(cnt == -1) swscanf(con.c_str(), L"%[^:]::DRV%i::%i::INSTR%n", tmp_port, &drv, &baud, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"%[^:]::%i::DRV%i::INSTR%n", tmp_port, &baud, &drv, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"%[^:]::DRV%i::INSTR%n", tmp_port, &drv, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"%[^:]::%i::INSTR%n", tmp_port, &baud, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"%[^:]::INSTR%n", tmp_port, &cnt);
#elif defined (__linux__)
	if(cnt == -1) swscanf(con.c_str(), L"TTY%[^:]::DRV%i::%i::INSTR%n", tmp_port, &drv, &baud, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"TTY%[^:]::%i::DRV%i::INSTR%n", tmp_port, &baud, &drv, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"TTY%[^:]::DRV%i::INSTR%n", tmp_port, &drv, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"TTY%[^:]::%i::INSTR%n", tmp_port, &baud, &cnt);
	if(cnt == -1) swscanf(con.c_str(), L"TTY%[^:]::INSTR%n", tmp_port, &cnt);
#endif

	if(cnt != -1) {
#if defined (__WIN32)
        swscanf(tmp_port, L"COM%i", &port_num);
        if(port_num <= 8) swprintf(port, L"%s", tmp_port);
        else swprintf(port, L"\\\\.\\%s", tmp_port);
#elif defined (__linux__)
        (void) port_num;
        //swprintf(port, L"/dev/tty%s", tmp_port);
        swprintf(port, 16, L"/dev/tty%s", tmp_port);
#endif
		if(rcv_params) {
			wcstombs(rcv_params->szPort, port, wcslen(port) + 1);
			rcv_params->uBaudRate = baud;
			rcv_params->ucParity = 0; // default
            rcv_params->ucDataBits = 8; // default
			rcv_params->ucStopBits = 0; // default
			rcv_params->ucRTSCtrl = 0; // default
			rcv_params->ucDTRCtrl = 0; // default
			rcv_params->ucDriver = drv;
		}
		return true;

	} else { // Check more human standart as port, baud, parity, data, stop
		int b, d; double s; wchar_t p;
    #if defined (_WIN32)
		cnt = swscanf(con.c_str(), L"%*[ \\.]%3s%i,%i,%lc,%i,%lf", tmp_port, &port_num, &b, &p, &d, &s);
		if(cnt <= 0) cnt = swscanf(con.c_str(), L"%3s%i,%i,%lc,%i,%lf", tmp_port, &port_num, &b, &p, &d, &s);
	#elif defined (__linux__)
		cnt = swscanf(con.c_str(), L"TTY%s,%i,%c,%i,%lf", tmp_port, &b, &p, &d, &s);
		if(cnt <= 0) cnt = swscanf(con.c_str(), L"/DEV/TTY%s,%i,%lc,%i,%lf", tmp_port, &b, &p, &d, &s);
	#endif
		if(cnt > 0) {
        #if defined (_WIN32)
			--cnt;
            if(port_num <= 8) swprintf(port, L"%s%i", tmp_port, port_num);
			else swprintf(port, L"\\\\.\\%s%i", tmp_port, port_num);
		#elif defined (__linux__)
            // swprintf(port, L"/dev/tty%s", tmp_port);
            swprintf(port, 16, L"/dev/tty%s", tmp_port);
        #endif
			if(rcv_params) {
				wcstombs(rcv_params->szPort, port, wcslen(port) + 1);
				rcv_params->uBaudRate = (cnt >= 2) ? b : 115200;
				rcv_params->ucParity = 0; // default
				rcv_params->ucDataBits = (cnt >= 4) ? d : 8;
				rcv_params->ucStopBits = (cnt >= 5) ? (s * 2 - 2) : 0; // default
				rcv_params->ucRTSCtrl = 0; // default
				rcv_params->ucDTRCtrl = 0; // default
				rcv_params->ucDriver = 0;

				if(cnt >= 3) {
					switch(p) {
						default:
						case L'N': rcv_params->ucParity = 0; break; // None
						case L'S': rcv_params->ucParity = 4; break; // Space
						case L'M': rcv_params->ucParity = 3; break; // Mark
						case L'E': rcv_params->ucParity = 2; break; // Even
						case L'O': rcv_params->ucParity = 1; break; // Odd
					}
				}
			}
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::ConnectA(const char * addr)
{
	if(!addr) return ConnectParam(FParameters);

	SERIAL_PORT_PARAMETERS params;
	memcpy(&params, &FParameters, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	FLastError = 0;
    if(isCorrectAddressA(addr, &params))
        return ConnectParam(params);

	DoWrongStringError();
	return false;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::ConnectW(const wchar_t * addr)
{
	if(!addr) return ConnectParam(FParameters);

	SERIAL_PORT_PARAMETERS params;
	memcpy(&params, &FParameters, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	FLastError = 0;
	if(isCorrectAddressW(addr, &params)) return ConnectParam(params);

	DoWrongStringError();
	return false;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CBaseSerialPort::Disconnect()
{
	FLastError = 0;
	// FParameters.szPort[0] = 0;
	if(FConnectionString) { delete[] FConnectionString; FConnectionString = NULL; }
#if defined (_WIN32)
	if(handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
#elif defined (__linux__)
	if(handle >= 0) close(handle);
	handle = -1;
#else
#	error CBaseSerialPort::Disconnect not supported
#endif
	FActive = false;
	return true;
}

//-----------------------------------------------------------------------------
/* virtual */ int CBaseSerialPort::checkData()
{
	FLastError = 0;
#if defined (_WIN32)
	COMSTAT comstat;
	ULONG Errors;
	if(!ClearCommError(handle, &Errors, &comstat)) {
		DoError(GetLastError());
		return -1;
	}
	if(Errors & CE_BREAK) ClearCommBreak(handle);
	return comstat.cbInQue;
#elif defined (__linux__)
	int bytes;
	if(ioctl(handle, FIONREAD, &bytes) == -1) {
		DoError(errno);
		return -1;
	}
	return bytes;
#else
#	error CBaseSerialPort::checkData not supported
	return 0;
#endif
}

//-----------------------------------------------------------------------------
bool CBaseSerialPort::flush()
{
    FLastError = 0;
#if defined (_WIN32)
	if(FlushFileBuffers(handle)) return true;
	return DoError(GetLastError());
#elif defined (__linux__)
	if(tcflush(handle, TCIOFLUSH) == 0) return true;
	return DoError(errno);
#else
#   error CBaseSerialPort::flush not supported
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool CBaseSerialPort::purge()
{
	FLastError = 0;
#if defined (_WIN32)
	if(PurgeComm(handle, PURGE_TXCLEAR | PURGE_RXCLEAR)) return true;
	return DoError(GetLastError());
#elif defined (__linux__)
	// I think it's synonims
	if(tcflush(handle, TCIOFLUSH) == 0) return true;
	return DoError(errno);
#else
#   error CBaseSerialPort::purge not supported
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool CBaseSerialPort::clearRx()
{
	// Flush...
	int t_len = checkData();
	if(t_len <= 0) return false;

	unsigned char * t_data = new unsigned char[t_len];
	if(readData(t_data, t_len) < 0) {
		delete[] t_data;
		return false;
	}
	delete[] t_data;

	return true;
}

//-----------------------------------------------------------------------------
bool CBaseSerialPort::setRTS(bool val)
{
    FLastError = 0;

#if defined (_WIN32)
	if(EscapeCommFunction(handle, val ? SETRTS : CLRRTS)) return true;
	return DoError(GetLastError());
#elif defined (__linux__)
    int status;
    if(ioctl(handle, TIOCMGET, &status) == 0) {
        if(val) status |= TIOCM_RTS;
        else status &=~TIOCM_RTS;
		if(ioctl(handle, TIOCMSET, status) == 0) return true;
	}
	return DoError(errno);
#else
#   error CBaseSerialPort::setRTS not supported
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool CBaseSerialPort::setDTR(bool val)
{
	FLastError = 0;

#if defined (_WIN32)
	if(EscapeCommFunction(handle, val ? SETDTR : CLRDTR)) return true;
	return DoError(GetLastError());
#elif defined (__linux__)
	int status;
	if(ioctl(handle, TIOCMGET, &status) == 0) {
		if(val) status |= TIOCM_DTR;
		else status &=~TIOCM_DTR;
		if(ioctl(handle, TIOCMSET, status) == 0) return true;
	}
	return DoError(errno);
#else
#   error CBaseSerialPort::setDTR not supported
	return false;
#endif
}

//-----------------------------------------------------------------------------
void CBaseSerialPort::setParamStringA(const char * param)
{
	// [baud=int[,parity=([,data[,stop]]]]
	int b, d;
	double s;
	char p;
	int cnt = sscanf(param, "%i,%c,%i,%lf", &b, &p, &d, &s);
	if(cnt >= 1) FParameters.uBaudRate = b;
	if(cnt >= 2) {
		switch(p) {
			default:
			case 'n': case 'N': FParameters.ucParity = 0; break; // None
			case 's': case 'S': FParameters.ucParity = 4; break; // Space
			case 'm': case 'M': FParameters.ucParity = 3; break; // Mark
			case 'e': case 'E': FParameters.ucParity = 2; break; // Even
			case 'o': case 'O': FParameters.ucParity = 1; break; // Odd
		}
	}
	if(cnt >= 3) FParameters.ucDataBits = d;
	if(cnt >= 4) FParameters.ucStopBits = s * 2 - 2;
	if(FActive) setupPort(true, false);
}

//-----------------------------------------------------------------------------
void CBaseSerialPort::setParamStringW(const wchar_t * param)
{
    // [baud=int[,parity=([,data[,stop]]]]
	int b, d;
    double s;
    wchar_t p;
	int cnt = swscanf(param, L"%i,%lc,%i,%lf", &b, &p, &d, &s);
	if(cnt >= 1) FParameters.uBaudRate = b;
    if(cnt >= 2) {
		switch(p) {
			default:
			case L'n': case L'N': FParameters.ucParity = 0; break; // None
			case L's': case L'S': FParameters.ucParity = 4; break; // Space
			case L'm': case L'M': FParameters.ucParity = 3; break; // Mark
			case L'e': case L'E': FParameters.ucParity = 2; break; // Even
			case L'o': case L'O': FParameters.ucParity = 1; break; // Odd
        }
    }
	if(cnt >= 3) FParameters.ucDataBits = d;
    if(cnt >= 4) FParameters.ucStopBits = s * 2 - 2;
	if(FActive) setupPort(true, false);
}

//-----------------------------------------------------------------------------
//************************ End of CBaseSerialPort *****************************
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//************************ Start CSerialPortSync ******************************
//-----------------------------------------------------------------------------

CSerialPortSync::CSerialPortSync(): CBaseSerialPort()
{
	setClassName("CSerialPortSync");
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortSync::isCorrectAddressA(const char * addr, void * dst) const
{
	SERIAL_PORT_PARAMETERS params;
	memcpy(&params, &FParameters, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	if(!CBaseSerialPort::isCorrectAddressA(addr, &params)) return false;

	if(params.ucDriver) return false;

	if(dst)
		if( ((SERIAL_PORT_PARAMETERS *) dst)->uSize == sizeof(SERIAL_PORT_PARAMETERS) )
			memcpy(dst, &params, sizeof(SERIAL_PORT_PARAMETERS));

	return true;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortSync::isCorrectAddressW(const wchar_t * addr, void * dst) const
{
	SERIAL_PORT_PARAMETERS params;
	memcpy(&params, &FParameters, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	if(!CBaseSerialPort::isCorrectAddressW(addr, &params)) return false;

	if(params.ucDriver) return false;

	if(dst)
		if( ((SERIAL_PORT_PARAMETERS *) dst)->uSize == sizeof(SERIAL_PORT_PARAMETERS) )
			memcpy(dst, &params, sizeof(SERIAL_PORT_PARAMETERS));

	return true;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortSync::ConnectParam(const SERIAL_PORT_PARAMETERS & Params)
{
	if(FActive) Disconnect();
	FLastError = 0;
	memcpy(&FParameters, &Params, sizeof(SERIAL_PORT_PARAMETERS));

#if defined (_WIN32)
	handle = CreateFileA(Params.szPort, GENERIC_READ | GENERIC_WRITE,
						0, NULL, OPEN_EXISTING, 0, NULL); // No overlapped
	if(handle == INVALID_HANDLE_VALUE) return DoError(GetLastError());
	FActive = true;
#elif defined (__linux__)
    handle = open(Params.szPort, O_RDWR | O_NOCTTY | O_NDELAY);
	if(handle == -1) DoError(errno);
	FActive = true;
	fcntl(handle, F_SETFL, 0);
	//fcntl(handle, F_SETFL, FNDELAY);
#else
#	error CSerialPortSync::ConnectParam not supported
#endif

	if(FActive) {
//		if(FConnectionString) delete[] FConnectionString;
//		FConnectionString = new char[128];
//		sprintf(FConnectionString, "com@%s,%u,%c,%u,%f",
//			FParameters.szPort,
//			FParameters.uBaudRate, 'n',
////			FParameters.ucParity == 4 ? 's' :
////			FParameters.ucParity == 3 ? 'm' :
////			FParameters.ucParity == 2 ? 'e' :
////			FParameters.ucParity == 1 ? 'o' : 'n',
//			FParameters.ucDataBits,
//			(FParameters.ucStopBits + 2) / 2.);

		if(!setupPort(true, true)) {
			int e_code = FLastError;
			Disconnect();
			DoError(e_code);
			return false;
		}
	}
    return true;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortSync::readData(void * dst, unsigned int len)
{
	FLastError = 0;
	unsigned long rd = (unsigned int)-1;
#if defined (_WIN32)
	if(!ReadFile(handle, dst, len, &rd, NULL)) { DoError(GetLastError()); rd = (unsigned int)-1; }
#elif defined (__linux__)
	rd = read(handle, dst, len);
	if(rd == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortSync::readData not supported
#endif
	return rd;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortSync::writeData(const void * src, unsigned int len)
{
	FLastError = 0;
	unsigned long wr = (unsigned int)-1;
#if defined (_WIN32)
	if(!WriteFile(handle, src, len, &wr, NULL)) { DoError(GetLastError()); wr = (unsigned int)-1; }
#elif defined (__linux__)
	wr = write(handle, src, len);
	if(wr == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortSync::readData not supported
#endif
	return wr;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortSync::readDataTimeout(void * dst, unsigned int len, unsigned int ms)
{
	FLastError = 0;
	unsigned long rd = (unsigned int)-1;
#if defined (_WIN32)
	unsigned long rit = FCt.ReadIntervalTimeout;
	FCt.ReadIntervalTimeout = ms;
	setupPort(false, true);

	if(!ReadFile(handle, dst, len, &rd, NULL)) {
		DoError(GetLastError());
		rd = (unsigned int)-1;
	}

	FCt.ReadIntervalTimeout = rit;
	setupPort(false, true);
#elif defined (__linux__)
	#warning Do CSerialPortSync::readDataTimeout
    (void)ms;

	rd = read(handle, dst, len);
	if(rd == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortSync::readDataTimeout not supported
#endif
	return rd;
}


//-----------------------------------------------------------------------------
//************************ End of CSerialPortSync *****************************
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//************************ Start CSerialPortAsync *****************************
//-----------------------------------------------------------------------------
CSerialPortAsync::CSerialPortAsync(): CBaseSerialPort()
{
#if defined (_WIN32)
    FReadTimeout = INFINITE;
    FWriteTimeout = INFINITE;
#endif
	setClassName("CSerialPortAsync");
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortAsync::isCorrectAddressA(const char * addr, void * dst) const
{
	SERIAL_PORT_PARAMETERS params;
	memset(&params, 0, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	if(!CBaseSerialPort::isCorrectAddressA(addr, &params)) return false;

    if(params.ucDriver) return false;

	if(dst)
		if( ((SERIAL_PORT_PARAMETERS *) dst)->uSize == sizeof(SERIAL_PORT_PARAMETERS) )
			memcpy(dst, &params, sizeof(SERIAL_PORT_PARAMETERS));

	return true;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortAsync::isCorrectAddressW(const wchar_t * addr, void * dst) const
{
	SERIAL_PORT_PARAMETERS params;
	memset(&params, 0, sizeof(SERIAL_PORT_PARAMETERS));
	params.uSize = sizeof(SERIAL_PORT_PARAMETERS);

	if(!CBaseSerialPort::isCorrectAddressW(addr, &params)) return false;

    if(params.ucDriver) return false;

	if(dst)
		if( ((SERIAL_PORT_PARAMETERS *) dst)->uSize == sizeof(SERIAL_PORT_PARAMETERS) )
			memcpy(dst, &params, sizeof(SERIAL_PORT_PARAMETERS));

	return true;
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortAsync::ConnectParam(const SERIAL_PORT_PARAMETERS & Params)
{
	if(FActive) Disconnect();
	FLastError = 0;
	memcpy(&FParameters, &Params, sizeof(SERIAL_PORT_PARAMETERS));



#if defined (_WIN32)
	handle = CreateFileA(Params.szPort, GENERIC_READ | GENERIC_WRITE,
						0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL); // Overlapped
	if(handle == INVALID_HANDLE_VALUE)
		return DoError(GetLastError());
	FActive = true;
#elif defined (__linux__)
    handle = open(Params.szPort, O_RDWR | O_NOCTTY | O_NDELAY);
	if(handle == -1) DoError(errno);
	FActive = true;
	fcntl(handle, F_SETFL, 0);
	//fcntl(handle, F_SETFL, FNDELAY);
#else
#	error CSerialPortAsync::ConnectA not supported
#endif

	if(FActive) {
		FConnectionString = new char[strlen(FParameters.szPort) + 1];
		strcpy(FConnectionString, FParameters.szPort);

		if(!setupPort(true, true)) {
			int e_code = FLastError;
			Disconnect();
			DoError(e_code);
			return false;
		}
	}
    return true;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortAsync::readData(void * dst, unsigned int len)
{
	FLastError = 0;
	unsigned long rd = (unsigned int)-1;
#if defined (_WIN32)
	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, true, true, NULL);

	ReadFile(handle, dst, len, &rd, &overlapped);
	rd = (unsigned int)-1;
	DWORD signal = WaitForSingleObject(overlapped.hEvent, FReadTimeout);
	if(signal == WAIT_OBJECT_0) GetOverlappedResult(handle, &overlapped, &rd, false);
	else if(signal == WAIT_TIMEOUT) rd = 0;

	if(rd == (unsigned int)-1) DoError(GetLastError());
	CloseHandle(overlapped.hEvent);
#elif defined (__linux__)
	rd = read(handle, dst, len);
	if(rd == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortAsync::readData not supported
#endif
	return rd;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortAsync::writeData(const void * src, unsigned int len)
{
	FLastError = 0;
	unsigned long wr = (unsigned int)-1;
#if defined (_WIN32)
	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, true, true, NULL);

    WriteFile(handle, src, len, &wr, &overlapped);
	wr = (unsigned int)-1;
	DWORD signal = WaitForSingleObject(overlapped.hEvent, FWriteTimeout);
	if(signal == WAIT_OBJECT_0) GetOverlappedResult(handle, &overlapped, &wr, false);
	else if(signal == WAIT_TIMEOUT) wr = 0;

	if(wr == (unsigned int)-1) DoError(GetLastError());
	CloseHandle(overlapped.hEvent);
#elif defined (__linux__)
	wr = write(handle, src, len);
	if(wr == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortAsync::readData not supported
#endif
	return wr;
}

//-----------------------------------------------------------------------------
/* virtual */ int CSerialPortAsync::readDataTimeout(void * dst, unsigned int len, unsigned int ms)
{
	FLastError = 0;
	unsigned long rd = (unsigned int)-1;
#if defined (_WIN32)
	OVERLAPPED overlapped;
	overlapped.hEvent = CreateEvent(NULL, true, true, NULL);

	ReadFile(handle, dst, len, &rd, &overlapped);
	rd = (unsigned int)-1;
	DWORD signal = WaitForSingleObject(overlapped.hEvent, ms);
	if(signal == WAIT_OBJECT_0) GetOverlappedResult(handle, &overlapped, &rd, false);
	else if(signal == WAIT_TIMEOUT) rd = 0;

	if(rd == (unsigned int)-1) DoError(GetLastError());
	CloseHandle(overlapped.hEvent);
#elif defined (__linux__)
	#warning Do CSerialPortAsync::readDataTimeout
    (void)ms;

	rd = read(handle, dst, len);
	if(rd == (unsigned int)-1) DoError(errno);
#else
#	error CSerialPortAsync::readDataTimeout not supported
#endif
	return rd;
}

//-----------------------------------------------------------------------------
//************************ End of CSerialPortAsync ****************************
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//************************ Start CSerialPortDrv *******************************
//-----------------------------------------------------------------------------
CSerialPortDrv::CSerialPortDrv(): CSerialPortSync()
{
	setClassName("CSerialPortDrv");
}

//-----------------------------------------------------------------------------
/* virtual */ bool CSerialPortDrv::setupPort(bool setup_dcb, bool setup_cto)
{
	if(!CSerialPort::setupPort(setup_dcb, setup_cto)) return false;
	if(FParameters.ucDriver) {
		char drv_data[3];
		drv_data[0] = 0xC0;
		drv_data[1] = 0x10 + FParameters.ucDriver;
		drv_data[2] = FParameters.uBaudRate == 921600 ? 2 : 1;
		if(CSerialPort::writeData(drv_data, 3) != 3) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
int CSerialPortDrv::writeData(const void * src, unsigned int len)
{
	char drv_data[4];
	if(FParameters.ucDriver == 0) return CSerialPort::writeData(src, len);

	drv_data[0] = 0xC0;
	drv_data[1] = 0x20 + FParameters.ucDriver;
	drv_data[2] = (len >> 8);
	drv_data[3] = len & 0xFF;

	if(CSerialPort::writeData(drv_data, 4) != 4) return -1;
	return CSerialPort::writeData(src, len);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//************************ End of CSerialPortDrv ******************************
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//************************ Start TSerialPort **********************************
//-----------------------------------------------------------------------------
#if defined (__BORLANDC__)

__fastcall TSerialPort::TSerialPort(TComponent * AOwner): TComponent(AOwner)
{
	FSync = true;
	FRTSControl = sprcNone;
	FDriverType = spdtNone;

	device = new CSerialPortSync();

	FNotifyWnd = AllocateHWnd(FTimerProc);
	FTimerID = 0;
	FUpdatePeriod = 9;
}

//-----------------------------------------------------------------------------
__fastcall TSerialPort::~TSerialPort()
{
	if(FTimerID) { KillTimer(FNotifyWnd, 1); FTimerID = 0;}
	if(device) {
		// Disconnect(); // Can occurs error
		delete device;
		device = NULL;
	}
	DeallocateHWnd(FNotifyWnd);
}

//-----------------------------------------------------------------------------
//-------- Protected section (properties) -------------------------------------
void __fastcall TSerialPort::setSync(bool val)
{
	FSync = val; // Under construction
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setRTSControl(TSerialPortRTSCtrl val)
{
	FRTSControl = val; // Under construction
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setDriverType(TSerialPortDriverType val)
{
	FDriverType = val; // under construction
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setUpdatePeriod(int val)
{
	if(device) {
		if(val != FUpdatePeriod) {
			if(device->isActive()) {
				if(FUpdatePeriod > 0) { FTimerID = SetTimer(FNotifyWnd, 1, FUpdatePeriod, NULL); }
				else if(FTimerID) { KillTimer(FNotifyWnd, 1); FTimerID = 0;}
			}
			FUpdatePeriod = val;
		}
	}
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::emitDeviceClassError()
{
#ifdef UNICODE
    if(FOnError) FOnError(this, -1, L"Creation device class error");
#else
    if(FOnError) FOnError(this, -1,  "Creation device class error");
#endif
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::FTimerProc(TMessage &msg)
{
	if(device) {
		if((msg.Msg == WM_TIMER) && device->isActive())
		{
			int len = device->checkData();
			if(len < 0) {
				if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
				if(!device->isActive()) Disconnect();
			} else if((len > 0 ) && FOnDataReceive) {
				BYTE * buffer = new BYTE[len];
				ULONG rd = device->readData(buffer, len);
				if(rd < 0) {
					if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
					if(!device->isActive()) Disconnect();
				} else if(rd > 0) FOnDataReceive(this, buffer, rd);
				delete[] buffer;
			}
		}
	}
	msg.Result = DefWindowProc(FNotifyWnd, msg.Msg, msg.WParam, msg.LParam);
}

//-----------------------------------------------------------------------------
//-------- Public section  ----------------------------------------------------
bool __fastcall TSerialPort::Connect(const String &Address)
{
	if(device) {
    	if(device->isActive()) Disconnect();

		if(device->Connect(Address.c_str())) {
			if(FOnConnect) FOnConnect(this);
		} else {
			if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
		}

		if(device->isActive()) {
			if(FUpdatePeriod > 0) FTimerID = SetTimer(FNotifyWnd, 1, FUpdatePeriod, NULL);
		}

		return device->isActive();
	}
	emitDeviceClassError();
	return false;
}
//-----------------------------------------------------------------------------
bool __fastcall TSerialPort::Disconnect()
{
	if(device) {
		if(FTimerID) { KillTimer(FNotifyWnd, 1); FTimerID = 0;}
		if(device->isActive()) {
			device->Disconnect();
			if(FOnDisconnect) FOnDisconnect(this);
			return true;
		} else {
			int ECode = device->getLastError();
#if defined (_WIN32)
			if((ECode == ERROR_ACCESS_DENIED) || (ECode == ERROR_BAD_COMMAND))
#elif defined (__linux__)
			if(ECode == 13)
#endif
			{
				if(FOnDisconnect) FOnDisconnect(this);
				return true;
			}
		}
		return false;
	}
	emitDeviceClassError();
	return false;
}

//-----------------------------------------------------------------------------
int __fastcall TSerialPort::WriteData(const void * src, unsigned int len)
{
	if(device) {
		if(device->isActive()) {
			if(len > 0) { if(FOnDataTransmit) FOnDataTransmit(this, (BYTE *)src, len); }
			int ans = device->writeData(src, len);
			if(ans < 0) {
				if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
				if(!device->isActive()) Disconnect();
			}
			return ans;
		} return 0;
	}
	emitDeviceClassError();
	return -1;
}

//-----------------------------------------------------------------------------
int __fastcall TSerialPort::ReadData(void * dst, unsigned int len)
{
	if(device) {
		if(device->isActive()) {
			int ans = device->readData(dst, len);
			if(ans > 0) { if(FOnDataReceive) FOnDataReceive(this, (BYTE *)dst, ans); }
			else if(ans < 0) {
				if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
				if(!device->isActive()) Disconnect();
			}
			return ans;
		}
		return 0;
	}
	emitDeviceClassError();
	return -1;
}

//-----------------------------------------------------------------------------
int __fastcall TSerialPort::CheckData()
{
	if(device) {
		if(device->isActive()) {
			// if(FOnDataReceive(this, dst, len);
			int ans = device->checkData();
			if(ans < 0) {
				if(FOnError) FOnError(this, device->getLastError(), device->getErrorString(device->getLastError()));
				if(!device->isActive()) Disconnect();
			}
			return ans;
		} return 0;
	}
	emitDeviceClassError();
	return -1;
}

//-----------------------------------------------------------------------------
int __fastcall TSerialPort::SetTimeouts(const COMMTIMEOUTS &to)
{
	if(device) {
		device->setTimeouts(to);
		return 1;
	}
	emitDeviceClassError();
	return -1;
}

//-----------------------------------------------------------------------------
int __fastcall TSerialPort::Flush()
{
	if(device) {
		return device->flush();
	}
	emitDeviceClassError();
	return -1;
}

//-----------------------------------------------------------------------------
/* static */int __fastcall TSerialPort::FindDevices(TStrings * Dest)
{
	if(!Dest) return 0;
	// Dest->Clear();
#ifdef UNICODE
	WSVECTOR list;
	WSVECTOR::iterator it;
#else
	SVECTOR list;
	SVECTOR::iterator it;
#endif
	CSerialPort::findDevices(list);

	for(it = list.begin(); it != list.end(); it++)
		Dest->Add(it->c_str());

	return Dest->Count;
}

//-----------------------------------------------------------------------------
/* static */ int __fastcall TSerialPort::FindDevicesVXI(TStrings * Dest)
{
    if(!Dest) return 0;
    // Dest->Clear();
#ifdef UNICODE
    WSVECTOR list;
    WSVECTOR::iterator it;
#else
    SVECTOR list;
    SVECTOR::iterator it;
#endif
    CSerialPort::findDevicesVXI(list);

    for(it = list.begin(); it != list.end(); it++)
        Dest->Add(it->c_str());

    return Dest->Count;
}

//-----------------------------------------------------------------------------
unsigned int __fastcall TSerialPort::getBaudrate()
{
	if(device) return device->getBaudrate();
	emitDeviceClassError();
	return 0;
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setBaudrate(unsigned int val)
{
	if(device) {
		device->setBaudrate(val);
		int err_code = device->getLastError();
		if(err_code) {
			if(FOnError) FOnError(this, err_code, device->getErrorString(err_code));
		}
		return;
	}
	emitDeviceClassError();
}

//-----------------------------------------------------------------------------
unsigned short __fastcall TSerialPort::getDataBits()
{
	if(device) return device->getDataBits();
	emitDeviceClassError();
	return 0;
}

//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setDataBits(unsigned short val)
{
	if(device) {
		device->setDataBits(val);
		int err_code = device->getLastError();
		if(err_code) {
			if(FOnError) FOnError(this, err_code, device->getErrorString(err_code));
		}
		return;
	}
	emitDeviceClassError();
}
//-----------------------------------------------------------------------------
unsigned short __fastcall TSerialPort::getStopBits()
{
	if(device) return device->getStopBits();
	emitDeviceClassError();
	return 0;
}
//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setStopBits(unsigned short val)
{
	if(device) {
		device->setStopBits(val);
		int err_code = device->getLastError();
		if(err_code) {
			if(FOnError) FOnError(this, err_code, device->getErrorString(err_code));
		}
		return;
	}
	emitDeviceClassError();
}
//-----------------------------------------------------------------------------
unsigned short __fastcall TSerialPort::getParity()
{
	if(device) return device->getParity();
	emitDeviceClassError();
	return 0;
}
//-----------------------------------------------------------------------------
void __fastcall TSerialPort::setParity(unsigned short val)
{
	if(device) {
		device->setParity(val);
		int err_code = device->getLastError();
		if(err_code) {
			if(FOnError) FOnError(this, err_code, device->getErrorString(err_code));
		}
		return;
	}
	emitDeviceClassError();
}

//-----------------------------------------------------------------------------
//************************ End of TSerialPort *********************************
//-----------------------------------------------------------------------------

#endif // __BORLANDC__

//-----------------------------------------------------------------------------
#if defined (QT_CORE_LIB)

QSerialPort::QSerialPort(QObject * parent): QObject(parent)
{
    device = new CSerialPort();
}

//-----------------------------------------------------------------------------
QSerialPort::~QSerialPort()
{
    if(device) {
        delete device;
        device = NULL;
    }
}

//-----------------------------------------------------------------------------
void QSerialPort::emitDeviceClassError()
{
    emit crashed(-1);
    emit crashed("Creation device class error");
}


//-----------------------------------------------------------------------------
int QSerialPort::writeData(const void * src, unsigned int len)
{
    if(device) {
        if(device->isActive()) {
            if(len > 0) emit transmitted(len);
            int ans = device->writeData(src, len);
            if(ans < 0) {
                int err_code = device->getLastError();
                QString err_text = device->getErrorStringA(err_code);
                emit crashed(err_code);
                emit crashed(err_text);
            }
            return ans;
        }
        return 0;
    }
    emitDeviceClassError();
    return -1;
}

//-----------------------------------------------------------------------------
int QSerialPort::readData(void * dst, unsigned int len)
{
    if(device) {
        if(device->isActive()) {
            int ans = device->readData(dst, len);
            if(ans < 0) {
                int err_code = device->getLastError();
                QString err_text = device->getErrorStringA(err_code);
                emit crashed(err_code);
                emit crashed(err_text);
            } else {
                emit received(ans);
            }
            return ans;
        }
        return 0;
    }
    emitDeviceClassError();
    return -1;
}

//-----------------------------------------------------------------------------
int QSerialPort::checkData()
{
    if(device) {
        if(device->isActive()) {
            int ans = device->checkData();
            if(ans < 0) {
                int err_code = device->getLastError();
                QString err_text = device->getErrorStringA(err_code);
                emit crashed(err_code);
                emit crashed(err_text);
            }
            return ans;
        }
        return 0;
    }
    emitDeviceClassError();
    return -1;
}

//-----------------------------------------------------------------------------
/* static */ int QSerialPort::findDevices(QStringList & dst)
{
    SVECTOR lst;
    CBaseSerialPort::findDevicesA(lst);
    if(!lst.empty()) {
        for(SVECTOR::iterator it = lst.begin(); it != lst.end(); it++)
            dst.append(it->c_str());
    }
    return dst.count();
}

//-----------------------------------------------------------------------------
/* static */ int QSerialPort::findDevicesVXI(QStringList & dst)
{
    SVECTOR lst;
    CBaseSerialPort::findDevicesVXIA(lst);
    if(!lst.empty()) {
        for(SVECTOR::iterator it = lst.begin(); it != lst.end(); it++)
            dst.append(it->c_str());
    }
    return dst.count();
}

//-----------------------------------------------------------------------------
void QSerialPort::setBaudrate(int b)
{
    if(device) {
        device->setBaudrate(b);
        int err_code = device->getLastError();
        if(err_code) {
            QString err_text = device->getErrorStringA(err_code);
            emit crashed(err_code);
            emit crashed(err_text);
        }
        return;
    }
    emitDeviceClassError();
}

//-----------------------------------------------------------------------------
int QSerialPort::getBaudrate()
{
    if(device) {
        int b = device->getBaudrate();
        int err_code = device->getLastError();
        if(err_code) {
            QString err_text = device->getErrorStringA(err_code);
            emit crashed(err_code);
            emit crashed(err_text);
            return 0;
        }
        return b;
    }
    emitDeviceClassError();
    return 0;
}

//-----------------------------------------------------------------------------
bool QSerialPort::isActive()
{
    if(device) {
        return device->isActive();
    }
    emitDeviceClassError();
    return false;
}

//-----------------------------------------------------------------------------
bool QSerialPort::activate()
{
    if(device) {
        deactivate();
        int ans = device->ConnectA(NULL); // Use previous parameters
        if(!ans) {
            int err_code = device->getLastError();
            QString err_text = device->getErrorStringA(err_code);
            emit crashed(err_code);
            emit crashed(err_text);
        } else {
            emit activated();
        }
        return ans;
    }
    emitDeviceClassError();
    return false;
}

//-----------------------------------------------------------------------------
bool QSerialPort::activate(const QString &a)
{
    if(device) {
        deactivate();
        int ans = device->ConnectA(a.toLatin1().data()); // Use previous parameters
        if(!ans) {
            int err_code = device->getLastError();
            QString err_text = device->getErrorStringA(err_code);
            emit crashed(err_code);
            emit crashed(err_text);
        } else {
            emit activated();
        }
        return ans;
    }
    emitDeviceClassError();
    return false;
}

//-----------------------------------------------------------------------------
bool QSerialPort::deactivate()
{
    if(device) {
        if(device->isActive()) {
            device->Disconnect();
            emit deactivated();
            return true;
        }
        return false;
    }
    emitDeviceClassError();
    return false;
}

//-----------------------------------------------------------------------------
#endif // QT_CORE_LIB

//***************************************************************************
//** BaseDevice.cpp                                                        **
//** Virtual device component                                              **
//**   OS: windows, linux                                                  **
//**   Version: 2.0 CrossLib                                               **
//**   Autor: Pavel Brain                                                  **
//**   Last modified: 27-04-2017                                           **
//***************************************************************************

#include <BaseDevice.h>

#if defined (_WIN32)
#   include <windows.h>
#endif

#if defined (__linux__)
#	include <sys/time.h>
#endif

#if defined (_WIN32)
//-----------------------------------------------------------------------------
const char * getWindowsErrorA(unsigned int e_code)
{
	const char * e_buf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   NULL, e_code, 0 /*0x400*/, (char *)&e_buf, 0, NULL);
	return e_buf;
}

//-----------------------------------------------------------------------------
const wchar_t * getWindowsErrorW(unsigned int e_code)
{
	const wchar_t * e_buf;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				   NULL, e_code, 0 /*0x400*/, (wchar_t *)&e_buf, 0, NULL);
	return e_buf;
}

#elif defined (__linux__)
//-----------------------------------------------------------------------------
const char * getLinuxError(unsigned int e_code)
{
	return strerror(e_code);
}

//-----------------------------------------------------------------------------
const wchar_t * getLinuxErrorW(unsigned int e_code)
{
    return (wchar_t *)strerror(e_code);
}

//-----------------------------------------------------------------------------
unsigned long int GetTickCount()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

//-----------------------------------------------------------------------------
void crosslib_SleepEx(unsigned int ms)
{
    unsigned long int en = GetTickCount() + ms;
    while(GetTickCount() < en) crosslib_IDLE();
}

//-----------------------------------------------------------------------------
//************************ Start CBaseDevice **********************************
//-----------------------------------------------------------------------------
CBaseDevice::CBaseDevice(const char * Name):FActive(false), FLastError(0)
{
	if(Name) {
		FClassName = new char[strlen(Name) + 1];
		strcpy(FClassName, Name);
	} else {
		FClassName = NULL;
	}
	FConnectionString = NULL;
}

//-----------------------------------------------------------------------------
/* virtual */ CBaseDevice::~CBaseDevice()
{
	if(FClassName) { delete[] FClassName; FClassName = NULL; }
	if(FConnectionString) {delete[] FConnectionString; FConnectionString = NULL; }
}

//-----------------------------------------------------------------------------
void CBaseDevice::setClassName(const char * Name)
{
	if(FClassName) { delete[] FClassName; FClassName = NULL; }

	if(Name) {
		FClassName = new char[strlen(Name) + 1];
		strcpy(FClassName, Name);
	} else {
		FClassName = NULL;
	}
}

//-----------------------------------------------------------------------------
void CBaseDevice::DoWrongStringError()
{
#if defined (_WIN32)
    DoError(ERROR_FILE_NOT_FOUND);
#elif defined (__linux__)
    DoError(ENOENT);
#endif
}

//-----------------------------------------------------------------------------
/* virtual */ int CBaseDevice::waitData(unsigned int length, unsigned int ms)
{
	FLastError = 0;
	int rcv_length;

	unsigned long int cr, en;
	en = GetTickCount() + ms;
	do {
        crosslib_IDLE();
		rcv_length = checkData();
		if(rcv_length == -1) return -1;
		cr = GetTickCount();
	} while ((en > cr) && ((unsigned int)rcv_length < length));

	return rcv_length;
}

//-----------------------------------------------------------------------------
int CBaseDevice::readDataTimeout(void * dst, unsigned int length, unsigned int ms)
{
	FLastError = 0;
	int rcv_length;

	unsigned long int cr, en;
	en = GetTickCount() + ms;
	do {
        crosslib_IDLE();
		rcv_length = checkData();
		if(rcv_length == -1) return -1;
		cr = GetTickCount();
	} while ((en > cr) && ((unsigned int)rcv_length < length));

	return readData(dst, std::min((unsigned int)rcv_length, length));
}

//-----------------------------------------------------------------------------
int CBaseDevice::readDataAll(void ** dst)
{
	FLastError = 0;

	int rcv_length = checkData();
	if(rcv_length <= 0) return rcv_length;

	*dst = new unsigned char[rcv_length];
	return readData(*dst, rcv_length);
}

//-----------------------------------------------------------------------------
int CBaseDevice::flushAll()
{
	FLastError = 0;

	int ans;
	do {
		ans = checkData();
		if(ans <= 0) return ans;

		unsigned char * dst = new unsigned char[ans];
		ans = readData(dst, ans);
		delete[] dst;
    } while(ans);

	return ans;
}

//-----------------------------------------------------------------------------
//************************ End of CBaseDevice *********************************
//-----------------------------------------------------------------------------


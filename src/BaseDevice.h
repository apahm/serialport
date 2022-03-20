//***************************************************************************
//** BaseDevice.h                                                          **
//** Virtual device component                                              **
//**   OS: windows, linux                                                  **
//**   Version: 2.0 CrossLib                                               **
//**   Autor: Pavel Brain                                                  **
//**   Last modified: 27-04-2017                                           **
//***************************************************************************

#ifndef __BASEDEVICE_H
#define __BASEDEVICE_H

#include "crosslib.h"

//-----------------------------------------------------------------------------
// Standart OS errors formatting
#if defined (_WIN32)
	const char * getWindowsErrorA(unsigned int e_code);
	const wchar_t * getWindowsErrorW(unsigned int e_code);
#	ifdef UNICODE
	inline const wchar_t * getWindowsError(unsigned int e_code) { return getWindowsErrorW(e_code); }
#	else
	inline const char    * getWindowsError(unsigned int e_code) { return getWindowsErrorA(e_code); }
#	endif
#elif defined (__linux__)
    const char * getLinuxError(unsigned int e_code);
    const wchar_t * getLinuxErrorW(unsigned int e_code);
    unsigned long int GetTickCount();
#endif

void crosslib_SleepEx(unsigned int ms);

//-----------------------------------------------------------------------------
// Base class for all devices
class CBaseDevice /* Virtual */
{
private:
protected:
    bool FActive;
    int FLastError;
	char * FClassName;
	char * FConnectionString;

	void setClassName(const char * Name);
	inline virtual bool DoError(int ECode = 0) { FLastError = ECode; return ECode == 0; }

    void DoWrongStringError();

public:
	CBaseDevice(const char * Name);
    virtual ~CBaseDevice();

	inline const char * className() const { return FClassName; }
	inline bool classNameIs(const char * Name) const { return strcmp(Name, FClassName) == 0; }
	inline const char * connectionString() const { return FConnectionString; }

	inline bool isActive() const { return FActive; }
	inline int  getLastError() const { return FLastError; }

	virtual bool isCorrectAddressA(const char    * addr, void * dst) const = 0;
	virtual bool isCorrectAddressW(const wchar_t * addr, void * dst) const = 0;

#ifdef UNICODE
	inline bool isCorrectAddress(const wchar_t * addr, void * dst) const { return isCorrectAddressW(addr, dst); }
#else
	inline bool isCorrectAddress(const char    * addr, void * dst) const { return isCorrectAddressA(addr, dst); }
#endif


	virtual bool ConnectA(const char * addr) = 0;
	virtual bool ConnectW(const wchar_t * addr) = 0;

#ifdef UNICODE
	inline virtual bool Connect(const wchar_t * addr)  { return ConnectW(addr); }
	inline bool Activate(const wchar_t * addr) { return Connect(addr); }
#else
	inline virtual bool Connect(const char    * addr) { return ConnectA(addr); }
	inline bool Activate(const char   * addr) { return Connect(addr); }
#endif

	virtual bool Disconnect() = 0;
	inline bool Deactivate() {return Disconnect(); }

	virtual int checkData() = 0;
	virtual int waitData(unsigned int length, unsigned int ms);

	virtual int readData(void * dst, unsigned int length) = 0;
	virtual int writeData(const void * src, unsigned int length) = 0;

	virtual int readDataTimeout(void * dst, unsigned int length, unsigned int ms);
	virtual int readDataAll(void ** dst);

	virtual int flushAll();

	virtual const char    * getErrorStringA(int e_code) const = 0;
	virtual const wchar_t * getErrorStringW(int e_code) const = 0;

#ifdef UNICODE
	inline const wchar_t * getErrorString(int e_code) const { return getErrorStringW(e_code); }
#else
	inline const char    * getErrorString(int e_code) const { return getErrorStringA(e_code); }
#endif
};

//-----------------------------------------------------------------------------
#if defined (__BORLANDC__)
typedef void __fastcall (__closure *TIODataEvent)(System::TObject *, const BYTE *, ULONG);
typedef void __fastcall (__closure *TFailEvent)(System::TObject *, ULONG, const String &);
#endif // _BORLANDC_

//-----------------------------------------------------------------------------
#if defined (QT_CORE_LIB)
//
#endif // QT_CORE_LIB


//-----------------------------------------------------------------------------
#endif // __BASEDEVICE_H

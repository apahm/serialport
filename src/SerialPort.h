//***************************************************************************
//** SerialPort.h                                                          **
//** Automated COM/TTY component                                           **
//** Based on Win API, IOCNTL                                              **
//**   OS: Windows / Linux                                                 **
//**   Version: 2.0 CrossLib                                               **
//**   Autor: Pavel Brain                                                  **
//**   Last modified: 27-04-2017                                           **
//***************************************************************************

#ifndef __SERIALPORT_H
#define __SERIALPORT_H

#include <BaseDevice.h>

//-----------------------------------------------------------------------------
// OS depended includes
#if defined (_WIN32)
#   include <windows.h>
#elif defined (__linux__)
#   include <dirent.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <errno.h>
#   include <termios.h>
#   include <sys/ioctl.h>
#   include <sys/time.h>
#endif

//-----------------------------------------------------------------------------
// STL includes
#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Addittional struct defines
typedef struct {
	unsigned int uSize;
	unsigned int uBaudRate;
	unsigned char ucParity;
	unsigned char ucDataBits;
	unsigned char ucStopBits;
	unsigned char ucRTSCtrl;
	unsigned char ucDTRCtrl;
	unsigned char ucDriver; // For Ex version
	char szPort[16];
} /*SERIAL_PORT_PARAMETERSA, */SERIAL_PORT_PARAMETERS;

//-----------------------------------------------------------------------------
//typedef struct {
//	unsigned int uSize;
//	unsigned int uBaudRate;
//	unsigned char ucParity;
//	unsigned char ucDataBits;
//	unsigned char ucStopBits;
//	unsigned char ucRTSCtrl;
//	unsigned char ucDTRCtrl;
//	unsigned char ucDriver; // For Ex version
//	wchar_t szPort[16];
//} SERIAL_PORT_PARAMETERSW;

//-----------------------------------------------------------------------------
//#ifdef UNICODE
//	typedef SERIAL_PORT_PARAMETERSW SERIAL_PORT_PARAMETERS;
//#else
//	typedef SERIAL_PORT_PARAMETERSA SERIAL_PORT_PARAMETERS;
//#endif // !UNICODE

#define comDataBits8    	8
#define comDataBits7    	7
#define comDataBits6    	6

#define comStopBitsOne    	0
#define comStopBitsOneHalf  1
#define comStopBitsTwo    	2

#define comParityNone    	0
#define comParityOdd    	1
#define comParityEven    	2
#define comParityMark    	3
#define comParitySpace    	4


//-----------------------------------------------------------------------------
class CBaseSerialPort: public CBaseDevice /* Virtual */
{
private:
protected:
#if defined (_WIN32)
	HANDLE handle;
	DCB FDcb;
	COMMTIMEOUTS FCt;
#elif defined (__linux__)
	int handle;
	struct termios opts;
#endif

	SERIAL_PORT_PARAMETERS FParameters;

	inline unsigned int IntToBaud(unsigned int baudrate) const;

	virtual bool DoError(int ECode = 0);
	virtual bool setupPort(bool setup_dcb, bool setup_cto);

public:
	CBaseSerialPort();
    virtual ~CBaseSerialPort();

	// Class independed section
	virtual bool isCorrectAddressA(const    char * addr, void * dst) const;
	virtual bool isCorrectAddressW(const wchar_t * addr, void * dst) const;

	virtual bool ConnectParam(const SERIAL_PORT_PARAMETERS & Params) = 0;
	//virtual bool ConnectParamA(const SERIAL_PORT_PARAMETERSA & Params) = 0;
	//virtual bool ConnectParamW(const SERIAL_PORT_PARAMETERSW & Params) = 0;
	virtual bool ConnectA(const char    * addr);
	virtual bool ConnectW(const wchar_t * addr);
	virtual bool Disconnect();

	virtual int checkData();

	// Class depended section
	bool flush();
	bool purge();

	bool clearRx();

	bool setRTS(bool val);
	bool setDTR(bool val);

#if defined (_WIN32)
	inline HANDLE getHandle() const {return handle;}
#elif defined (__linux__)
	inline int getHandle() const {return handle;}
#endif

	// Properties section
//#ifdef UNICODE
//	inline const wchar_t * getPort() const { return FParameters.szPort; }
//#else
//	inline const char    * getPort() const { return FParameters.szPort; }
//#endif
	inline const char    * getPort() const { return FParameters.szPort; }

	inline void setBaudrate(unsigned int baudrate) { FParameters.uBaudRate = baudrate; if(FActive) setupPort(true, false); }
	inline unsigned int getBaudrate() const { return FParameters.uBaudRate; }

	inline void setDataBits(unsigned char databits) { FParameters.ucDataBits = databits; if(FActive) setupPort(true, false); }
	inline unsigned char getDataBits() const { return FParameters.ucDataBits; }

	inline void setStopBits(unsigned char stopbits) { FParameters.ucStopBits = stopbits; if(FActive) setupPort(true, false); }
	inline unsigned char getStopBits() const { return FParameters.ucStopBits; }

	inline void setParity(unsigned char parity)   { FParameters.ucParity  = parity; if(FActive) setupPort(true, false); }
	inline unsigned char getParity() const { return FParameters.ucParity; }

	inline void setRTSCtrl(unsigned char ctrl)     { FParameters.ucRTSCtrl = ctrl; if(FActive) setupPort(true, false);}
	inline unsigned char getRTSCtrl() const { return FParameters.ucRTSCtrl; }

	inline void setDTRCtrl(unsigned char ctrl)     { FParameters.ucDTRCtrl = ctrl; if(FActive) setupPort(true, false);}
	inline unsigned char getDTRCtrl() const { return FParameters.ucDTRCtrl; }

#if defined(_WIN32)
    inline void setTimeouts(const COMMTIMEOUTS &timeout) { FCt = timeout; if(FActive) setupPort(false, true); }
	inline COMMTIMEOUTS getTimeouts() 	    { return FCt; }
#endif

    inline SERIAL_PORT_PARAMETERS parameters() const { return FParameters; }

	void setParamStringA(const char    * param);
	void setParamStringW(const wchar_t * param);

	// Static section
	virtual const char    * getErrorStringA(int e_code) const;
	virtual const wchar_t * getErrorStringW(int e_code) const;

	static int findDevicesA(SVECTOR & dst);
	static int findDevicesW(WSVECTOR & dst);

    static int findDevicesVXIA(SVECTOR & dst);
    static int findDevicesVXIW(WSVECTOR & dst);
#ifdef UNICODE
    inline static int findDevices(WSVECTOR & dst) { return findDevicesW(dst); }
    inline static int findDevicesVXI(WSVECTOR & dst) { return findDevicesVXIW(dst); }
#else
    inline static int findDevices( SVECTOR & dst) { return findDevicesA(dst); }
    inline static int findDevicesVXI( SVECTOR & dst) { return findDevicesVXIA(dst); }
#endif
};

//-----------------------------------------------------------------------------
// CSerialPort using Sync algorithms. It is MAIN Serial Port class
class CSerialPortSync: public CBaseSerialPort
{
private:
protected:
public:
	CSerialPortSync();
	//CSerialPortSync(const CBaseSerialPort &port);
	~CSerialPortSync() { }

	// Class independed section
	virtual bool isCorrectAddressA(const    char * addr, void * dst) const;
	virtual bool isCorrectAddressW(const wchar_t * addr, void * dst) const;

	virtual bool ConnectParam(const SERIAL_PORT_PARAMETERS & Params);
//	virtual bool ConnectParamA(const SERIAL_PORT_PARAMETERSA & Params);
//	virtual bool ConnectParamW(const SERIAL_PORT_PARAMETERSW & Params);
//	inline virtual bool ConnectA(const char    * addr) { return CBaseSerialPort::ConnectA(addr); }
//	inline virtual bool ConnectW(const wchar_t * addr) { return CBaseSerialPort::ConnectW(addr); }

	virtual int readData(void * dst, unsigned int len);
	virtual int writeData(const void * src, unsigned int len);

	virtual int readDataTimeout(void * dst, unsigned int len, unsigned int ms);

};

typedef CSerialPortSync CSerialPort;

//-----------------------------------------------------------------------------
// CSerialPort using Async algorithms
class CSerialPortAsync: public CBaseSerialPort
{
// #warning need to update and test
private:
protected:
	unsigned int FReadTimeout; // can be INFINITE
	unsigned int FWriteTimeout;
public:
	CSerialPortAsync();
	~CSerialPortAsync() { }

	// Class independed section
	virtual bool isCorrectAddressA(const    char * addr, void * dst) const;
	virtual bool isCorrectAddressW(const wchar_t * addr, void * dst) const;

	virtual bool ConnectParam(const SERIAL_PORT_PARAMETERS & Params);
//	virtual bool ConnectParamA(const SERIAL_PORT_PARAMETERSA & Params);
//	virtual bool ConnectParamW(const SERIAL_PORT_PARAMETERSW & Params);
//	inline virtual bool ConnectA(const char    * addr)  { return CBaseSerialPort::ConnectA(addr); }
//	inline virtual bool ConnectW(const wchar_t * addr)  { return CBaseSerialPort::ConnectW(addr); }

	virtual int readData(void * dst, unsigned int len);
	virtual int writeData(const void * src, unsigned int len);

	virtual int readDataTimeout(void * dst, unsigned int len, unsigned int ms);

	// Properties section
	inline void setReadTimeout(unsigned int rd_timeout) { FReadTimeout = rd_timeout; }
	inline unsigned int getReadTimeout() { return FReadTimeout; }

	inline void setWriteTimeout(unsigned int wr_timeout) { FWriteTimeout = wr_timeout; }
	inline unsigned int getWriteTimeout() { return FWriteTimeout; }
};


//-----------------------------------------------------------------------------
// CSerialPort extended class fo using with RANNET driver
class CSerialPortDrv: public CSerialPortSync
{
private:
protected:
	virtual bool setupPort(bool setup_dcb, bool setup_cto);

public:
	CSerialPortDrv();
	~CSerialPortDrv() { }

	virtual int writeData(const void * src, unsigned int len);

	// Properties
    inline void setDriver(unsigned char drv) { if(FParameters.ucDriver != drv) {FParameters.ucDriver = drv; setupPort(false, false);} }
	inline unsigned char getDriver() const { return FParameters.ucDriver; }
};

#if defined (__BORLANDC__)
//-----------------------------------------------------------------------------
// TSerialPort class for CBuilder or Delphi
//-----------------------------------------------------------------------------

typedef enum { spdtNone = 0, spdtFullDuplex1 = 1, spdtHalfDuplex1 = 2, spdtFullDuplex2 = 3, spdtHalfDuplex2 = 4 } TSerialPortDriverType;
typedef enum { sprcNone = 0, sprcHardware = 1, sprcSoftware } TSerialPortRTSCtrl;

//-----------------------------------------------------------------------------
class TSerialPort: public TComponent
{
private:
protected:
	CBaseSerialPort * device;

	UINT FTimerID;
	HWND FNotifyWnd;
	int FUpdatePeriod;

	bool FSync;
	TSerialPortRTSCtrl FRTSControl;
	TSerialPortDriverType FDriverType;

	TNotifyEvent FOnConnect;
	TNotifyEvent FOnDisconnect;
	TIODataEvent FOnDataReceive;
	TIODataEvent FOnDataTransmit;
	TFailEvent FOnError;

	void __fastcall setSync(bool val);
	void __fastcall setRTSControl(TSerialPortRTSCtrl val);
	void __fastcall setDriverType(TSerialPortDriverType val);
	void __fastcall setUpdatePeriod(int val);

	void __fastcall emitDeviceClassError();

	inline bool __fastcall getActive() { if(device) return device->isActive(); return false; }

    unsigned int __fastcall getBaudrate();
	void __fastcall setBaudrate(unsigned int val);
	unsigned short __fastcall getDataBits();
	void __fastcall setDataBits(unsigned short val);
	unsigned short __fastcall getStopBits();
	void __fastcall setStopBits(unsigned short val);
	unsigned short __fastcall getParity();
	void __fastcall setParity(unsigned short val);

	void __fastcall FTimerProc(TMessage &msg);

public:
	__fastcall TSerialPort(TComponent * AOwner);
	__fastcall ~TSerialPort();

	bool __fastcall Connect(const String &Address);
	bool __fastcall Disconnect();

	int __fastcall WriteData(const void * src, unsigned int len);
	int __fastcall ReadData(void * dst, unsigned int len);
	int __fastcall CheckData();

	int __fastcall SetTimeouts(const COMMTIMEOUTS &to);
	int __fastcall Flush();

    static int __fastcall FindDevices(TStrings * Dest);
    static int __fastcall FindDevicesVXI(TStrings * Dest);

	__property bool Active = { read = getActive };

__published:

	__property bool Sync = {read = FSync, write = setSync, default = true };
	__property TSerialPortRTSCtrl RTSControl = {read = FRTSControl, write = setRTSControl, default = sprcNone };
	__property TSerialPortDriverType DriverType = {read = FDriverType, write = setDriverType, default = spdtNone };

	__property int UpdatePeriod = { read = FUpdatePeriod, write = setUpdatePeriod, default = 0 };

	__property unsigned int Baudrate = { read = getBaudrate, write = setBaudrate, default = 115200};
	__property unsigned short DataBits = { read = getDataBits, write = setDataBits, default = 8};
	__property unsigned short StopBits = { read = getStopBits, write = setStopBits, default = 0};
	__property unsigned short Parity = { read = getParity, write = setParity, default = comParityNone};


	__property TNotifyEvent OnConnect = { read = FOnConnect, write = FOnConnect };
	__property TNotifyEvent OnDisconnect = { read = FOnDisconnect, write = FOnDisconnect };
	__property TIODataEvent OnDataReceive = { read = FOnDataReceive, write = FOnDataReceive };
	__property TIODataEvent OnDataTransmit = { read = FOnDataTransmit, write = FOnDataTransmit };
	__property TFailEvent  OnError = { read = FOnError, write = FOnError };
};

#endif // __BORLANDC__



//-----------------------------------------------------------------------------
#if defined (QT_CORE_LIB)

//-----------------------------------------------------------------------------
// QSerialPort class for Qt compiler
//-----------------------------------------------------------------------------

#include <QStringList>

class QSerialPort: public QObject
{
	Q_OBJECT
private:
protected:

    CBaseSerialPort * device;

    void emitDeviceClassError();

public:
    QSerialPort(QObject * parent = 0);
	~QSerialPort();

    int writeData(const void * src, unsigned int len);
    int readData(void * dst, unsigned int len);
    int checkData();

    static int findDevices(QStringList & dst);
    static int findDevicesVXI(QStringList & dst);

    void setBaudrate(int b);
	int getBaudrate();
    bool isActive();

public slots:
    bool activate();
    bool activate(const QString &a);
    bool deactivate();

signals:
	void activated();
	void deactivated();
	void received(int);
	void transmitted(int);
	void crashed(int);
	void crashed(const QString &);

};

#endif // QT_CORE_LIB


//-----------------------------------------------------------------------------
#endif // __SERIALPORT_H

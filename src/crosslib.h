//***************************************************************************
//** crosslib.h                                                            **
//** CrossLib v 2.0                                                        **
//** Now suppoted OS                                                       **
//**   Windows                                                             **
//**   Ubuntu 12.04                                                        **
//**   Debian 6.0, 7.0                                                     **
//**                                                                       **
//** Now supports comilers:                                                **
//**   Borland C (6.0, XE)                                                 **
//**   Qt Creator 4.8, 5.0                                                 **
//**                                                                       **
//**   Autor: Pavel Brain                                                  **
//**   Last modified: 27-04-2017                                           **
//***************************************************************************

#ifndef __CROSSLIB_H
#define __CROSSLIB_H

// Operated definition:
// #define 	_WIN32 	= OS Windows
// #define  __linux__ 	= OS linux
// #define __APPLE__	= OS Macintosh
// #define __BORLANDC__ = BC compiler
// #define QT_CORE_LIB	= Qt IDE (as usual GCC compiler)
// #define __GNUC__		= GCC compiler
// #define __STDC__		= Standart C compiler
// #define UNICODE		= Use wide strings


// Cross compiler definitions
//---------------------------------------------------------------------------
#if defined (__BORLANDC__)
#  	include <SysUtils.hpp>
#  	include <Classes.hpp>
#endif

//---------------------------------------------------------------------------
#if defined(QT_CORE_LIB)
#   include <QString>
#   include <QObject>
#   if QT_VERSION >= 0x050000
#       define toAscii toLatin1
#   endif
#endif


// Cross lib definitions

// crosslib_Sleep
#if defined (_WIN32)
//#	include <Windows.h>
#   define crosslib_Sleep(ms) 	Sleep(ms)

#elif defined (__linux__)
//#   include <unistd.h>
#   define crosslib_Sleep(ms) 	usleep((ms) * 1000)
#else
#   define crosslib_Sleep(ms) (void)
#endif

// crosslib_IDLE
#if defined (__BORLANDC__)
#	include <Forms.hpp>
#   define crosslib_IDLE() 		Application->ProcessMessages()
#elif defined (QT_CORE_LIB)
#   include <QApplication>
#   define crosslib_IDLE() 		QApplication::processEvents()
#elif defined (_WIN32)
#   define crosslib_IDLE() 		(void)
#else
#   define crosslib_IDLE()      (void)
#endif

//---------------------------------------------------------------------------
#ifndef __VECTORS_DEFINITIONS
#   define __VECTORS_DEFINITIONS

#   include <vector>
#   include <complex>
#   include <map>
#   include <string>
#   include <algorithm>

    typedef std::complex<double>        COMPLEX;
    typedef std::vector<unsigned char>  UCVECTOR;
    typedef std::vector<float>          FVECTOR;
    typedef std::vector<double>         DVECTOR;
    typedef std::vector<std::string>    SVECTOR;
    typedef std::vector<std::wstring>   WSVECTOR;
    typedef std::vector<int>            IVECTOR;
    typedef std::vector<std::complex<double> > CVECTOR;
    typedef std::map<double, std::map<double, double> > MAP3D;

#endif // __VECTORS_DEFINITIONS

// strings definition
//#if defined (__BORLANDC__)
//#   if defined (UNICODE)
//#       define cpcString	WideString
//#   else
//#       define cpcString	AnsiString
//#   endif
////  #define cpcString	String
//#elif defined (QT_CORE_LIB)
//#   define cpcString		QString
//#endif

//---------------------------------------------------------------------------
// Some function headers
typedef void (*pfProc)(void);
typedef void (*pfProcWithInt)(int);
typedef void (*pfProcWithPtr)(void *);

#endif // __CROSSLIB_H

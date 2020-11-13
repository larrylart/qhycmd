#ifndef _QHYUTILS_H
#define _QHYUTILS_H

// system headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <cstdarg>
#include <stdarg.h>

// wx includes
#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

void argLogMsg( const char* str_type, const char* str_section, const char* str_msg, ... );
void LogMsg( const char* str_type, const char* str_section, const char* str_msg, va_list args );

// define some macro functions
#define _LogMsg(Type,Message,...) argLogMsg(Type,__FUNCTION__,Message,## __VA_ARGS__)
#define _LogInfo(Message,...) argLogMsg("INFO",__FUNCTION__,Message,## __VA_ARGS__)
#define _LogError(Message,...) argLogMsg("ERROR",__FUNCTION__,Message,## __VA_ARGS__)

wxString getTimeToComplete( int no, double exp_sec, double proc_sec=2.0 );
wxString getTimeWhenComplete( int no, double exp_sec, double proc_sec=2.0 );

#endif

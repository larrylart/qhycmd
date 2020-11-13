////////////////////////////////////////////////////////////////////
// Just some simple helper function for logging etc
////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <time.h>
#include <stdint.h>

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// WX :: includes
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/utils.h> 
#include <wx/dir.h>
#include <wx/filefn.h> 
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

// include local
#include "qhycmd.h"
#include "qhy_utils.h"

//externals
DECLARE_APP(CQhyCmd)

// logger with arguments
////////////////////////////////////////////////////////////////////
void argLogMsg( const char* str_type, const char* str_section, const char* str_msg, ... )
{	
    va_list args;
    va_start(args,str_msg);
	LogMsg( str_type, str_section, str_msg, args );
	va_end(args);	
}

// base logger fucntion
////////////////////////////////////////////////////////////////////
void LogMsg( const char* str_type, const char* str_section, const char* str_msg, va_list args )
{
	wchar_t strBuff[5120];
	memset(&strBuff, 0, 5120*sizeof(wchar_t));
	//vsprintf( strBuff, str_msg, args);
	wxVsnprintf( strBuff, 5120, str_msg, args );
	
	wxString strMsg = wxString(strBuff);
	wxDateTime timeStamp = wxDateTime::Now();

	// skip info logs if not debug
	if( !g_bLogDebug && strcmp(str_type, "INFO") == 0 ) return;	
	
	printf("%s :%s:%s: %s\n", timeStamp.Format("%Y/%m/%d %H:%M:%S").GetData().AsChar(), str_type, str_section, strMsg.GetData().AsChar() );
}

// get hh:mm:ss to complete
////////////////////////////////////////////////////////////////////
wxString getTimeToComplete( int no, double exp_sec, double proc_sec )
{
	long finish_time = round(no*(exp_sec+proc_sec));
	int hh = (int) (finish_time/3600);
	int mm = (int) ((finish_time%3600)/60);
	int ss = (int) (finish_time%60);

	return( wxString::Format(wxT("%02d:%02d:%02d"), hh, mm, ss) );
}

// get hh:mm:ss when done
////////////////////////////////////////////////////////////////////
wxString getTimeWhenComplete( int no, double exp_sec, double proc_sec )
{
	unsigned long finish_time = round(no*(exp_sec+proc_sec));
	time_t time_now = wxDateTime::GetTimeNow()+finish_time;
	wxDateTime timeStamp(time_now);
		
	return( timeStamp.Format("%Y/%m/%d %H:%M:%S ") );	
}

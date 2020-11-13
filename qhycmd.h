#ifndef _QHYCMD_H
#define _QHYCMD_H

// WX :: includes
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/thread.h>
#include <wx/cmdline.h>

// global vars
extern bool g_bLogDebug;
	
// external classes
class CQhyCamera; 

// namespaces
using namespace std;

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, "h", "help", "Show this help message", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		
	{ wxCMD_LINE_OPTION, "e", "exposure", "Exposure time (seconds)", wxCMD_LINE_VAL_DOUBLE },
	{ wxCMD_LINE_OPTION, "cg", "gain", "Chip gain", wxCMD_LINE_VAL_NUMBER },
	{ wxCMD_LINE_OPTION, "co", "offset", "Chip offset", wxCMD_LINE_VAL_NUMBER },
	{ wxCMD_LINE_OPTION, "b", "bpp", "Bits per pixel:8, 16(default)", wxCMD_LINE_VAL_NUMBER },
	{ wxCMD_LINE_OPTION, "no", "expno", "Number of images to record", wxCMD_LINE_VAL_NUMBER },
	
	{ wxCMD_LINE_OPTION, "c", "cool", "Chip cool down temp", wxCMD_LINE_VAL_DOUBLE },
	{ wxCMD_LINE_SWITCH, "wt", "waittemp", "Wait for sensor to reach temperature" },
	
	{ wxCMD_LINE_OPTION, "bin", "binning", "binning 1x1, 2x2, etc", wxCMD_LINE_VAL_STRING },
	{ wxCMD_LINE_OPTION, "rm", "readmode", "Set camera read mode", wxCMD_LINE_VAL_NUMBER },
	{ wxCMD_LINE_SWITCH, "nos", "nooverscan", "Remove overscan area" },
	
	{ wxCMD_LINE_OPTION, "f", "folder", "Folder where to save images", wxCMD_LINE_VAL_STRING },
	{ wxCMD_LINE_OPTION, "t", "target", "Name of the target", wxCMD_LINE_VAL_STRING },
	{ wxCMD_LINE_OPTION, "fmt", "format", "Image output format (fits,tif,jpg,png)", wxCMD_LINE_VAL_STRING },
	
	{ wxCMD_LINE_SWITCH, "lc", "listcameras", "List available cameras" },
	{ wxCMD_LINE_SWITCH, "lr", "listreadmodes", "List camera read modes" },
	
	{ wxCMD_LINE_SWITCH, "d", "debug", "Run in debug mode" },
	{ wxCMD_LINE_SWITCH, "gui", "display", "Run with GUI mode" },
	{ wxCMD_LINE_NONE }
};

// class: CQhyCmd
class CQhyCmd : public wxAppConsole
{
public:
    CQhyCmd();
    virtual ~CQhyCmd(){};

    virtual bool OnInit();
	virtual int OnRun();
	virtual int OnExit();
	
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

	// save/load configuration
	int		SaveCfg();
	int		LoadCfg();

// data
public:

	bool m_bWaitForTemp;
	
	long int m_nNoOfImages;

	CQhyCamera*	m_pQhyCamera;
	
	wxString	m_strUserDir;
	wxString	m_strConfigFile;
	wxString	m_strSaveFolder;
	wxString	m_strImageFormat;

	wxString	m_strObserverName;
	wxString	m_strTargetName;
	
	bool m_bIsCameraList;
	bool m_bIsReadModesList;
	bool m_bRemoveOverScan;
	
	bool m_bIsGUI;
	bool m_bDisplay;	

};


#endif

////////////////////////////////////////////////////////////////////
// QHYCmd : A very simple command line tool used to take 
//			a series of images with with given arguments and save 
//			in standard formats, fits, tif etc 
// Created by: Larry Lart based on QHY SDK sample
////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

// WX :: includes
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/utils.h> 
#include <wx/dir.h>
#include <wx/filefn.h> 
#include <wx/filename.h>
#include <wx/stdpaths.h>

// local includes
#include "qhy_utils.h"
#include "qhy_camera.h"
#include "astro_image.h"

// main header
#include "qhycmd.h"

IMPLEMENT_APP_CONSOLE(CQhyCmd);

// globals
bool g_bLogDebug = false;
wxString g_strUserAppDataPath;

// class:	CQhyCmd
////////////////////////////////////////////////////////////////////
CQhyCmd::CQhyCmd()
{
	m_pQhyCamera = NULL;
	
	m_strObserverName = _T("");
	m_strSaveFolder = _T("");
	m_strTargetName = _T("qhyimage");
	m_strImageFormat = _T("FIT");
	m_strConfigFile = _T("qhycmd.ini");
	
	m_bIsCameraList = false; 
	m_bIsReadModesList = false;
	m_bIsGUI = false;
	m_bDisplay = false;

	m_bRemoveOverScan = false;
	m_bWaitForTemp = false;
	
	m_nNoOfImages = 1;	
}

// cleanup on exit
////////////////////////////////////////////////////////////////////
int CQhyCmd::OnExit()
{	
	if( m_pQhyCamera != NULL ) delete(m_pQhyCamera);
	
	return(0);
}

////////////////////////////////////////////////////////////////////
void CQhyCmd::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

////////////////////////////////////////////////////////////////////
bool CQhyCmd::OnCmdLineParsed(wxCmdLineParser& parser)
{
	//printf( "CQhyCmd :: parse arguments\n" );
	wxString strTmp = _T("");
	long int nTmp = 0;
	double fTmp = 0;
	
	// :: check if camera object is available
	if( m_pQhyCamera != NULL )
	{
		// set temperature
		if( parser.Found( "c", &fTmp ) && fTmp < 100 ) m_pQhyCamera->m_nCcdTargetTemp = fTmp;
		
		parser.Found( "cg", &m_pQhyCamera->m_nChipGain );
		parser.Found( "co", &m_pQhyCamera->m_nChipOffset );
		
		// :: exposure time
		if( parser.Found( "e", &m_pQhyCamera->m_nExposureTimeSec ) )
		{
			m_pQhyCamera->m_nExposureTime = (unsigned long) (m_pQhyCamera->m_nExposureTimeSec * 1000000.0);	
		}		
		
		// set read mode
		if( parser.Found( "rm", &nTmp ) && nTmp >= 0 ) m_pQhyCamera->m_nCameraReadMode = (int) nTmp;			
	}	
	
	parser.Found( "no", &m_nNoOfImages );
	
	// :: path to save simages
	if( parser.Found( "f", &strTmp ) && !strTmp.IsEmpty() ) 
	{
		if( !wxIsAbsolutePath(strTmp) ) strTmp = wxGetCwd() + "/" + strTmp;
		m_strSaveFolder = strTmp;
	}
	
	// target name - image file base name
	if( parser.Found( "t", &strTmp ) && !strTmp.IsEmpty() ) m_strTargetName = strTmp;	

	//printf("DEBUG :: exp=%lu exps=%.4f", m_nExposureTime, m_nExposureTimeSec );
	//exit(0);
	

	// image format
	wxString strFormat;
	if( parser.Found( "fmt", &strFormat ) ) 
	{
		if( strFormat.Upper().IsSameAs("FIT") ||  strFormat.Upper().IsSameAs("FIST") )
			m_strImageFormat = _T("FIT");
		else if( strFormat.Upper().IsSameAs("TIF") ||  strFormat.Upper().IsSameAs("TIFF") )
			m_strImageFormat = _T("TIF");
		else if( strFormat.Upper().IsSameAs("JPG") ||  strFormat.Upper().IsSameAs("JPEG") )
			m_strImageFormat = _T("JPG");
		else if( strFormat.Upper().IsSameAs("PNG") )
			m_strImageFormat = _T("PNG");			
	}

	// set if no overscan 
	if( parser.FoundSwitch( "nos" ) ) m_bRemoveOverScan = true; 

	// :: LIST AVAILABLE CAMERAS
	if( parser.FoundSwitch( "lc" ) ) m_bIsCameraList = true; 

	// :: LIST READ MODES
	if( parser.FoundSwitch( "lr" ) ) m_bIsReadModesList = true; 

	// :: WAIT FOR SENSOR TO REACH TEMPERATURE
	if( parser.FoundSwitch( "wt" ) ) m_bWaitForTemp = true; 
	
	// :: DEBUG
	if( parser.FoundSwitch( "d" ) ) g_bLogDebug = true; 
	// :: GUI
	if( parser.FoundSwitch( "gui" ) )
	{ 		
		m_bIsGUI = true;
		m_bDisplay = true;
	}
	 
    return( true );
}

// qhycmd main app init
////////////////////////////////////////////////////////////////////
bool CQhyCmd::OnInit()
{
	//printf( "CQhyCmd :: START INIT\n" );
		
	m_strUserDir = wxStandardPaths::Get().GetUserDataDir();
	//printf( "DEBUG :: user path=%s\n", m_strUserDir.GetData().AsChar() );
	
	// set config file name
	m_strConfigFile.Printf( wxT("%s/qhycmd.ini"), m_strUserDir );
	
	// check and create user path for the ini file
	if( !wxDirExists(m_strUserDir) ) wxFileName::Mkdir(m_strUserDir, 0777, wxPATH_MKDIR_FULL); 
	//exit(0);	

	// create camera object
	m_pQhyCamera = new CQhyCamera();
	
	// load config
	LoadCfg();

	// this will call the argument parser
    if( !wxAppConsole::OnInit() ) return( false );		
	wxThread::SetConcurrency(2);	

	// save config - after command line processing
	SaveCfg();	

	// initialize camera
	if( m_pQhyCamera->Init(true) != 0 )
	{
		printf( "ERROR :: failed to initialize camera\n" );
		return(false);
	}

	// just display a list of available cameras
	if( m_bIsCameraList )
	{
		m_pQhyCamera->getCameraList(); 
		delete(m_pQhyCamera); 
		return(false);
	}

	// just display readmodes and exit
	if( m_bIsReadModesList )
	{
		m_pQhyCamera->GetReadModes(); 
		delete(m_pQhyCamera); 
		return(false);
	}

	// if read mode set >=0 calll to set
	if( m_pQhyCamera->m_nCameraReadMode >= 0 ) m_pQhyCamera->SetReadMode(m_pQhyCamera->m_nCameraReadMode);

	m_pQhyCamera->SetToSingleFrame();
	m_pQhyCamera->GetExtraInfo();
	m_pQhyCamera->SetUsbTraffic( m_pQhyCamera->m_nUsbTraffic );

	m_pQhyCamera->SetCameraGain( m_pQhyCamera->m_nChipGain );
	m_pQhyCamera->SetCameraOffset( m_pQhyCamera->m_nChipOffset );
	m_pQhyCamera->SetExposureTime( m_pQhyCamera->m_nExposureTime );
	m_pQhyCamera->SetCameraResolution();
	m_pQhyCamera->SetBinningMode(m_pQhyCamera->m_nCamBinX, m_pQhyCamera->m_nCamBinY);
	m_pQhyCamera->SetBitResolution(16);

	// check if to set target temperature
	if( m_pQhyCamera->m_nCcdTargetTemp < 100 && m_pQhyCamera->m_nCcdTargetTemp >= -40 )
	{
		m_pQhyCamera->SetCCDTemp( m_pQhyCamera->m_nCcdTargetTemp );
	}
	
	return( true );	
}

////////////////////////////////////////////////////////////////////
// CQhyCmd main app loop
int CQhyCmd::OnRun()
{
	// check if destination folder exist
	if( !m_strSaveFolder.IsEmpty() && !wxDirExists(m_strSaveFolder) )
	{
		wxFileName::Mkdir(m_strSaveFolder, 0777, wxPATH_MKDIR_FULL);
	}
	
	////////////////
	// check if to wait for ccd to reach target temperature 
	if( m_bWaitForTemp )
	{
		double ccd_temp = m_pQhyCamera->GetCCDTemp();
		double temp_diff = m_pQhyCamera->m_nCcdTargetTemp - ccd_temp;
		if( abs(temp_diff) < 1.0 )
		{
			printf( "INFO :: sensor temperature=%.2f\n", ccd_temp );
			
		} else
		{
			printf( "INFO :: waiting for sensor to reach target temperature=%.2f ...\n", m_pQhyCamera->m_nCcdTargetTemp );
			while( abs(temp_diff) >= 1.0 )
			{
				sleep(1);
				ccd_temp = m_pQhyCamera->GetCCDTemp();
				temp_diff = m_pQhyCamera->m_nCcdTargetTemp - ccd_temp;				
				printf( "INFO :: sensor temperature at=%.2f to reach target=%.2f\n", ccd_temp, m_pQhyCamera->m_nCcdTargetTemp );
			}
			printf( "INFO :: sensor temperature=%.2f has reach target=%.2f within a degree tolerance\n", ccd_temp, m_pQhyCamera->m_nCcdTargetTemp );
			
		}
		
	} else
	{
		double ccd_temp = m_pQhyCamera->GetCCDTemp();
		printf( "INFO :: sensor temperature=%f\n", ccd_temp );
	}
		
	// start imaging - add a tolerance for camera command and conversion saving of 2 sec
	printf( "INFO :: START IMAGING=%d, time to completion: %s at %s\n", (int) m_nNoOfImages, 
				getTimeToComplete(m_nNoOfImages,m_pQhyCamera->m_nExposureTimeSec).GetData().AsChar(),
				getTimeWhenComplete(m_nNoOfImages,m_pQhyCamera->m_nExposureTimeSec).GetData().AsChar() );
	
	///////////////////////
	// do no of frames
	for( int i=1; i<=m_nNoOfImages; i++ ) 
	{
		CAstroImage rAstroImage;
		
		// first frame don't display remaing time as already in the header
		if( i == 1 )
			printf( "INFO :: TAKE IMAGE=%d\n", i );
		else
			printf( "INFO :: TAKE IMAGE=%d, time to completion: %s\n", i, getTimeToComplete((m_nNoOfImages-i),m_pQhyCamera->m_nExposureTimeSec).GetData().AsChar() );
		
		m_pQhyCamera->TakeSingleExposure( &rAstroImage );
				
		// make image name with counter
		char img_file[255];
		if( !m_strSaveFolder.IsEmpty() )
			sprintf( img_file, "%s/%s_%d", m_strSaveFolder.GetData().AsChar(), m_strTargetName.GetData().AsChar(), i );	
		else		
			sprintf( img_file, "%s_%d", m_strTargetName.GetData().AsChar(), i );		
		
		// save image
		if( m_bRemoveOverScan ) rAstroImage.RemoveOverscan(true);
		rAstroImage.SaveImage( img_file, m_strImageFormat, m_pQhyCamera  );
		
		// if to display
		if( m_bIsGUI && m_bDisplay )
		{
			// get 24b rgb for display
			cv::Mat* mat8uc3_rgb = rAstroImage.GetRGB24();
			cv::Mat rgb8BitMatShow;
			cv::resize(*mat8uc3_rgb, rgb8BitMatShow, cv::Size(), 0.20, 0.20);
			imshow("qhy image", rgb8BitMatShow);
			cv::waitKey(0);
		}

	}
	// end of loop to no of images
	
	return( 0 );
}

////////////////////////////////////////////////////////////////////
// Save configuration
////////////////////////////////////////////////////////////////////
int CQhyCmd::SaveCfg()
{
	FILE* pFile = NULL;	
	
	// open config file to write
	pFile = wxFopen( m_strConfigFile, wxT("w") );	

	wxFprintf( pFile, wxT("ObserverName=%s\n"), m_strObserverName.GetData().AsChar() );
	wxFprintf( pFile, wxT("TargetName=%s\n"), m_strTargetName.GetData().AsChar() );

	wxFprintf( pFile, wxT("CameraName=%s\n"), m_pQhyCamera->m_strCameraName.GetData().AsChar() );
	
	wxFprintf( pFile, wxT("Exposure=%lu\n"), m_pQhyCamera->m_nExposureTime );
	wxFprintf( pFile, wxT("ChipGain=%d\n"), m_pQhyCamera->m_nChipGain );
	wxFprintf( pFile, wxT("ChipOffset=%d\n"), m_pQhyCamera->m_nChipOffset );
	wxFprintf( pFile, wxT("UsbTraffic=%d\n"), m_pQhyCamera->m_nUsbTraffic );
	
	wxFprintf( pFile, wxT("CamBinX=%d\n"), m_pQhyCamera->m_nCamBinX );
	wxFprintf( pFile, wxT("CamBinY=%d\n"), m_pQhyCamera->m_nCamBinY );

	wxFprintf( pFile, wxT("CcdTargetTemp=%lf\n"), m_pQhyCamera->m_nCcdTargetTemp );
	
	// close my file handler
	fclose( pFile );

	return( 1 );	
}

////////////////////////////////////////////////////////////////////
// Load configuration
////////////////////////////////////////////////////////////////////
int CQhyCmd::LoadCfg()
{
	FILE* pFile = NULL;
	wxChar strLine[2000];
	double nVarFloatValue = 0;
	long nVarIntValue = 0;
	
	wxRegEx reVariable( wxT( "([a-zA-Z0-9_\\-]+)\\ *=\\ *([^\n^\r^\t]+)\\ *" ) );	
	
	// Open config file to read
	pFile = wxFopen( m_strConfigFile, wxT("r") );	
	
		// check if there is any configuration to load
	if( !pFile ) return( 0 );

	// Reading lines from cfg file
	while( !feof( pFile ) )
	{
		nVarIntValue = 0;
		nVarFloatValue = 0;		
		memset( strLine, 0, 2000*sizeof(wxChar) );
		 
		wxFgets( strLine, 2000, pFile );
		
		// skip if not a variable definition
		if( !reVariable.Matches( strLine ) ) continue;
		// get var and value string
		wxString strVarName = reVariable.GetMatch(strLine, 1 );
		wxString strVarValue = reVariable.GetMatch(strLine, 2 );

		// set by name
		if( strVarName.IsSameAs("ObserverName",false) ) 
			m_strObserverName = strVarValue; 
		else if( strVarName.IsSameAs("TargetName",false) ) 
			m_strTargetName = strVarValue; 
		else if( strVarName.IsSameAs("CameraName",false) ) 
			m_pQhyCamera->m_strCameraName = strVarValue; 
		else if( strVarName.IsSameAs("Exposure",false) ) 
			m_pQhyCamera->m_nExposureTime = (unsigned long) wxAtol(strVarValue);
		else if( strVarName.IsSameAs("ChipGain",false) ) 
			m_pQhyCamera->m_nChipGain = (long int) wxAtol(strVarValue);
		else if( strVarName.IsSameAs("ChipOffset",false) ) 
			m_pQhyCamera->m_nChipOffset = (long int) wxAtol(strVarValue);
		else if( strVarName.IsSameAs("UsbTraffic",false) ) 
			m_pQhyCamera->m_nUsbTraffic = (long int) wxAtol(strVarValue);
		else if( strVarName.IsSameAs("CamBinX",false) ) 
			m_pQhyCamera->m_nCamBinX = wxAtoi(strVarValue);
		else if( strVarName.IsSameAs("CamBinY",false) ) 
			m_pQhyCamera->m_nCamBinY = wxAtoi(strVarValue);
		else if( strVarName.IsSameAs("CcdTargetTemp",false) ) 
			m_pQhyCamera->m_nCcdTargetTemp = (double) wxAtof(strVarValue);
		
	}
	
	// close file
	fclose( pFile );

	return( 1 );	
}



////////////////////////////////////////////////////////////////////
// base class interface for qhy cameras
// Created by: Larry Lart based on QHY SDK sample
////////////////////////////////////////////////////////////////////
#ifndef _QHYCAMERA_H
#define _QHYCAMERA_H

// base
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>

#include "qhyccd.h"

#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

// namespaces in use
using namespace std;

// external classes
class CAstroImage;

/////////////////////////////////////////////////
class CQhyCamera 
{
// methods
public:
	CQhyCamera();
	~CQhyCamera();
	
	int Init(bool bAll=false);
	int getCameraList();
	
	void SDKVersion();
	void FirmWareVersion( qhyccd_handle *h );
	
	int GetReadModes();
	int SetReadMode( unsigned int read_mode );
	
	double GetCCDTemp();
	int SetCCDTemp( double ccd_temp );
	
	int SetToSingleFrame();
	int GetExtraInfo();
	int SetUsbTraffic( int _usb_traffic );
	
	int SetCameraGain( int _gain );
	int SetCameraOffset( int _offset );
	int SetExposureTime( int _exposure_time );
	
	int SetCameraResolution();
	int _SetCameraResolution( int _roiStartX, int _roiStartY, int _roiSizeX, int _roiSizeY );
	
	int SetBinningMode( int _camBinX, int _camBinY );
	int SetBitResolution( int _bit_resolution );
	
	int TakeSingleExposure( CAstroImage* pAstroImage );
	
/////////
// data
public:

	char m_camId[32];
	qhyccd_handle *m_pCamHandle;

	bool m_bIsColor;

	double chipWidthMM;
	double chipHeightMM;
	double pixelWidthUM;
	double pixelHeightUM;

	unsigned int roiStartX;
	unsigned int roiStartY;
	unsigned int roiSizeX;
	unsigned int roiSizeY;

	unsigned int overscanStartX;
	unsigned int overscanStartY;
	unsigned int overscanSizeX;
	unsigned int overscanSizeY;

	unsigned int effectiveStartX;
	unsigned int effectiveStartY;
	unsigned int effectiveSizeX;
	unsigned int effectiveSizeY;

	unsigned int maxImageSizeX;
	unsigned int maxImageSizeY;
	unsigned int bpp;
	unsigned int channels;

	wxString m_strCameraName;

	unsigned int retVal;

	// image data containers
	unsigned char *pImgData;

	// other global params
	long int m_nUsbTraffic;
	long int m_nChipGain;
	long int m_nChipOffset;
	
	unsigned long m_nExposureTime;
	double m_nExposureTimeSec;
	
	int m_nCamBinX;
	int m_nCamBinY;

	double m_nCcdTemp;
	double m_nCcdTargetTemp;
	
	int m_nCameraReadMode;
	
};

#endif

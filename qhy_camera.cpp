////////////////////////////////////////////////////////////////////
// base class interface for qhy cameras
// Created by: Larry Lart based on QHY SDK sample
////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstdarg>

// WX :: includes
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/utils.h> 
#include <wx/tokenzr.h>
#include <wx/datetime.h>

// local headers
#include "qhycmd.h"
#include "qhy_utils.h"
#include "astro_image.h"
#include "qhy_camera.h"

//externals
DECLARE_APP(CQhyCmd)

// constructor
////////////////////////////////////////////////////////////////////
CQhyCamera::CQhyCamera()
{	
	m_nUsbTraffic = 10;
	m_nChipGain = 10;
	m_nChipOffset = 140;
	// in microseconds
	m_nExposureTime = 20000;
	// in seconds
	m_nExposureTimeSec = (double) m_nExposureTime/1000000.0;
	m_nCamBinX = 1;
	m_nCamBinY = 1;
	m_nCameraReadMode = -1;	
	m_nCcdTargetTemp = 999;
	
	m_strCameraName = wxT("");
	
	m_pCamHandle = 0;
	pImgData = 0;
	
	m_bIsColor = false;
	
	m_nCcdTemp = 0.0;
//	m_nCcdTargetTemp = 0.0;
	
	//Init(true);
}

// destructor
////////////////////////////////////////////////////////////////////
CQhyCamera::~CQhyCamera( )
{	
	// close camera handle
	retVal = CloseQHYCCD(m_pCamHandle);
	if(retVal == QHYCCD_SUCCESS)
		_LogInfo("Close QHYCCD success.");
	else
		_LogError("Close QHYCCD failure, error: %d", retVal);

	// release sdk resources
	retVal = ReleaseQHYCCDResource();
	if (QHYCCD_SUCCESS == retVal)
		_LogInfo("SDK resources released.");
	else
		_LogError("Cannot release SDK resources, error %d.", retVal);
	
}

// just list the cameras
////////////////////////////////////////////////////////////////////
int CQhyCamera::getCameraList()
{ 	
	// init SDK
	unsigned int retVal = InitQHYCCDResource();
	if( QHYCCD_SUCCESS == retVal )
	{
		_LogInfo("SDK resources initialized.");

	} else
	{
		_LogError("Cannot initialize SDK resources, error: %d", retVal);
		return( 1 );
	}

	// scan cameras
	int camCount = ScanQHYCCD();
	if( camCount > 0 )
	{
		_LogInfo( "Number of QHYCCD cameras found: %d", camCount );
		printf( "Number of QHYCCD cameras found: %d\n", camCount );
		
	} else
	{
		_LogError("No QHYCCD camera found, please check USB or power.");
		return( 1 );
	}

	// loop thourgh available cameras and get their name for display
	for( int i = 0; i < camCount; i++ )
	{
		retVal = GetQHYCCDId(i, m_camId);
		m_strCameraName = wxString(m_camId).BeforeFirst('-');
		
		printf( "[%d] :: %s (%s)\n", i+1, m_strCameraName.GetData().AsChar(), m_camId );
		
	}

	return(0);
}

// initialization function
////////////////////////////////////////////////////////////////////
int CQhyCamera::Init( bool bAll )
{ 
	if( bAll ) SDKVersion();
	
	// init SDK
	unsigned int retVal = InitQHYCCDResource();
	if( QHYCCD_SUCCESS == retVal )
	{
		_LogInfo("SDK resources initialized.");

	} else
	{
		_LogError("Cannot initialize SDK resources, error: %d", retVal);
		return( 1 );
	}

	// scan cameras
	int camCount = ScanQHYCCD();
	if( camCount > 0 )
	{
		_LogInfo("Number of QHYCCD cameras found: %d", camCount);
		
	} else
	{
		_LogError("No QHYCCD camera found, please check USB or power.");
		return( 1 );
	}

	// iterate over all attached cameras
	bool camFound = false;

	for( int i = 0; i < camCount; i++ )
	{
		retVal = GetQHYCCDId(i, m_camId);
		if (QHYCCD_SUCCESS == retVal)
		{
			//  extract camera name
			m_strCameraName = wxString(m_camId).BeforeFirst('-');
			//_LogInfo("Camera name=%s", m_strCameraName.GetData().AsChar());
			
			_LogInfo("Application connected to the following camera from the list: Index: %d,  cameraID = %s", (i + 1), m_camId);					
			camFound = true;
			break;
		}
	}

	if( !camFound )
	{
		_LogError("The detected camera is not QHYCCD or other error.");
		// release sdk resources
		retVal = ReleaseQHYCCDResource();
		if (QHYCCD_SUCCESS == retVal)
		{
			_LogInfo("SDK resources released.");
		} else
		{
			_LogError("Cannot release SDK resources, error %d.", retVal);
		}
		return( 1 );
	}

	// open camera
	m_pCamHandle = OpenQHYCCD( m_camId );
	if( m_pCamHandle != NULL )
	{
		_LogInfo( "Open QHYCCD success." );
		
	} else
	{
		_LogError( "Open QHYCCD failure." );
		return( 1 );
	}	

	if( bAll ) FirmWareVersion( m_pCamHandle );

	return( 0 );
}

// get camera extra info such as chip size etc
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetToSingleFrame()
{
	// check camera support single frame
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CAM_SINGLEFRAMEMODE);
	if( retVal == QHYCCD_ERROR )
	{
		_LogError("The detected camera does not support single frame.");
		// release sdk resources
		retVal = ReleaseQHYCCDResource();
		if (QHYCCD_SUCCESS == retVal)
			_LogInfo("SDK resources released.");
		else
			_LogError("Cannot release SDK resources, error %d.", retVal);
		return( 1 );
	}

	// set single frame mode
	int mode = 0;
	retVal = SetQHYCCDStreamMode(m_pCamHandle, mode);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo("SetQHYCCDStreamMode set to: %d, success.", mode);

	} else
	{
		_LogError("SetQHYCCDStreamMode: %d failure, error: %d", mode, retVal);
		return( 1 );
	}

	// initialize camera
	retVal = InitQHYCCD(m_pCamHandle);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo("InitQHYCCD success.");
		
	} else
	{
		_LogError("InitQHYCCD faililure, error: %d", retVal);
		return( 1 );
	}

	// get overscan area
	retVal = GetQHYCCDOverScanArea(m_pCamHandle, &overscanStartX, &overscanStartY, &overscanSizeX, &overscanSizeY);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo("GetQHYCCDOverScanArea:");
		_LogInfo("Overscan Area startX x startY : %d x %d", overscanStartX, overscanStartY);
		_LogInfo("Overscan Area sizeX  x sizeY  : %d x %d", overscanSizeX, overscanSizeY);
		
	} else
	{
		_LogError("GetQHYCCDOverScanArea failure, error: %d", retVal);
		return( 1 );
	}

	// get effective area
	retVal = GetQHYCCDEffectiveArea(m_pCamHandle, &effectiveStartX, &effectiveStartY, &effectiveSizeX, &effectiveSizeY);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo("GetQHYCCDEffectiveArea:");
		_LogInfo("Effective Area startX x startY: %d x %d", effectiveStartX, effectiveStartY);
		_LogInfo("Effective Area sizeX  x sizeY : %d x %d", effectiveSizeX, effectiveSizeY);
		
	} else
	{
		_LogError("GetQHYCCDOverScanArea failure, error: %d", retVal);
		return( 1 );
	}

	return( 0 );
}

// get camera extra info such as chip size etc
////////////////////////////////////////////////////////////////////
int CQhyCamera::GetExtraInfo()
{
	// get chip info
	retVal = GetQHYCCDChipInfo(m_pCamHandle, &chipWidthMM, &chipHeightMM, &maxImageSizeX, &maxImageSizeY, &pixelWidthUM, &pixelHeightUM, &bpp);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo("GetQHYCCDChipInfo:");
		//_LogInfo("Effective Area startX x startY: %d x %d", effectiveStartX, effectiveStartY);
		_LogInfo("Chip  size width x height     : %.3f x %.3f [mm]", chipWidthMM, chipHeightMM);
		_LogInfo("Pixel size width x height     : %.3f x %.3f [um]", pixelWidthUM, pixelHeightUM);
		_LogInfo("Image size width x height     : %d x %d", maxImageSizeX, maxImageSizeY);
		
	} else
	{
		_LogError("GetQHYCCDChipInfo failure, error: %d", retVal);
		return( 1 );
	}

	// set ROI
	roiStartX = 0;
	roiStartY = 0;
	roiSizeX = maxImageSizeX;
	roiSizeY = maxImageSizeY;

	// check color camera and get bayer pattern 
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CAM_COLOR);
	if (retVal == BAYER_GB || retVal == BAYER_GR || retVal == BAYER_BG || retVal == BAYER_RG)
	{
		m_bIsColor = true;
		_LogInfo("This is a color camera.");
		//_LogInfo("even this is a color camera, in Single Frame mode THE SDK ONLY SUPPORT RAW OUTPUT.So please do not set SetQHYCCDDebayerOnOff() to true;");
		//SetQHYCCDDebayerOnOff(m_pCamHandle, true);
		//SetQHYCCDParam(m_pCamHandle, CONTROL_WBR, 20);
		//SetQHYCCDParam(m_pCamHandle, CONTROL_WBG, 20);
		//SetQHYCCDParam(m_pCamHandle, CONTROL_WBB, 20);

	} else
	{
		_LogInfo("This is a mono camera.");
	}

	return( 0 );
}

// set camera usb traffic
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetUsbTraffic( int _m_nUsbTraffic )
{
	// check traffic
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CONTROL_USBTRAFFIC);
	if (QHYCCD_SUCCESS == retVal)
	{
		retVal = SetQHYCCDParam(m_pCamHandle, CONTROL_USBTRAFFIC, _m_nUsbTraffic);
		if( QHYCCD_SUCCESS == retVal )
		{
			m_nUsbTraffic = _m_nUsbTraffic;
			_LogInfo("SetQHYCCDParam CONTROL_USBTRAFFIC set to: %d, success.", _m_nUsbTraffic);
			
		} else
		{
			_LogError("SetQHYCCDParam CONTROL_USBTRAFFIC failure, error: %d", retVal);
			return( 1 );
		}
	}

	return( 0 );
}

// set camera gain
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetCameraGain( int _gain )
{
	// check gain
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CONTROL_GAIN);
	if (QHYCCD_SUCCESS == retVal)
	{
		retVal = SetQHYCCDParam(m_pCamHandle, CONTROL_GAIN, _gain);
		if (retVal == QHYCCD_SUCCESS)
		{
			m_nChipGain = _gain;
			_LogInfo("SetQHYCCDParam CONTROL_GAIN set to: %d, success", _gain);
			
		} else
		{
			_LogError("SetQHYCCDParam CONTROL_GAIN failure, error: %d", retVal);
			return( 1 );
		}
	}	
	
	return( 0 );
}

// set camera offset
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetCameraOffset( int _offset )
{	
	// check offset
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CONTROL_OFFSET);
	if (QHYCCD_SUCCESS == retVal)
	{
		retVal = SetQHYCCDParam( m_pCamHandle, CONTROL_OFFSET, _offset );
		if( QHYCCD_SUCCESS == retVal )
		{
			m_nChipOffset = _offset;			
			_LogInfo( "SetQHYCCDParam CONTROL_GAIN set to: %d, success.", _offset );
			
		} else
		{
			_LogError( "SetQHYCCDParam CONTROL_GAIN failed." );
			return( 1 );
		}
	}	
	  
	return( 0 );
}

// set exposure time
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetExposureTime( int _m_nExposureTime )
{	
	// set exposure time
	retVal = SetQHYCCDParam(m_pCamHandle, CONTROL_EXPOSURE, _m_nExposureTime);
	if (QHYCCD_SUCCESS == retVal)
	{
		m_nExposureTime = _m_nExposureTime;		
		_LogInfo("SetQHYCCDParam CONTROL_EXPOSURE set to: %d, success.", m_nExposureTime);		
		
	} else
	{
		_LogError( "SetQHYCCDParam CONTROL_EXPOSURE failure, error: %d", retVal );
		return( 1 );
	}
  
	return( 0 );
}

////////////////////////////////////////////////////////////////////
int CQhyCamera::SetCameraResolution()
{
	_SetCameraResolution( roiStartX, roiStartY, roiSizeX, roiSizeY );
}

// set camera resolution
////////////////////////////////////////////////////////////////////
int CQhyCamera::_SetCameraResolution( int _roiStartX, int _roiStartY, int _roiSizeX, int _roiSizeY )
{
	// set image resolution
	retVal = SetQHYCCDResolution(m_pCamHandle, _roiStartX, _roiStartY, _roiSizeX, _roiSizeY);
	if (QHYCCD_SUCCESS == retVal)
	{
		roiStartX = _roiStartX;
		roiStartY = _roiStartY;
		roiSizeX = _roiSizeX;
		roiSizeY =_roiSizeY;
		
		_LogInfo("SetQHYCCDResolution roiStartX x roiStartY: %d x %d", roiStartX, roiStartY);
		_LogInfo("SetQHYCCDResolution roiSizeX  x roiSizeY : %d x %d", roiSizeX, roiSizeY);
		
	} else
	{
		_LogError( "SetQHYCCDResolution failure, error: %d", retVal );
		return( 1 );
	}	
	
	return( 0 );
}

// set binning mode
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetBinningMode( int _m_nCamBinX, int _m_nCamBinY )
{
	// set binning mode
	retVal = SetQHYCCDBinMode(m_pCamHandle, _m_nCamBinX, _m_nCamBinY);
	if (QHYCCD_SUCCESS == retVal)
	{
		m_nCamBinX = _m_nCamBinX;
		m_nCamBinY = _m_nCamBinY;
		_LogInfo("SetQHYCCDBinMode set to: binX: %d, binY: %d, success.", _m_nCamBinX, _m_nCamBinY);
	}
	else
	{
		_LogError( "SetQHYCCDBinMode failure, error: %d", retVal );
		return( 1 );
	}
  
	return( 0 );
}

// set bit resolution
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetBitResolution( int _bit_resolution=16 )
{
	// set bit resolution
	retVal = IsQHYCCDControlAvailable(m_pCamHandle, CONTROL_TRANSFERBIT);
	if (QHYCCD_SUCCESS == retVal)
	{
		retVal = SetQHYCCDBitsMode(m_pCamHandle, _bit_resolution);
		if( retVal == QHYCCD_SUCCESS )
		{
			_LogInfo( "SetQHYCCDParam CONTROL_GAIN set to: %d, success.", CONTROL_TRANSFERBIT );

		} else
		{
			_LogError( "SetQHYCCDParam CONTROL_GAIN failure, error: %d", retVal );
			return( 1 );
		}
	}	
	
	return( 0 );
}

// take single exposure
////////////////////////////////////////////////////////////////////
int CQhyCamera::TakeSingleExposure( CAstroImage* pAstroImage )
{
	int status = 0;
	
	// single frame
	_LogInfo( "ExpQHYCCDSingleFrame(m_pCamHandle) - start..." );
	retVal = ExpQHYCCDSingleFrame(m_pCamHandle);
	_LogInfo( "ExpQHYCCDSingleFrame(m_pCamHandle) - end..." );
	if (QHYCCD_ERROR != retVal)
	{
		_LogInfo("ExpQHYCCDSingleFrame success.");
		if( QHYCCD_READ_DIRECTLY != retVal )
		{
			// wait 1 second?? do we need this 
			sleep(1);
		}
		
	} else
	{
		_LogError( "ExpQHYCCDSingleFrame failure, error: %d", retVal );
		return( 1 );
	}

	// get requested memory lenght
	uint32_t length = GetQHYCCDMemLength(m_pCamHandle);
	if (length > 0)
	{
		pImgData = new unsigned char[length];
		memset(pImgData, 0, length);
		_LogInfo("Allocated memory for frame: %d [uchar].", length);
		
	} else
	{
		_LogError( "Cannot allocate memory for frame." );
		return( 1 );
	}

	// get single frame
	retVal = GetQHYCCDSingleFrame(m_pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);
	if( QHYCCD_SUCCESS == retVal )
	{
		_LogInfo("GetQHYCCDSingleFrame: %d x %d, bpp: %d, channels: %d, success.", roiSizeX, roiSizeY, bpp, channels);
		// copy to astro image cotainer	
		pAstroImage->SetFromData( roiSizeX, roiSizeY, bpp, channels, m_bIsColor, pImgData );

	} else
	{
		_LogError( "GetQHYCCDSingleFrame failure, error: %d", retVal );
		status = 1;
	}

	delete [] pImgData;

	retVal = CancelQHYCCDExposingAndReadout(m_pCamHandle);
	if (QHYCCD_SUCCESS == retVal)
	{
		_LogInfo( "CancelQHYCCDExposingAndReadout success." );
		
	} else
	{
		_LogError( "CancelQHYCCDExposingAndReadout failure, error: %d", retVal );
		return( 1 );
	}

	return( status );
}

////////////////////////////////////////////////////////////////////
// get QHY sdk version
////////////////////////////////////////////////////////////////////
void CQhyCamera::SDKVersion()
{
	unsigned int  YMDS[4];
	unsigned char sVersion[80];

	memset ((char *)sVersion,0x00,sizeof(sVersion));
	GetQHYCCDSDKVersion(&YMDS[0],&YMDS[1],&YMDS[2],&YMDS[3]);

	if ((YMDS[1] < 10)&&(YMDS[2] < 10))
		sprintf((char *)sVersion,"V20%d0%d0%d_%d",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
	else if ((YMDS[1] < 10)&&(YMDS[2] > 10))
		sprintf((char *)sVersion,"V20%d0%d%d_%d",YMDS[0],YMDS[1],YMDS[2],YMDS[3] );
	else if ((YMDS[1] > 10)&&(YMDS[2] < 10))
		sprintf((char *)sVersion,"V20%d%d0%d_%d",YMDS[0],YMDS[1],YMDS[2],YMDS[3] );		
	else
		sprintf((char *)sVersion,"V20%d%d%d_%d",YMDS[0],YMDS[1],YMDS[2],YMDS[3] );

	//fprintf(stderr,"QHYCCD SDK Version: %s\n", sVersion);
	_LogInfo("QHYCCD SDK Version: %s", sVersion);
}

////////////////////////////////////////////////////////////////////
// get QHY camera firmware version
////////////////////////////////////////////////////////////////////
void CQhyCamera::FirmWareVersion( qhyccd_handle *h )
{
	int i = 0;
	unsigned char fwv[32],FWInfo[256];
	unsigned int ret;
	memset (FWInfo,0x00,sizeof(FWInfo));
	ret = GetQHYCCDFWVersion(h,fwv);
	if(ret == QHYCCD_SUCCESS)
	{
		if((fwv[0] >> 4) <= 9)
			sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d",((fwv[0] >> 4) + 0x10), (fwv[0]&~0xf0),fwv[1]);
		else
			sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d",(fwv[0] >> 4), (fwv[0]&~0xf0),fwv[1]);

		_LogInfo("%s", FWInfo);

	} else
	{
		sprintf((char *)FWInfo,"Firmware version:Not Found!");
		_LogError("%s", FWInfo);
	}

	//fprintf(stderr,"%s\n", FWInfo);
  
}

////////////////////////////////////////////////////////////////////
// get QHY read modes
////////////////////////////////////////////////////////////////////
int CQhyCamera::GetReadModes()
{
	int ret = 0;
	unsigned int no_read_modes = 0;
	
	ret = GetQHYCCDNumberOfReadModes( m_pCamHandle, &no_read_modes );
	
	if( ret == QHYCCD_SUCCESS )
	{
		for( int i=0; i<no_read_modes; i++ )
		{
			char strReadModeName[50];			
			if( GetQHYCCDReadModeName(m_pCamHandle, i, strReadModeName) == QHYCCD_SUCCESS )
			{
				printf("READMODE[%d]=%s\n", i, strReadModeName);
			}
		}		
	}
	
	return(ret);
}

////////////////////////////////////////////////////////////////////
// set QHY read mode
////////////////////////////////////////////////////////////////////
int CQhyCamera::SetReadMode( unsigned int read_mode )
{
	int ret = SetQHYCCDReadMode( m_pCamHandle, read_mode );
	
	if( ret != QHYCCD_SUCCESS )
		_LogError( "SetQHYCCDReadMode: failed" );
	else
		_LogInfo( "current SetQHYCCDReadMode=%d success", read_mode );
	
	return( ret );
}

////////////////////////////////////////////////////////////////////
double CQhyCamera::GetCCDTemp()
{
	double m_nCcdTemp = GetQHYCCDParam(m_pCamHandle,CONTROL_CURTEMP);
	return(m_nCcdTemp);
}

////////////////////////////////////////////////////////////////////
int CQhyCamera::SetCCDTemp( double ccd_temp )
{
	int ret = SetQHYCCDParam(m_pCamHandle, CONTROL_COOLER, ccd_temp);
	if( ret != QHYCCD_SUCCESS )
		_LogError( "SetQHYCCDParam(CONTROL_COOLER): failed" );
	else
	{
		m_nCcdTargetTemp = ccd_temp;
		_LogInfo( "current SetQHYCCDParam(CONTROL_COOLER)=%f success", ccd_temp );
	}
	
	return(ret);
}

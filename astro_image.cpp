////////////////////////////////////////////////////////////////////
// astro image implementation 
// Created by: Larry Lart 
////////////////////////////////////////////////////////////////////

// WX :: includes
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/utils.h> 
#include <wx/dir.h>
#include <wx/filefn.h> 
#include <wx/filename.h>

// include local
#include "qhycmd.h"
#include "qhy_utils.h"
#include "qhy_camera.h"
#include "astro_image.h"

//externals
DECLARE_APP(CQhyCmd)

// constructor
////////////////////////////////////////////////////////////////////
CAstroImage::CAstroImage()
{
	m_rgb48BitMat = NULL;
	m_rgb24BitMat = NULL;
	m_bayer16BitMat = NULL;
	m_bayer8BitMat = NULL;
	m_mono16BitMat = NULL;
	m_mono8BitMat = NULL;
	
	m_nWidth = 0;
	m_nHeight = 0;
	m_nSourceType = 0;
	m_isColor = false;
	m_nSourceChannels = 0;
	m_nSourceBpp = 0;
	
	m_bRemoveOverScan = false;
}
	
// destructor
////////////////////////////////////////////////////////////////////
CAstroImage::~CAstroImage()
{
	CleanAll();
}

////////////////////////////////////////////////////////////////////
void CAstroImage::CleanAll()
{
	if( m_rgb48BitMat != NULL ){ delete(m_rgb48BitMat); m_rgb24BitMat = NULL; }	
	if( m_rgb24BitMat != NULL ){ delete(m_rgb24BitMat); m_rgb48BitMat = NULL; }
	if( m_bayer16BitMat != NULL ){ delete(m_bayer16BitMat); m_bayer16BitMat = NULL; }
	if( m_bayer8BitMat != NULL ){ delete(m_bayer8BitMat); m_bayer8BitMat = NULL; }
	if( m_mono16BitMat != NULL ){ delete(m_mono16BitMat); m_mono16BitMat = NULL; }
	if( m_mono8BitMat != NULL ){ delete(m_mono8BitMat); m_mono8BitMat = NULL; }
}

// make cv:mat container from raw image data
////////////////////////////////////////////////////////////////////
int CAstroImage::SetFromData( int width, int height, int bpp, int channels, bool isColor, unsigned char *pImgData)
{
	m_nWidth = width;
	m_nHeight = height;
	m_isColor = isColor;
	m_nSourceChannels = channels;
	m_nSourceBpp = bpp;
	
	CleanAll();
	
	// todo: implement the other image variants
	// more than 8 bits, color, 1 channel is in bayer format
	if( bpp > 8 && channels == 1 && isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_16UC1, (char*) pImgData);
		m_bayer16BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_16UC1);
		_temp.copyTo(*m_bayer16BitMat);
		
		m_nSourceType = IMGSRC_COLOR_BAYER16;
		
	// :: 8 bits, 1 channel & color - raw bayer pattern image
	} else if( bpp == 8 && channels == 1 && isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_8UC1, (char*) pImgData);
		m_bayer8BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_8UC1);
		_temp.copyTo(*m_bayer8BitMat);
		
		m_nSourceType = IMGSRC_COLOR_BAYER8;		
		
	// :: more than 8 bit,  color, 3 channels
	} else if( bpp > 8 && channels == 3 && isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_16UC3, (char*) pImgData);
		m_rgb48BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_16UC3);
		_temp.copyTo(*m_rgb48BitMat);
		
		m_nSourceType = IMGSRC_COLOR_RGB48;		
		
	// :: 8 bits, 3 channel & color 
	} else if( bpp == 8 && channels == 3 && isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_8UC3, (char*) pImgData);
		m_rgb24BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_8UC3);
		_temp.copyTo(*m_rgb24BitMat);
		
		m_nSourceType = IMGSRC_COLOR_RGB24;			
		
	// :: more than 8 bit,  mono, 1 channel
	} else if( bpp > 8 && channels == 1 && !isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_16UC1, (char*) pImgData);
		m_mono16BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_16UC1);
		_temp.copyTo(*m_mono16BitMat);
		
		m_nSourceType = IMGSRC_MONO16;				
		
	// :: 8 bits, 1 channel & mono 
	} else if( bpp == 8 && channels == 1 && !isColor )
	{
		cv::Mat _temp(m_nHeight, m_nWidth, CV_8UC1, (char*) pImgData);
		m_mono8BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_8UC1);
		_temp.copyTo(*m_mono8BitMat);
		
		m_nSourceType = IMGSRC_MONO8;				
	}
	
	return(0);
}

// debayer or re 
////////////////////////////////////////////////////////////////////
cv::Mat* CAstroImage::getDebayered()
{
	_LogInfo( "getDebayered=%d", m_nSourceType );
	
	// check if debayer applies 
	if( m_nSourceType != IMGSRC_COLOR_BAYER16 && 
		m_nSourceType != IMGSRC_COLOR_BAYER8 ) 
	{
		if( m_nSourceType == IMGSRC_COLOR_RGB24 )
			return(m_rgb24BitMat);
		else if( m_nSourceType == IMGSRC_COLOR_RGB48 )
			return(m_rgb48BitMat);
		else
			return(NULL);
	}
	
	cv::Mat* img_ret = NULL;
	
	// debayer by type
	if( m_nSourceType == IMGSRC_COLOR_BAYER16 )
	{
		//if( m_rgb48BitMat != NULL ) delete(m_rgb48BitMat);
		if( m_rgb48BitMat != NULL ) return(m_rgb48BitMat);
		
		m_rgb48BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_16UC3);
		cv::cvtColor(*m_bayer16BitMat, *m_rgb48BitMat, CV_BayerRG2RGB);
		img_ret = m_rgb48BitMat;
		
	} else if( m_nSourceType == IMGSRC_COLOR_BAYER8 )
	{
		//if( m_rgb24BitMat != NULL ) delete(m_rgb24BitMat);
		if( m_rgb24BitMat != NULL ) return(m_rgb24BitMat);
		
		m_rgb24BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_8UC3);
		cv::cvtColor(*m_bayer8BitMat, *m_rgb24BitMat, CV_BayerRG2RGB);		
		img_ret = m_rgb24BitMat;
	}
	
	return(img_ret);
}


// save image by format
////////////////////////////////////////////////////////////////////
int CAstroImage::SaveImage( const char* file_name, wxString img_fmt, CQhyCamera* pQhyCamera )
{
	int ret = 0;
		
	if( img_fmt.Upper().IsSameAs("FIT") )
		ret = SaveFits( file_name, pQhyCamera );
	else if( img_fmt.Upper().IsSameAs("TIF") )
		ret = SaveTiff( file_name );
	else if( img_fmt.Upper().IsSameAs("JPG") )
		ret = SaveJpeg( file_name );
	else if( img_fmt.Upper().IsSameAs("PNG") )
		ret = SavePng( file_name );
	
	return(ret);
}

// save image as tiff
////////////////////////////////////////////////////////////////////
int CAstroImage::SaveTiff( const char* file_name )
{	
	wxString _filename(file_name);
	_filename += ".tif";
	
	cv::Mat* to_save = getDebayered();	
	if( to_save != NULL ) cv::imwrite(_filename.GetData().AsChar(), *to_save);	
	
	return(0);
}

// save image as jpeg
////////////////////////////////////////////////////////////////////
int CAstroImage::SaveJpeg( const char* file_name )
{
	wxString _filename(file_name);
	_filename += ".jpg";
	
	// save jpeg in 24 bit rgb
	this->getDebayered();	
	if( m_rgb24BitMat == NULL ) GetRGB24();
	
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(100);
	
	cv::imwrite( _filename.GetData().AsChar(), *m_rgb24BitMat, compression_params );	
	
	return(0);
}

// save image as png
////////////////////////////////////////////////////////////////////
int CAstroImage::SavePng( const char* file_name )
{
	wxString _filename(file_name);
	_filename += ".png";
	
	cv::Mat* to_save = getDebayered();	
	if( to_save != NULL ) cv::imwrite(_filename.GetData().AsChar(), *to_save);	
	
	return(0);
}

// save image as tiff
////////////////////////////////////////////////////////////////////
cv::Mat* CAstroImage::GetRGB24()
{
	if( m_rgb24BitMat != NULL ) return(m_rgb24BitMat);
	this->getDebayered();	
	
	// Convert the 16-bit per channel RGB image to 8-bit per channel - for display 
	m_rgb24BitMat = new cv::Mat(m_nHeight, m_nWidth, CV_8UC3);
	m_rgb48BitMat->convertTo(*m_rgb24BitMat, CV_8UC3, 1.0/256); // more efficeint crop bits instead ?

	return(m_rgb24BitMat);
}

// save image as fits
////////////////////////////////////////////////////////////////////
int CAstroImage::SaveFits(  const char* file_name, CQhyCamera* pQhyCamera )
{	
    // use overscan area
    int onlyStartX,onlyStartY,onlySizeX,onlySizeY;

	// remove file if exists
	wxString _filename(file_name);
	_filename += ".fits";
	// check if exist remove
	if( wxFileExists(_filename) ) wxRemoveFile(_filename);


    //cv::Mat FitImg(cvSize(ix.imageX ,ix.imageY ), IPL_DEPTH_16U, 1 );
    ////FitImg->imageData = (char*)Buf;
    //memcpy(FitImg.imageData, Buf, FitImg.imageSize);

	// if true remove overscan area 
    if(m_bRemoveOverScan)
    {
        onlyStartX = pQhyCamera->effectiveStartX;
        onlyStartY = pQhyCamera->effectiveStartY;
        onlySizeX = pQhyCamera->effectiveSizeX;
        onlySizeY = pQhyCamera->effectiveSizeY;
		
    } else
    {
        onlyStartX = 0;
        onlyStartY = 0;
        onlySizeX = pQhyCamera->roiSizeX ;
        onlySizeY = pQhyCamera->roiSizeY ;
    }
    cv::Mat OnlyImg(cvSize(onlySizeX,onlySizeY), IPL_DEPTH_16U, 1 );

    //cvSetImageROI(FitImg, cvRect(onlyStartX,onlyStartY,onlySizeX,onlySizeY));
    //cvCopy(FitImg, OnlyImg, NULL);
    //cvResetImageROI(FitImg);
    
	cv::Rect roi(onlyStartX,onlyStartY,onlySizeX,onlySizeY);
	(*m_bayer16BitMat)(roi).copyTo(OnlyImg);

    fitsfile *fptr;
    int status = 0;//, ii, jj;
    long  fpixel = 1, naxis = 2, nelements;//, exposure;
    long naxes[2] = {onlySizeX, onlySizeY };
    char description[100];

    fits_create_file(&fptr, _filename.GetData().AsChar(), &status);
    fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);

	// header part
	int v = 0;
	double f = 0.0;	
	SetFitsHeaderStr(fptr, "AUTHOR", "Larry", description, &status );
	SetFitsHeaderStr(fptr, "BAYERPAT", "RGGB   ", description, &status );	
	SetFitsHeaderStr(fptr, "CAMERA", pQhyCamera->m_strCameraName.GetData().AsChar(), description, &status );
	SetFitsHeaderStr(fptr, "INSTRUME", "QHYCCD  ", description, &status );	
	
	// INT VALUES
	//v=2; fits_update_key(fptr, TINT, "NAXIS", &v, description, &status);
	//fits_update_key(fptr, TINT, "NAXIS1", &onlySizeX, description, &status);
	//fits_update_key(fptr, TINT, "NAXIS2", &onlySizeY, description, &status);
	fits_update_key(fptr, TINT, "OFFSET", &pQhyCamera->CHIP_OFFSET, description, &status);
	fits_update_key(fptr, TINT, "BITPIX", &pQhyCamera->bpp, description, &status);
	fits_update_key(fptr, TINT, "GAIN", &pQhyCamera->CHIP_GAIN, description, &status);
	fits_update_key(fptr, TINT, "XBINNING", &pQhyCamera->camBinX, description, &status);
	fits_update_key(fptr, TINT, "YBINNING", &pQhyCamera->camBinY, description, &status);
	

//	fits_update_key(fptr, TINT, keywords, &v, description, &status);
	
	//fits_update_key(fptr, TDOUBLE, keywords, &f, description, &status);
//	f=1.0; fits_update_key(fptr, TDOUBLE, "BSCALE", &f, description, &status); -num overflow? - should be int ?
//	f=0.0; fits_update_key(fptr, TDOUBLE, "BZERO", &f, description, &status); - numerical overflow?
	fits_update_key(fptr, TDOUBLE, "SET-TEMP", &f, description, &status);
	
	// exposure time float in seconds from microseconds
	f = (double) pQhyCamera->EXPOSURE_TIME/1000000.0;	
	fits_update_key(fptr, TDOUBLE, "EXPOSURE", &f, description, &status);
	fits_update_key(fptr, TDOUBLE, "EXPTIME", &f, description, &status);
	
	fits_update_key(fptr, TDOUBLE, "XPIXSZ", &pQhyCamera->pixelWidthUM, description, &status);
	fits_update_key(fptr, TDOUBLE, "YPIXSZ", &pQhyCamera->pixelHeightUM, description, &status);
	
	//fits_update_key(fptr, TLOGICAL, keywords, &ss, description, &status);
	fits_update_key(fptr, TLOGICAL, "SIMPLE", (void*)"T", description, &status);
	fits_update_key(fptr, TLOGICAL, "EXTEND", (void*)"T", description, &status);

    nelements = naxes[0] * naxes[1];

    fits_write_img(fptr, TUSHORT, fpixel, nelements, (void*) OnlyImg.data, &status);
    fits_close_file(fptr, &status);
    fits_report_error(stderr, status);

	return(1);	
}

////////////////////////////////////////////////////////////////////
void CAstroImage::SetFitsHeaderStr( fitsfile* fptr, const char* str_label, const char* str_value, const char* str_desc, int* status )
{
	fits_update_key(fptr, TSTRING, (char*) str_label, (char*) str_value, (char*) str_desc, status);
}

////////////////////////////////////////////////////////////////////
// base class interface for astro images
// Created by: Larry Lart based on QHY SDK sample
////////////////////////////////////////////////////////////////////
#ifndef _ASTROIMG_H
#define _ASTROIMG_H

// base
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include "fitsio.h"

#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/imgproc.hpp>
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

// defines
#define IMGSRC_COLOR_RGB24			1
#define IMGSRC_COLOR_RGB48			2
#define IMGSRC_COLOR_BAYER8			3
#define IMGSRC_COLOR_BAYER16		4
#define IMGSRC_MONO8				5
#define IMGSRC_MONO16				6

// namespaces in use
using namespace std;
using namespace cv;

// external classes
class CQhyCamera;

/////////////////////////////////////////////////
class CAstroImage 
{
// methods
public:
	CAstroImage();
	~CAstroImage();

	void CleanAll();

	int SetFromData( int width, int height, int bpp, int channels, bool isColor, unsigned char *pImgData );
	cv::Mat* getDebayered();

	// used for display
	cv::Mat*  GetRGB24();
	
	int SaveImage( const char* file_name, wxString img_fmt, CQhyCamera* pQhyCamera );
	int SaveTiff( const char* file_name  );
	int SaveJpeg( const char* file_name  );
	int SavePng( const char* file_name  );
	
	
	int SaveFits( const char* file_name, CQhyCamera* pQhyCamera );
	void SetFitsHeaderStr( fitsfile *fptr, const char *str_label, const char *str_value, const char *str_desc, int* status );

	// call to remove overscan area
	void RemoveOverscan( bool _value=true ){ m_bRemoveOverScan = _value; }

/////////
// data
public:

	int		m_nWidth;
	int		m_nHeight;
	int		m_nSourceType;
	bool	m_isColor;
	int		m_nSourceChannels;
	int		m_nSourceBpp;

	// container for color images 3 channels
	cv::Mat* m_rgb48BitMat;	
	cv::Mat* m_rgb24BitMat;

	// containers for color image in bayer pattern, 1 channel
	cv::Mat* m_bayer16BitMat;
	cv::Mat* m_bayer8BitMat;

	// containers for monochrome image, 1 channel 
	cv::Mat* m_mono16BitMat;	
	cv::Mat* m_mono8BitMat;

		
	bool m_bRemoveOverScan;
};

#endif

#include "MvPlateDetector.h"
#include <highgui.h>

#ifdef  MV_DRAW_DEBUG_LINUX
#include     <unistd.h>  
#include     <sys/types.h>
#include     <sys/stat.h>
#include	 <sys/mman.h>
#include     <sys/stat.h>
#include     <fcntl.h> 
#include     <termios.h>
#include     <dirent.h>
#else
#include     <direct.h>
#endif

using namespace std;

MvPlateDetector::MvPlateDetector(void)
{
	m_pVehicleCascade = NULL;
	m_pCascade = NULL;
}


MvPlateDetector::~MvPlateDetector(void)
{
	if((m_pCascade != NULL)||(m_pVehicleCascade!=NULL))
	{
		mvUnInit();
	}
}

bool MvPlateDetector::mvInit(const char * pName)
{
	bool bRet = true;

	m_pCascade = (CvHaarClassifierCascade*)cvLoad("./plate.xml"/*pName*/ , 0, 0, 0 );   //载入分类器       
	if( !m_pCascade )    
	{   
		bRet = false;
		printf("Load XML Failed"); 
		return bRet;
	} 
	
	m_pVehicleCascade = (CvHaarClassifierCascade *)cvLoad("./VehicleData.xml",0,0,0);
	{
		if(!m_pVehicleCascade)
		{
			bRet = false;
			printf("Load XML Failed");
			return bRet;
		}
	}

	return bRet;
}

void MvPlateDetector::mvUnInit()
{
	if(m_pVehicleCascade)
	{
		cvReleaseHaarClassifierCascade(&m_pVehicleCascade);
		m_pVehicleCascade = 0;
	}

	if(m_pCascade)
	{
		cvReleaseHaarClassifierCascade(&m_pCascade);
		m_pCascade = 0;
	}
}

vector<CvRect> MvPlateDetector::mvDetectPos(const IplImage *pImage)
{
	vector<CvRect> vecRet;

	if(!m_pCascade || !m_pVehicleCascade || pImage == NULL || pImage->width < 15 || pImage->height < 15)
	{
		return vecRet;
	}

	CvMemStorage* storage = 0;
	double ThresholdRate=0.1;
	storage = cvCreateMemStorage(0);         
	cvClearMemStorage( storage ); 

	CvSeq* targets = cvHaarDetectObjects( pImage, m_pCascade, storage, 
		1.2, 5, 0,cvSize(14, 14)/*,cvSize(150, 150) */);   //多尺度检测函数

	for( int i = 0; i < (targets ? targets->total : 0); i++ )  
	{  
		CvRect* r = (CvRect*)cvGetSeqElem( targets, i );
		CvRect rt = *r;

		vecRet.push_back(rt);
	}

	cvReleaseMemStorage(&storage);

	return vecRet;
}

bool MvPlateDetector::verifySizes(Rect mr)
{

	if(mr.width < 50 || mr.width>200)
	{
		return false;
	}

	float r= (float)mr.width / (float)mr.height;


	if(r < 2.5 || r>6)
	{
		return false;
	}

	return true;

}

vector<CvRect> MvPlateDetector::mvDetectPos_Ext(const IplImage *pImage)
{
	vector<CvRect> vecRet;

	if(!m_pCascade || !m_pVehicleCascade || pImage == NULL || pImage->width < 15 || pImage->height < 15)
	{
		return vecRet;
	}

	CvMemStorage* storage = 0;
	double ThresholdRate=0.1;
	storage = cvCreateMemStorage(0);         
	cvClearMemStorage( storage ); 

	CvSeq* targets = cvHaarDetectObjects( pImage, m_pVehicleCascade, storage, 
		1.1, 5, 0,cvSize(50, 50)/*,cvSize(150, 150) */);   //多尺度检测函数

	for( int i = 0; i < (targets ? targets->total : 0); i++ )  
	{  
		CvRect* r = (CvRect*)cvGetSeqElem( targets, i );
		CvRect rt = *r;

		vecRet.push_back(rt);
	}

	cvReleaseMemStorage(&storage);

	return vecRet;
}


vector<Rect> MvPlateDetector::VerifyPos(const Mat img_ori)
{
	if(m_bDebugOn)
	{
		imshow("ori", img_ori);
	}


	Mat img_gray;
	cvtColor(img_ori, img_gray, CV_BGR2GRAY);

	Mat img_blur;

	//Finde vertical lines. Car plates have high density of vertical lines
	Mat img_sobel;
	//Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 2, 0, BORDER_DEFAULT);
	bilateralFilter(img_gray, img_blur,5, 10,5/2);
	if(m_bDebugOn)
		imshow("gray", img_blur);

	//threshold image
	Mat img_threshold;
	threshold(img_blur, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
	if(m_bDebugOn)
		imshow("Threshold", img_threshold);

	//Morphplogic operation close
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3) );
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
	if(m_bDebugOn)
		imshow("Close", img_threshold);
	Mat thresh;
	img_threshold.copyTo(thresh);

	//Find contours of possibles plates
	vector< vector< Point> > contours;
	findContours(img_threshold,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc= contours.begin();
	vector<Rect> rects;
	if(m_bDebugOn)
		imshow("img_clone", thresh);

	//Remove patch that are no inside limits of aspect ratio and area.    
	while (itc!=contours.end())
	{
		//Create bounding rect of object
		Rect mr= boundingRect(Mat(*itc));
		Mat roiA(thresh, mr);
		int nCount = countNonZero(roiA);
		if(nCount < mr.area()*.4)
		{
			itc= contours.erase(itc);
			continue;
		}
		/*if(m_bDebugOn)
		{
			if(mr.y > img_ori.rows*.7
			|| mr.y < img_ori.rows*.1
			|| (mr.y+mr.height)<img_ori.rows*.3
			|| (mr.y+mr.height)>img_ori.rows*.9
			|| mr.x*1.8 > img_ori.cols
			|| (mr.x+mr.width)*2.2<img_ori.cols
			|| (mr.x+mr.width) > img_ori.cols*.9
			|| mr.x<img_ori.cols*.1
			)
			{
				itc= contours.erase(itc);
				continue;
			}
		}*/

		if( !verifySizes(mr))
		{
			itc= contours.erase(itc);
		}else
		{
			++itc;
			rects.push_back(mr);
		}
	}

	if(m_bDebugOn)
	{

		// Draw blue contours on a white image
		cv::Mat result;
		img_ori.copyTo(result);
		cv::drawContours(result,contours,
			-1, // draw all contours
			cv::Scalar(255,0,0), // in blue
			1); // with a thickness of 1

		for(int j=0; j< rects.size(); j++)
		{

			//For better rect cropping for each posible box
			//Make floodfill algorithm because the plate has white background
			//And then we can retrieve more clearly the contour box
			rectangle(result, rects[j], Scalar(0,0,255),2);
		}
		imshow("result", result);

	}

	return rects;
}

bool MvPlateDetector::IsCarWithPlate(const IplImage * pImg, CvRect rt)
{
	Mat input(pImg);

	Mat SmallImage;
	SmallImage.create(600,800, CV_8UC3);
	resize(input,SmallImage,SmallImage.size());
	
	Mat img_gray;
	cvtColor(SmallImage, img_gray, CV_BGR2GRAY);

	Mat img_blur, img_plate_mat;

	//Finde vertical lines. Car plates have high density of vertical lines
	bilateralFilter(img_gray, img_blur,5, 10,5/2);
	bilateralFilter(img_gray, img_plate_mat, 5,10,5/2);

	Mat carMat;
	carMat.create(300,400,CV_8UC1);
	resize(img_blur,carMat,carMat.size());

	IplImage * pSmallCar = &IplImage(carMat);

	float xRatio = input.cols/(float)800;
	float yRatio = input.rows/(float)600;

	//检测是否为车
	vector<CvRect> vecCar = mvDetectPos_Ext(pSmallCar);
	vector<CvRect>::iterator it;
	bool bIs = false;

	int nMinX, nMaxX;
	int nMinY, nMaxY;
	int nX1,nX2,nY1,nY2;

#ifdef  MV_SAVE_RESULT_IMG

	IplImage *pCopySrcImg = cvCreateImage(cvSize(input.cols, input.rows), pImg->depth, pImg->nChannels);
	cvZero(pCopySrcImg);
	cvCopy(pImg, pCopySrcImg);

	cvRectangle(pCopySrcImg, cvPoint(rt.x, rt.y),
		cvPoint((rt.x+rt.width), (rt.y+rt.height)), CV_RGB(0, 255, 255), 2);

	for(it=vecCar.begin(); it!=vecCar.end(); it++)
	{
		cvRectangle(pCopySrcImg, cvPoint((it->x*2*xRatio),(it->y*2*yRatio)),
			cvPoint(((it->x+it->width)*2*xRatio), ((it->y+it->height)*2*yRatio)), CV_RGB(255, 0, 0), 2);
	}

	for(it=vecCar.begin(); it!=vecCar.end(); it++)
	{
		cvRectangle(pSmallCar, cvPoint((it->x),(it->y)),
			cvPoint(((it->x+it->width)), ((it->y+it->height))), CV_RGB(0, 0, 0), 3);
	}

	char buffer[256];
	static int nFirstFlag = 1;
	if (nFirstFlag==1)
	{
#ifdef  MV_DRAW_DEBUG_LINUX
		mkdir("./No_plate_file",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		mkdir("./No_plate_file/CarImg",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		mkdir("./No_plate_file/PlateImg",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		mkdir("./No_plate_file/ResultImg",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
#else
		_mkdir("./No_plate_file");
		_mkdir("./No_plate_file/CarImg");
		_mkdir("./No_plate_file/PlateImg");
		_mkdir("./No_plate_file/ResultImg");
#endif
	}

	sprintf(buffer, "./No_plate_file/CarImg/%d.jpg", nFirstFlag);
	cvSaveImage(buffer, pSmallCar);
	nFirstFlag++;

#endif

	for(it=vecCar.begin(); it!=vecCar.end(); it++)
	{
		if((it->x*2*xRatio) > rt.x)
		{
			nMinX = it->x*2*xRatio;
			nX1 = rt.x;
		}
		else
		{
			nMinX = rt.x;
			nX1 = it->x*2*xRatio;
		}

		if((it->y*2*yRatio) > rt.y)
		{
			nMinY = it->y*2*yRatio;
			nY1 = rt.y;
		}
		else
		{
			nMinY = rt.y;
			nY1 = it->y*2*yRatio;
		}


		if(((it->x+it->width)*2*xRatio) < rt.x+rt.width)
		{
			nMaxX = (it->x+it->width)*2*xRatio;
			nX2 = rt.x+rt.width;
		}
		else
		{
			nMaxX = rt.x+rt.width;
			nX2 = (it->x+it->width)*2*xRatio;
		}

		if(((it->y+it->height)*2*yRatio) < rt.y+rt.height)
		{
			nMaxY = (it->y+it->height)*2*yRatio;
			nY2 = rt.y+rt.height;
		}
		else
		{
			nMaxY = rt.y+rt.height;
			nY2 = (it->y+it->height)*2*yRatio;
		}

		if( nMaxX>nMinX && nMaxY>nMinY && (nMaxX-nMinX)*(nMaxY-nMinY)*2>rt.width*rt.height)
		{
			bIs =true;
			break;
		}

	}

	if(!bIs)
	{
		return true;			//不是车，也要过滤
	}

	Mat plateMat;
	plateMat.create(600, 800, CV_8UC1);
	resize(img_plate_mat, plateMat, plateMat.size());
	IplImage *pImgIn = &IplImage(plateMat);

	vector<CvRect> vecRet = mvDetectPos(pImgIn);

#ifdef  MV_SAVE_RESULT_IMG

	//把车牌框画出来
	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		cvRectangle(pCopySrcImg, cvPoint((it->x*xRatio),(it->y*yRatio)),
			cvPoint(((it->x+it->width)*xRatio), ((it->y+it->height)*yRatio)), CV_RGB(0, 0, 255), 2);
	}

	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		cvRectangle(pImgIn, cvPoint((it->x),(it->y)),
			cvPoint(((it->x+it->width)), ((it->y+it->height))), CV_RGB(0, 0, 0), 3);
	}

	static int nPlateNum=1;
	sprintf(buffer, "./No_plate_file/PlateImg/%d.jpg", nPlateNum);
	cvSaveImage(buffer, pImgIn);

	sprintf(buffer, "./No_plate_file/ResultImg/%d.jpg", nPlateNum);
	cvSaveImage(buffer, pCopySrcImg);

	nPlateNum++;

#endif
	
	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		if((it->x*xRatio) < nX1 || ((it->x+it->width)*xRatio)>nX2)
		{
			continue;
		}

		if((it->y*yRatio) < nY1 || ((it->y+it->height)*yRatio)>nY2)
		{
			continue;
		}

		return true;		//是车，过滤
	}
	
	return false;
	
}

bool MvPlateDetector::JudgeIsRealVehicle(const IplImage * pImg, CvRect rt)
{
	Mat input(pImg);

	Mat SmallImage;
	SmallImage.create(600,800, CV_8UC3);
	resize(input,SmallImage,SmallImage.size());

	Mat img_gray;
	cvtColor(SmallImage, img_gray, CV_BGR2GRAY);

	Mat img_blur;

	//Finde vertical lines. Car plates have high density of vertical lines
	bilateralFilter(img_gray, img_blur,5, 10,5/2);

	Mat carMat;
	carMat.create(300,400,CV_8UC1);
	resize(img_blur,carMat,carMat.size());

	IplImage * pSmallCar = &IplImage(carMat);

	float xRatio = pImg->width/(float)800;
	float yRatio = pImg->height/(float)600;

	//检测是否为车
	vector<CvRect> vecCar = mvDetectPos_Ext(pSmallCar);
	vector<CvRect>::iterator it;
	bool bIs = false;

	int nMinX, nMaxX;
	int nMinY, nMaxY;

	for(it=vecCar.begin(); it!=vecCar.end(); it++)
	{
		if(it->x*2*xRatio > rt.x)
		{
			nMinX = it->x*2*xRatio;
		}
		else
		{
			nMinX = rt.x;
		}

		if(it->y*2*yRatio > rt.y)
		{
			nMinY = it->y*2*yRatio;
		}
		else
		{
			nMinY = rt.y;
		}

		if((it->x+it->width)*2*xRatio < rt.x+rt.width)
		{
			nMaxX = (it->x+it->width)*2*xRatio;
		}
		else
		{
			nMaxX = rt.x+rt.width;
		}

		if((it->y+it->height)*2*yRatio < rt.y+rt.height)
		{
			nMaxY = (it->y+it->height)*2*yRatio;
		}
		else
		{
			nMaxY = rt.y+rt.height;
		}

		if( nMaxX>nMinX && nMaxY>nMinY && (nMaxX-nMinX)*(nMaxY-nMinY)*2>rt.width*rt.height)
		{
			bIs = true;
			break;
		}

	}

	return bIs;
}
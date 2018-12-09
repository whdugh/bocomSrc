
#ifndef _MV_SIFT_H_
#define _MV_SIFT_H_

#include "declare.h"


void mvComputeSiftDescriptor(unsigned char *grayImage,
							 CvSize size,
							 CvPoint *corners,
							 int num_keypts,
							 MvInputData *pInputData,
							 int *sift_histogram,
							 unsigned char **sift_descriptor);

void mvComputeSiftDescriptor9(unsigned char *pGrayImage,
							  CvSize size,
							  CvPoint *pCorners,
							  int nKeyPoint,
							  MvInputData *pInputData,
							  unsigned int *pSiftHistogram,
							  unsigned char *pSiftDescriptor);

#endif
/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//    Copyright (c) 2001-2007 Intel Corporation. All Rights Reserved.
//
//  Intel® Integrated Performance Primitives JPEG Viewer Sample for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel® Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel® IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
//
*/

#ifndef __JPEG_H__
#define __JPEG_H__

#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif
#ifndef __BASESTREAMIN_H__
#include "basestreamin.h"
#endif
#ifndef __BASESTREAMOUT_H__
#include "basestreamout.h"
#endif
#ifndef __IPPIMAGE_H__
#include "ippimage.h"
#endif


typedef struct _PARAMS_JPEG
{
  int    quality;
  int    restart_interval;
  int    huffman_opt;
  int    point_transform;
  int    predictor;
  int    comment_size;
  Ipp8u  comment[128];
  JMODE  mode;
  JCOLOR color;
  JSS    sampling;
  JDD    dct_scale;

} PARAMS_JPEG;


JERRCODE ReadImageJPEG(CBaseStreamInput* in, PARAMS_JPEG* param, CIppImage* image);
JERRCODE SaveImageJPEG(CIppImage* image, PARAMS_JPEG param, CBaseStreamOutput* out);

#endif // __JPEG_H__

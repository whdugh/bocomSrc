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

#ifndef __IPPIMAGE_H__
#define __IPPIMAGE_H__

#ifndef __JPEGBASE_H__
#include "jpegbase.h"
#endif



typedef enum _MT_TYPE
{
  MT_UNSUPPORTED = 0,
  MT_RAW         = 1,
  MT_BMP         = 2,
  MT_PNM         = 3,
  MT_JPEG        = 4,
  MT_JPEG2000    = 5,
  MT_DICOM       = 6

} MT_TYPE;


void RGBA_FPX_to_BGRA(Ipp8u* data,int width,int height);
void BGRA_to_RGBA(Ipp8u* data,int width,int height);

class CIppImage
{
public:
  CIppImage(void);
  virtual ~CIppImage(void);

  JCOLOR Color(void)             { return m_color; }
  void Color(JCOLOR color)       { m_color = color; }

  JSS  Sampling(void)            { return m_sampling; }
  void Sampling(JSS sampling)    { m_sampling = sampling; }

  IppiSize Size(void)            { return m_roi; }

  int Width(void)                { return m_roi.width; }
  int Height(void)               { return m_roi.height; }
  void Width(int width)          { m_roi.width = width; }
  void Height(int height)        { m_roi.height = height; }

  int Step(void)                 { return m_step; }
  int NChannels(void)            { return m_nchannels; }
  int Precision(void)            { return m_precision; }
  int Order(void)                { return m_order; }
  void NChannels(int nchannels)  { m_nchannels = nchannels; }
  void Precision(int precision)  { m_precision = precision; }
  void Order(int order)          { m_order = order; } // 0 - pixel, 1 - plane
  
  int    Alloc(IppiSize roi, int nchannels, int precision, int align = 0);
  int    Free(void);
  int    CopyFrom(Ipp8u*   pSrc, int srcStep, IppiSize roi);
  int    CopyFrom(Ipp16s*  pSrc, int srcStep, IppiSize roi);
  int    CopyBits(Ipp8u*   pDst, int dstStep, IppiSize roi);
  int    ReduceBits(Ipp8u* pDst, int dstStep, IppiSize roi);
  int    SwapChannels(int *order);

  operator Ipp8u*(void)  { return (Ipp8u*) m_imageData; }
  operator Ipp16s*(void) { return (Ipp16s*)m_imageData; }

protected:
  int      m_step;
  int      m_nchannels;
  int      m_precision;
  int      m_order;
  IppiSize m_roi;
  JCOLOR   m_color;
  JSS      m_sampling;
  void*    m_imageData;
};

#endif // __IPPIMAGE_H__


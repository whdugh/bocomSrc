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

#include "../StdAfx.h"

#ifndef __JPEG_H__
#include "jpeg.h"
#endif
#ifndef __METADATA_H__
#include "metadata.h"
#endif
#ifndef __JPEGENC_H__
#include "jpegenc.h"
#endif
#ifndef __JPEGDEC_H__
#include "jpegdec.h"
#endif


JERRCODE ReadImageJPEG(
  CBaseStreamInput* in,
  PARAMS_JPEG*      param,
  CIppImage*        image)
{
  int      r;
  int      size;
  int      step;
  int      nchannels;
  int      precision;
  int      dd_factor;
  JCOLOR   in_color;
  JCOLOR   out_color = JC_GRAY;
  JSS      sampling;
  JDD      dct_scale;
  Ipp8u*   comment;
  IppiSize roi;
  JERRCODE jerr;
  CJPEGMetaData metadata;
  CJPEGDecoder  decoder;

  jerr = decoder.SetSource(in);
  if(JPEG_OK != jerr)
    return jerr;

  jerr = decoder.ReadHeader(&roi.width,&roi.height,&nchannels,&in_color,&sampling,&precision);
  if(JPEG_OK != jerr)
    return jerr;

  decoder.Comment(&comment,&size);

  dct_scale = param->dct_scale;

  param->mode         = decoder.Mode();
  param->color        = in_color;
  param->sampling     = sampling;
  param->comment_size = IPP_MIN(size,sizeof(param->comment));

  if(param->comment_size != 0)
    ippsCopy_8u(comment,param->comment,param->comment_size);
  else
    param->comment[0] = '\0';

  switch(nchannels)
  {
  case 1:
    out_color = in_color == JC_UNKNOWN ? JC_UNKNOWN : JC_GRAY;
    nchannels = 1;
    break;

  case 3:
    out_color = in_color == JC_UNKNOWN ? JC_UNKNOWN : JC_RGB;
    nchannels = 3;
    break;

  case 4:
    out_color = in_color == JC_UNKNOWN ? JC_UNKNOWN : JC_CMYK;
    nchannels = 4;
    break;

  default:
    break;
  }

  image->Color(out_color);

  if(JPEG_BASELINE == param->mode || JPEG_PROGRESSIVE == param->mode)
  {
    switch(dct_scale)
    {
    case JD_1_1:
    default:  
        dd_factor = 1;
        break;

    case JD_1_2:
        dd_factor = 2;
        break;

    case JD_1_4:
        dd_factor = 4;
        break;

    case JD_1_8:
        dd_factor = 8;
        break;
    }
  }
  else
  {
    dct_scale = JD_1_1;
    dd_factor = 1;
  }

  if(precision <= 8)
    step = roi.width * nchannels + DIB_PAD_BYTES(roi.width,nchannels);
  else
  {
    step = roi.width * nchannels * sizeof(Ipp16s) + DIB_PAD_BYTES(roi.width,nchannels);
    dd_factor = 1;
  }

  step = step / dd_factor;
  size = step * roi.height;

  roi.width  /= dd_factor;
  roi.height /= dd_factor;

  r = image->Alloc(roi,nchannels,precision);
  if(0 != r)
  {
    return JPEG_ERR_ALLOC;
  }

  if(precision <= 8)
  {
    Ipp8u* ptr = *image;
    jerr = decoder.SetDestination(ptr,image->Step(),image->Size(),image->NChannels(),out_color,JS_444,8,dct_scale);
  }
  else
  {
    Ipp16s* ptr = *image;
    jerr = decoder.SetDestination(ptr,image->Step(),image->Size(),image->NChannels(),out_color);
  }

  if(JPEG_OK != jerr)
    return jerr;

  jerr = decoder.ReadData();
  if(JPEG_OK != jerr)
    return jerr;

  return JPEG_OK;
} // ReadImageJPEG()


JERRCODE SaveImageJPEG(
  CIppImage*         image,
  PARAMS_JPEG        param,
  CBaseStreamOutput* out)
{
  JERRCODE     jerr;
  CJPEGEncoder encoder;

  if(image->Precision() > 8)
  {
    JCOLOR color;
    if(image->Precision() == 12)
      color = image->NChannels() == 1 ? JC_GRAY : JC_UNKNOWN;
    else
      color = image->Color();

    jerr = encoder.SetSource((Ipp16s*)*image,image->Step(),image->Size(),image->NChannels(), color,JS_444,image->Precision());
    if(JPEG_OK != jerr)
    {
      return jerr;
    }
  }
  else
  {
    jerr = encoder.SetSource((Ipp8u*)*image,image->Step(),image->Size(),image->NChannels(),image->Color());
    if(JPEG_OK != jerr)
    {
      return jerr;
    }
  }

  jerr = encoder.SetDestination(out);
  if(JPEG_OK != jerr)
  {
    return jerr;
  }

  if(param.mode == JPEG_LOSSLESS) 
  {
    jerr = encoder.SetParams(
             param.mode,
             param.color,
             param.sampling,
             param.restart_interval,
             param.huffman_opt,
             param.point_transform,
             param.predictor);
  }
  else
    jerr = encoder.SetParams(
             param.mode,
             param.color,
             param.sampling,
             param.restart_interval,
             param.huffman_opt,
             param.quality);

  if(JPEG_OK != jerr)
  {
    return jerr;
  }

  jerr = encoder.WriteHeader();
  if(JPEG_OK != jerr)
  {
    return jerr;
  }

  jerr = encoder.WriteData();
  if(JPEG_OK != jerr)
  {
    return jerr;
  }

  return JPEG_OK;
} // SaveImageJPEG()


/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __MEMBUFFOUT_H__
#include "membuffout.h"
#endif


CMemBuffOutput::CMemBuffOutput(void)
{
  m_buf     = 0;
  m_buflen  = 0;
  m_currpos = 0;

  return;
} // ctor


CMemBuffOutput::~CMemBuffOutput(void)
{
  Close();
  return;
} // dtor


JERRCODE CMemBuffOutput::Open(Ipp8u* pBuf, int buflen)
{
  if(0 == pBuf)
    return JPEG_ERR_PARAMS;

  m_buf     = pBuf;
  m_buflen  = buflen;
  m_currpos = 0;

  return JPEG_OK;
} // CMemBuffOutput::Open()


JERRCODE CMemBuffOutput::Close(void)
{
  m_buf     = 0;
  m_buflen  = 0;
  m_currpos = 0;

  return JPEG_OK;
} // CMemBuffOutput::Close()


JERRCODE CMemBuffOutput::Write(void* buf,uic_size_t len,uic_size_t* cnt)
{
  uic_size_t wb;

  wb = (uic_size_t)IPP_MIN((int)len,m_buflen - m_currpos);

  ippsCopy_8u((Ipp8u*)buf,m_buf + m_currpos,wb);

  m_currpos += wb;

  *cnt = wb;

  if(len != wb)
    return JPEG_ERR_BUFF;

  return JPEG_OK;
} // CMemBuffOutput::Write()

int CMemBuffOutput::GetBufLen()
{
	return m_buflen;
} // CMemBuffOutput::GetBufLen()


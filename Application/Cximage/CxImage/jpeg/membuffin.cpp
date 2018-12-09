/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __MEMBUFFIN_H__
#include "membuffin.h"
#endif


CMemBuffInput::CMemBuffInput(void)
{
  m_buf     = 0;
  m_buflen  = 0;
  m_currpos = 0;

  return;
} // ctor


CMemBuffInput::~CMemBuffInput(void)
{
  Close();
  return;
} // dtor


JERRCODE CMemBuffInput::Open(Ipp8u* pBuf, int buflen)
{
  if(0 == pBuf)
    return JPEG_ERR_PARAMS;

  m_buf     = pBuf;
  m_buflen  = buflen;
  m_currpos = 0;

  return JPEG_OK;
} // CMemBuffInput::Open()


JERRCODE CMemBuffInput::Close(void)
{
  m_buf     = 0;
  m_buflen  = 0;
  m_currpos = 0;

  return JPEG_OK;
} // CMemBuffInput::Close()


JERRCODE CMemBuffInput::Seek(long offset, int origin)
{
  switch(origin)
  {
  case UIC_SEEK_CUR:
    {
      if(m_currpos + offset >= m_buflen)
      {
        return JPEG_ERR_BUFF;
      }
      else
        m_currpos += offset;

      break;
    }

  case UIC_SEEK_SET:
  case UIC_SEEK_END:
  default:
    return JPEG_NOT_IMPLEMENTED;
  }

  return JPEG_OK;
} // CMemBuffInput::Seek()


JERRCODE CMemBuffInput::Read(void* buf,uic_size_t len,uic_size_t* cnt)
{
  uic_size_t rb;

  rb = (uic_size_t)IPP_MIN((int)len,m_buflen - m_currpos);

  ippsCopy_8u(m_buf + m_currpos,(Ipp8u*)buf,rb);

  m_currpos += rb;

  *cnt = rb;

  if(len != rb)
    return JPEG_ERR_BUFF;

  return JPEG_OK;
} // CMemBuffInput::Read()



/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __STDFILEIN_H__
#include "stdfilein.h"
#endif


CStdFileInput::CStdFileInput(void)
{
  m_fin = 0;
  return;
} // ctor


CStdFileInput::~CStdFileInput(void)
{
  Close();
  return;
} // dtor


JERRCODE CStdFileInput::Open(char* name)
{
  m_fin = fopen(name,"rb");
  if(0 == m_fin)
    return JPEG_ERR_FILE;

  return JPEG_OK;
} // CStdFileInput::Open()


JERRCODE CStdFileInput::Close(void)
{
  if(0 != m_fin)
  {
    fclose(m_fin);
    m_fin = 0;
  }

  return JPEG_OK;
} // CStdFileInput::Close()


JERRCODE CStdFileInput::Seek(long offset, int origin)
{
  int r;

  r = fseek(m_fin,offset,origin);
  if(0 != r)
    return JPEG_ERR_FILE;

  return JPEG_OK;
} // CStdFileInput::Seek()


JERRCODE CStdFileInput::Read(void* buf,uic_size_t len,uic_size_t* cnt)
{
  size_t cb;
  size_t rb;

  rb = (size_t)len;

  cb = fread(buf,sizeof(unsigned char),rb,m_fin);

  *cnt = (uic_size_t)cb;

  if(cb != rb)
    return JPEG_ERR_FILE;

  return JPEG_OK;
} // CStdFileInput::Read()


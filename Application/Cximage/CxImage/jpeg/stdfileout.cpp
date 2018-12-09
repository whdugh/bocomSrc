/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __STDFILEOUT_H__
#include "stdfileout.h"
#endif


CStdFileOutput::CStdFileOutput(void)
{
  m_fout = 0;
  return;
} // ctor


CStdFileOutput::~CStdFileOutput(void)
{
  Close();
  return;
} // dtor


JERRCODE CStdFileOutput::Open(char* name)
{
  m_fout = fopen(name,"wb");
  if(0 == m_fout)
    return JPEG_ERR_FILE;

  return JPEG_OK;
} // CStdFileOutput::Open()


JERRCODE CStdFileOutput::Close(void)
{
  if(0 != m_fout)
  {
    fclose(m_fout);
    m_fout = 0;
  }

  return JPEG_OK;
} // CStdFileOutput::Close()


JERRCODE CStdFileOutput::Write(void* buf,uic_size_t len,uic_size_t* cnt)
{
  size_t cb;
  size_t rb;

  rb = (size_t)len;

  cb = fwrite(buf,sizeof(unsigned char),rb,m_fout);

  *cnt = (uic_size_t)cb;

  if(cb != rb)
    return JPEG_ERR_FILE;

  return JPEG_OK;
} // CStdFileOutput::Read()


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
#define __MEMBUFFOUT_H__

#include <stdio.h>
#ifndef __BASESTREAM_H__
#include "basestream.h"
#endif
#ifndef __BASESTREAMOUT_H__
#include "basestreamout.h"
#endif


class CMemBuffOutput : public CBaseStreamOutput
{
public:
  CMemBuffOutput(void);
  ~CMemBuffOutput(void);

  JERRCODE Open(Ipp8u* pBuf, int buflen);
  JERRCODE Close(void);

  JERRCODE Write(void* buf,uic_size_t len,uic_size_t* cnt);
  
  int GetBufLen();//tony add

private:
  JERRCODE Open(char* name) { name = name; return JPEG_NOT_IMPLEMENTED; }

protected:
  Ipp8u*  m_buf;
  int     m_buflen;
  int     m_currpos;

};


#endif // __MEMBUFFOUT_H__


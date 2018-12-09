/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __BASESTREAMIN_H__
#define __BASESTREAMIN_H__

#include "jpegbase.h"
#include "basestream.h"


class CBaseStreamInput : public CBaseStream
{
public:
  CBaseStreamInput(void) {}
  ~CBaseStreamInput(void) {}

  virtual JERRCODE Seek(long offset, int origin) = 0;
  virtual JERRCODE Read(void* buf,uic_size_t len,uic_size_t* cnt) = 0;
};


#endif // __BASESTREAMIN_H__


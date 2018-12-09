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
#define __STDFILEIN_H__

#include <stdio.h>
#ifndef __BASESTREAM_H__
#include "basestream.h"
#endif
#ifndef __BASESTREAMIN_H__
#include "basestreamin.h"
#endif


class CStdFileInput : public CBaseStreamInput
{
public:
  CStdFileInput(void);
  ~CStdFileInput(void);

  JERRCODE Open(char* name);
  JERRCODE Close(void);

  JERRCODE Seek(long offset, int origin);
  JERRCODE Read(void* buf,uic_size_t len,uic_size_t* cnt);

protected:
  FILE*  m_fin;

};


#endif // __STDFILEIN_H__


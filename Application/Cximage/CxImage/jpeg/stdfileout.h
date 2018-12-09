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
#define __STDFILEOUT_H__

#include <stdio.h>
#ifndef __BASESTREAM_H__
#include "basestream.h"
#endif
#ifndef __BASESTREAMOUT_H__
#include "basestreamout.h"
#endif


class CStdFileOutput : public CBaseStreamOutput
{
public:
  CStdFileOutput(void);
  ~CStdFileOutput(void);

  JERRCODE Open(char* name);
  JERRCODE Close(void);

  JERRCODE Write(void* buf,uic_size_t len,uic_size_t* cnt);

protected:
  FILE*  m_fout;

};


#endif // __STDFILEOUT_H__


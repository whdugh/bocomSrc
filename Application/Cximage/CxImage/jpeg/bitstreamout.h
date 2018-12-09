/*
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2006 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __BITSTREAMOUT_H__
#define __BITSTREAMOUT_H__

#include "jpegbase.h"


class CBaseStreamOutput;

const int ENC_DEFAULT_BUFLEN = 4096; // internal intermediate output buffer

class CBitStreamOutput
{
public:
  CBitStreamOutput(void);
  virtual ~CBitStreamOutput(void);

  JERRCODE Attach(CBaseStreamOutput* out);
  JERRCODE Detach(void);

  JERRCODE Init(int bufSize = ENC_DEFAULT_BUFLEN);
  JERRCODE FlushBuffer(int nBytes = ENC_DEFAULT_BUFLEN);

  Ipp8u* GetDataPtr(void) const { return m_pData; }
  int GetDataLen(void) const { return m_DataLen; }
  int GetCurrPos(void) const { return m_currPos; }
  void SetCurrPos(int cp) { m_currPos = cp; return; }

  int NumOfBytes(void) const { return m_nBytesWritten; }

  JERRCODE WriteByte(int byte);
  JERRCODE WriteWord(int word);
  JERRCODE WriteDword(int dword);

protected:
  CBaseStreamOutput* m_out;

  Ipp8u*             m_pData;
  int                m_DataLen;
  int                m_currPos;

  int                m_nBytesWritten;
};


#endif // __BITSTREAMOUT_H__


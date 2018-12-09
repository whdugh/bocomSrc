// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include <memory.h>

//#define RL_DEBUG  // un-comment to test response layer

class ResponseLayer
{
public:

  int m_nWidth, m_nHeight, m_nStep, filter;
  float *m_pResponses;
  unsigned char *m_pLaplacian;

  ResponseLayer(int width, int height, int step, int filter)
  {
    assert(width > 0 && height > 0);
    
    this->m_nWidth = width;
    this->m_nHeight = height;
    this->m_nStep = step;
    this->filter = filter;

    m_pResponses = new float[m_nWidth*m_nHeight];
    m_pLaplacian = new unsigned char[m_nWidth*m_nHeight];

    memset(m_pResponses,0,sizeof(float)*m_nWidth*m_nHeight);
    memset(m_pLaplacian,0,sizeof(unsigned char)*m_nWidth*m_nHeight);
  }

  ~ResponseLayer()
  {
    if (m_pResponses)
		delete [] m_pResponses;
    if (m_pLaplacian) 
		delete [] m_pLaplacian;
  }

  inline unsigned char getLaplacian(unsigned int row, unsigned int column)
  {
    return m_pLaplacian[row * m_nWidth + column];
  }

  inline unsigned char getLaplacian(unsigned int row, unsigned int column, ResponseLayer *src)
  {
    int scale = this->m_nWidth / src->m_nWidth;

    #ifdef RL_DEBUG
    assert(src->getCoords(row, column) == this->getCoords(scale * row, scale * column));
    #endif

    return m_pLaplacian[(scale * row) * m_nWidth + (scale * column)];
  }

  inline float getResponse(unsigned int row, unsigned int column)
  {
    return m_pResponses[row * m_nWidth + column];
  }

  inline float getResponse(unsigned int row, unsigned int column, ResponseLayer *src)
  {
    int scale = this->m_nWidth / src->m_nWidth;

    #ifdef RL_DEBUG
    assert(src->getCoords(row, column) == this->getCoords(scale * row, scale * column));
    #endif

    return m_pResponses[(scale * row) * m_nWidth + (scale * column)];
  }

#ifdef RL_DEBUG
  std::vector<std::pair<int, int>> coords;

  inline std::pair<int,int> getCoords(unsigned int row, unsigned int column)
  {
    return coords[row * width + column];
  }

  inline std::pair<int,int> getCoords(unsigned int row, unsigned int column, ResponseLayer *src)
  {
    int scale = this->width / src->width;
    return coords[(scale * row) * width + (scale * column)];
  }
#endif
};

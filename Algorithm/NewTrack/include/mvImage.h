// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef _MVIMAGE_H__
#define _MVIMAGE_H__

template<class T>
class MvImage
{
public:
	MvImage();
	void mvInit(const int nWidth,const int nHeight,const T &val);

	MvImage(const int nWidth,const int nHeight,bool bInit = true);
	void mvInit(const T &val);
	~MvImage();

	MvImage<T> *copy() const;

	int width() const { return m_nWidth; }
	int height() const { return m_nHeight; }

	/* image data. */
	T *m_pData;

	/* row pointers. */
	T **m_ppAccess;

	void mvSet(const int x,const int y,T &val);
	void mvGet(const int x,const int y,T &val);
	T* & operator[](int y)
	{
		//if(y<0 || y>=m_nHeight)
		//	return m_ppAccess[0];
		return m_ppAccess[y];
	}

private:
	int m_nWidth,m_nHeight;
};

template<class T>
MvImage<T>::MvImage()
{
	m_nWidth = 0;
	m_nHeight = 0;
	m_pData = NULL;
	m_ppAccess = NULL;
}

template<class T>
void MvImage<T>::mvInit(const int nWidth,const int nHeight,const T &val)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	if(m_pData != NULL)
	{
		delete []m_pData;
	}
	m_pData = new T[m_nWidth*m_nHeight];

	if(m_ppAccess != NULL)
		delete []m_ppAccess;
	m_ppAccess = new T*[m_nHeight];

	for(int i=0; i<m_nHeight; i++)
	{
		m_ppAccess[i] = &m_pData[i*m_nWidth];
	}

	T *ptr = &m_ppAccess[0][0];
	T *end = &m_ppAccess[m_nHeight-1][m_nWidth-1];
	while (ptr <= end)
		*ptr++ = val;

}

template<class T>
MvImage<T>::MvImage(const int nWidth,const int nHeight,bool bInit /* = true */)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_pData = new T[m_nWidth*m_nHeight];
	m_ppAccess = new T*[m_nHeight];

	for(int i=0; i<m_nHeight; i++)
	{
		m_ppAccess[i] = &m_pData[i*m_nWidth];
	}

	if(bInit)
		memset(m_pData,0,m_nWidth*m_nHeight*sizeof(T));
}

template <class T>
MvImage<T>::~MvImage()
{
	delete []m_pData;
	delete []m_ppAccess;
}

template<class T>
void MvImage<T>::mvInit(const T &val)
{
	T *ptr = m_ppAccess[0][0];
	T *end = m_ppAccess[m_nHeight-1][m_nWidth-1];
	while (ptr <= end)
		*ptr++ = val;
}

template <class T>
MvImage<T> * MvImage<T>::copy() const
{
	MvImage<T> *im = new MvImage<T>(m_nWidth, m_nHeight, false);
	memcpy(im->m_pData, m_pData, m_nWidth * m_nWidth * sizeof(T));
	return im;
}

template <class T>
void MvImage<T> ::mvSet(const int x,const int y,T &val)
{
	if(x<0 || x>=m_nWidth)
		return;
	if(y<0 || y>= m_nHeight)
		return;
	m_ppAccess[y][x] = val;
}

template <class T>
void MvImage<T> ::mvGet(const int x,const int y,T &val)
{
	if(x<0 || x>=m_nWidth)
		return;
	if(y<0 || y>= m_nHeight)
		return;
	val = m_ppAccess[y][x] ;
}

typedef struct stVisitMark 
{
	int node;
	bool bVisit;
	stVisitMark()
	{
		bVisit = false;
	}
}MvVisitMark;

#endif
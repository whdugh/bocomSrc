/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：CvxText.h
* 摘要: Opencv文本扩展类（支持在IplImage上写汉字）
* 版本: V2.0
* 作者: 於锋
* 完成日期: 2009年6月18日
*/

#ifndef OPENCV_CVX_TEXT
#define OPENCV_CVX_TEXT

#include "cv.h"
#include "highgui.h"
#include "ft2build.h"

#include FT_FREETYPE_H

class CvxText
{
  public:

     /**
      * 装载字库文件
      */
     CvxText();
     ~CvxText();

	 //初始化
	 void Init(int nFontSize = 25);
	 //释放资源
	 void UnInit();

     //================================================================
     //================================================================

     /**
      * 获取字体。目前有些参数尚不支持。
      *
      * \param font        字体类型, 目前不支持
      * \param size        字体大小/空白比例/间隔比例/旋转角度
      * \param underline   下画线
      * \param diaphaneity 透明度
      *
      * \sa setFont, restoreFont
      */

     void getFont(int *type,CvScalar *size=NULL, bool *underline=NULL, float *diaphaneity=NULL);

     /**
      * 设置字体。目前有些参数尚不支持。
      *
      * \param font        字体类型, 目前不支持
      * \param size        字体大小/空白比例/间隔比例/旋转角度
      * \param underline   下画线
      * \param diaphaneity 透明度
      *
      * \sa getFont, restoreFont
      */

     void setFont(int *type,CvScalar *size=NULL, bool *underline=NULL, float *diaphaneity=NULL);

     /**
      * 恢复原始的字体设置。
      *
      * \sa getFont, setFont
      */

     void restoreFont();

     //================================================================
     //================================================================
     /**
      * 输出汉字。遇到不能输出的字符将停止。
      *
      * \param img   输出的影象
      * \param text  文本内容
      * \param pos   文本位置
      * \param color 文本颜色
      *
      * \return 返回成功输出的字符长度，失败返回-1。
      */
     int putText(IplImage *img, const wchar_t *text, CvPoint pos, CvScalar color);

     //================================================================
     //================================================================

  private:

     // 输出当前字符, 更新m_pos位置

     void putWChar(IplImage *img, wchar_t wc, CvPoint &pos, CvScalar color);

     //================================================================
     //================================================================

  private:

     FT_Library   m_library;   // 字库
     FT_Face      m_face;      // 字体
     int m_nFontSize;  //字体大小

     //================================================================
     //================================================================

     // 默认的字体输出参数

     int         m_fontType;
     CvScalar   m_fontSize;
     bool      m_fontUnderline;
     float      m_fontDiaphaneity;

     //================================================================
     //================================================================
     bool m_bInitFT;
};

#endif // OPENCV_CVX_TEXT

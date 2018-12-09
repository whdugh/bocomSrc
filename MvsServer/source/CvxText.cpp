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

#include <wchar.h>
#include <assert.h>
#include <locale.h>
#include <ctype.h>
#include "CvxText.h"
#include "Common.h"

CvxText::CvxText()
{
    m_nFontSize = 25;
    m_bInitFT = false;
}


CvxText::~CvxText()
{

}

//初始化
void CvxText::Init(int nFontSize)
{
	// 打开字库文件, 创建一个字体
 
	/*
	1, It creates a new instance of the FreeType 2 library, and sets the handle library to it.
	   2, It loads each module that FreeType knows about in the library. Among others, your new library object is able to handle TrueType, Type 1, CID-keyed & OpenType/CFF fonts gracefully.
	*/
	if(FT_Init_FreeType(&m_library)) throw;

	if(g_PicFormatInfo.nFont == 1)
	{
	    if(access("./simhei.ttf",F_OK)==0)
	    {
            if(FT_New_Face(m_library, "./simhei.ttf", 0, &m_face)) throw;
	    }
	    else
	    {
            if(FT_New_Face(m_library, "./simkai.ttf", 0, &m_face)) throw;
	    }
	}
	if(g_PicFormatInfo.nFont == 2)
	{
	    if(access("./simsun.ttc",F_OK)==0)
	    {
            if(FT_New_Face(m_library, "./simsun.ttc", 0, &m_face)) throw;
	    }
	    else
	    {
            if(FT_New_Face(m_library, "./simkai.ttf", 0, &m_face)) throw;
	    }
	}
	else
	{
        if(FT_New_Face(m_library, "./simkai.ttf", 0, &m_face)) throw;
	}

	// 设置字体输出参数
	m_nFontSize = nFontSize;

	restoreFont();

	// 设置C语言的字符集环境

	setlocale(LC_ALL, "");

	// 透明处理
	float p = 1;
	setFont(NULL, NULL, NULL, &p);

	m_bInitFT = true;
}

// 释放FreeType资源
void CvxText::UnInit()
{
    if(m_bInitFT)
    {
        FT_Done_Face    (m_face);
        FT_Done_FreeType(m_library);
		m_bInitFT = false;
    }
}


// 设置字体参数:
//
// font         - 字体类型, 目前不支持
// size         - 字体大小/空白比例/间隔比例/旋转角度
// underline   - 下画线
// diaphaneity   - 透明度
void CvxText::getFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{
     if(type) *type = m_fontType;
     if(size) *size = m_fontSize;
     if(underline) *underline = m_fontUnderline;
     if(diaphaneity) *diaphaneity = m_fontDiaphaneity;
}

void CvxText::setFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{
     // 参数合法性检查

     if(type)
     {
        if(type >= 0) m_fontType = *type;
     }
     if(size)
     {
        m_fontSize.val[0] = fabs(size->val[0]);
        m_fontSize.val[1] = fabs(size->val[1]);
        m_fontSize.val[2] = fabs(size->val[2]);
        m_fontSize.val[3] = fabs(size->val[3]);
     }
     if(underline)
     {
        m_fontUnderline   = *underline;
     }
     if(diaphaneity)
     {
        m_fontDiaphaneity = *diaphaneity;
     }
}

// 恢复原始的字体设置
void CvxText::restoreFont()
{
     m_fontType = 0;            // 字体类型(不支持)

     m_fontSize.val[0] = m_nFontSize;// 字体大小
     m_fontSize.val[1] = 0.5;   // 空白字符大小比例
     m_fontSize.val[2] = 0.1;   // 间隔大小比例
     m_fontSize.val[3] = 0;      // 旋转角度(不支持)

     m_fontUnderline   = false;   // 下画线(不支持)

     m_fontDiaphaneity = 1.0;   // 色彩比例(可产生透明效果)

     // 设置字符大小

     FT_Set_Pixel_Sizes(m_face, (int)m_fontSize.val[0], 0);
}

// 输出文本
int CvxText::putText(IplImage *img, const wchar_t *text, CvPoint pos, CvScalar color)
{
     if(img == NULL) return -1;
     if(text == NULL) return -1;

     //

     int i;
     for(i = 0; text[i] != '\0'; ++i)
     {
        // 输出当前的字符
		//printf("x=%d,y=%d\n",pos.x,pos.y);

        putWChar(img, text[i], pos, color);
     }
     return i;
}

// 输出当前字符, 更新m_pos位置
void CvxText::putWChar(IplImage *img, wchar_t wc, CvPoint &pos, CvScalar color)
{
     // 根据unicode生成字体的二值位图

     FT_UInt glyph_index = FT_Get_Char_Index(m_face, wc);
     FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
     FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_MONO);

     //

     FT_GlyphSlot slot = m_face->glyph;

     // 行列数

     int rows = slot->bitmap.rows;
     int cols = slot->bitmap.width;

     //

     for(int i = 0; i < rows; ++i)
     {
        for(int j = 0; j < cols; ++j)
        {
           int off  = ((img->origin==0)? i: (rows-1-i))
              * slot->bitmap.pitch + j/8;

           if(slot->bitmap.buffer[off] & (0xC0 >> (j%8)))
           {
              int r = (img->origin==0)? pos.y - (rows-1-i): pos.y + i;
              int c = pos.x + j;

			  if(wc == 0x2d || wc == 0x4e00)//针对"-"特殊处理
			  {
				  if(m_nFontSize <= 20)
				  {
					  r -= 5;
				  }
				  else if(m_nFontSize <= 25)
				  {
					  r -= 10;
				  }
				  else if(m_nFontSize <= 40)
				  {
					  r -= 15;
				  }
				  else if(m_nFontSize <= 50)
				  {
					  r -= 20;
				  }
				  else if(m_nFontSize <= 80)
				  {
					  r -= 28;
				  }
				  else if(m_nFontSize <= 100)
				  {
					  r -= 38;
				  }
				  else if(m_nFontSize > 100)
				  {
					  r -= 48;
				  }
			  }
			  //统一卡口、违章 图片叠字中 . / 0-9 : A-Z a-z 特殊处理
              if((wc >= 0x2e && wc <= 0x3a)|| (wc >= 0x41 && wc <= 0x5a)|| (wc >= 0x61 && wc <= 0x7a))
              {
				  if(m_nFontSize <= 20)
                  {
                     r -= 3;
                  }
                  else if(m_nFontSize <= 25)
                  {
                     r -= 4;
                  }
                  else if(m_nFontSize <= 40)
                  {
                     r -= 6;
                  }
                  else if(m_nFontSize <= 50)
                  {
                     r -= 8;
                  }
                  else if(m_nFontSize <= 80)
                  {
                     r -= 12;
                  }
                  else if(m_nFontSize <= 100)
                  {
                     r -= 16;
                  }
                  else if(m_nFontSize > 100)
                  {
                     r -= 24;
                  }
              }

              if(r >= 0 && r < img->height
                 && c >= 0 && c < img->width)
              {
                 CvScalar scalar = cvGet2D(img, r, c);

                 // 进行色彩融合

                 float p = m_fontDiaphaneity;
                 for(int k = 0; k < 4; ++k)
                 {
                    scalar.val[k] = scalar.val[k]*(1-p) + color.val[k]*p;
                 }

                 cvSet2D(img, r, c, scalar);
              }
           }
        } // end for
     } // end for

     // 修改下一个字的输出位置

     double space = m_fontSize.val[0]*m_fontSize.val[1];
     double sep   = m_fontSize.val[0]*m_fontSize.val[2];

     pos.x += (int)((cols? cols: space) + sep);
}

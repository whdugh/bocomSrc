//自适应地选取各种模型
#ifndef __MODEL_SELECTOR_H
#define __MODEL_SELECTOR_H
	
#include "libHeader.h"

//前景提取模式
enum ENUM_FKIMG_EXT_MODEL
{
	DEFAULT_FK_EXT_MODEL = 0,
	EASY_FK_EXT_MODEL,
	HARD_FK_EXT_MODEL
};

//角点提取模式
enum ENUM_KEYPT_EXT_MODEL
{
	DEFAULT_KEYPT_EXT_MODEL = 0,
	EASY_KEYPT_EXT_MODEL,
	HARD_KEYPT_EXT_MODEL
};

typedef struct StruModelSelector
{
public:
	StruModelSelector( );
	~StruModelSelector( );

private:
	void mvInitVar( );

private:
	ENUM_FKIMG_EXT_MODEL m_nFkImgExtrackModel;  //前景提取模式

	ENUM_KEYPT_EXT_MODEL m_nKeyPtsExtrackModel; //角点提取模式

}MvModelSelector;



//--------------------------------

#endif
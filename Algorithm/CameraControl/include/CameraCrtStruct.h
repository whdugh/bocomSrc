#ifndef CAMERACRTSTRUCT_H
#define CAMERACRTSTRUCT_H

#define CLIGHT_KINDS (6) /*定义用于计算顺光、逆光的种类的直方图*/
const float CEPSILONN = 0.0000001f;

#ifndef	_LIGHT_CARROAD
#define _LIGHT_CARROAD
enum LIGHT_CARROAD
{
	TOWARDS_LIGHT = 1,
	INV_LIGHT,
	LEFT_SIDE_LIGHT, 
	RIGHT_SIDE_LIGHT,
	NORMAL_LIGHT
}; // 张安发光线类型 1 为顺光 2 为逆光  3 为左亮右黑 4 为左黑右亮
#endif

enum _COLORCHNNL
{
	CB_G_R = 0,
	CR_G_B
};

enum _C_MODE_DN
{
	C_NIGHT = 0,
	C_DAY
};

#ifdef LINUX
	#define RGB_CHNNL_CAMERA
#endif

#ifdef LINUX
	//#define BG_SAVE_LIGHT
	//#define TEST_CAMERA_CONTROL_SAVE
#else

	//#define BG_TIME_PRINTF
	//#define DEBUG_CARD_BG_LIGHT
	//#define DEBUG_LINUX_CARDCAR
	//#define BG_LIGHT_PRINTF
	#define BG_SAVE_LIGHT
	#define TEST_CAMERA_CONTROL_SAVE

#endif










#endif
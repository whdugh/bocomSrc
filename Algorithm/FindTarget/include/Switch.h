#include "tinyxml.h"

// 和调试有关的开关、设置。
class Switch
{
public:
	//static bool CreateFolder;
	static bool SaveImages;
	static int  SaveImageWidth;
	static int  SaveImageHeight;
	static bool SaveInputParas;
	static time_t  SaveImageParasTimeStart;
	static time_t  SaveImageParasTimeEnd;

	static bool SaveClassVehicleType; //保存车型分类图片！
	// 保存红灯区域图片，防红灯闪烁功能。

	
	static bool SaveRedLightRgnImage;

	static bool SavePlateImage;

	static bool ForceStraightRedLight;
	
	static bool SaveBusVanType;


	// 防止在测试时，因为车辆来回开。两次经过检测区域时间间隔很短导致的车牌漏掉。
	static bool PreventPlateMiss;

	//修正视频速度相关！
	static bool DEBUG_CORRECT_VIDEO_SPEED;
	
	//控制雷达速度相关！
	static bool Debug_Radar_Speed;

	static void LoadSwtichValueFromXml(TiXmlNode *pConfigNode);

	//判断是否超速
	static bool OnTime;
};

#ifndef OBJ_TYPE_FOR_OUTPUT
#define OBJ_TYPE_FOR_OUTPUT
//车辆类型粗分；
enum ObjTypeForOutput
{
	OTHER = 1,     // 未知
	PERSON,        // 行人5
	TWO_WHEEL,     // 两轮车
	SMALL,         // 小 1
	MIDDLE,        // 中 2
	BIG,           // 大 3
};

////车辆类型细分；
//enum SubVehicleTypeForOutput
//{
//	SVT_Default = 0,     //0    未计算
//	SVT_Bus,             //1	大巴
//	SVT_Truck,           //2	卡车
//	SVT_MiniBus,       //3	中巴
//	SVT_Car,             //4	轿车
//	SVT_UnKnown,         //5    未知
//	SVT_WrongPos,        //6	车牌位置太偏
//	SVT_MiniVan         //7	小型货车
//};
#endif

#ifndef DETECT_TYPE_H
#define DETECT_TYPE_H

// 需要与应用端的CHANNEL_DETECT_KIND保持一致
#define DO_CAR_NUM_DETECT         0x001           // 是否进行车牌过滤。程序内部默认打开。
#define DO_FIND_TARGET            0x002           // 是否进行卡口目标检测
#define DO_ELE_POLICE             0x004           // 电子警察
#define DO_COLOR_DETECT           0x008           // 检测颜色
#define DO_VEHICLE_TYPE		      0x010           // 车型检测
#define DO_LOOP_DETECT            0x100           // 线圈检测
#define DO_ON_BUS_VIO_DET         0x200           //  车载检测
#define DO_RADAR_DETECT           0x400           // 雷达检测！
#define DO_VIOLATION_DETECT       0x800           //违章检测

#endif
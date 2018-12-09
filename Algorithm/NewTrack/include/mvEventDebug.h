// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
#ifndef __MV_EVENT_DEBUG_H
	#define __MV_EVENT_DEBUG_H	

	#include "DebugMacroDefine.h"
	//MVDEBUGON为事件调试的控制开关,若为发布版,则关掉该宏。
	//         该宏在DebugMacroDefine.h中定义。

	#ifdef MVDEBUGON
		#ifdef LINUX
			//#define DEBUG_TEST_ONLINUX  //注意!在服务器上不测试test2时需关掉,测试test2需打开
			//#define DEBUG_COFEATURE
		#endif
		//#define  MVDEBUGON1
		#define  MVDEBUGON2
	#endif

	#ifndef LINUX
		#define MVDEBUGTEST				//本地进行测试
	#else
		#ifdef DEBUG_TEST_ONLINUX
			#define MVDEBUGTEST  //单独进行时耗测试
		#endif
	#endif


	#ifdef MVDEBUGON
		#define MAXDEBUGLINE   40000       //调试信息的最大行数
		#define DEBUGLINEBYTE  256         //一行调试信息的字节数
		#define STOPJUDGEINTERVALTIME 10   //停车调试的保存间隔
		#define APPENDSTOPJUDGETIME 9      //停车调试的追加时间
		
		#define CHANGEJUDGEINTERVALTIME 10  //变道调试的保存间隔
		#define APPENDCHANGEJUDGETIME 3     //变道调试的追加时间

		//#define DEBUG_TJ_FLUX_VELO	//调试天津的流量和速度
	#endif

	#ifdef MVDEBUGON1
		//调试过程中主要用到的存文本和图像的宏(可通过配置文件来控制)
		#define  DEBUG_OBJECT_RESULT	//存最近3万张目标结果(5帧一张)
		#define  DEBGU_EVENT_TEST		//存事件结果
		#define  DEBGU_APPEAR_TEST      //存目标出现结果
		#define  DEBUG_SAVE_STABRESULT  //存稳像结果
		#define  DEBUG_CLIENT_SHOW      //在客户端显示目标结果
		#ifdef ALERTOBJ_ON_GIVELOCATION
			#define DEBUG_ALERTOBJ_ON_GIVELOCATION	//将目标出现报警于给定的位置
		#endif		

		//#define DEBUG_OBJECT_TYPE      //类型判断
		#ifdef DEBUG_OBJECT_TYPE
			#define DEBUG_TYPE_SIMHOG    //HoG检测及数据收集
		//	#define DEBUG_OBJECT_TYPE2
		#endif	

		//------以上宏，在调试中不需要关闭------


	//	#define  DEBUG_EVENT_ALERT    //事件报警调试
	//	#define	 DEBUG_ERROR          //调试异常
	//  #define  DEBUG_PEOPLERUN      //奔跑
	//  #define  DEBUG_PEOPLE_CROWD   //聚集
	//  #define  DEBUG_DERELICT       //遗弃
	//  #define  DEBUG_FANTATRACK	  //判断是否为奇异轨迹
	//  #define  DEBUG_SAVE_DETECTEDPEOPLE   //保存检测到的人
	//  #define  DEBUG_SAVE_RESULT     
	//	#define  LINUX_CARNUMOBJECT
	//	#define  DEBUG_PERSS_LINES    //压线报警调试

		#ifdef LINUX
			#define DEBUG_SAVE_RESULT
		#else	
			//#define DEBUG_EXTRACT_KEYPT	
			//#define DEBUG_SHOW_APPEAR
			//#define DEBUG_TYPEJUDGE_WINDOW
			//#define DEBUG_HIGHBUILD       //高楼监控
			//#define DEBUG_NORATION_TRACK  //对不合理的轨迹进行删除
			//#define DEBGU_EVENT_SAVE_DERELICT    //遗弃物的检测结果
			//#define DEBUG_ADDTR_TOVEH 	
			//#define DEBUG_TRACK_NOJOIN_VEHICLE   //对group中的有效轨迹无法参入到vehicle中进行调试
			//#define DEBUG_SHOWWINDOW     //显示窗口
			//#define DEBUG_APPEAR         //目标出现
			//#define DEBUE_ISDAY          //是否为白天
			//#define DEBGU_SIFT           //SIFT计算
			//#define DEBUG_SPLITANDMEGER  //分离与合并
			//#define DEBUG_SPLITANDMEGER2
			//#define DEBUG_CROWSTOPCONFIRM      //拥堵停车的确认
			//#define DEBGU_MERGE_SMALL_VEHICLE  //对小车进行合并
			//#define DEBUG_SAVE         //mvget_result 保存结果图像
			//#define DEBUG_SAVE_ALL     //文本和图像的保存
			//#define DEBGU_EVENT_SAVE   
			//#define DEBGU_HYP_SAVE 
			//#define DEBUG_PRINT_VEHICLE    //对vehicle信息进行打印
			//#define DEBUG_VEHICLE_HISTORY  //对车辆的质心历史进行考察		
			//#define DEBUG_BG_CORNER           //背景角点
			//#define DEBUG_DIRECTION           //轨迹的方向计算    
			//#define DEBUG_GRAYIMG_BG          //grayImg背景
			//#define DEBUG_DERELICT			//遗撒检测
			//#define DEBUG_EXISTLONGVEHICLE	//大车判断
			//#define DEBUG_STASTATICS			//交通参数统计
			//#define DEBUG_BIGVEHICLE_SOBEL  
			//#define DEBUG_FORBBID                        
			//#define DEBUG_ADDTRTOVEHICLE	//将没有veh_idx的轨迹加入到合适的vehicle中
			//#define DEBUG_TRACK           //轨迹拥堵性判断
			//#define DEBUG_SURF
			//#define DEBUG_LCSS		  //基于编辑距离
			//#define DEBUG_EVENT_CROSS	  //横穿调试
			//#define DEBUG_DTW
		#endif  //#ifndef LINUX
	#endif


	//以下供贺、杨单独调试使用
	#ifdef MVDEBUGON2
		#ifndef LINUX 
			#define DEBUG_HYP
		#endif 
	#endif 


	#ifdef DEBUG_HYP
		//#define DEBUG_TIME
		#define DEBUG9  //mvget_result模块的输出,和显示

		#define  DEBGU_MERGE_BURNLINE   //线段的合并
		//#define  DEBUG_MATCHTR
		//#define  DEBUG_TR  //轨迹匹配
		//#define  DEBUG_VIBE_BACKGROUND

		//#define  DEBUG_TYPE_WIN32  //window下的目标类型判断
		//#define  DEBUG_SAVE_DETECTEDPEOPLE  //保存检测到的人
		//#define  DEBUG_VIBE_BACKGROUND      //张安发的背景模型

		//#define  DEBGU_MERGE_BURNLINE   //线段的合并
		//#define  DEBUG_GROUPING         //grouping
		//#define  DEBUG_EXIST_BIGVEHICLE
		//#define  DEBGU_EVENT_TEST     //存事件结果
		//#define  DEBGU_MERGER_LINE    //合并后的线段 
		//#define  DEBUG_JUDGE_PEOPLE
		//#define  DEBUG_HOGJUDGEAREA
		//#define  DEBUG_HOGJUDGE
		//#define  DEBUG_OBJADJUST
		//#define  DEBUG_TIME_TYPE	 //类型判断耗时的文本保存
		//#define  DEBUG_TIME_ADJU	 //目标调整耗时的文本保存
		//#define  DEBUG_TIME		 //耗时的文本保存

		//#define  DEBUG_TYPE_SOBEL   //利用sobel判断类型
		//#define  DEBUG_TYPE_LINE	  //利用线段判断类型
		//#define  DEBUG_TYPE_FOREIMG	//利用前景判断类型

		//#define  DEBUG_TIME_TYPE		//类型判断耗时的文本保存
		//#define  DEBUG_PEOPLERUN      //奔跑
		//#define  DEBUG_PEOPLE_CROWD   //聚集
		//#define  DEBUG_BGMODEL_SHOW
		//#define  DEBUG_TIME			//各阶段耗时的文本保存

		//#define  DEBUG_SAVE_BGCORNERANDLINE

		//#define  DEBUG_MEDIANBG
	
		//#define  DEBUG_DERELICT
	#endif

	//运行过程中的时间消耗
	#ifdef DEBUG_RUNNING_TIME
		#define  DEBUG_TIME			 //各阶段耗时的文本保存
		#define  DEBUG_FRAMETIME	 //存该帧总耗时的文本保存
		#define  DEBUG_TIME_FORE     //前景检测耗时的文本保存
		#define	 DEBUG_TIME_TYPE	 //类型判断耗时的文本保存
		#define  DEBUG_TIME_HOG      //HOG检测耗时的文本保存
	#endif

	//背景和前景模型
	#ifdef DEBUG_BGFK_MODEL
		#define DEBUG_BGMODEL_FOREIMG   //背景模型和前景图
		#define DEBUG_GETFOREIMG        //抠取前景
		#define DEBUG_GETFOREIMG_MAIN   //抠取前景
		#define USE_QUICK_BGLINE		//较为快速的背景线段模型(10s更新一次)
		#define DEBUG_BGMODEL_SHOW      //背景模型的显示
		#define DEBUG_FOREGROUND_SHOW   //前景的显示
	#endif

	//绘制各步骤的结果
	#ifdef DEBUG_DRAW_STEPRESULT
		#define  DEBUG_DRAW_STEPRESULT_PROCESS   //图像处理阶段的结果
		#define  DEBUG_DELETE_TRACK              //轨迹删除的调试
		#define  DEBUG_DRAW_STEPRESULT_TRGROUP   //轨迹聚类阶段的结果
		#define  DEBUG_DRAW_STEPRESULT_VEHICLE   //目标构造阶段的结果
		#define  DEBUG_DRAW_STEPRESULT_ADJOBJ	 //目标调整阶段的结果
		#define  DEBUG_DRAW_STEPRESULT_MAIN	     //系统主阶段的结果
	#endif

	//目标类型判断调试
	#ifdef DEBUG_OBJECT_TYPE2
		#define  DEBUG_HOG     //利用hog判断目标是否为行人
		//#ifdef USE_LINEHOG
		//	#define  DEBUG_LINEHOG    //显示线段hog行人检测
		//#endif

		#define  DEBUG_TYPE_TXT          //存类型判断的文本信息
		#define  DEBUG_CONFIRM_DEFTYPE   //对目标的默认类型进行确认
		#define  DEBUG_TYPE_WITH_PEOPLE  
		#define  DEBUG_TYPE              //目标类型判断
		#define  DEBUG_TYPE_DEFALUT      //目标默认类型
		#define  DEBGU_TYPE_NIGHT 
		#define  DEBUG_TYPE1
		#define  DEBUG_TYPE2
		#define  DEBUG_TYPE3
		#define  DEBUG_TYPE4
		#define  DEBUG_TYPE_CLOSE
		#define  DEBUG_VEHICLE_TYPE    //判断目标是否为车
		#define  DEBGU_TYPEJUDEG       //目标类型判断
	#endif

	//原来的各模块调试
	#ifdef DEBUG_MODULE_OLD
		#define DEBUG0		//disjoint-set模块的输出
		#define DEBUG1  
		#define DEBUG2		//车辆和轨迹信息的输出
		#define DEBUG3		//分离模块的输出
		#define DEBUG4		//合并模块的输出
		#define DEBUG5		//删除模块的输出
		#define DEBUG6		//背景提取模块的输出
		#define DEBUG7		//匹配模块的输出
		#define DEBUG10		//track平移、估计模块 
		#define DEBUG12		//max_size的窗口
		#define DEBUG13		//拥堵判断
		#define DEBUG14		//合理性判断
		#define DEBUG16		//线段背景的文字输出
	#endif

	//大车检测
	#ifdef DEBUG_BIGVEHICLE_DETECT
		#define  DEBUG_BIG_VEHICLE_JUDGE                   //大车判断时的结果
		#define  DEBUG_BIG_VEHICLE_JUDGE_WITH_LINECOM      //大车判断时的结果
		#define  DEBUG_MERGER_BIG_VEHICLE               //新版大车合并时的结果
		#define  DEBUG_BIGVEHICLE_TRACK                    //大车跟踪 
		#define  DEBUG_BIGVEHICLE_LOCAL                    //大车定位 
		#define  DEBUG_BIGVEHICLE_DETECT_LINE3COM_SURE     //大车检测的进一步确认 
		#define  DEBUG_BIGVEHICLE_MEGER_DRAW
		#define  DEBUG_BIGVSWITCH3_DETECT_DRAW   //大车检测中3叉结构 
		#define  DEBUG_BIGVVERTIC_DETECT_DRAW    //大车检测中垂直结构 
		#define  DEBUG_BIGVPARALL_DETECT_DRAW    //大车检测中平行结构 
		#define  DEBUG_FILTER_LINECOM3           //对3边组合体进行过滤
		#define  DEBUG_OLDBIGV_JUDGE             //原大车判断
		#define  DEBUG_BIGV_DETECT_OLD
		#define  DEBUG_3DMODEL_AREA              //大车3D模型的判断区域
		#define  DEBUG_3DMODEL                   //大车3D模型
		#define  DEBUG_3DMODEL_TRACK             //将在大车3D模型内的轨迹加入到目标
	#endif
	 
	//vehicle的检测
	#ifdef DEBUG_VEHICLE_DETECT  
		#define  DEBUG_VEHICLE_DETECT_2  
		#define  DEBUG_VEHICLE_DETECT_3
		#define  DEBUG_VEHICLE_DETECT_4
		#define  DEBUG_VEHICLE_DETECT_5
	#endif

	//vehicle的检测
	#ifdef DEBUG_VEHICLE_DETECT  
		#define  DEBGU_MERGE_VEHICLE        //vehicle的合并
		#define  DEBUG_MERGER_BIGVEHICLE    //对大车进行合并
		#define  DEBUG_MERGER_SPLITVEHICLE  //对分碎目标进行合并
		#define  DEBUG_MERGER_SMALL_VEHICLE //合并小车时的结果
		#define  DEBUG_LINEMERGER_CONDITION //合并小车时线段的要求
	#endif

	//阴影的检测
	#ifdef DEBUG_SHADOW_LIGHT_DETECT  
		#define  DEBUG_DETECT_SHADOW_TRACK  //检测阴影轨迹
		#define  DEBUG_TRACK_SHADOW         //阴影
		#define  DEBUG_SHADOW_DIR		    //阴影方向确定
		#define  DEBUG_SHADOW_TWOOBJ		//候选阴影和阴影制造者

		#define  DEBUG_SHADOW_WITH_FOREIMG  //利用前景信息判阴影
		#define  DEBUG_SHADOW_NOJOIN        //阴影不参入报警
		#define  DEBUG_SHADOW_DIR           //阴影方向

		#define  DEBUG_LIGHT_DETECT          //灯光检测
	#endif

	#ifdef DEBUG_YKP
		//#define  DEBUG_TIME			   //存各阶段的耗时的文本保存
		//#define DEBUG2 //车辆和轨迹信息的输出
		//#define  DEBUG_GROUPING           //grouping
		//#define  DEBUG_TR_GROUPING
		//#define DEBUG_PRINT_VEHICLE
		#define DEBUG9  //mvget_result模块的输出,和显示
		#define DEBGU_MERGE_BURNLINE
		#define  DEBGU_MERGER_LINE   //线段的合并 
		//#define  DEBUG_BG_LINE				//背景线段
		//#define DEBUG_BGMODEL_SHOW          //背景模型的显示
		//#define DEBUG_FOREGROUND_SHOW  //前景的显示
		//#define DEBUG_ACTIVITY_ANALYSIS
		//#define DEBUG_NEWLSD
		//#define DEBUG_COFEATURE
		//#define DEBUG_CARNUM
		//#define  DEBUG_STATIC_FEATURE
		#define DEBUG_VIBE_BACKGROUND
		#define DEBUG_CROWD
	#endif

	#ifdef DEBUG_GROUPING
		//#define  DEBUG_GROUPING_PRINT_BG  //写出背景角点，grouping
		//#define  DEBUG_GROUPING_SCANF_BG  //读入背景角点，grouping
		//#define  DEBUG_GROUPING           //grouping
		//#define  DEBUG_TR_GROUPING
	#endif

	#ifdef DEBUG_LINES_CORNER
		//#define  DEBGU_MERGE_BURNLINE   //合并线段
		//#define  DEBGU_MERGER_LINE      //线段的合并
		//#define  DEBUG_BG_LINE		  //背景线段

		//#define  DEBUG_LONGTERM_BG       //长期背景角点
		//#define  DEBUG_LONGTERM_SOBEL_BG //长期的sobel背景
	#endif

	#ifdef DEBUG_WRONG_ALERT
		//#define  DEBUG_NOALTER_APPEAR     //不报目标出现
		//#define  DEBUG_NOALTER_APPEAR2    //不报越界后的大车的车顶
		//#define  DEBUG_ERROR_ALERT        //事件报警去误检
		//#define  DEBUG_ERROR_ALERT_TRACK  //事件报警去误检(轨迹)
		//#define  DEBUG_ERROR_ALERT_BIGV   //事件报警去误检(大车)
		//#define  DEBUG_ERROR_ALERT_FAST   //事件报警去误检(超速)

		//#define  DEBUG_TEMPVEHICLE_NOAPPEAR_SET  //对晚上抓地进行处理

		//#define  DEBUG_LIGHT_NOAPPEAR_SET     //设置灯光不报
		//#define  DEBUG_SHADOW_NOAPPEAR_SET    //设置阴影不报
		//#define  DEBUG_MISMATCH_NOAPPEAR_SET  //设置误匹配不报
		//#define  DEBUG_NOSUITBG_NOAPPEAR_SET  //设置背景误报不报
		//#define  DEBUG_SKIPAREAOBJ_NOAPPEAR_SET  //设置屏蔽区域目标不报
		//#define  DEBUG_NOAPPEAR_SET           //设置一些目标不报出现
	#endif

	#ifdef	DEBUG_SAVE_RESULT
		//#define  DEBGU_EVENT_TEST      //存事件结果	
		//#define  DEBGU_APPEAR_TEST     //存目标出现结果
		//#define  DEBUG_TYPE_TXT        //存类型判断的文本信息
		//#define  DEBUG_STOP_TXT        //存停车判断的文本信息
		//#define  DEBUG_TIME			 //存各阶段的耗时的文本保存
		//#define  DEBUG_SAVE_VEHICLE_RESULT    //vehicle结果（每帧均存,由ifSaveAll.txt控制;含track,group,vehicle,line）
		//#define  DEBUG_HOG             //存利用Hog进行行人检测时的图像
		//#define  DEBUG_3DMODEL         //存最近1万张3D模型结果（含存三叉和区域）
		//#define  DEBGU_APPEAR_FORB			//非法的目标出现结果
		//#define  DEBUG_PRINT_SHADOW			//对阴影方向的计算和判断
		//#define  DEBUG_EVENTDETECT_CONFIG		//从配置文件中读文件来设置事件检测的参数
		//#define  USE_SOBEL_BGIMG				//sobel背景模型 
		//#define  DEBUG_BIGVEHICLE_DETECT_DRAW //大车检测 
		//#define  DEBUG_Line3ComDetect			//线段3边组合来检测大车
		//#define  DEBUG_HOG_COLOR				//存颜色判断时的行人hog检测区域
	#endif
  

#endif

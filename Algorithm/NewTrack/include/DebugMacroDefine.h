#ifndef __DEBUG_MACRO_DEFINE_H

#define __DEBUG_MACRO_DEFINE_H

//--------------控制宏的定义------------
#define AN_VERSION_NUBER ("Version2013.12.05") //版本序号

//--------------调试宏的定义------------
//#ifndef LINUX
//	#define DEBUG_ON  //调试总开关(若为发布版,请关掉该宏)
//#endif

//#define DEBUG_FOR_BEIJING_QUEUE_LEN  //调试北京队列长度

#ifdef DEBUG_ON
	#ifndef LINUX 
		#define SHOW_MAINSTEP_RESULT //显示主要的运行结果
	#endif
	//#define DEBUG_ON_ANALYSIS  //ANALYSIS中的主要调试
	//#define DEBUG_CONFIG       //对读取的配置信息进行调试
	//#define MVDEBUGON			 //事件检测的调试开关宏

	//临时调试的宏，宏里面的代码一般生命周期不长
	//#define DEBUG_TEMP_HYP   //贺岳平临时调试的宏，他人使用时请关闭
	//#define DEBUG_TEMP_GXJ   //顾晓军临时调试的宏，他人使用时请关闭
#endif  //DEBUG_ON


#ifdef DEBUG_ON_ANALYSIS
	//获取程序的启动初始时间，作为一个全局参数
	//#define DEGBUG_GOBAL_INITDATETIME

	//打印进出信息
	//#define PRINTF_INOUT_INFO       

	//时耗测试
	//#define DEBUG_ANALYSIS_TIME    

	//背景和前景处理过程
	//#define DEBUG_FKBGIMG

	//角点提取调试
	//#define SHOW_ANALYSIS_KEYPOINT    

	//轨迹形成调试
	//#define SHOW_ANALYSIS_TRACK    
 
	//Group形成调试
	//#define SHOW_ANALYSIS_GROUP   

	//目标检测调试
	//#define SHOW_ANALYSIS_OBJDETECT   

	//初始化过程显示
	//#define DEBUG_INIT_RESULT       

	//图像处理过程
	//#define DEBUG_IMGPROCESS

	//网格停车法检测
	//#define DEBUG_GRID_STOP      

	//图像网格法
	#define DEBUG_IMAGE_GRID
	
	//漏报和误报调试
	//#define DEBUG_LOSS_ERROR_STOP

	//线段提取调试
	//#define DEBUG_SHOW_LINE_EXT_RESULT2

	//目标类型判断的调试
	//#define DEBUG_OBJTYPE_JUDGE	
	
	//照射光区域检测
	//#define DEBUG_BRIGHT_LAMP_LIGHT_DETECT

	//新的匹配跟踪方法
	//#define DEBUG_PT_MATCH_TRACK  //新的点SIFT匹配跟踪
	//#define DEBUG_RECTS_TLD_TRACK   //TLD跟踪
	
	//轨迹属性调试
	//#define DEBUG_TRACK_PROPERTY

	//保存停车缓存
	//#define SAVE_STOP_CACHE_DEBUG  

	//对目标信息的获取进行调试
	//#define DEBUG_GET_OBJ_INFO

	//---------------------detail----------------------//
	//打印进出信息   
	#ifdef  PRINTF_INOUT_INFO
		#define PRINTF_INOUT_INIT_INFO      //打印初始化信息
		#define PRINTF_INOUT_UNINIT_INFO    //打印释放信息
		#define PRINTF_INOUT_MAINSTEP_INFO  //打印主要步骤信息
		#define PRINTF_INOUT_DETAIL_INFO    //打印详细信息
	#endif

	//时耗测试
	#ifdef DEBUG_ANALYSIS_TIME 
	//	#define DEBUG_ANALYSIS_TOTAL         //总时耗
	//	#define DEBUG_ANALYSIS_MAINSTEPP     //主要步骤的时耗
		#define DEBUG_ANALYSIS_VIBEBGMODEL   //ViBe背景模型的时耗
	    #define DEBUG_ANALYSIS_MEDIANBGMODEL //中值背景模型的时耗
	//	#define DEBUG_ANALYSIS_TRACKGROUP    //轨迹形成group的时耗
	#endif

	//背景和前景处理过程
	#ifdef DEBUG_FKBGIMG
	//	#define SAVEIMG_FOR_FKIMGCHECK     //存图来查前景图
	//	#define SAVEIMG_FOR_BGIMGCHECK     //存图来查背景图

	//	#define SHOW_ANALYSIS_VIBE_FK      //ViBe前景
		
		//显示中值背景模型
	    #define SHOW_MEDIAN_BGMODEL
		#ifdef  SHOW_MEDIAN_BGMODEL
		//	#define SHOW_MEDIAN_BGIMG_RESULT   //灰度中值背景的结果
			#define SHOW_RGB_MEDIAN_BGIMG      //彩色中值背景的结果
			#define SHOW_GET_FKIMG_RGBMEDIAN   //彩色背景模型抠取前景
		#endif

		//ghost移除和运动静止前景获取
	//	#define DEBUG_GHOST_MVSTFK    
		#ifdef DEBUG_GHOST_MVSTFK
			#define SHOW_GRID_GHOSHT   //网格ghost
		//	#define SHOW_MOVE_STOP_FK  //运动静止前景
			#ifdef SHOW_GRID_GHOSHT
				#define TEST_GHOSTAREA_JUDGE //测试ghost区域
			#endif
		//	#define SHOW_CALC_EDGE_SIMILAR   //相似性计算
		#endif

		//阴影移除
	//  #define DEBUG_SHADOW_REMOVE    
		#ifdef DEBUG_SHADOW_REMOVE
		//	#define SHOW_STOP_SHADOW_DETECT   //静止阴影的检测
		#endif

	//	#define TEST_AREA_CHANG4BG  //测试区域的背景是否改变
	//	#define TEST_ILLUM_CHANGE   //测试光照度是否发生改变
	#endif

	//亮区/车灯/灯光检测
	#ifdef DEBUG_BRIGHT_LAMP_LIGHT_DETECT		
		//#define DEBGU_NIGHT_BRIGHTAREA  //夜间亮区判断
		//#define DEBUG_VEHICLE_WITH_LAMP //车灯检测
		//#define SHOW_LAMP_LIGHT_AREA	//车灯的照射区域
		//#define SHOW_LINE_SLOPE         //对衰减区域拟和线段

		//#define SHINEDETECT_USE_TIME    //灯光判断的耗时

		//#define DEBUG_GIVEAREA_IS_LIGHT //判断给定区域是否为灯光
	#endif

	//初始化过程显示
	#ifdef DEBUG_INIT_RESULT
		//各种区域
	    //#define SHOW_SKIP_MASK_IMG     //忽略区域和mask区域 

		//标定及坐标
		#define  DEBUG_CALCOORD    
		#ifdef DEBUG_CALCOORD
			#define SHOW_CALIBRATE_CONGIG       //显示标定配置结果
			#define SHOW_ORI_CALC_RESULT        //显示方向计算结果
			#define SHOW_CARSZ_CALC_RESULT      //显示车大小计算结果
		#endif
	#endif


    //图像处理过程
	#ifdef DEBUG_IMGPROCESS
	//	#define DEBUG_SHOW_LINE        //线段提取结果
	//	#define DEBUG_SHOW_SOBEL       //sobel提取结果
	//	#define SHOW_OPTIFLOWBM        //光流法块匹配
	//	#define DEBUG_PRINTF_ILLUM     //打印光照度信息
	#endif
	

	//网格停车法检测    
	#ifdef DEBUG_GRID_STOP	
		#define SAVE_INIT_GRID           //保存初始网格
		#ifndef LINUX
		//	#define DEBUG_USE_MULTI_TIMESTAMP  //使用多倍的时间戳来加快现实中时间速度
			#ifdef DEBUG_USE_MULTI_TIMESTAMP
				#define TIMESTAMP_U2R_RATIO  1.0  //1.0使用的和实际的时间戳间隔比
			#endif
		#endif	
		#define DEBUG_GRID                 //网格法
		#ifdef DEBUG_GRID                  
		//	 #define SHOW_ONEGRID_FEAT	        //显示某一网格的特征
		//	 #define SHOW_IF_EXIST_LOT_STOPCAR  //显示是否存在大量的停止的车
		//	 #define SHOW_IMAGE_ENHANCE		    //显示图像增强
		//	 #define SHOW_EXPECT_IMAGE			//显示图像预期

		//	 #define TEST_CORRELATION			//测试相关性
			 #ifdef  TEST_CORRELATION
			//	#define TEST_CORRELATION_EVERY_FRAM_BLOCK //对每帧/块测试相关性
			 #endif
 
		//	 #define DEBUGE_HAVE_OBJECT			//调试是否存在目标
		//	 #define DEBUG_GRID_TIME			//测试时耗
		#endif
	#endif

	 
	//图像网格法    
	#ifdef DEBUG_IMAGE_GRID	
		#define DEBUG_IMGGRID_JAMALERT   //图像网格法进行拥堵检测
		//#define DEBUG_IMGGRID_QUEUE_LEN  //图像网格法计算队列长度
	#endif

	//目标类型判断的调试
	#ifdef DEBUG_OBJTYPE_JUDGE
		#define DEBUG_HOG_PEO_DET //HoG行人检测
		#ifdef DEBUG_HOG_PEO_DET
			#define DEBUG_HOGSCAN  //HoG扫瞄
			#ifdef DEBUG_HOGSCAN
				#define TEST_HOG_GXJ  //使用寻找人头区域，再用HOG模型检测宏
				//#define SHOW_DETECT_AREA        //显示检测区域
				//#define SHOW_FIND_HEAD_RESULT   //定义人头顶点检测显示结果的宏
				//#define SHOW_RESULT
				//#define DETECT_BIG_REGIN        //检测大区域（区域中可能含有人头）
				//#define USE_PEO_MODEL  //使用行人检测
				//#define USE_HIGHHEAD_MODEL  //使用检出率高的人头模型
			#endif
		#endif
		#define SHOW_NEW_HOGDET_RESULT //显示新HoG检测结果
		#define SAVE_NEW_HOGDET_RESULT //保存新HoG检测结果
	#endif

	//新的点匹配跟踪模块的调试
	#ifdef DEBUG_PT_MATCH_TRACK
		//#define DEBUG_SHOW_TRACK		   //显示轨迹
		  #define DEBUG_SHOW_KEYPT_MATCH   //显示点匹配情况
		//#define SAVE_SIFTTRACK_IMG      //保存Sift角点提取和匹配跟踪的图像
		//#define TEST_SIFT_THROD         //测试寻找SIFT匹配阈值
		//#define DEBUG_SHOW_DIFF_TRACK   //显示不同轨迹
	#endif

	//轨迹属性调试
	#ifdef DEBUG_TRACK_PROPERTY
		//#define DEBUG_SHOW_AGAINST_CROSS_TR   //显示逆行横穿轨迹
		//#define DEBUG_BIGVEHICLE_TOP          //大车顶部
		//#define SHOW_MOVE2STOP_TRACKS           //显示由动到静轨迹
	#endif
	
	//HOG检测
	//#define DEBUG_SHOW_PEODET_RESULT  //显示行人检测结果

	////利用LTD对区域进行跟踪                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
	#ifdef DEBUG_RECTS_TLD_TRACK	
		#define DEBUG_SHOW_TLD_RESULT   //显示LTD结果
		#define SHOW_TLD_SHOOTH_RESULT  //显示LTD平滑后的结果
	#endif


	//获取目标信息
	#ifdef DEBUG_GET_OBJ_INFO
		//#define SHOW_VEHICLE_INFO   
		//#define SHOW_VEH_BOT_LOC   //车底定位
		#ifdef SHOW_VEH_BOT_LOC    //车底定位
			#define SHOW_VEH_BOT_SHADOW_LOC   //车底阴影
			#define SHOW_VEH_BOT_HORIEDGE_LOC  //车底水平边
		#endif

		//#define SHOW_VEH_BOT_LOC_SYMMETRY //车对称轴
	#endif

#endif  //DEBUG_ON_ANALYSIS

#ifdef DEBUG_TEMP_GXJ
		//#define SHOW_TEMP_RESULT
		//#define SHOW_NEW_SHADOW_TEMP_IMAGE		//论文的中间显示图
		//#define SAVE_IMAGE
		//#define SHOW_TEMP_IMAGE
		//#define USE_DETECT_CAR_BOTTOM
		/*#ifdef MVDEBUGON
				#define TEMP_DEBUG
		#endif*/
		#define  SHOW_OBJECT_V
#endif


#ifdef DEBUG_CONFIG   //调试配置
		#ifndef LINUX  //window下本地调试
		//	#define DEBUG_CONGIG_STAB_IN_CODE   //在程序内写代码来配置稳像
		#endif
#endif  //DEBUG_CONFIG


#endif

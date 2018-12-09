#ifndef XINGTAICENTER_H
#define XINGTAICENTER_H

#include "structdef.h"
#include "Common.h"
#include "CSeekpaiDB.h"
#include "XmlParaUtil.h"
#include "CSocketBase.h"
#include "CSeekpaiDB.h"
//



//#ifndef mapMaxSpeed
//typedef map<UINT32,UINT32> mapMaxSpeed; //车道对应的限速map
//#endif
//
//#ifndef mapChanMaxSpeed
//typedef map<UINT32,mapMaxSpeed> mapChanMaxSpeed; //某通道对应的限速map
//#endif

#ifndef MsgResult
typedef std::list<std::string> MsgResult;
#endif

#ifndef CS_MSG_QUEUE
typedef multimap<string, string> CS_MSG_QUEUE;
#endif

////心跳包
//typedef struct _HeartBeatPacket
//{
//	UINT16 nHeader;//包头
//	UINT16 nType;//包类型
//	UINT32 nLength;//值的长度
//	UINT32 nValue;//值
//	UINT16 nTail;//包尾
//	_HeartBeatPacket()
//	{
//		nHeader = 0xAAAA;
//		nType = 1103;
//		nLength = 4;
//		nValue = 0xAAAAAAAA;
//		nTail = 0x5555;
//	}
//
//}HeartBeatPacket;

//车牌信息包体 
typedef  struct  _Type_Struct_Frt_Vhc_Data                                 
{                                                                         
	UINT32     bid;             //本包标志序号  （递增的）                        
	char     cphm1[13];        //前拍车牌号码                                   
	char     cphm2[13];        //后拍车牌号码                                   
	char     cplx1;            //前拍车牌类型                                   
	char     cplx2;            //后拍车牌类型                                   
	char     cpys;             //车牌颜色                                     
	char     wflx[20];            //违法类型                           
	char     csys;             //车身颜色                                     
	char     tgsj1[28];         //时间1                                     
	char	 tgsj2[28];         //时间2                                     
	char     tgsj3[28];         //时间3                                     
	char     tgsj4[28];         //时间4                                     
	float    speed;             //速度                                      
	float    speedlimit;        //限速                                      
	char     jcdid[9];         //监测点编号                                    
	char     qdid[5];          //前端id                                     
	char     xsfx;             //行驶方向                                     
	char     cdid[6];          //车道编号                                     
	char     cllx[5];          //车辆类型，这里修改了一下，原来的是  char  
	short    zxd;            //置信度                                        
	UINT32     gkjip;           //图片存放工控机IP                                 
	char     tpzs;            //图片张数                                      
	char     tpid1[100];        //图片序号1    这里可能是溢出的，即28字节未必够用
	char     tpid2[100];        //图片序号2                                    
	char     tpid3[100];        //图片序号3                                    
	char     tpid4[100];        //图片序号4                                    
	char     bl[16];           //保留字段                                     
	char     cb[16];          //车标                                        
	char     qpsfwc;           //前拍是否完成标志                                 
	char     hpsfwc;           //后拍是否完成标志                                 
	char     qhsfyz;           //前后拍是否一致                                  
	UINT32     fbcd;           //方波长度                                       
	UINT32     cpcd1;          //图片1长度                                      
	UINT32     cpcd2;          //图片2长度                                      
	UINT32     cpcd3;          //图片3长度                                      
	UINT32     cpcd4;          //图片4长度                                      
	char     bcbz;           //补传标志  
	_Type_Struct_Frt_Vhc_Data()
	{
		bid = 0;
		memset(cphm1,0,13);
		memset(cphm2,0,13);
		cplx1 = '\0';
		cplx2 = '\0';
		cpys = '\0';
		memset(wflx,0,20);
		csys = '\0';
		memset(tgsj1,0,28);
		memset(tgsj2,0,28);
		memset(tgsj3,0,28);
		memset(tgsj4,0,28);
		speed = 0.0;
		speedlimit = 0.0;
		memset(jcdid,0,9);
		memset(qdid,0,5);
		xsfx = '\0';
		memset(cdid,0,6);
		memset(cllx,0,5);
		zxd = 0;
		gkjip = 0;
		tpzs = '\0';
		memset(tpid1,0,100);
		memset(tpid2,0,100);
		memset(tpid3,0,100);
		memset(tpid4,0,100);
		memset(bl,0,16);
		memset(cb,0,16);
		qpsfwc = '\0';
		hpsfwc = '\0';
		qhsfyz = '\0';
		fbcd = 0;
		cpcd1 = 0;
		cpcd2 = 0;
		cpcd3 = 0;
		cpcd4 = 0;
		bcbz = '\0';
	}

}__attribute__((packed)) Type_Struct_Frt_Vhc_Data; 


//车牌信息+图片包
//typedef struct _PlatePicPacket
//{
//	UINT16 nHeader;//包头
//	UINT16 nType;//包类型
//	UINT32 nLength;//值的长度(sizeof(Type_Struct_Frt_Vhc_Data) + nStrPicData.size())
//	Type_Struct_Frt_Vhc_Data nVhcData;//车牌信息
//	string nStrPicData;//图片信息
//	UINT16 nTail;//包尾
//	_PlatePicPacket()
//	{
//		nHeader = 0xAAAA;
//		nType = 1101;
//		nLength = 0;
//		nStrPicData.clear();
//		nTail = 0x5555;
//	}
//
//}PlatePicPacket;
                 
typedef struct _SYSTEMTIME {                  
	unsigned short wYear;                               
	unsigned short wMonth;                              
	unsigned short wDayOfWeek;                          
	unsigned short wDay;                                
	unsigned short wHour;                               
	unsigned short wMinute;                             
	unsigned short wSecond;
	unsigned short wMilliseconds;    
	_SYSTEMTIME()
	{
		wYear = 0;
		wMonth = 0;
		wDayOfWeek = 0;
		wDay = 0;
		wHour = 0;
		wMinute = 0;
		wSecond = 0;
		wMilliseconds = 0;
	}
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME; 


class XingTaiCenter:public mvCSocketBase
{
public:
	XingTaiCenter();

	~XingTaiCenter();

	bool AddResult(std::string& strResult);

	void DealResult();

	bool OnResult(std::string& result);

	bool Init();

	void UnInit();

	/*void GetChanLimitSpeedMap();*/

	string GetOverSpeedCode(RECORD_PLATE& nPlate);

	UINT32 GetLimitSpeed(RECORD_PLATE& nPlate);

	string CarNumColorTansform(RECORD_PLATE& nPlate);

	string CarNumTypeCovert(RECORD_PLATE& nPlate);

	string CarDirectionCovert(RECORD_PLATE& nPlate);

	string GetPlatePicPath(RECORD_PLATE& nPlate,int index);

	bool ConnectToCS();

	bool GetCenterLinkFlag();

	int GetCenterSocket();

	int GetHeartBeatFailureCount();

	void CloseCenterSocket();

	bool SendMsg(const int nSocket,const string& strData,int nType);

	void encryption(char* buf,size_t len);

	string GeneratePlateAndPicMsg(RECORD_PLATE *sPlate);

	string GenerateViolationCode(RECORD_PLATE& nPlate);

	bool mvRecvSocketMsg(int nSocket);

	bool OnSysTimeSetup(SYSTEMTIME &nSysTemTime);

	void SetHeartBeatCount(int& nHeartBeatCount);

	bool GetPlateHistoryRecord(string &strRecord);

	UINT32 GetSpeed(int nChannel,int nRoadId);

	string GetComposedSingleImageByPath(RECORD_PLATE& plate,UINT32& nPicSize1,UINT32& nPicSize2,UINT32& nPicSize3,UINT32& nPicSize4);

	//存图测试
	void SaveImageTest(unsigned char* pJpgData,int srcstep);

protected:

private:

	int m_nCenterSocket;

	bool m_nCenterLink;

	int m_nHeartBeatCount;

	//检测结果信号互斥
	pthread_mutex_t m_Result_Mutex;

	//检测结果消息列表
	MsgResult	m_ChannelResultList;

	//CRoadCarnumDetect pCarnumDetect;

	//关闭中心端连接所
	pthread_mutex_t m_CenterLinkClose_Mutex;
	//test
	int m_nCount;
	///test


};
extern XingTaiCenter g_XingTaiCenter;
#endif
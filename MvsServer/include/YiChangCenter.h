#pragma once
#include "Common.h"
#include "CSocketBase.h"
#include "MysqlQuery.h"
#include "FtpCommunication.h"
#include "CSeekpaiDB.h"
#include "Common.h"
#include "structdef.h"
#include "XmlParaUtil.h"
#include <map>
#include "mvdspapi.h"
#include "VehicleConfig.h"


////车牌信息包体 
//typedef  struct  _Type_Struct_Frt_Vhc_Data                                 
//{                                                                         
//	UINT32     bid;             //本包标志序号  （递增的）                        
//	char     cphm1[13];        //前拍车牌号码                                   
//	char     cphm2[13];        //后拍车牌号码                                   
//	char     cplx1;            //前拍车牌类型                                   
//	char     cplx2;            //后拍车牌类型                                   
//	char     cpys;             //车牌颜色                                     
//	char     wflx[20];            //违法类型                           
//	char     csys;             //车身颜色                                     
//	char     tgsj1[28];         //时间1                                     
//	char	 tgsj2[28];         //时间2                                     
//	char     tgsj3[28];         //时间3                                     
//	char     tgsj4[28];         //时间4                                     
//	float    speed;             //速度                                      
//	float    speedlimit;        //限速                                      
//	char     jcdid[9];         //监测点编号                                    
//	char     qdid[5];          //前端id                                     
//	char     xsfx;             //行驶方向                                     
//	char     cdid[6];          //车道编号                                     
//	char     cllx[5];          //车辆类型，这里修改了一下，原来的是  char  
//	short    zxd;            //置信度                                        
//	UINT32     gkjip;           //图片存放工控机IP                                 
//	char     tpzs;            //图片张数                                      
//	char     tpid1[100];        //图片序号1    这里可能是溢出的，即28字节未必够用
//	char     tpid2[100];        //图片序号2                                    
//	char     tpid3[100];        //图片序号3                                    
//	char     tpid4[100];        //图片序号4                                    
//	char     bl[16];           //保留字段                                     
//	char     cb[16];          //车标                                        
//	char     qpsfwc;           //前拍是否完成标志                                 
//	char     hpsfwc;           //后拍是否完成标志                                 
//	char     qhsfyz;           //前后拍是否一致                                  
//	UINT32     fbcd;           //方波长度                                       
//	UINT32     cpcd1;          //图片1长度                                      
//	UINT32     cpcd2;          //图片2长度                                      
//	UINT32     cpcd3;          //图片3长度                                      
//	UINT32     cpcd4;          //图片4长度                                      
//	char     bcbz;           //补传标志  
//	_Type_Struct_Frt_Vhc_Data()
//	{
//		bid = 0;
//		memset(cphm1,0,13);
//		memset(cphm2,0,13);
//		cplx1 = '\0';
//		cplx2 = '\0';
//		cpys = '\0';
//		memset(wflx,0,20);
//		csys = '\0';
//		memset(tgsj1,0,28);
//		memset(tgsj2,0,28);
//		memset(tgsj3,0,28);
//		memset(tgsj4,0,28);
//		speed = 0.0;
//		speedlimit = 0.0;
//		memset(jcdid,0,9);
//		memset(qdid,0,5);
//		xsfx = '\0';
//		memset(cdid,0,6);
//		memset(cllx,0,5);
//		zxd = 0;
//		gkjip = 0;
//		tpzs = '\0';
//		memset(tpid1,0,100);
//		memset(tpid2,0,100);
//		memset(tpid3,0,100);
//		memset(tpid4,0,100);
//		memset(bl,0,16);
//		memset(cb,0,16);
//		qpsfwc = '\0';
//		hpsfwc = '\0';
//		qhsfyz = '\0';
//		fbcd = 0;
//		cpcd1 = 0;
//		cpcd2 = 0;
//		cpcd3 = 0;
//		cpcd4 = 0;
//		bcbz = '\0';
//	}
//
//}Type_Struct_Frt_Vhc_Data; 


class CYiChangCenter : public mvCSocketBase
{
public:
	CYiChangCenter(void);
	~CYiChangCenter(void);

public:
	bool Init(void);
	void Unit(void);
	bool AddResult(const std::string& strResult);

public:
	void SetHeartBeatCount(const int& nHeartBeatCount){m_nHeartBeatCount=nHeartBeatCount;};
	int  GetHeartBeatFailureCount(void){return m_nHeartBeatCount;};
	int	 GetCenterSocket(void){return m_nCenterSocket;};
	bool GetCenterLinkFlag(void){return m_bCenterLink;};

	bool ConnectToCS(void);
	bool SendMsg(const int nSocket,const string& strData,int nType);
	bool mvRecvSocketMsg(int nSocket);
	void CloseCenterSocket(void);
	void DealResult(void);
	void OnSendResult( std::string& strResult);

	int  SendImageByFtp(string strPic, string strRemotepath);   // 把图片发送到ftp 上
	// 如果车牌号码是****的 则转换成 0000
	string CardChang(string  strCard);
	// 解析红灯的开始时间 和结束时间
	int MakeRedLightBeginEndTime(RECORD_PLATE * pPlate, string & strBeginTime, string & strEndTime,int nRedLightTime);
	int MakeRedLightBeginEndTime(MysqlQuery sqlQuery, string & strBeginTime, string & strEndTime,int nRedLightTime);
	// 得到这个车道的类型
	int GetRoadIDType(int nChannelID, int RoadWayID);
	//根据通道号，车道号获取相应车道的限速值
	void GetSpeedAndRedLightTime(int nChannel,int nRoadId,UINT32& nSpeed,UINT32& nRedLightTime);
	// 数据发送完成的时候， 我们要修改下 数据库的记录
	int UpdateMysl(int id);  
	// 制作目录
	string MakeSaveImagePath(RECORD_PLATE * pPlate);
	string MakeSaveImagePath(MysqlQuery sqlQuery);
	// 发送历史数据从数据库里面查询出来数据，并把这数据发送过去
	int  SendHistory();
	// 把数据库里面的信息组合成 xml 文件， 并返回xml 形式的字符串
	string ComposeSqlDataToXML( MysqlQuery sqlQuery);

	//获取随机防伪码
	int GetRandCode();

	string m_strHistoryRemotePath;
protected:
	int		m_nHeartBeatCount;
	int		m_nCenterSocket;
	bool		m_bCenterLink;

private:
	//检测结果信号互斥
	pthread_mutex_t		m_Result_Mutex;
	pthread_mutex_t		m_send_mutex;
	std::list<string>	 	m_listRecord;
};
extern CYiChangCenter g_YiChangCenter;
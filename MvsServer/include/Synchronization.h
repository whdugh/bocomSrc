// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：Synchronization.h
    功能：相机同步头文件
    作者：张要要
    时间：2009-11-27
**/

#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include "CSocketBase.h"
#include "global.h"

using namespace std;

#ifndef UINT_32
typedef unsigned int UINT_32;
#endif
#ifndef SYN_MSG_LIST
typedef list<string> SYN_MSG_LIST;
#endif
#ifndef REPEAT_QUEUE
typedef map<UINT_32, string> REPEAT_QUEUE;
#endif
#ifndef TRACK_MAP
typedef map<int64_t, TRACK_GROUP> TRACK_MAP;
#endif

typedef struct CHAR_DATA_VAR
{
    //int         nSend;  //1表示特征数据已经发送给对方
    int         nDone;  //1表示经过比对，可直接发送
    UINT_32     uId;    //该记录的唯一标识
    UINT_32     uTimeStamp; //压入队列的时间点，精确到秒
    float       fAccuracy;  //车牌检测准确度
    int         nWidth;     //检测到的物体/车牌宽度
    int         nDistance;  //车牌中心到相对边缘的距离
    int         nType;      //0表示事件，1表示车牌
    char        szPlate[MAX_PLATE];    //车牌信息

    CHAR_DATA_VAR()
    {
        //nSend = 0;
        nDone = 0;
        uId = 0;
        uTimeStamp = 0;
        fAccuracy = 0.0;
        nWidth = 0;
        nDistance = 0;
        nType = 0;
        memset(szPlate, 0, MAX_PLATE);
    }
} CHAR_VAR;

class mvCSynchronization:public mvCSocketBase
{
 public:
    /*构造函数*/
    mvCSynchronization();
    /*析构函数*/
    ~mvCSynchronization();

 public:
    /*初始化工作函数*/
    bool mvInit();
    /*清理工作函数*/
    void mvUnInit();

 public:
    /*重新设置选项*/
    bool mvReSetOption();
    /*读取标定点*/
	bool mvGetSynMarkerGroup();
	//bool mvCountDetectAreaCenter();
	/*返回检测区域中心点*/
	int mvGetDetectAreaCenter() const;
	/*主线程调用接口*/
    void mvConnOrLink();
    /*压入一条特征数据到左同步队列*/
    bool mvPushOneLeftRecord(const string& strRec, const SYN_CHAR_DATA& charData);//SYN_CHAR_DATA charData);
    /*压入一条特征数据到右同步队列*/
    bool mvPushOneRightRecord(const string& strRec, const SYN_CHAR_DATA& charData);//SYN_CHAR_DATA charData);
    /*从左同步队列取一条记录*/
    bool mvGetOneLeftRecord(string& strRec);
    /*从右同步队列取一条记录*/
    bool mvGetOneRightRecord(string& strRec);
    /*设置套接字属性*/
	bool mvMySetSocketOpt(int nSocket, int nOpt);
	/*设置左标定点*/
	bool mvSetLeftTrack(TRACK_GROUP& tGroup);
	/*设置右标定点*/
	bool mvSetRightTrack(TRACK_GROUP& tGroup);
//    bool mvSendOneCharacterData(int nSocket, const string& strRec);
    /*处理一条左边消息*/
    bool mvDealOneLeftSynMsg(const string& strMsg);
    /*处理一条右边消息*/
    bool mvDealOneRightSynMsg(const string& strMsg);
//    bool mvGetTheFirstLeftCharRecord(string& strRec);
//    bool mvGetTheFirstRightCharRecord(string& strRec);
    /*弹出一条左边消息*/
    bool mvPopOneLeftSynMsg(string&);
    /*弹出一条右边消息*/
    bool mvPopOneRightSynMsg(string&);

 public:
    /*关闭左连接套接字*/
    void mvMyClosePassiveSocket();
    /*关闭右连接套接字*/
    void mvMyCloseActiveSocket();
    /*压入一条左边消息*/
    bool mvPushOneLeftSynMsg(const string& strMsg);
    /*压入一条右边消息*/
    bool mvPushOneRightSynMsg(const string& strMsg);
    /*发送左标定点到左边主机*/
	bool mvSendMarkerToLeft();
	/*接收左套接字消息*/
    bool mvRecvLeftSocketMsg(string& strRecvMsg);
	/*接收右套接字消息*/
    bool mvRecvRightSocketMsg(string& strRecvMsg);

 public:
    int  m_nPassiveSocket;
    bool m_bSynInitSuccess;
    bool m_bSendMarkerToLeft;
    bool m_bSendMacToRight;
    bool m_bSendMarkerToRight;
    bool m_bRecvLeftMac;
    bool m_bRecvLeftMarker;
    bool m_bRecvRightMarker;
    //侦听套接字
    int m_nAcceptSocket;
    //TCP连接套接字
    int m_nActiveSocket;
    int  m_nSynPort;
    int  m_nLeftLinkCount;

 private:
    /*获取相机类型*/
    void mvGetCameraType();
    /*准备左侧连接套接字*/
    bool mvPrepareSocket();
    /*准备右侧连接套接字*/
    bool mvPrepareActiveSocket();
    /*从数据库获取左侧主机MAC*/
    bool mvGetLeftMacAddress();
    /*存储左侧主机MAC到数据库*/
    bool mvStoreLeftMacAddress();
    /*获取本机MAC*/
	bool mvGetMyMacAddress();
	/*获取右侧主机IP*/
	bool mvGetRightHostIp();
	/*开启处理线程*/
	bool mvStartThreads();
	/*弹出一条待处理的左侧同步记录*/
    bool mvPopTheFirstLeftRecord();
	/*弹出一条待处理的右侧同步记录*/
    bool mvPopTheFirstRightRecord();
    /*记录是否满足发送条件*/
    bool mvCanBeSend(string& strRec);
    /*发送数据*/
	bool mvSend(int nSocket, UINT_32 uCmdType, string strMsg = "");
    /*把轨迹map转换为xml*/
    bool mvTrackMapToString(TRACK_MAP mapTrack, string& strXml, int nLorR);
    /*把xml转换为轨迹map*/
    bool mvStringToTrackMap(const string& strXml, TRACK_MAP& mapTrack);
    /*打开日志文件*/
    void mvOpenLogFile();
    /*关闭日志文件*/
    void mvCloseLogFile();
	/*获取一条待处理的左侧同步记录*/
    bool mvGetTheFirstLeftRecord(string& strRec);
	/*获取一条待处理的右侧同步记录*/
    bool mvGetTheFirstRightRecord(string& strRec);

 private:
    /*删除一条左侧记录*/
	bool mvOnDelOneLeftRecord(UINT_32 uId);//(const string& strMsg);
	/*删除一条右侧记录*/
	bool mvOnDelOneRightRecord(UINT_32 uId);//(const string& strMsg);
	/*修改记录在数据库中的状态*/
	bool mvOnUpdateRecStatusInDB(UINT_32 uType, UINT_32 uId, int nStatus);
	/*同步处理一条左侧记录*/
	bool mvOnCmpLeftRecord(const string& strRecord);
	/*同步处理一条右侧记录*/
	bool mvOnCmpRightRecord(const string& strRecord);
	/*同步处理*/
    int mvHowToDeal(const string& strRecvChar, const string& strMyChar, int nLorR);
    /*计算矩阵*/
	void mvOnComputeMatrix(TRACK_GROUP& tGroupA, TRACK_GROUP& tGroupB, CvMat& matrix);
	/*验证收到的左侧主机MAC*/
	bool mvDealLeftMacAddress(const string&);
	/*处理收到的左侧主机的标定点*/
	bool mvDealLeftMarkerGroup(const string&);
	/*处理收到的右侧主机的标定点*/
	bool mvDealRightMarkerGroup(const string&);
	/*发送本机MAC地址到右边主机*/
	bool mvSendMacToRight();
	/*发送右标定点到右侧主机*/
	bool mvSendMarkerToRight();
	/*计算重合区域面积*/
	bool mvMatchCount(TRACK_MAP map1, TRACK_MAP map2, const CvMat& matrix, int nLorR);
	/*计算重合区域面积比例*/
	float mvGetMatchPercent(TRACK_GROUP track1, TRACK_GROUP track2, const CvMat& matrix, int nLorR);
    /*用rect计算重合区域面积比例*/
    float mvRectMatchPercent(const CvRect &r1, const CvRect &r2);
    /*计算左边检测区域最高点和最低点Y坐标映射*/
    void mvOnGetLeftAreaY(int nLeftY1, int nLeftY2);
    /*计算右边检测区域最高点和最低点Y坐标映射*/
    void mvOnGetRightAreaY(int nRightY1, int nRightY2);

 private:
    bool m_bUnInited;
    CvMat* m_pLeftMatrix;
    CvMat* m_pRightMatrix;
    //float m_fLeftMatrix[3][3];
    //float m_fRightMatrix[3][3];
    //float** m_fpLeftMatrix;
    //float** m_fpRightMatrix;
    TRACK_GROUP m_sLeftTrack;
    TRACK_GROUP m_sRightTrack;
    string m_strSynHost;
    string m_strLeftMacAddress;
    string m_strMyMacAddress;
    //u_char m_szMyMacAddr[6];
    int m_nDetectAreaCenter;
    int m_nSynTimeCount;
    int m_nAreaY[2];
    int m_nLeftAreaTop;
    int m_nLeftAreaBottom;
    int m_nRightAreaTop;
    int m_nRightAreaBottom;

 private:
    SYN_MSG_LIST m_listLeftSynMsg;
    SYN_MSG_LIST m_listRightSynMsg;
    REPEAT_QUEUE m_mapLeftRepeatQueue;
    REPEAT_QUEUE m_mapRightRepeatQueue;

 private:
    pthread_mutex_t m_LeftSynMsgMutex;
    pthread_mutex_t m_RightSynMsgMutex;
    pthread_mutex_t m_LeftQueueMutex;
    pthread_mutex_t m_RightQueueMutex;
};

extern mvCSynchronization g_SynProcess;
extern bool g_bLeftLink;
extern bool g_bRightLink;
extern int  g_nCameraType;
extern int  g_nSynTimeGap;
extern float g_fMatchAreaPercent;
extern float g_fSynGoodPercent;

#endif

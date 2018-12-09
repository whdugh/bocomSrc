//与服务端的接口
#ifndef __INTERFACE_4SERVER_H
#define __INTERFACE_4SERVER_H
	//远景预置位存图的结构体
	typedef struct StruSaveRemotePic
	{
	public:
		StruSaveRemotePic( );
		void mvInitVar( );

		//设置远景预置位存图数量
		void mvSetRemotePicCount(int nCount = 3);

		//设置远景预置位存图时间间隔
		void mvSetRemotePicInterval(int nTimeInterval = 60);//单位为秒

	public:
		int m_nSavePicCnt;        //图像保存的张数
		int m_nSavePicInterval;   //图像保存的时间间隔

	}CfgSaveRemotePic;


	//远景近景预置位组合存图的结构体
	typedef struct StruRemoteNearCombPic
	{
	public:
		StruRemoteNearCombPic( );
		void mvInitVar( );

		//设置远景近景预置位组合存图数量
		void mvSetRemoteNearCombPicCount(int nCount = 2);

		//设置远景近景预置位组合存图时间间隔
		void mvSetRemoteNearCombInterval(int nTimeInterval = 120);//单位为秒

	public:
		int m_nSaveComPicCnt;        //组合图像保存的张数
		int m_nSaveComPicInterval;   //组合图像保存的时间间隔

	}CfgSaveRemoteNearCombPic;


	//与服务端接口的结构体
	typedef struct StruInterface4Server
	{
	public:
		StruInterface4Server( );
		void mvInitVar( );

	public:
		StruSaveRemotePic	   m_cfgSaveRemotePic;
		StruRemoteNearCombPic  m_cfgSaveRemoteNearCombPic;
	}CfgInterface4Server;

#endif
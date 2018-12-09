// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "BocomServerManage.h"
#include "Common.h"
#include "CommonHeader.h"

CBocomServerManage g_BocomServerManage;

CBocomServerManage::CBocomServerManage()
{
	//
}

CBocomServerManage::~CBocomServerManage()
{
	UnInit();
}

//初始化,开始侦听服务
bool CBocomServerManage::Init()
{
	LogNormal("CBocomServerManage::Init()");
	bool bRet = false;

	//IP1
	bool bRet1 = false;
	
#ifdef LS_QINGTIAN_IVAP		
	bRet1 = AddBocomServer(g_nPPort, 2);
#else
	bRet1 = AddBocomServer(g_nPPort, 1);
#endif
	

#ifdef HAIGUAN_SERVER
	//IP2
	bool bRet2 = AddBocomServer(g_nPPort2, 2);

	//IP3
	bool bRet3 = AddBocomServer(g_nPPort3, 3);
#endif

	
#ifdef HAIGUAN_SERVER
	if(bRet1)
	{
		if(bRet2 && bRet3)
		{
			bRet = true;
		}
	}	
#else
	if(bRet1)
	{	
		bRet = true;	
	}
#endif

	return bRet;
}

//反初始化,释放资源
bool CBocomServerManage::UnInit()
{
	LogNormal("CBocomServerManage::UnInit() \n");
	bool bRet = true;
	
	if(m_pBocomServerMap.size() > 0)
	{
		BocomServerPtrMap::iterator it_b = m_pBocomServerMap.begin();
		for(; it_b != m_pBocomServerMap.end(); it_b++)
		{
			bRet = it_b->second->UnInit();
		}

		m_pBocomServerMap.clear();
	}

	return bRet;
}

//添加BocomServer
bool CBocomServerManage::AddBocomServer(int nPort, int nId)
{
	LogNormal("CBocomServerManage::AddBocomServer");
	bool bRet = false;

	CBocomServer *pBocomServer = new CBocomServer();
	if(pBocomServer)
	{
		bRet = pBocomServer->Init(nPort, nId);
		if(bRet)
		{
			m_pBocomServerMap.insert(BocomServerPtrMap::value_type(nPort,pBocomServer));
			LogNormal("m_pBocomServerMap.insert nPort:%d map:%d", nPort, m_pBocomServerMap.size());
		}		
	}

	return bRet;
}

//添加一条数据
bool CBocomServerManage::AddResult(std::string& strResult)
{
	//LogNormal("CBocomServerManage::AddResult %d ", strResult.size());
	bool bRet = false;

	//向各个端口分发数据
	BocomServerPtrMap::iterator it_b = m_pBocomServerMap.begin();
	for(; it_b != m_pBocomServerMap.end(); it_b++)
	{
		bRet = it_b->second->AddResult(strResult);
	}

	return bRet;
}

//处理一条命令
bool CBocomServerManage::mvOnDealOneMsg()
{	
	bool bRet = false;
	//处理各个端口数据
	BocomServerPtrMap::iterator it_b = m_pBocomServerMap.begin();
	for(; it_b != m_pBocomServerMap.end(); it_b++)
	{
		bRet = it_b->second->mvOnDealOneMsg();
	}

	return bRet;
}

bool CBocomServerManage::GetServerDeviceStatus()
{
	bool bRet = false;

	BocomServerPtrMap::iterator it_b = m_pBocomServerMap.begin();
	for(; it_b != m_pBocomServerMap.end(); it_b++)
	{
		bRet = it_b->second->CheckSocketStatus();
	}

	return bRet;
}

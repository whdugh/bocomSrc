// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#include "GECamCfgInfo.h"
//#include "Common.h"
#include "CommonHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGECamCfgInfo::CGECamCfgInfo()
{
	pCMDToCamera = NULL;
	memset(&m_CfgInfo, 0, sizeof(EE_CfgInfo));
}

CGECamCfgInfo::~CGECamCfgInfo()
{

}

BOOL CGECamCfgInfo::GetAllCfgInfo(CAMERA_CONFIG& cam_cfg)
{
    printf("====CGECamCfgInfo::GetAllCfgInfo()===\n");
	BOOL bRet = FALSE;
	int i;

	if (pCMDToCamera == NULL)
	{
		return bRet;
	}

	memset(&m_CfgInfo, 0, sizeof(EE_CfgInfo));

	do
	{
    //1. 快门，增益控制
		for (i = 0; i < MAX_RUNMODE_COUNT; i++)
		{
			if (pCMDToCamera->SendCMD(EE_EXPOSURE_GAIN|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwGain = pCMDToCamera->GetBackDataDWORD();

				cam_cfg.uGain = m_CfgInfo.ExposureInfo[i].dwGain;

				printf("==m_CfgInfo.ExposureInfo[i].dwGain=%d===\n", m_CfgInfo.ExposureInfo[i].dwGain);
			}
			else
			{
				break;
			}

			if (pCMDToCamera->SendCMD(EE_EXPOSURE_SHUTTERTIME|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwShutterTime = pCMDToCamera->GetBackDataDWORD();
				cam_cfg.uSH = m_CfgInfo.ExposureInfo[i].dwShutterTime;
				printf("==m_CfgInfo.ExposureInfo[i].dwShutterTime=%d===\n", m_CfgInfo.ExposureInfo[i].dwShutterTime);
			}
			else
			{
				break;
			}

			/*if (pCMDToCamera->SendCMD(EE_EXPOSURE_GAINLIMIT|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwGainLimit = pCMDToCamera->GetBackDataDWORD();
				cam_cfg.nMaxGain = m_CfgInfo.ExposureInfo[i].dwGainLimit;
			}
			else
			{
				break;
			}

			if (pCMDToCamera->SendCMD(EE_EXPOSURE_SHUTTERTIMEUPPERLIMIT|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwShutterTimeUpperLimit = pCMDToCamera->GetBackDataDWORD();
				cam_cfg.nMaxSH = m_CfgInfo.ExposureInfo[i].dwShutterTimeUpperLimit;
			}
			else
			{
				break;
			}*/

			if (pCMDToCamera->SendCMD(EE_EXPOSURE_MEANBRIGHTNESSTHRESHOLD|((i+1)<<8), EE_METHOD_GET)) //平均亮度期望值
			{
				m_CfgInfo.ExposureInfo[i].dwMeanBrightnessThreshold = pCMDToCamera->GetBackDataDWORD();
			}
			else
			{
				break;
			}

			if (pCMDToCamera->SendCMD(EE_EXPOSURE_MODE|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwMode = pCMDToCamera->GetBackDataDWORD();
				cam_cfg.ASC = m_CfgInfo.ExposureInfo[i].dwMode;
			}
			else
			{
				break;
			}

			if (pCMDToCamera->SendCMD(EE_EXPOSURE_FLASHLAMPENABLE|((i+1)<<8), EE_METHOD_GET))
			{
				m_CfgInfo.ExposureInfo[i].dwFlashLampEnable = pCMDToCamera->GetBackDataDWORD();
			}
			else
			{
				break;
			}
		}

		if (i != MAX_RUNMODE_COUNT)
		{
			break;
		}

    //2. 白平衡，LUT, 图像采样 控制
		if (pCMDToCamera->SendCMD(EE_APPEARANCE_WHITEBALANCEENABLE, EE_METHOD_GET))
		{
			m_CfgInfo.ApperanceInfo.dwWhiteBalanceEnable = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_APPEARANCE_WHITEBALANCEMODE, EE_METHOD_GET))
		{
			m_CfgInfo.ApperanceInfo.dwWhiteBalanceMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_APPEARANCE_IMGSAMPLEFMT, EE_METHOD_GET))
		{
			m_CfgInfo.ApperanceInfo.dwIMGSampleFMT = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_APPEARANCE_LUTENABLE, EE_METHOD_GET))
		{
			m_CfgInfo.ApperanceInfo.dwLUTEnable = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}

		if (pCMDToCamera->SendCMD(EE_METERING_AUTOMR, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.dwAutoMR = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}

    //3. 测光区域 控制
		if (pCMDToCamera->SendCMD(EE_METERING_AUTOMRPERIOD, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.dwAutoMRPeriod = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_METERING_MRBOTTOM, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.nMRBottom = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_METERING_MRLEFT, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.nMRLeft = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_METERING_MRRIGHT, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.nMRRight = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_METERING_MRTOP, EE_METHOD_GET))
		{
			m_CfgInfo.MeteringInfo.nMRTop = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}

    //4. 抓拍 控制
		if (pCMDToCamera->SendCMD(EE_GRABINFO_REMARK, EE_METHOD_GET))
		{
			strncpy(m_CfgInfo.GrabInfo.szRemark, pCMDToCamera->GetBackDataPtr(), sizeof(m_CfgInfo.GrabInfo.szRemark));
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_GRABINFO_LANENUM, EE_METHOD_GET))
		{
			m_CfgInfo.GrabInfo.dwLaneNum = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_GRABINFO_TRGTYPE, EE_METHOD_GET))
		{
			m_CfgInfo.GrabInfo.dwTrgType = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_GRABINFO_COILPITCH, EE_METHOD_GET))
		{
			m_CfgInfo.GrabInfo.dwCoilPitch = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_GRABINFO_SWITCHAUTOGRAB, EE_METHOD_GET))
		{
			m_CfgInfo.GrabInfo.dwSwitchAutoGrab = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}

    //5. 网络地址 控制
		if (pCMDToCamera->SendCMD(EE_NETWORK_DEVIP, EE_METHOD_GET))
		{
			m_CfgInfo.NetworkInfo.dwIP = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_NETWORK_DEVMAC, EE_METHOD_GET))
		{
			memcpy(m_CfgInfo.NetworkInfo.aMAC, pCMDToCamera->GetBackDataPtr(), sizeof(m_CfgInfo.NetworkInfo.aMAC));
		}
		else
		{
			break;
		}
    //6. 相机运行模式，重启相机，恢复出厂设置 控制
		if (pCMDToCamera->SendCMD(EE_SYSTEM_DATETIME, EE_METHOD_GET))
		{
			memcpy(m_CfgInfo.SystemInfo.aDataTime, pCMDToCamera->GetBackDataPtr(), sizeof(m_CfgInfo.SystemInfo.aDataTime));
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_SYSTEM_WATCHDOGENABLE, EE_METHOD_GET))
		{
			m_CfgInfo.SystemInfo.dwWatchDogEnable = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_SYSTEM_WATCHDOGTIMER, EE_METHOD_GET))
		{
			m_CfgInfo.SystemInfo.dwWatchDogTimer = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_SYSTEM_TEMPERATURE, EE_METHOD_GET))
		{
			m_CfgInfo.SystemInfo.fTemperature = pCMDToCamera->GetBackDataFloat();
		}
		else
		{
			break;
		}

/*
		for (i = 0; i < EE_CAM_IO_NUM; i++)
		{
			if (pCMDToCamera->SendCMD(EE_IO_OPTION, EE_METHOD_GET, (unsigned int)i))
			{
				memcpy(m_CfgInfo.IOInfo.aIOOptions[i], pCMDToCamera->GetBackDataPtr(), EE_CAM_IO_OPTIONLEN);
			}
			else
			{
				break;
			}
		}
		if (i != EE_CAM_IO_NUM)
		{
			break;
		}
*/
		if (pCMDToCamera->SendCMD(EE_STREAM1_RUNMODE, EE_METHOD_GET))
		{
			m_CfgInfo.StreamInfo.dwRunMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}

    //7. 光偶输入，输出 控制
		if (pCMDToCamera->SendCMD(EE_ISO_INMODE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwInMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_OUTMODE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwOutMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_POWERSYNENABLE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwPowerSynEnable = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_POWERSYNACMODE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwPowerSynACMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_POWERSYNDELAYTIME, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwPowerSynDelayTime = pCMDToCamera->GetBackDataDWORD();
			cam_cfg.EEN_delay = m_CfgInfo.ISOInfo.dwPowerSynDelayTime;
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_FLASHLAMPENABLE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwFlashLampEnable = pCMDToCamera->GetBackDataDWORD();
			cam_cfg.EEN_on = m_CfgInfo.ISOInfo.dwFlashLampEnable;
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_FLASHLAMPMODE, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwFlashLampMode = pCMDToCamera->GetBackDataDWORD();
		}
		else
		{
			break;
		}
		if (pCMDToCamera->SendCMD(EE_ISO_FLASHLAMPOUTWIDTH, EE_METHOD_GET))
		{
			m_CfgInfo.ISOInfo.dwFlashLampOutWidth = pCMDToCamera->GetBackDataDWORD();
			cam_cfg.EEN_width = m_CfgInfo.ISOInfo.dwFlashLampOutWidth;
		}
		else
		{
			break;
		}

		bRet = TRUE;
	} while(0);


	return bRet;
}



#ifdef BASLER_CAMERA
#include "BaslerCamera.h"
#include "CommonHeader.h"

using namespace Pylon;

WaitObjectEx BaslerCamera::m_WaitObjectEx(WaitObjectEx::Create());

void* StreamThread(void* arg)
{
	try
	{
		BaslerCamera *pBasler = (BaslerCamera*)arg;
		pBasler->RecvVideoStream();
	}
	catch (...)
	{}
	pthread_exit((void *)0);
}

void BaslerCamera::RecvVideoStream()
{
	WaitObjects waitObjects;
	waitObjects.Add (m_StreamGrabber->GetWaitObject()); // getting informed about buffers
	waitObjects.Add (m_WaitObjectEx);

	// Start acquisition on the camera
	CCommandPtr ptrAcquisitionStart = m_CameraNodeMap->GetNode("AcquisitionStart");
	ptrAcquisitionStart->Execute();

	unsigned int index;			// index of the wait object that has been signalled
	unsigned int nSucc = 0;		// number of successfully grabbed images
	GrabResult Result;			// grab result

	try
	{
		while(!m_bStopCamera)
		{
			if(waitObjects.WaitForAny(INFINITE, &index))
			{
				switch (index)
				{
				case 0:  // a grabbed buffer is available
					if (m_StreamGrabber->RetrieveResult (Result))
					{
						if (Result.Succeeded())
						{
							struct timeval now;
							gettimeofday(&now, NULL);

							yuv_video_buf header;
							header.ts = (int64_t)now.tv_sec * 1000000 + now.tv_usec;
							header.uTimestamp = now.tv_sec;
							header.nFrameRate = m_cfg.nFrequency;
							header.width = m_nWidth;
							header.height = m_nHeight;
							memcpy(header.cType,"VYUY",4);
							header.nSeq = m_uSeq;
							header.uGain = m_cfg.uGain;
							header.uMaxGain = m_cfg.nMaxGain;
							header.size = Result.GetPayloadSize();
							header.uFrameType = 2;//帧图像			

							memcpy(m_pFrameBuffer, (void*)&header, sizeof(header));
							memcpy(m_pFrameBuffer + sizeof(header), (void*)Result.Buffer(), Result.GetPayloadSize());

							m_uSeq++;
							AddFrame(1);
							m_StreamGrabber->QueueBuffer(Result.Handle(), Result.Context());
						}
					}
					else
					{
						throw "";
					}
					break;
				default:
					throw "";
				}
			}
			else
				break;
		}
	}
	catch (...)
	{}
	CCommandPtr ptrAcquisitionStop = m_CameraNodeMap->GetNode("AcquisitionStop");
	ptrAcquisitionStop->Execute();
}

BaslerCamera::BaslerCamera(int nCameraType)
{
	m_nCameraType = nCameraType;

	if(m_nCameraType == BASLER_200)
	{
		m_nWidth = 1600;
		m_nHeight = 1200;

	}
	else if(m_nCameraType == BASLER_500)
	{
		m_nWidth = 2448;
		m_nHeight = 2048;
	}
	else
	{
		m_nWidth = 0;
		m_nHeight = 0;
	}

	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;
	m_bEndCapture = false;
	//线程ID
	m_nThreadId = 0;

	m_uSeq = 0;
	m_nPreFieldSeq  = 0;
	m_nCurIndex = 0;
	m_nFirstIndex = 0;
	m_nFrameSize=0;

	m_Camera = NULL;
	m_Buffer = NULL;

	m_uMaxPE = 0;
	m_uMaxGain = 0;
	m_uMaxGamma = 40;

	m_TlFactory = &CTlFactory::GetInstance();

	m_pthreadId = 0;
}

BaslerCamera::~BaslerCamera()
{
}

bool BaslerCamera::Open()
{
	for(int i=0; i< MAX_SIZE; i++)
	{
		m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(char));
	}
	m_pFrameBuffer = m_FrameList[0];
	m_bStopCamera = false;
	
	m_TlFactory->EnumerateDevices(m_Devices);
	if (m_Devices.empty())
	{
		cerr<<"no camera connect"<<endl;
		LogError("没有可连接的相机\n");
		return false;
	}

	try
	{
		m_Camera = m_TlFactory->CreateDevice(m_Devices[0]);
		m_Camera->Open(Stream | Control);
	}
	catch ( GenICam::GenericException &e)
	{
		return false;
	}
	m_StreamGrabber = m_Camera->GetStreamGrabber(0);
	m_StreamGrabber->Open();

	INodeMap *StreamParameters = m_StreamGrabber->GetNodeMap();
	m_CameraNodeMap = m_Camera->GetNodeMap();

	CEnumerationPtr ptrPixelFormat = m_CameraNodeMap->GetNode("PixelFormat");

	IEnumEntry *PixelFormatEntry = ptrPixelFormat->GetEntryByName ("YUV422Packed");
	if (PixelFormatEntry && GenApi::IsAvailable (PixelFormatEntry))
	{
		ptrPixelFormat->FromString ("YUV422Packed");
	}

	CIntegerPtr pHeartbeatTimeout = m_Camera->GetTLNodeMap()->GetNode("HeartbeatTimeout");
	pHeartbeatTimeout->SetValue(300000);//单位毫秒 Max=4294967295,Min=500

	CBooleanPtr ptrRemoveLimits = m_CameraNodeMap->GetNode("RemoveLimits");
	ptrRemoveLimits->SetValue(true);
	
	CEnumerationPtr ptrGammaSelector = m_CameraNodeMap->GetNode("GammaSelector");
	if(ptrGammaSelector&&GenApi::IsAvailable(ptrGammaSelector))
	{
		if(ptrGammaSelector->GetEntryByName ("User") && 
			GenApi::IsAvailable (ptrGammaSelector->GetEntryByName ("User")))
		{
			ptrGammaSelector->FromString("User");
		}
	}

	const CIntegerPtr Width = m_CameraNodeMap->GetNode("Width");
	const CIntegerPtr Height = m_CameraNodeMap->GetNode("Height");
	const CIntegerPtr PaylodSize = m_CameraNodeMap->GetNode("PayloadSize");

	Width->SetValue(Width->GetMax()& ~3); //need to align to DWORD for Windows Bitmap
	Height->SetValue(Height->GetMax());
	CEnumerationPtr(m_CameraNodeMap->GetNode("AcquisitionMode"))->FromString("Continuous");
	const size_t ImageSize = (size_t)(PaylodSize->GetValue());

	CIntegerPtr(StreamParameters->GetNode("MaxBufferSize"))->SetValue(ImageSize);
	CIntegerPtr(StreamParameters->GetNode("MaxNumBuffer"))->SetValue(1);
	m_nWidth = (UINT32)Width->GetValue();
	m_nHeight = (UINT32)Height->GetValue();

	m_StreamGrabber->PrepareGrab();
	m_Buffer = new BYTE[ ImageSize]; //byte array to store Pylon image

	m_hBuffer = m_StreamGrabber->RegisterBuffer((void*)m_Buffer, ImageSize);

	m_StreamGrabber->QueueBuffer(m_hBuffer, 0);

	CEnumerationPtr ptrTriggerMode = m_CameraNodeMap->GetNode("TriggerMode");
	ptrTriggerMode->FromString("Off");
	CEnumerationPtr ptrTriggerSource = m_CameraNodeMap->GetNode("TriggerSource");
	ptrTriggerSource->FromString("Software");

	//设置白平衡
	CEnumerationPtr ptrBalanceWhiteAuto = m_CameraNodeMap->GetNode("BalanceWhiteAuto");
	if (ptrBalanceWhiteAuto->GetEntryByName("Continuous") && 
		GenApi::IsAvailable(ptrBalanceWhiteAuto->GetEntryByName("Continuous")))
	{
		//相机具有连续白平衡功能
		ptrBalanceWhiteAuto->FromString("Continuous");
		printf("==================ptrBalanceWhiteAuto->FromString  =Continuous \n");
	}
	else
	{
		//相机没有连续白平衡功能
		ptrBalanceWhiteAuto->FromString("Once");
		printf("==================ptrBalanceWhiteAuto->FromString = Once \n");
	}

	//打开帧频设置功能
	CBooleanPtr ptrAcquisitionFrameRateEnable = m_CameraNodeMap->GetNode("AcquisitionFrameRateEnable");
	ptrAcquisitionFrameRateEnable->SetValue(true);
	//设置帧率
	CFloatPtr ptrAcquisitionFrameRateAbs = m_CameraNodeMap->GetNode ("AcquisitionFrameRateAbs");
	if(m_nCameraType == BASLER_200)
	{
		ptrAcquisitionFrameRateAbs->SetValue(15);

	}
	else if(m_nCameraType == BASLER_500)
	{
		ptrAcquisitionFrameRateAbs->SetValue(10);
	}

	//载入相机参数
	m_cfg.uKind = m_nCameraType;
	LoadCameraProfile(m_cfg);
	m_cfg.nIndex = -1;
	ControlCamera(m_cfg);

	LightControl(true);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if (pthread_create(&m_pthreadId, &attr, StreamThread, this) != 0)
	{
		//失败
		LogError("创建视频流接收线程失败, 服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
	pthread_attr_destroy(&attr);
	return true;
}

bool BaslerCamera::Close()
{
	if (!m_Devices.empty())
	{
		// Signal the thread to terminate
		m_bStopCamera = true;
		// Join with the thread, i.e., wait until it has terminated
		if(m_pthreadId != 0)
		{
			m_WaitObjectEx.Signal();
			pthread_join(m_pthreadId, NULL);
			m_pthreadId = 0;
			m_WaitObjectEx.Reset();
		}

		if (m_Camera)
		{
			m_StreamGrabber->CancelGrab ();
			for (GrabResult r; m_StreamGrabber->RetrieveResult (r););

			// It is necessary to deregister buffer before freeing the memory
			m_StreamGrabber->DeregisterBuffer(m_hBuffer);
			// Free all resources used for grabbing
			m_StreamGrabber->FinishGrab ();
			m_StreamGrabber->Close();

			m_Camera->Close();
			m_TlFactory->DestroyDevice(m_Camera);
		}

		if (m_Buffer != NULL)
		{
			delete m_Buffer;
			m_Buffer = NULL;
		}

		//释放内存
		pthread_mutex_lock(&m_FrameMutex);
		for (int i=0; i<MAX_SIZE; i++)
		{
			if(m_FrameList[i] != NULL)
			{
				free(m_FrameList[i]);
				m_FrameList[i] = NULL;
			}
			m_nFrameSize = 0;
			m_pFrameBuffer = NULL;
		}
		pthread_mutex_unlock(&m_FrameMutex);
	}
	return true;
}

bool BaslerCamera::ReOpen()
{
	Close();
	Open();
}


//获取相机默认模板
void BaslerCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
	cfg.ASC = 0;
	cfg.uSM = 1;

	//曝光时间最大值
	cfg.nMaxPE = 2000;

	cfg.AGC = 0;
	cfg.nMaxGain = 20;

	cfg.nMaxGamma = m_uMaxGamma;

	cfg.nDigital = 2;
	cfg.nMaxDigital = 4;

	//触发方式
	cfg.nMode = 0;
	//极性
	cfg.uPol = 1;

	if(m_nCameraType == BASLER_500)
	{
		cfg.nFrequency = 10;
	}
	else
	{
		cfg.nFrequency = 15;
	}
}

//参数转换
void BaslerCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg, bool bReverseConvert, bool bFirstLoad)
{
}

//控制
int BaslerCamera::ManualControl(CAMERA_CONFIG cfg)
{
	if(cfg.nIndex == (int)CAMERA_CMD)
	{
		if (strcmp(cfg.chCmd,"asc")==0)
		{
			cfg.nIndex = (int)CAMERA_ASC;
			cfg.ASC = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"agc")==0)
		{
			cfg.nIndex = (int)CAMERA_AGC;
			cfg.AGC = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"sh")==0)
		{
			cfg.nIndex = (int)CAMERA_SH;
			cfg.uSH = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"ga")==0)
		{
			cfg.nIndex = (int)CAMERA_GAIN;
			cfg.uGain = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"gama")==0)
		{
			cfg.nIndex = (int)CAMERA_GAMMA;
			cfg.nGamma = (int)(cfg.fValue);
			printf("==00000000===now the cfg.fValue = %f\n",cfg.fValue);
			printf("==00000000===now the cfg.nGamma = %f\n",cfg.nGamma);
		}
		else if (strcmp(cfg.chCmd,"pe")==0)
		{
			cfg.nIndex = (int)CAMERA_PE;
			cfg.uPE = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"epl")==0)
		{
			cfg.nIndex = (int)CAMERA_POL;
			cfg.uPol = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"pen")==0)
		{
			cfg.nIndex = (int)CAMERA_EEN;
			cfg.EEN_on = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"pei")==0)
		{
			cfg.nIndex = (int)CAMERA_PEI;
			cfg.EEN_delay = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"pew")==0)
		{
			cfg.nIndex = (int)CAMERA_PEW;
			cfg.EEN_width = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"ds")==0)
		{
			cfg.nIndex = (int)CAMERA_DIGITALSHIFT;
			cfg.nDigital = (int)cfg.fValue;
		}
	}

	//控制相机
	ControlCamera(cfg);
	//读相机设置
	ReadCameraSetting(cfg);
	//保存相机设置
	WriteCameraIni(cfg);
}

void BaslerCamera::ControlCamera(CAMERA_CONFIG cfg)
{
	//自动快门
	if (cfg.nIndex == -1 || cfg.nIndex == (int)CAMERA_ASC)
	{
		if (cfg.ASC == 0)
		{
			CEnumerationPtr ptrExposureAuto = m_CameraNodeMap->GetNode("ExposureAuto");
			ptrExposureAuto->FromString("Off");
		}
		else
		{
			CEnumerationPtr ptrExposureAuto = m_CameraNodeMap->GetNode("ExposureAuto");
			ptrExposureAuto->FromString("Continuous");
		}

		m_cfg.ASC = cfg.ASC;
	}

	//亮度增强
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_DIGITALSHIFT)
	{
		CIntegerPtr ptrDigitalShift = m_CameraNodeMap->GetNode("DigitalShift");
		if(cfg.nDigital <= ptrDigitalShift->GetMax() && cfg.nDigital >= ptrDigitalShift->GetMin())
		{
			ptrDigitalShift->SetValue(cfg.nDigital);

			m_cfg.nDigital = cfg.nDigital;
		}
	}

	//曝光时间
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_PE)
	{
		CEnumerationPtr ptrExposureAuto = m_CameraNodeMap->GetNode("ExposureAuto");
		if (0 == ptrExposureAuto->ToString().compare("Off"))
		{
			CFloatPtr ptrExposureTimeAbs = m_CameraNodeMap->GetNode("ExposureTimeAbs");
			if ( !(cfg.uPE < ptrExposureTimeAbs->GetMin() || cfg.uPE > ptrExposureTimeAbs->GetMax()) )
			{
				ptrExposureTimeAbs->SetValue(cfg.uPE);
			}
			if (0 == m_uMaxPE)
			{
				m_uMaxPE = ptrExposureTimeAbs->GetMax();
			}

			m_cfg.uPE = cfg.uPE;
		}
	}

	//自动增益
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_AGC)
	{
		CEnumerationPtr ptrGainAuto = m_CameraNodeMap->GetNode("GainAuto");
		if (cfg.AGC == 0)
		{
			ptrGainAuto->FromString("Off");
		}
		else
		{
			ptrGainAuto->FromString("Continuous");
		}

		m_cfg.AGC = cfg.AGC;
	}

	//增益控制
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_GAIN)
	{
		UINT32 gain = cfg.uGain * GAIN_DB_UNIT;
		CEnumerationPtr ptrGainAuto = m_CameraNodeMap->GetNode("GainAuto");
		if(0 == ptrGainAuto->ToString().compare("Off"))
		{

			//CBooleanPtr ptrRemoveLimits = m_CameraNodeMap->GetNode("RemoveLimits");
			//ptrRemoveLimits->SetValue(true);

			CEnumerationPtr ptrGainSelector = m_CameraNodeMap->GetNode("GainSelector");
			CIntegerPtr ptrGainRaw = m_CameraNodeMap->GetNode("GainRaw");

			if (ptrGainSelector->GetEntryByName("All")&&GenApi::IsAvailable(ptrGainSelector->GetEntryByName("All")))
			{

				if (gain > m_cfg.nMaxGain*GAIN_DB_UNIT)
				{
					gain = m_cfg.nMaxGain*GAIN_DB_UNIT;
				}

				if (gain < GAIN_MIN)
				{
					gain = GAIN_MIN;
				}

				ptrGainSelector->FromString("All");
				ptrGainRaw->SetValue(gain);
				printf("set the gain of basler is = %d\n", gain);
				
				m_cfg.uGain = cfg.uGain;
			}
		}
	}

	//gamma
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_GAMMA)
	{
		CBooleanPtr ptrGammaEnable = m_CameraNodeMap->GetNode("GammaEnable");
		CFloatPtr ptrGamma = m_CameraNodeMap->GetNode("Gamma");
		ptrGammaEnable->SetValue(true);
		printf("====ControlCamera=cfg.nGamma = %f \n",cfg.nGamma);
		printf("====ControlCamera=gamma is must in (%f---%f)\n",ptrGamma->GetMin(),ptrGamma->GetMax());
		if(cfg.nGamma >(ptrGamma->GetMax())*10||cfg.nGamma < (ptrGamma->GetMin())*10)
		{
			printf("ControlCamera====gamma is must in (%f---%f)\n",ptrGamma->GetMin(),ptrGamma->GetMax());

		}
		else
		{
			printf("====ControlCamera000000=====cfg.nGamma =%f---m_cfg.nGamma=%f\n",cfg.nGamma,m_cfg.nGamma);
			ptrGamma->SetValue((cfg.nGamma)*0.1);
			printf("====ControlCamera000000=====ptrGamma->GetValue() = %f \n",ptrGamma->GetValue());
		}
		m_cfg.nGamma = (int)cfg.nGamma;
		printf("====ControlCamera000000=====cfg.nGamma =%f---m_cfg.nGamma=%f\n",cfg.nGamma,m_cfg.nGamma);
	}

	//频闪开关
	if ( cfg.nIndex == CAMERA_EEN)
	{
		if (cfg.EEN_on <= 0)
		{
			//关闭频闪
			CEnumerationPtr ptrLineSelector = m_CameraNodeMap->GetNode("LineSelector");
			CEnumerationPtr ptrLineSource = m_CameraNodeMap->GetNode("LineSource");
			ptrLineSelector->FromString("Out1");
			ptrLineSource->FromString("UserOutput");
		}
		else
		{
			//打开频闪
			CEnumerationPtr ptrLineSelector = m_CameraNodeMap->GetNode("LineSelector");
			CEnumerationPtr ptrLineSource = m_CameraNodeMap->GetNode("LineSource");
			CBooleanPtr ptrLineInverter = m_CameraNodeMap->GetNode("LineInverter");
			ptrLineSelector->FromString("Out1");
			ptrLineSource->FromString("TimerActive");
		}

		m_cfg.EEN_on = cfg.EEN_on;
	}

	//极性
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_POL)
	{
		bool bPol = cfg.uPol;
		CEnumerationPtr ptrLineSelector = m_CameraNodeMap->GetNode("LineSelector");
		CEnumerationPtr ptrLineSource = m_CameraNodeMap->GetNode("LineSource");
		CBooleanPtr ptrLineInverter = m_CameraNodeMap->GetNode("LineInverter");
		ptrLineSelector->FromString("Out1");
		ptrLineSource->FromString("TimerActive");
		ptrLineInverter ->SetValue(!bPol); //false高电平, true低电平

		m_cfg.EEN_delay = cfg.EEN_delay;
	}

	//脉冲延时
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_PEI)
	{
		CFloatPtr ptrTimerDelayAbs = m_CameraNodeMap->GetNode("TimerDelayAbs");
		ptrTimerDelayAbs ->SetValue(cfg.EEN_delay);
		m_cfg.EEN_delay = cfg.EEN_delay;
	}

	//脉冲宽度
	if (cfg.nIndex == -1 || cfg.nIndex == CAMERA_PEW)
	{
		CFloatPtr ptrTimerDurationAbs = m_CameraNodeMap->GetNode("TimerDurationAbs");
		ptrTimerDurationAbs ->SetValue(cfg.EEN_width);

		m_cfg.EEN_width = cfg.EEN_width;
	}
}

//灯控制
int   BaslerCamera::LightControl(bool bInit)
{
	CAMERA_CONFIG cfg = m_cfg;

	if(!bInit)
	{
		//判断是否开灯
		cfg.uType = 2;
		memset(cfg.chCmd,0,sizeof(cfg.chCmd));
		memcpy(cfg.chCmd,"PEN",3);
		ReadCameraSetting(cfg);
	}
	else
	{
		///////////////////////////////闪光灯控制
		int nDayNight = DayOrNight(1);
		if(nDayNight==0)//晚间
		{
			LogNormal("OpenLight\n");

			cfg.EEN_on = 1;//打开
			cfg.nIndex = CAMERA_EEN;
			ControlCamera(cfg);

			//设置gama
			cfg.nIndex = CAMERA_GAMMA;
			cfg.nGamma = 7;

			//手动白平衡
			CEnumerationPtr ptrBalanceWhiteAutoNight = m_CameraNodeMap->GetNode("BalanceWhiteAuto");
			ptrBalanceWhiteAutoNight->FromString("Off");

			CEnumerationPtr ptrBalanceRatioSelector =  m_CameraNodeMap->GetNode("BalanceRatioSelector");
			ptrBalanceRatioSelector->FromString("Red");
			CIntegerPtr ptrBalanceRatioRaw = m_CameraNodeMap->GetNode("BalanceRatioRaw");
			ptrBalanceRatioRaw->SetValue(105);

			ptrBalanceRatioSelector->FromString("Green");
			ptrBalanceRatioRaw->SetValue(96);

			ptrBalanceRatioSelector->FromString("Blue");
			ptrBalanceRatioRaw->SetValue(91);

			ControlCamera(cfg);
			usleep(1000*5);
		}
		else if(nDayNight==1)//白天
		{
			LogNormal("CloseLight\n");

			if(g_nHasHighLight == 0)
			{
				cfg.EEN_on = 0;//关闭
				cfg.nIndex = CAMERA_EEN;
				ControlCamera(cfg);
			}
			else if(g_nHasHighLight == 1)
			{
				cfg.EEN_on = 1;//打开
				cfg.nIndex = CAMERA_EEN;
				ControlCamera(cfg);
			}

			//设置白平衡
			CEnumerationPtr ptrBalanceWhiteAutoDay = m_CameraNodeMap->GetNode("BalanceWhiteAuto");
			if (ptrBalanceWhiteAutoDay->GetEntryByName("Continuous") && 
				GenApi::IsAvailable(ptrBalanceWhiteAutoDay->GetEntryByName("Continuous")))
			{
				//相机具有连续白平衡功能
				ptrBalanceWhiteAutoDay->FromString("Continuous");
				printf("==================ptrBalanceWhiteAuto->FromString  =Continuous \n");
			}
			else
			{
				//相机没有连续白平衡功能
				ptrBalanceWhiteAutoDay->FromString("Once");
				printf("==================ptrBalanceWhiteAuto->FromString = Once \n");
			}


			//设置gama
			cfg.nIndex = CAMERA_GAMMA;
			cfg.nGamma = 8;
			printf("====LightControl===000000=====cfg.nGamma =%f \n",cfg.nGamma);
			ControlCamera(cfg);
		}
	}
	return cfg.EEN_on;
}

//相机自动控制
int  BaslerCamera::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
	//fIncrement *= 100;

	//设置白平衡
	//CEnumerationPtr ptrBalanceWhiteAuto = m_CameraNodeMap->GetNode("BalanceWhiteAuto");
	//if (ptrBalanceWhiteAuto->GetEntryByName("Continuous") && 
	//	GenApi::IsAvailable(ptrBalanceWhiteAuto->GetEntryByName("Continuous")))
	//{
		//相机具有连续白平衡功能
		//ptrBalanceWhiteAuto->FromString("Continuous");
	//	printf("==================ptrBalanceWhiteAuto->FromString  =Continuous \n");
	//}
	//else
	//{
		//相机没有连续白平衡功能
	//	ptrBalanceWhiteAuto->FromString("Once");
	//	printf("==================ptrBalanceWhiteAuto->FromString = Once \n");
	//}
	
	CAMERA_CONFIG cfg = m_cfg;//获取当前相机配置

	//LogNormal("AutoControl fRate=%0.2f,fIncrement=%0.2f,uGain=%d,uSH=%d\n",fRate,fIncrement,cfg.uGain,cfg.uSH);

	int nMaxGain = m_cfg.nMaxGain;//2000
	int nMaxPE = m_cfg.nMaxPE;
	int nMinGain = 0;
	int nMinPE = 5;
	if(!bDetectCarnum)
	{
		nMaxGain = m_cfg.nMaxGain2;
		nMaxPE = m_cfg.nMaxPE2;
	}

	if( (cfg.uPE == nMaxPE)&& (fRate > 1)) //PE不能再大
	{
		fRate = 1;
	}
	if( (cfg.uGain == nMaxGain)&& (fIncrement > 0)) //GAIN不能再大
	{
		fIncrement = 0;
	}


	if( (cfg.uPE == nMinPE)&& (fRate < 1))  //PE不能再小
	{
		fRate = 1;
	}
	if( (cfg.uGain == nMinGain)&& (fIncrement < 0)) //GAIN不能再小
	{
		fIncrement = 0;
	}

	if(fRate>1)
		cfg.uPE = cfg.uPE*fRate+0.5;
	else if(fRate<1)
		cfg.uPE = cfg.uPE*fRate;

	if( fIncrement >0  || fIncrement < 0)
		cfg.uGain += fIncrement;


	if(cfg.uGain >=nMaxGain)//增益
		cfg.uGain = nMaxGain;
	else if(cfg.uGain<=nMinGain)
		cfg.uGain = nMinGain;

	if(cfg.uPE>=nMaxPE)
		cfg.uPE = nMaxPE;
	else if(cfg.uPE<=nMinPE)
		cfg.uPE = nMinPE;

	if(cfg.ASC == 0)
	{
		if( fRate > 1 || fRate < 1)
		{
			cfg.nIndex = CAMERA_PE;
			ControlCamera(cfg);
			//LogNormal("AutoControl uSH=%d\n",cfg.uSH);
		}
	}

	if(cfg.AGC == 0)
	{
		if( fIncrement >0  || fIncrement < 0)
		{
			cfg.nIndex = CAMERA_GAIN;
			printf("=========ControlCamera====auto gain = %d \n",cfg.uGain);
			ControlCamera(cfg);
			//LogNormal("AutoControl uGain=%d\n",cfg.uGain);
		}
	}

	if(fRate > 1 || fRate < 1 ||
		fIncrement >0  || fIncrement < 0)
	{
		m_bCameraControl = true;
		printf("=========WriteCameraIni====auto gain = %d \n",cfg.uGain);
		//写配置文件
		WriteCameraIni(cfg);
		LogNormal("AutoControl uGain=%d,uPE=%d,nEn=%d\n",cfg.uGain,cfg.uPE,nEn);
	}

	//判断是否开灯
	cfg.EEN_on = LightControl();
	

	if(cfg.EEN_on > -1)
	{
		m_cfg.EEN_on = cfg.EEN_on;

		if(g_nHasHighLight == 0)
		{
			if(g_LightTimeInfo.nLightTimeControl == 0)
			{
				//如果增益和曝光时间很大仍然未开灯则开灯
				if((nEn == 1))//进入晚间模式
				{
					if(cfg.EEN_on != 1)//根据亮度去开灯
					{
						LogNormal("AutoControl OpenLight,uGain=%d,uPE=%d\n",cfg.uGain,cfg.uPE);
						cfg.EEN_on = 1;//打开
						cfg.nIndex = CAMERA_EEN;
						ControlCamera(cfg);
						usleep(1000*5);

						//设置gama
						cfg.nIndex = CAMERA_GAMMA;
						cfg.nGamma = 7;
						printf("====AutoControl===000000=====cfg.nGamma=%f, cfg.nMaxGamma=%f \n",cfg.nGamma,cfg.nMaxGamma);
						ControlCamera(cfg);
						usleep(1000*1);
					}

				}
				else if((nEn == 0))
				{

					if( (cfg.EEN_on != 0))//根据亮度去关灯
					{
						LogNormal("AutoControl CloseLight,uGain=%d,uPE=%d\n",cfg.uGain,cfg.uPE);

						//关灯
						cfg.EEN_on = 0;//关闭
						cfg.nIndex = CAMERA_EEN;
						ControlCamera(cfg);

						m_nCountGain = 0;
						usleep(1000*1);
						//设置gama
						cfg.nIndex = CAMERA_GAMMA;
						cfg.nGamma = 8;
						printf("====AutoControl===000000=====cfg.nGamma=%f, cfg.nMaxGamma=%f \n",cfg.nGamma,cfg.nMaxGamma);
						ControlCamera(cfg);
					}
				}
			}
			else if(g_LightTimeInfo.nLightTimeControl == 1)
			{
				int nDayNight = DayOrNight(1);
				if(nDayNight == 0)
				{
					if(cfg.EEN_on != 1)//根据亮度去开灯
					{
						LogNormal("强制开灯,uGain=%d,uPE=%d\n",cfg.uGain,cfg.uPE);
						cfg.EEN_on = 1;//打开
						cfg.nIndex = CAMERA_EEN;
						ControlCamera(cfg);
						usleep(1000*5);

						//设置gama
						cfg.nIndex = CAMERA_GAMMA;
						cfg.nGamma = 7;
						printf("====AutoControl===000000=====cfg.nGamma=%f, cfg.nMaxGamma=%f \n",cfg.nGamma,cfg.nMaxGamma);
						ControlCamera(cfg);
						usleep(1000*1);
					}

				}
				else if(nDayNight == 1)
				{
					if( (cfg.EEN_on != 0))//根据亮度去关灯
					{
						LogNormal("强制关灯,uGain=%d,uPE=%d\n",cfg.uGain,cfg.uPE);

						//关灯
						cfg.EEN_on = 0;//关闭
						cfg.nIndex = CAMERA_EEN;
						ControlCamera(cfg);

						m_nCountGain = 0;
						usleep(1000*1);
						//设置gama
						cfg.nIndex = CAMERA_GAMMA;
						cfg.nGamma = 8;
						printf("====AutoControl===000000=====cfg.nGamma=%f, cfg.nMaxGamma=%f \n",cfg.nGamma,cfg.nMaxGamma);
						ControlCamera(cfg);
					}
				}
			}
		}
		else if(g_nHasHighLight == 1)//有爆闪灯则不管白天晚上都需要打开闪光灯
		{
			//如果未开灯强制开灯
			if(cfg.EEN_on != 1)
			{
				cfg.EEN_on = 1;//打开
				cfg.nIndex = CAMERA_EEN;
				ControlCamera(cfg);
			}
		}
	}

	return 1;
}

//读相机参数
int BaslerCamera::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
	cfg.uKind = m_nCameraType;
	//LoadCameraProfile(cfg);
	//cfg = m_cfg;
	cfg.nMaxGain = m_cfg.nMaxGain;
	cfg.nMaxPE = m_cfg.nMaxPE;
	cfg.nMaxDigital = m_cfg.nMaxDigital;
	printf("====ReadCameraSetting===00000000===now cfg.nMaxGamma = %f\n",cfg.nMaxGamma);
	printf("====ReadCameraSetting===00000000===now  m_cfg.nMaxGamma = %f\n", m_cfg.nMaxGamma);

	cfg.nMaxGamma = m_cfg.nMaxGamma;

	printf("====ReadCameraSetting===00000000===now cfg.nMaxGamma = %f\n",cfg.nMaxGamma);
	printf("====ReadCameraSetting===00000000===now  m_cfg.nMaxGamma = %f\n", m_cfg.nMaxGamma);

	//自动快门
	CEnumerationPtr ptrExposureAuto = m_CameraNodeMap->GetNode("ExposureAuto");
	if(0 == ptrExposureAuto->ToString().compare("Continuous"))
	{
		cfg.ASC = 1;
	}
	else if (0 == ptrExposureAuto->ToString().compare("Off"))
	{
		cfg.ASC = 0;
	}

	//亮度增强
	CIntegerPtr ptrDigitalShift = m_CameraNodeMap->GetNode("DigitalShift");
	cfg.nDigital = ptrDigitalShift->GetValue();
	//cfg.nMaxDigital = ptrDigitalShift->GetMax();

	//快门模式
	cfg.uSM = 1;

	//gamma
	CBooleanPtr ptrGammaEnable = m_CameraNodeMap->GetNode("GammaEnable");
	CFloatPtr ptrGamma = m_CameraNodeMap->GetNode("Gamma");
	cfg.nGamma = (ptrGamma->GetValue())*10;
	printf("===ReadCameraSetting===00000000===now ptrGamma->GetValue() = %f\n",ptrGamma->GetValue());
	printf("====ReadCameraSetting===00000000===now cfg.nGamma = %f\n",cfg.nGamma);
	printf("====ReadCameraSetting===00000000===now cfg.nMaxGamma = %f\n",cfg.nMaxGamma);


	//当前曝光时间
	CFloatPtr ptrExposureTimeAbs = m_CameraNodeMap->GetNode("ExposureTimeAbs");
	cfg.uPE = ptrExposureTimeAbs->GetValue();

	//当前曝光时间最大值
	//cfg.nMaxPE = ptrExposureTimeAbs->GetMax();

	//自动增益
	CEnumerationPtr ptrGainAuto = m_CameraNodeMap->GetNode("GainAuto");
	if (0 == ptrGainAuto->ToString().compare("Off"))
	{
		cfg.AGC = 0;

		//增益值
		CIntegerPtr ptrGainRaw = m_CameraNodeMap->GetNode("GainRaw");
		if(ptrGainRaw->GetAccessMode() == RW)
			cfg.uGain = (int)ptrGainRaw->GetValue()/GAIN_DB_UNIT;

	}
	else if (0 == ptrGainAuto->ToString().compare("Continuous"))
	{
		cfg.AGC = 1;
	}

	//触发方式
	cfg.nMode = 0;

	//补光灯控制
	CEnumerationPtr ptrLineSelector = m_CameraNodeMap->GetNode("LineSelector");
	CEnumerationPtr ptrLineSource = m_CameraNodeMap->GetNode("LineSource");
	CBooleanPtr ptrLineInverter = m_CameraNodeMap->GetNode("LineInverter");
	if (0 == ptrLineSelector->ToString().compare("Out1") &&
		0 == ptrLineSource->ToString().compare("TimerActive"))
	{
		//频闪开关
		cfg.EEN_on = 1;
	}
	else
	{
		cfg.EEN_on = 0;
	}

	//极性
	cfg.uPol = ! ptrLineInverter->GetValue();

	//脉冲延时
	CFloatPtr ptrTimerDelayAbs = m_CameraNodeMap->GetNode("TimerDelayAbs");
	cfg.EEN_delay = ptrTimerDelayAbs->GetValue();

	//脉冲宽度
	CFloatPtr ptrTimerDurationAbs = m_CameraNodeMap->GetNode("TimerDurationAbs");
	cfg.EEN_width = ptrTimerDurationAbs->GetValue();

	if(cfg.uType == 2)//单项
	{
		int nValue = -1;
		if (strcmp(cfg.chCmd,"asc")==0)
		{
			nValue = cfg.ASC;
		}
		else if (strcmp(cfg.chCmd,"agc")==0)
		{
			nValue = cfg.AGC;
		}
		else if (strcmp(cfg.chCmd,"sh")==0)
		{
			nValue = cfg.uSH;
		}
		else if (strcmp(cfg.chCmd,"ga")==0)
		{
			nValue = cfg.uGain;
		}
		else if (strcmp(cfg.chCmd,"gama")==0)
		{
			nValue = cfg.nGamma;
			printf("====ReadCameraSetting===00000000===cfg.nGamma = %f,cfg.nMaxGamma=%f \n",cfg.nGamma,cfg.nMaxGamma);
		}
		else if (strcmp(cfg.chCmd,"pe")==0)
		{
			nValue = cfg.uPE;
		}
		else if (strcmp(cfg.chCmd,"epl")==0)
		{
			if(cfg.nMode == 0 )
				nValue = cfg.uPol;
		}
		else if (strcmp(cfg.chCmd,"pen")==0 ||
			strcmp(cfg.chCmd,"PEN")==0)
		{
			if(cfg.nMode == 0 )
			{
				nValue = cfg.EEN_on;
				cfg.EEN_on = nValue;
			}
		}
		else if (strcmp(cfg.chCmd,"pei")==0)
		{
			nValue = cfg.EEN_delay;
		}
		else if (strcmp(cfg.chCmd,"pew")==0)
		{
			if(cfg.nMode == 0 )
			{
				nValue = cfg.EEN_width;
			}
		}
		else if (strcmp(cfg.chCmd,"ds")==0)
		{
			if(cfg.nMode == 0 )
			{
				nValue = cfg.nDigital;
			}
		}

		if(nValue != -1)
		{
			memset(cfg.chCmd,0,sizeof(cfg.chCmd));
			sprintf(cfg.chCmd,"=%d",nValue);
		}
	}

	if(m_nCameraType == BASLER_200)
	{
		cfg.uKind = BASLER_200;
	}
	else if(m_nCameraType == BASLER_500)
	{
		cfg.uKind = BASLER_500;
	}
	else
	{
		printf("camera type error!\n");
	}

	return 1;
}

bool BaslerCamera::ChangeMode()
{
	return true;
}

#endif

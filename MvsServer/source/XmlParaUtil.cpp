#include "XmlParser.h"
#include "XmlParaUtil.h"
#include "VehicleConfig.h"
#include "Common.h"
#include "CommonHeader.h"

/*
#ifndef _DEBUG
    #define _DEBUG
#endif
*/

typedef std::multimap<int,int> ModelIndexMap;

//生成相机模板
bool CXmlParaUtil::AddCameraParaModel(CAMERA_CONFIG& cfg)
{
    char buf[64];
	memset(buf, 0, sizeof(buf));

    XMLNode xml,camera,temp;

    std::string strCameraParaXml = "./profile/CameraPara.xml";
    //判断CameraPara.xml是否存在
    if(access(strCameraParaXml.c_str(),F_OK) != 0)//不存在
    {
        xml = XMLNode::createXMLTopNode("CameraPara");
    }
    else //存在
    {
        xml = XMLNode::parseFile(strCameraParaXml.c_str()).getChildNode("CameraPara");
        if(xml.isEmpty())
        {
            xml = XMLNode::createXMLTopNode("CameraPara");
        }

        for(int i =0;i<xml.nChildNode();i++)
        {
            camera = xml.getChildNode(i);
						
			if(g_nDetectMode == 2)
			{
				temp = camera.getChildNode("CamId");
				if(!temp.isEmpty())
				{
					XMLCSTR strText = temp.getText();
					if(xmltoi(strText) == cfg.uCameraID)
					{
						camera.deleteNodeContent();
						break;
					}
				}	
			}
			else 
			{
				temp = camera.getChildNode("Type");
				if(!temp.isEmpty())
				{
					XMLCSTR strText = temp.getText();					
					if(xmltoi(strText) == cfg.uKind)
					{
						camera.deleteNodeContent();
						break;
					}					               
				}			
			} 
        }//End of for
    }

    if(!xml.isEmpty())
    {
        camera = xml.addChild("Camera");	      
		
		temp = camera.addChild("Type");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",cfg.uKind);
		temp.addText(buf);

		temp = camera.addChild("CamId");
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", cfg.uCameraID);
		temp.addText(buf);

        temp = camera.addChild("Name");
        if(cfg.uKind == JAI_CAMERA_LINK_FIELD)
            sprintf(buf,"JAI_CAMERA_LINK_FIELD");
        else if(cfg.uKind == JAI_CAMERA_LINK_FIELD_P)
            sprintf(buf,"JAI_CAMERA_LINK_FIELD_P");
        else if(cfg.uKind == JAI_CAMERA_LINK_FRAME)
            sprintf(buf,"JAI_CAMERA_LINK_FRAME");
        else if(cfg.uKind == PTG_ZEBRA_200)
            sprintf(buf,"PTG_ZEBRA_200");
        else if(cfg.uKind == PTG_ZEBRA_500)
            sprintf(buf,"PTG_ZEBRA_500");
        else if(cfg.uKind == JAI_CL_500)
            sprintf(buf,"JAI_CL_500");
        else if(cfg.uKind == ROSEEK_CAMERA) //锐视200万相机
            sprintf(buf,"ROSEEK_CAMERA");
        else if(cfg.uKind == DSP_ROSEEK_200_310) //锐视200万dsp相机
            sprintf(buf, "DSP_ROSEEK_200_310");
		else if(cfg.uKind == DSP_ROSEEK_200_385)
			sprintf(buf, "DSP_ROSEEK_200_385");
        else if(cfg.uKind == DSP_ROSEEK_500_335) //锐视500万相机
            sprintf(buf, "DSP_ROSEEK_500_335");
		else if(cfg.uKind == DSP_ROSEEK_200_380) //锐视200万dsp相机
            sprintf(buf, "DSP_ROSEEK_200_380");
        else if(cfg.uKind == DSP_ROSEEK_500_330) //锐视500万相机
            sprintf(buf, "DSP_ROSEEK_500_330");
		else if(cfg.uKind == PTG_GIGE_200)
			sprintf(buf,"PTG_GIGE_200");
		else if(cfg.uKind == PTG_GIGE_500)
            sprintf(buf,"PTG_GIGE_500");
		else if(cfg.uKind == BOCOM_301_200)
            sprintf(buf,"BOCOM_301_200");
		else if(cfg.uKind == BOCOM_302_500)
            sprintf(buf,"BOCOM_302_500");
		else if(cfg.uKind == BOCOM_301_500)
            sprintf(buf,"BOCOM_301_500");
		else if(cfg.uKind == BOCOM_302_200)
            sprintf(buf,"BOCOM_302_200");
		else if(cfg.uKind == BASLER_200)
			sprintf(buf,"BASLER_200");
		else if(cfg.uKind == DSP_ROSEEK_400_340) //锐视400万相机
			sprintf(buf, "DSP_ROSEEK_400_340");
		else if(cfg.uKind == DSP_500_C501K)
			sprintf(buf, "DSP_500_C501K");
		else if(cfg.uKind == DSP_200_C201K)
			sprintf(buf, "DSP_200_C201K");
		else if(cfg.uKind == DSP_ROSEEK_600_465)
			sprintf(buf, "DSP_ROSEEK_600_465");
        else
            sprintf(buf,"OTHER_CAMERA");

        temp.addText(buf);

        temp = camera.addChild("SM");
        memset(buf, 0, sizeof(buf));
        if(cfg.uSM<0)
        cfg.uSM = 0;
        sprintf(buf,"%d",cfg.uSM);
        temp.addText(buf);

        temp = camera.addChild("POL");
        memset(buf, 0, sizeof(buf));
        if(cfg.uPol<0)
        cfg.uPol = 0;
        sprintf(buf,"%d",cfg.uPol);
        temp.addText(buf);

        temp = camera.addChild("EEN_DELAY");
        memset(buf, 0, sizeof(buf));
        if(cfg.EEN_delay<0)
        cfg.EEN_delay = 0;
        sprintf(buf,"%d",cfg.EEN_delay);
        temp.addText(buf);

        temp = camera.addChild("EEN_WIDTH");
        memset(buf, 0, sizeof(buf));
        if(cfg.EEN_width<=0)
        cfg.EEN_width = 1000;
        sprintf(buf,"%d",cfg.EEN_width);
        temp.addText(buf);
        printf("cfg.ASC=%d,cfg.AGC=%d",cfg.ASC,cfg.AGC);
        temp = camera.addChild("ASC");
        memset(buf, 0, sizeof(buf));
        if(cfg.ASC<0)
        cfg.ASC = 0;
        sprintf(buf,"%d",cfg.ASC);
        temp.addText(buf);

        temp = camera.addChild("AGC");
        memset(buf, 0, sizeof(buf));
        if(cfg.AGC<0)
        cfg.AGC = 0;
        sprintf(buf,"%d",cfg.AGC);
        temp.addText(buf);

		temp = camera.addChild("DigitalShift");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",cfg.nDigital);
		temp.addText(buf);

        temp = camera.addChild("MaxPE");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxPE<=0)
        cfg.nMaxPE = 1665;
        sprintf(buf,"%d",cfg.nMaxPE);
        temp.addText(buf);

        temp = camera.addChild("MaxSH");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxSH<=0)
        cfg.nMaxSH = 2000;
        sprintf(buf,"%d",cfg.nMaxSH);
        temp.addText(buf);

        temp = camera.addChild("MaxGain");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxGain<=0)
        cfg.nMaxGain = 20;
        sprintf(buf,"%d",cfg.nMaxGain);
        temp.addText(buf);

        temp = camera.addChild("MaxPE2");//非机动车道极大值
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxPE2<=0)
        cfg.nMaxPE2 = 1665;
        sprintf(buf,"%d",cfg.nMaxPE2);
        temp.addText(buf);

        temp = camera.addChild("MaxSH2");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxSH2<=0)
        cfg.nMaxSH2 = 2000;
        sprintf(buf,"%d",cfg.nMaxSH2);
        temp.addText(buf);

        temp = camera.addChild("MaxGain2");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxGain2<=0)
        cfg.nMaxGain2 = 20;
        sprintf(buf,"%d",cfg.nMaxGain2);
        temp.addText(buf);

		temp = camera.addChild("MaxDigitalShift");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",cfg.nMaxDigital);
		temp.addText(buf);

        temp = camera.addChild("Gamma");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMaxGamma<=0)
        cfg.nMaxGamma = 100;
        sprintf(buf,"%d",cfg.nMaxGamma);
        temp.addText(buf);

        temp = camera.addChild("Mode");
        memset(buf, 0, sizeof(buf));
        if(cfg.nMode<0)
        cfg.nMode = 0;
        sprintf(buf,"%d",cfg.nMode);
        temp.addText(buf);

        temp = camera.addChild("LightType");
        memset(buf, 0, sizeof(buf));
        if(cfg.nLightType<0)
        cfg.nLightType = 1;
        sprintf(buf,"%d",cfg.nLightType);
        temp.addText(buf);

        temp = camera.addChild("UseLUT");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",cfg.UseLUT);
        temp.addText(buf);

        temp = camera.addChild("Frequency");//触发频率
        memset(buf, 0, sizeof(buf));
        if(cfg.nFrequency<=0)
        cfg.nFrequency = 10;
        sprintf(buf,"%d",cfg.nFrequency);
        temp.addText(buf);
    }

    if(!xml.writeToFile(strCameraParaXml.c_str()))
    {
        LogError("生成模板失败!\r\n");
        return false;
    }
    //
    return true;
}

//载入相机模板
bool CXmlParaUtil::LoadCameraParaModel(CAMERA_CONFIG& cfg,bool bLoadByServer)
{
    std::string strCameraParaXml = "./profile/CameraPara.xml";

    if(access(strCameraParaXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode xml,camera,temp;
        xml = XMLNode::parseFile(strCameraParaXml.c_str()).getChildNode("CameraPara");

        for(int i =0;i<xml.nChildNode();i++)
        {
            camera = xml.getChildNode(i);		

            temp = camera.getChildNode("Type");
            if(!temp.isEmpty())
            {
                XMLCSTR strText = temp.getText();
                if(xmltoi(strText) == cfg.uKind)
                {					
					if(g_nDetectMode == 2)
					{
						temp = camera.getChildNode("CamId");
						if(!temp.isEmpty())
						{
							strText = temp.getText();
							if(xmltoi(strText) == cfg.uCameraID)
							{
								//LogNormal("LoadCameraParaModel=cfg.uCameraID=%d=\n", cfg.uCameraID);
							}
							else
							{
								//LogNormal("222Lcfg.uCameraID=%d=,cfg.uKind=%d,xmltoi(strText)=%d\n", \
									cfg.uCameraID, cfg.uKind, xmltoi(strText));
								//return false;
								continue;
							}
						}
						else
						{
							continue;
						}
					}//End of DSP相机按cfg.uCameraID标识
					

                    temp = camera.getChildNode("Gamma");
                    if(!temp.isEmpty())
                    {
                            strText = temp.getText();
                            if(strText)
                            {
                                cfg.nMaxGamma = xmltoi(strText);
                                if(cfg.nMaxGamma <= 0)
                                {
                                    cfg.nMaxGamma = 100;
                                }
                            }
                    }

                    temp = camera.getChildNode("Mode");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMode = xmltoi(strText);

                        if(cfg.nMode<0)
                        cfg.nMode = 0;
                    }

                    temp = camera.getChildNode("LightType");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nLightType = xmltoi(strText);

                        if(cfg.nLightType<0)
                        cfg.nLightType = 1;
                    }

                    temp = camera.getChildNode("UseLUT");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.UseLUT = xmltoi(strText);
                    }

                    temp = camera.getChildNode("Frequency");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nFrequency = xmltoi(strText);

                        if(cfg.nFrequency<=0)
                        cfg.nFrequency = 10;
                    }

                    temp = camera.getChildNode("SM");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.uSM = xmltoi(strText);

                        if(cfg.uSM<0)
                        cfg.uSM = 0;
                    }

                    temp = camera.getChildNode("POL");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.uPol = xmltoi(strText);

                        if(cfg.uPol<0)
                        cfg.uPol = 0;
                    }

                    temp = camera.getChildNode("EEN_DELAY");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.EEN_delay = xmltoi(strText);

                         if(cfg.EEN_delay<0)
                        cfg.EEN_delay = 0;
                    }

                    temp = camera.getChildNode("EEN_WIDTH");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.EEN_width = xmltoi(strText);

                        if(cfg.EEN_width<=0)
                        cfg.EEN_width = 1000;
                    }

                    temp = camera.getChildNode("ASC");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.ASC = xmltoi(strText);

                        if(cfg.ASC<0)
                        cfg.ASC = 0;
                    }

                    temp = camera.getChildNode("AGC");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.AGC = xmltoi(strText);

                        if(cfg.AGC<0)
                        cfg.AGC = 0;
                    }

                    temp = camera.getChildNode("MaxPE");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxPE = xmltoi(strText);

                        if(cfg.nMaxPE<=0)
                        cfg.nMaxPE = 2000;
                    }

                    temp = camera.getChildNode("MaxPE2");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxPE2 = xmltoi(strText);

                        if(cfg.nMaxPE2<=0)
                        cfg.nMaxPE2 = 2000;
                    }

                    temp = camera.getChildNode("MaxSH");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxSH = xmltoi(strText);

                        if(cfg.nMaxSH<=0)
                        cfg.nMaxSH = 2000;
                    }

                    temp = camera.getChildNode("MaxSH2");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxSH2 = xmltoi(strText);

                        if(cfg.nMaxSH2<=0)
                        cfg.nMaxSH2 = 2000;
                    }

                    temp = camera.getChildNode("MaxGain");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxGain = xmltoi(strText);

                        if(cfg.nMaxGain<=0)
                        cfg.nMaxGain = 20;
                    }

                    temp = camera.getChildNode("MaxGain2");
                    if(!temp.isEmpty())
                    {
                        strText = temp.getText();
                        if(strText)
                        cfg.nMaxGain2 = xmltoi(strText);

                        if(cfg.nMaxGain2<=0)
                        cfg.nMaxGain2 = 20;
                    }

					temp = camera.getChildNode("DigitalShift");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
							cfg.nDigital = xmltoi(strText);

						if(cfg.nDigital<0)
							cfg.nDigital = 2;
					}

					temp = camera.getChildNode("MaxDigitalShift");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
							cfg.nMaxDigital = xmltoi(strText);

						if(cfg.nMaxDigital>4)
							cfg.nMaxDigital = 4;
					}

                    //printf("cfg.uSM=%d,cfg.uPol=%d,cfg.EEN_delay=%d,cfg.EEN_width=%d,cfg.ASC=%d,cfg.AGC=%d,cfg.nMaxPE=%d,cfg.nMaxSH=%d,cfg.nMaxGain=%d\n",cfg.uSM,cfg.uPol,cfg.EEN_delay,cfg.EEN_width,cfg.ASC,cfg.AGC,cfg.nMaxPE,cfg.nMaxSH,cfg.nMaxGain);
                    return true;
                } //End of if(xmltoi(strText) == cfg.uKind)
            }//End of if(!temp.isEmpty())
        }
    }
    return false;
}

//载入系统设置
bool CXmlParaUtil::LoadSystemSetting()
{
    std::string strSystemXml = "./system.xml";

    if(access(strSystemXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode xml,setting,temp,com;
        XMLCSTR strText;
        xml = XMLNode::parseFile(strSystemXml.c_str()).getChildNode("SystemSetting");

        {
            setting = xml.getChildNode("BasicSetting");
            {
                temp = setting.getChildNode("ServerPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
					#ifdef PRESSURE_TEST
                    g_nEPort = xmltoi(strText);
					#endif
                }

                temp = setting.getChildNode("VideoPath");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strVideo = strText;
                }

                temp = setting.getChildNode("PicPath");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strPic = strText;
                }

                temp = setting.getChildNode("BackPath");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strBack = strText;
                }

				temp = setting.getChildNode("FtpHome");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strFtpHome = strText;
                }
				
                printf("g_nPPort=%d,g_strVideo=%s,g_strPic=%s,g_strBack=%s,g_strFtpHome=%s\n",g_nPPort,g_strVideo.c_str(),g_strPic.c_str(),g_strBack.c_str(),g_strFtpHome.c_str());
            }

            setting = xml.getChildNode("DBSetting");
            {
                temp = setting.getChildNode("DBHost");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strDbHost = strText;
                }

                temp = setting.getChildNode("DBPort");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_nDbPort = xmltoi(strText);
                }

                temp = setting.getChildNode("DBName");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strDatabase = strText;
                }

                temp = setting.getChildNode("DBUser");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strUser = strText;
                }

                temp = setting.getChildNode("DBPass");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_strPw = strText;
                }

                temp = setting.getChildNode("DiskDay");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                g_uDiskDay = xmltoi(strText);
                }

                printf("g_uDiskDay=%d,g_nDbPort=%d,g_strDbHost=%s,g_strDatabase=%s,g_strUser=%s,g_strPw=%s\n",g_uDiskDay,g_nDbPort,g_strDbHost.c_str(),g_strDatabase.c_str(),g_strUser.c_str(),g_strPw.c_str());
            }

            setting = xml.getChildNode("ComSetting");
            {
                for(int j =0;j<setting.nChildNode();j++)
                {
                    com = setting.getChildNode(j);

                    COM_PARAMETER com_para;
                    temp = com.getChildNode("ComID");
                    strText = temp.getText();
                    if(strText)
                    {
                        com_para.nComPort = xmltoi(strText);
                    }

                    temp = com.getChildNode("BaudRate");
                    strText = temp.getText();
                    if(strText)
                    {
                        com_para.nBaud = xmltoi(strText);
                    }

                    temp = com.getChildNode("DataBit");
                    strText = temp.getText();
                    if(strText)
                    {
                        com_para.nDataBits = xmltoi(strText);
                    }

                    temp = com.getChildNode("StopBit");
                    strText = temp.getText();
                    if(strText)
                    {
                        com_para.nStopBits = xmltoi(strText);
                    }

                    temp = com.getChildNode("Parity");
                    strText = temp.getText();
                    if(strText)
                    {
                        com_para.nParity = xmltoi(strText);
                    }

                    temp = com.getChildNode("ComUse");
                    strText = temp.getText();
                    if(strText)
                    {
                        if(strcmp(strText,"Camera")==0)
                        {
                            com_para.nComUse = 1;
                            g_CameraComSetting = com_para;
                        }
                        else if(strcmp(strText,"RedLight")==0)
                        {
                            com_para.nComUse = 2;
                            g_RedLightComSetting = com_para;
                        }
						else if(strcmp(strText,"RedLightA")==0)
						{
							com_para.nComUse = 14;
							g_RedLightComSetting = com_para;
						}
                        else if(strcmp(strText,"DetectVehicle")==0)
                        {
                            com_para.nComUse = 3;
                            g_DHComSetting = com_para;
                        }
                        else if(strcmp(strText,"Light")==0)
                        {
                            com_para.nComUse = 4;
                            g_LightComSetting = com_para;
                        }
                        else if(strcmp(strText,"VIS")==0)
                        {
                            com_para.nComUse = 5;
                            g_VisComSetting = com_para;
                        }
                        else if(strcmp(strText,"GPS")==0)
                        {
                            com_para.nComUse = 6;
                            g_GpsComSetting = com_para;
                        }
                        else if(strcmp(strText,"Radar")==0)
                        {
                            com_para.nComUse = 7;
                            g_RadarComSetting = com_para;
                        }
                        else if(strcmp(strText,"Flash")==0)
                        {
                            com_para.nComUse = 8;
                            g_FlashComSetting = com_para;
                        }
						else if(strcmp(strText,"Door")==0)
                        {
                            com_para.nComUse = 9;
                            g_DoorComSetting = com_para;
                        }
						else if (strcmp(strText, "Dio")==0) // dio
						{
							com_para.nComUse = 10;
							g_DioComSetting = com_para;      
						}
						else if (strcmp(strText, "JHCtechScreen")==0) 
						{
							com_para.nComUse = 11;
							g_ScreenComSetting = com_para;      
						}
						else if (strcmp(strText, "WeiQiangScreen")==0)
						{
							com_para.nComUse = 12;
							g_ScreenComSetting = com_para;      
						}
						else if (strcmp(strText, "RadarHuiChang")==0)
						{
							com_para.nComUse = 13;
							g_RadarComSetting = com_para;
							LogNormal("com port:%d", com_para.nComPort);
						}
                    }
                    g_mapComSetting.insert(COM_PARAMETER_MAP::value_type(com_para.nComPort,com_para));
                }
            }

            setting = xml.getChildNode("YunTaiSetting");
            {
                //是否需要进行控制
                temp = setting.getChildNode("NeedControl");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       g_ytControlSetting.nNeedControl = xmltoi(strText);
                    }
                }

                //控制方式
                temp = setting.getChildNode("ControlMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nControlMode = xmltoi(strText);
                        g_nControlMode = g_ytControlSetting.nControlMode;
                    }
                }

				//LogNormal("-------5.");
				// 图片样式的控制     
				temp = setting.getChildNode("PicComPoseMode");
				if (!temp.isEmpty())
				{
					strText = temp.getText();
					if (strText)
					{
						g_ytControlSetting.nPicComPoseMode = xmltoi(strText);

					}
				}
                //键盘编号
                temp = setting.getChildNode("KeyBoardID");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nKeyBoardID = xmltoi(strText);
                        g_nKeyBoardID = g_ytControlSetting.nKeyBoardID;
                    }
                }

                //vis服务地址
                temp = setting.getChildNode("VisHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_strVisHost = strText;
                        memcpy(g_ytControlSetting.szVisHost,g_strVisHost.c_str(),g_strVisHost.size());
                    }
                }

                //vis服务端口
                temp = setting.getChildNode("VisPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nVisPort = xmltoi(strText);
                        g_nVisPort = g_ytControlSetting.nVisPort;
                    }
                }

                //协议类型
                temp = setting.getChildNode("Protocal");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nProtocalType = xmltoi(strText);
                    }
                }

                //是否存在多个预置位
                temp = setting.getChildNode("MultiPreSet");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nMultiPreSet = xmltoi(strText);
                       g_nMultiPreSet = g_ytControlSetting.nMultiPreSet;
                    }
                }

                //是否存在近景预置位
                temp = setting.getChildNode("HasLocalPreSet");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ytControlSetting.nHasLocalPreSet = xmltoi(strText);
                    }
                }

				//Serial服务地址
				temp = setting.getChildNode("SerialHost");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						string strSerialHost = strText;
						memcpy(g_ytControlSetting.szSerialHost,strSerialHost.c_str(),strSerialHost.size());
					}
				}

				//Serial服务端口
				temp = setting.getChildNode("SerialPort");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nSerialPort = xmltoi(strText);
					}
				}

				//地址码
				temp = setting.getChildNode("AddressCode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nAddressCode = xmltoi(strText);
					}
				}

				//违停抓拍时间
				temp = setting.getChildNode("StopInterval");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nStopInterval = xmltoi(strText);
					}
				}

				//远景图片时间间隔
				temp = setting.getChildNode("RemotePicInterval");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nRemotePicInterval = xmltoi(strText);
					}
				}

				//预置位模式
				temp = setting.getChildNode("PreSetMode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nPreSetMode = xmltoi(strText);
						g_nPreSetMode = g_ytControlSetting.nPreSetMode;
					}
				}

				//是否使用3D智能抓拍方式
				temp = setting.getChildNode("CameraAutoMode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nCameraAutoMode = xmltoi(strText);
						printf("g_ytControlSetting.nCameraAutoMode = %d \n",g_ytControlSetting.nCameraAutoMode);
					}
				}


				//未识别车牌是否入库
				temp = setting.getChildNode("CarnumToDBMode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nCarnumToDBMode = xmltoi(strText);
						printf("g_ytControlSetting.nCarnumToDBMode = %d \n",g_ytControlSetting.nCarnumToDBMode);
					}
				}

				//手动抓拍时间间隔
				temp = setting.getChildNode("HandCatchTime");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_ytControlSetting.nHandCatchTime = xmltoi(strText);
						printf("g_ytControlSetting.nHandCatchTime = %d \n",g_ytControlSetting.nHandCatchTime);
					}
				}

            }


            setting = xml.getChildNode("MonitorHostSetting");
            {
                //监控主机地址
                temp = setting.getChildNode("MonitorHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strMonitorHost = strText;
                       memcpy(g_monitorHostInfo.chMonitorHost,strMonitorHost.c_str(),strMonitorHost.size());
                    }
                }

                //监控主机端口
                temp = setting.getChildNode("MonitorPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_monitorHostInfo.uMonitorPort = xmltoi(strText);
                    }
                }

                //监控主机用户名
                temp = setting.getChildNode("UserName");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strUserName = strText;
                       memcpy(g_monitorHostInfo.chUserName,strUserName.c_str(),strUserName.size());
                    }
                }

                //监控主机密码
                temp = setting.getChildNode("PassWord");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strPassWord = strText;
                       memcpy(g_monitorHostInfo.chPassWord,strPassWord.c_str(),strPassWord.size());
                    }
                }

				//sip服务器编号
				temp = setting.getChildNode("SipServerCode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						string strSipServerCode = strText;
						memcpy(g_monitorHostInfo.chSipServerCode,strSipServerCode.c_str(),strSipServerCode.size());
					}
				}

				//sip客户端编号
				temp = setting.getChildNode("SipClientCode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						string strSipClientCode = strText;
						memcpy(g_monitorHostInfo.chSipClientCode,strSipClientCode.c_str(),strSipClientCode.size());
					}
				}
            }

			setting = xml.getChildNode("NtpTimeInfo");
			{
				temp = setting.getChildNode("Time1");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Ntp_Time.nTime1 = xmltoi(strText);
					}
					else
					{
						g_Ntp_Time.nTime1 = 0;
					}
				}

				temp = setting.getChildNode("Time2");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Ntp_Time.nTime2 = xmltoi(strText);
					}
					else
					{
						g_Ntp_Time.nTime2 = 0;
					}
				}

				temp = setting.getChildNode("Time3");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Ntp_Time.nTime3 = xmltoi(strText);
					}
					else
					{
						g_Ntp_Time.nTime3 = 0;
					}
				}
				temp = setting.getChildNode("NtpServerIp");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ntp_Time.chHost, "%s", strText);
					}
					else
					{
						memset(g_Ntp_Time.chHost,0,SKP_MAX_HOST);
					}
				}

			}
			setting = xml.getChildNode("KafkaSet");
			{
				temp = setting.getChildNode("SwitchUploading");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Kafka.uSwitchUploading = xmltoi(strText);
					}
					else
					{
						g_Kafka.uSwitchUploading = 0;
					}
				}

				temp = setting.getChildNode("Item");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Kafka.chItem, "%s", strText);
					}
					else
					{
						memset(g_Kafka.chItem,0,64);
					}
				}

				temp = setting.getChildNode("Brand");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Kafka.chBrand, "%s", strText);
					}
					else
					{
						memset(g_Kafka.chBrand,0,32);
					}
				}

				temp = setting.getChildNode("Version");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Kafka.chVersion, "%s", strText);
					}
					else
					{
						memset(g_Kafka.chVersion,0,32);
					}
				}

				temp = setting.getChildNode("Topic");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Kafka.chTopic, "%s", strText);
					}
					else
					{
						memset(g_Kafka.chTopic,0,32);
					}
				}

				temp = setting.getChildNode("UpdateType");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Kafka.uUpdateType = xmltoi(strText);
					}
					else
					{
						g_Kafka.uUpdateType = 0;
					}
				}
				temp = setting.getChildNode("CheckModal");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Kafka.uCheckModal = xmltoi(strText);
					}
					else
					{
						g_Kafka.uCheckModal = 0;
					}
				}
			}

			setting = xml.getChildNode("CheckTime");
			{
				temp = setting.getChildNode("MornFastigiumBegin");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nMornFastigiumBegin = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nMornFastigiumBegin = 0;
					}
				}

				temp = setting.getChildNode("MornFastigiumEnd");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nMornFastigiumEnd = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nMornFastigiumEnd = 0;
					}
				}

				temp = setting.getChildNode("NightFastigiumBegin");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nNightFastigiumBegin = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nNightFastigiumBegin = 0;
					}
				}

				temp = setting.getChildNode("NightFastigiumEnd");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nNightFastigiumEnd = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nNightFastigiumEnd = 0;
					}
				}

				temp = setting.getChildNode("OrdinaryMornBegin");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nOrdinaryMornBegin = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nOrdinaryMornBegin = 0;
					}
				}

				temp = setting.getChildNode("OrdinaryMornEnd");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nOrdinaryMornEnd = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nOrdinaryMornEnd = 0;
					}
				}

				temp = setting.getChildNode("OrdinaryNightBegin");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nOrdinaryNightBegin = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nOrdinaryNightBegin = 0;
					}
				}

				temp = setting.getChildNode("OrdinaryNightEnd");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_CheckTime.nOrdinaryNightEnd = xmltoi(strText);
					}
					else
					{
						g_CheckTime.nOrdinaryNightEnd = 0;
					}
				}
			}

			setting = xml.getChildNode("FtpPathInfo");
			{
				temp = setting.getChildNode("UseFlag");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Ftp_Path.nUseFlag = xmltoi(strText);
					}
					else
					{
						g_Ftp_Path.nUseFlag = 0;
					}
				}
				temp = setting.getChildNode("ServerType");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_Ftp_Path.nServerType = xmltoi(strText);
					}
					else
					{
						g_Ftp_Path.nServerType = 999;
					}
				}
				temp = setting.getChildNode("VideoPath");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ftp_Path.strVideoPath, "%s", strText);
					}
					else
					{
						memset(g_Ftp_Path.strVideoPath,0,SRIP_LOCATION_MAXLEN);
					}
				}
				temp = setting.getChildNode("WfPicPath");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ftp_Path.strWfPicPath, "%s", strText);
					}
					else
					{
						memset(g_Ftp_Path.strWfPicPath,0,SRIP_LOCATION_MAXLEN);
					}
				}
				temp = setting.getChildNode("WfTxtPath");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ftp_Path.strWfTxtPath, "%s", strText);
					}
					else
					{
						memset(g_Ftp_Path.strWfTxtPath,0,SRIP_LOCATION_MAXLEN);
					}
				}
				temp = setting.getChildNode("KkPicPath");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ftp_Path.strKkPicPath, "%s", strText);
					}
					else
					{
						memset(g_Ftp_Path.strKkPicPath,0,SRIP_LOCATION_MAXLEN);
					}
				}
				temp = setting.getChildNode("FsTxtPath");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						sprintf(g_Ftp_Path.strFsTxtPath, "%s", strText);
					}
					else
					{
						memset(g_Ftp_Path.strFsTxtPath,0,SRIP_LOCATION_MAXLEN);
					}
				}
			}
			 setting = xml.getChildNode("3GInfo");
            {
                //是否存在3G
                temp = setting.getChildNode("Exist3G");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       g_nExist3G = xmltoi(strText);
                    }
                }
				//3G IP
				temp = setting.getChildNode("3GIP");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_str3GIp = strText;
					}
				}
				//3G TYPE
				temp = setting.getChildNode("3GTYPE");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_n3GTYPE = xmltoi(strText);
					}
				}
			}

            //智能控制器设置
            setting = xml.getChildNode("ExpoMonitorSetting");
            {
                //智能控制器主机地址
                temp = setting.getChildNode("ExpoMonitorHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strExpoMonitorHost = strText;
                       memcpy(g_ExpoMonitorInfo.chExpoMonitorHost,strExpoMonitorHost.c_str(),strExpoMonitorHost.size());
                    }
                }

                //智能控制器主机端口
                temp = setting.getChildNode("ExpoMonitorPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_ExpoMonitorInfo.uExpoMonitorPort = xmltoi(strText);
                    }
                }
            }

            //比对服务器器设置
            setting = xml.getChildNode("MatchHostSetting");
            {
                //比对服务器主机地址
                temp = setting.getChildNode("MatchHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strMatchHost = strText;
                       memcpy(g_MatchHostInfo.chMatchHost,strMatchHost.c_str(),strMatchHost.size());
                    }
                }

                //比对服务器端口
                temp = setting.getChildNode("MatchPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_MatchHostInfo.uMatchPort = xmltoi(strText);
                    }
                }

                //是否有比对服务器
                temp = setting.getChildNode("HasMatchHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_MatchHostInfo.uHasMatchHost = xmltoi(strText);
                    }
                }
            }

			//尾号限行设置
            setting = xml.getChildNode("PlateLimit");
            {
                //是否尾号限行
                temp = setting.getChildNode("IsPlateLimit");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       g_PlateLimit.nIsPlateLimit = xmltoi(strText);
                    }
                }
				LogNormal("nIsPlateLimit=%c\n",g_PlateLimit.nIsPlateLimit);
                //限行尾号
                temp = setting.getChildNode("PlateNumber");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
						string strPlateNumber = strText;
						memcpy(g_PlateLimit.chPlateNumber,strPlateNumber.c_str(),strPlateNumber.size());
                    }
                }
				LogNormal("chPlateNumber=%c\n",g_PlateLimit.chPlateNumber);
            }

			//区间测速主机设置
            setting = xml.getChildNode("DistanceHostSetting");
            {
                //区间测速主机地址
                temp = setting.getChildNode("DistanceHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                       string strMatchHost = strText;
                       memcpy(g_DistanceHostInfo.chDistanceHost,strMatchHost.c_str(),strMatchHost.size());
					   g_MvsNextHostIp = strMatchHost;
                    }
                }

                //区间测速主机端口
                temp = setting.getChildNode("DistancePort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_DistanceHostInfo.uDistancePort = xmltoi(strText);
						g_MvsNextRecPort =  g_DistanceHostInfo.uDistancePort;
                    }
                }

				//区间测速主机距离
                temp = setting.getChildNode("Distance");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_DistanceHostInfo.uDistance = xmltoi(strText);
						g_MvsDistance =  g_DistanceHostInfo.uDistance;
                    }
                }

				//是否区间测速
				temp = setting.getChildNode("IsDistanceCalculate");
				if (!temp.isEmpty())
				{
					strText = temp.getText();
					if (strText)
					{
						g_DistanceHostInfo.bDistanceCalculate = xmltoi(strText);
						g_bMvsDistanceCal = g_DistanceHostInfo.bDistanceCalculate;
					}
				}

				//u盘拷贝开始时间
				temp = setting.getChildNode("UsbCopyTime");
				if (!temp.isEmpty())
				{
					strText = temp.getText();
					if (strText)
					{
						g_DistanceHostInfo.uUsbCopyTime = xmltoi(strText);
					}
				}

				//是否拷贝录像
				temp = setting.getChildNode("CopyVideo");
				if (!temp.isEmpty())
				{
					strText = temp.getText();
					if (strText)
					{
						g_DistanceHostInfo.bCopyVideo = xmltoi(strText);
					}
				}
            }

			//GPS设置
            setting = xml.getChildNode("GpsSetting");
            {
                //GPS类型
                temp = setting.getChildNode("GpsType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if (strText)
					{
						g_GpsSetInfo.nType = xmltoi(strText);
					}
                }
            }

			//信号机设置
            setting = xml.getChildNode("SignalSetting");
            {
				//是否存在信号机
				temp = setting.getChildNode("SignalExist");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if (strText)
					{
						g_SignalSetInfo.nExist = xmltoi(strText);
					}
                }

                //信号机类型
                temp = setting.getChildNode("SignalType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if (strText)
					{
						g_SignalSetInfo.nType = xmltoi(strText);
					}
                }

				//信号机模式
                temp = setting.getChildNode("SignalMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if (strText)
					{
						g_SignalSetInfo.nMode = xmltoi(strText);
					}
                }
            }


			//应用管理服务器设置
			setting = xml.getChildNode("AmsHostSetting");
			{
				//应用管理服务器主机地址
				temp = setting.getChildNode("AmsHost");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						string strAmsHost = strText;
						memcpy(g_AmsHostInfo.chAmsHost,strAmsHost.c_str(),strAmsHost.size());
					}
				}

				//应用管理服务器端口
				temp = setting.getChildNode("AmsPort");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_AmsHostInfo.uAmsPort = xmltoi(strText);
					}
				}

				//是否有应用管理服务器
				temp = setting.getChildNode("HasAmsHost");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_AmsHostInfo.uHasAmsHost = xmltoi(strText);
					}
				}

				//是否备份检测器
				temp = setting.getChildNode("BakType");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						//g_nBakType = xmltoi(strText);
						g_AmsHostInfo.uBakType = xmltoi(strText);
					}

					LogNormal("g_AmsHostInfo.uBakType=%d \n", g_AmsHostInfo.uBakType);
				}
			}


            //开光灯时间信息
            setting = xml.getChildNode("LightTimeSetting");
            {
                //是否定时开光灯
                temp = setting.getChildNode("LightTimeControl");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_LightTimeInfo.nLightTimeControl = xmltoi(strText);
                    }
                }

                //春季开光灯时间
                temp = setting.getChildNode("SpringLightTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_LightTimeInfo.nSpringLightTime = xmltoi(strText);
                    }
                }

                //夏季开光灯时间
                temp = setting.getChildNode("SummerLightTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_LightTimeInfo.nSummerLightTime = xmltoi(strText);
                    }
                }
                //秋季开光灯时间
                temp = setting.getChildNode("AutumnLightTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_LightTimeInfo.nAutumnLightTime = xmltoi(strText);
                    }
                }

                //冬季开光灯时间
                temp = setting.getChildNode("WinterLightTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_LightTimeInfo.nWinterLightTime = xmltoi(strText);
                    }
                }
            }


            //图片格式信息设置
            setting = xml.getChildNode("PicFormatInfo");
            {
                //字图位置
                temp = setting.getChildNode("WordPos");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nWordPos = xmltoi(strText);
                    }
                }

				temp = setting.getChildNode("PicFlag");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nPicFlag = xmltoi(strText);
					}
				}

                //前景颜色
                temp = setting.getChildNode("ForeColor");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nForeColor = xmltoi(strText);
                    }
                }

                //背景颜色
                temp = setting.getChildNode("BackColor");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nBackColor = xmltoi(strText);
                    }
                }

                //字体
                temp = setting.getChildNode("Font");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nFont = xmltoi(strText);
                    }
                }

                //车身颜色
                temp = setting.getChildNode("CarColor");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nCarColor = xmltoi(strText);
                    }
                }

                //车辆类型
                temp = setting.getChildNode("CarType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nCarType = xmltoi(strText);
                    }
                }

                //车标
                temp = setting.getChildNode("CarBrand");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nCarBrand = xmltoi(strText);
                    }
                }

                //车速
                temp = setting.getChildNode("CarSpeed");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nCarSpeed = xmltoi(strText);
                    }
                }

				//限速值
                temp = setting.getChildNode("SpeedLimit");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nSpeedLimit = xmltoi(strText);
                    }
                }

				//是否存在空白区域
                temp = setting.getChildNode("SpaceRegion");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nSpaceRegion = xmltoi(strText);
                    }
                }

                //违章类型
                temp = setting.getChildNode("ViolationType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nViolationType = xmltoi(strText);
                    }
                }

                //车牌号码
                temp = setting.getChildNode("CarNum");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nCarNum = xmltoi(strText);
                    }
                }

                //车道编号
                temp = setting.getChildNode("RoadIndex");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nRoadIndex = xmltoi(strText);
                    }
                }

                //字体大小
                temp = setting.getChildNode("FontSize");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nFontSize = xmltoi(strText);
                        if(g_PicFormatInfo.nFontSize <= 0)
                        {
                            g_PicFormatInfo.nFontSize = 25;
                        }
                        else if(g_PicFormatInfo.nFontSize >= 300)
                        {
                            g_PicFormatInfo.nFontSize = 300;
                        }
                    }
                }

                //文字区域高度
                temp = setting.getChildNode("ExtentHeight");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nExtentHeight = xmltoi(strText);
                        if(g_PicFormatInfo.nExtentHeight < 0)
                        {
                            g_PicFormatInfo.nExtentHeight = 60;
                        }
                        else if(g_PicFormatInfo.nExtentHeight >= 300)
                        {
                            g_PicFormatInfo.nExtentHeight = 300;
                        }
                    }
                }

                //文字加在图片上
                temp = setting.getChildNode("WordOnPic");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nWordOnPic = xmltoi(strText);
                    }
                }

                //存储小图
                temp = setting.getChildNode("SmallPic");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nSmallPic = xmltoi(strText);
                    }
                }

				//jpg压缩比例
				temp = setting.getChildNode("JpgQuality");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nJpgQuality = xmltoi(strText);
						if(g_PicFormatInfo.nJpgQuality < 5)
						{
							g_PicFormatInfo.nJpgQuality = 5;
						}
					}
				}

				//违章图片是否需要叠加小图
				temp = setting.getChildNode("SmallViolationPic");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nSmallViolationPic = xmltoi(strText);
					}
				}

				//图片上叠加文字的偏移量
				temp = setting.getChildNode("WordOffSetX");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nOffsetX = xmltoi(strText);
					}
				}
				
				temp = setting.getChildNode("WordOffSetY");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nOffsetY = xmltoi(strText);
					}
				}

				//第二车身颜色
                temp = setting.getChildNode("SecondCarColor");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nSecondCarColor = xmltoi(strText);
                    }
                }

				//缩放比例
                temp = setting.getChildNode("ResizeScale");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_PicFormatInfo.nResizeScale = xmltoi(strText);
                    }
                }
				
				//文字行数
				temp = setting.getChildNode("WordLine");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_PicFormatInfo.nWordLine = xmltoi(strText);
					}
				}
            }


            //录像格式信息设置
            setting = xml.getChildNode("VideoFormatInfo");
            {
                //帧率
                temp = setting.getChildNode("FrameRate");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_VideoFormatInfo.nFrameRate = xmltoi(strText);

                        if(g_VideoFormatInfo.nFrameRate == 0)
                        g_fFrameRate = 1;
                        else if(g_VideoFormatInfo.nFrameRate == 1)
                        g_fFrameRate = 2.5;
                        else if(g_VideoFormatInfo.nFrameRate == 2)
                        g_fFrameRate = 5;
                        else if(g_VideoFormatInfo.nFrameRate == 3)
                        g_fFrameRate = 7.5;
                        else if(g_VideoFormatInfo.nFrameRate == 4)
                        g_fFrameRate = 10;
                        else if(g_VideoFormatInfo.nFrameRate == 5)
                        g_fFrameRate = 12.5;
                        else if(g_VideoFormatInfo.nFrameRate == 6)
                        g_fFrameRate = 15;
						else if(g_VideoFormatInfo.nFrameRate == 7)
                        g_fFrameRate = 17.5;
                        else if(g_VideoFormatInfo.nFrameRate == 8)
                        g_fFrameRate = 20;
						else if(g_VideoFormatInfo.nFrameRate == 9)
                        g_fFrameRate = 22.5;
                        else if(g_VideoFormatInfo.nFrameRate == 10)
                        g_fFrameRate = 25;
                    }
                }

                //分辨率
                temp = setting.getChildNode("Resolution");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_VideoFormatInfo.nResolution = xmltoi(strText);

                        if(g_VideoFormatInfo.nResolution == 0)
                        {
                            g_nVideoWidth = 400;
                            g_nVideoHeight = 300;
                        }
                        else if(g_VideoFormatInfo.nResolution == 1)
                        {
                            g_nVideoWidth = 480;
                            g_nVideoHeight = 270;
                        }
                        else if(g_VideoFormatInfo.nResolution == 2)
                        {
                            g_nVideoWidth = 600;
                            g_nVideoHeight = 450;
                        }
                        else if(g_VideoFormatInfo.nResolution == 3)
                        {
                            g_nVideoWidth = 640;
                            g_nVideoHeight = 360;
                        }
                        else if(g_VideoFormatInfo.nResolution == 4)
                        {
                            g_nVideoWidth = 800;
                            g_nVideoHeight = 600;
                        }
                        else if(g_VideoFormatInfo.nResolution == 5)
                        {
                            g_nVideoWidth = 960;
                            g_nVideoHeight = 540;
                        }
                        else if(g_VideoFormatInfo.nResolution == 6)
                        {
                            g_nVideoWidth = 1000;
                            g_nVideoHeight = 750;
                        }
                        else if(g_VideoFormatInfo.nResolution == 7)
                        {
                            g_nVideoWidth = 1280;
                            g_nVideoHeight = 720;
                        }
                        else if(g_VideoFormatInfo.nResolution == 8)
                        {
                            g_nVideoWidth = 1200;
                            g_nVideoHeight = 900;
                        }
                        else if(g_VideoFormatInfo.nResolution == 9)
                        {
                            g_nVideoWidth = 1600;
                            g_nVideoHeight = 900;
                        }
                        else if(g_VideoFormatInfo.nResolution == 10)
                        {
                            g_nVideoWidth = 1600;
                            g_nVideoHeight = 1200;
                        }
                        else if(g_VideoFormatInfo.nResolution == 11)
                        {
                            g_nVideoWidth = 1920;
                            g_nVideoHeight = 1080;
                        }
						else if(g_VideoFormatInfo.nResolution == 12)
                        {
                            g_nVideoWidth = 2400;
                            g_nVideoHeight = 2000;
                        }
                    }
                }

                //编码方式
                temp = setting.getChildNode("EncodeFormat");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_nEncodeFormat = xmltoi(strText);
                        g_VideoFormatInfo.nEncodeFormat = g_nEncodeFormat;
                    }
                }

                //录像时长
                temp = setting.getChildNode("TimeLength");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_VideoFormatInfo.nTimeLength = xmltoi(strText);
                    }
                }

                //扩展avi
                temp = setting.getChildNode("AviHeaderEx");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_nAviHeaderEx = xmltoi(strText);
                        g_VideoFormatInfo.nAviHeaderEx = g_nAviHeaderEx;
                    }

                }

                //是否需要发送rtsp
                temp = setting.getChildNode("SendRtsp");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_nSendRTSP = xmltoi(strText);
                        g_VideoFormatInfo.nSendRtsp = g_nSendRTSP;
                    }
                }

				//是否开启Sip视频服务
				temp = setting.getChildNode("SipService");
				if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_VideoFormatInfo.nSip = xmltoi(strText);
                    }
                }

				//是否需要发送h264
				temp = setting.getChildNode("SendH264");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_VideoFormatInfo.nSendH264 = xmltoi(strText);
					}
				}
            }

            setting = xml.getChildNode("OtherSetting");
            {
                temp = setting.getChildNode("DayTime");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nDay = xmltoi(strText);
                }

                temp = setting.getChildNode("DuskTime");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nDusk = xmltoi(strText);
                }

                temp = setting.getChildNode("NightTime");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nNight = xmltoi(strText);
                }

                //是否存储相机日志
                temp = setting.getChildNode("DspLog");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nDspLog = xmltoi(strText);
                }

                //是否需要相机控制
                temp = setting.getChildNode("CameraControl");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nCameraControl = xmltoi(strText);
                }

                //存图数量
                temp = setting.getChildNode("SaveImageCount");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nSaveImageCount = xmltoi(strText);

                    if(g_nSaveImageCount < 1)
                    {
                        g_nSaveImageCount = 1;
                    }
                    else if(g_nSaveImageCount > 2)
                    {
                        g_nSaveImageCount = 2;
                    }
                }

                //是否只发送违章数据
                temp = setting.getChildNode("SendViolationOnly");
                if(!temp.isEmpty())
                {
					strText = temp.getText();
					if(strText)
					g_nSendViolationOnly = xmltoi(strText);
                }

                //是否去抖动
                temp = setting.getChildNode("CheckShake");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_nCheckShake = xmltoi(strText);
                }


                //回写等待时间
                temp = setting.getChildNode("WriteBackTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nWriteBackTime = xmltoi(strText);
                }

                //时钟同步服务器
                temp = setting.getChildNode("SynClockHost");
                if(!temp.isEmpty())
                {
                strText = temp.getText();
                if(strText)
                g_strSynClockHost = strText;
                }

                //中心认证服务器
                temp = setting.getChildNode("AuthenticationHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strAuthenticationHost = strText;
                }

                //是否相机同步
                temp = setting.getChildNode("DoSynProcess");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nDoSynProcess = xmltoi(strText);
                }

                //是否有中心端
                temp = setting.getChildNode("HasCenterServer");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nHasCenterServer = xmltoi(strText);
                }

                //中心端类型
                temp = setting.getChildNode("ServerType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nServerType = xmltoi(strText);
                }

                //访问控制服务器地址
                temp = setting.getChildNode("ControlServerHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strControlServerHost = strText;
                }

                //访问控制服务器端口
                temp = setting.getChildNode("ControlServerPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nControlServerPort = xmltoi(strText);
                }

                //中心端地址
                temp = setting.getChildNode("CenterServerHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strCenterServerHost = strText;
                }

                //中心端端口
                temp = setting.getChildNode("CenterServerPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nCenterServerPort = xmltoi(strText);
                }
                //ftp服务器地址
                temp = setting.getChildNode("FtpServerHost");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strFtpServerHost = strText;
                }

                //ftp用户名
                temp = setting.getChildNode("FtpUserName");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strFtpUserName = strText;
                }

                //ftp密码
                temp = setting.getChildNode("FtpPassWord");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_strFtpPassWord = strText;
                }

                //ftp端口
                temp = setting.getChildNode("FtpPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nFtpPort = xmltoi(strText);
                }

                //ftp存储路径
                temp = setting.getChildNode("FtpRemotePath");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        string strPath = strText;
                        memcpy(g_ftpRemotePath,strPath.c_str(),strPath.size());
                    }
                }

                //检测器编号
                temp = setting.getChildNode("DetectorID");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_strDetectorID = strText;
                        g_CenterServer.mvSetDetectorId(strText); //设置检测设备编号
                    }
                }

                //车道类型
                temp = setting.getChildNode("ChanWayType");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_nRoadType = xmltoi(strText);
                    }
                }

                //通讯方式
                temp = setting.getChildNode("CommunicationMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_nCommunicationMode = xmltoi(strText);
                    }
                }

                //同步允许的时间误差
                temp = setting.getChildNode("SynTimeGap");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nSynTimeGap = xmltoi(strText);
                }

                //同步匹配的面积重合率阈值
                temp = setting.getChildNode("MatchAreaPercent");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_fMatchAreaPercent = xmltof(strText);
                }

                //同步匹配的轨迹重合率阈值
                temp = setting.getChildNode("SynGoodPercent");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_fSynGoodPercent = xmltof(strText);
                }

                //是否布控报警
                temp = setting.getChildNode("DetectSpecialCarNum");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nDetectSpecialCarNum = xmltoi(strText);
                }

                //是否需要切换相机
                temp = setting.getChildNode("SwitchCamera");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nSwitchCamera = xmltoi(strText);
                }

                //工作方式
                temp = setting.getChildNode("WorkMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nWorkMode = xmltoi(strText);
                }

                //视频爆闪控制
                temp = setting.getChildNode("FlashControl");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nFlashControl = xmltoi(strText);
                }

                //卡口图片组合方式
                temp = setting.getChildNode("PicMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nPicMode = xmltoi(strText);
                }

                //闯红灯图片组合方式
                temp = setting.getChildNode("VtsPicMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nVtsPicMode = xmltoi(strText);
                }

                //历史视频播放方式
                temp = setting.getChildNode("HistoryPlayMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nHistoryPlayMode = xmltoi(strText);
                }

                //是否有智能控制器
                temp = setting.getChildNode("HasExpoMonitor");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nHasExpoMonitor = xmltoi(strText);
                }

                //是否有爆闪灯
                temp = setting.getChildNode("HasHighLight");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nHasHighLight = xmltoi(strText);
                }

                //是否需要发送历史记录
                temp = setting.getChildNode("SendHistoryRecord");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nSendHistoryRecord = xmltoi(strText);
                }

                //是否有ntp-server
                temp = setting.getChildNode("NtpServer");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nNtpServer = xmltoi(strText);
                }

                //校时方式
                temp = setting.getChildNode("ClockMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nClockMode = xmltoi(strText);
                }

				//载入违章车牌数据
				temp = setting.getChildNode("LoadBasePlateInfo");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
						g_nLoadBasePlateInfo = xmltoi(strText);
				}

				//本地车牌
				temp = setting.getChildNode("LocalPlate");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						string strLocalPlate = strText;
						g_strLocalPlate = strLocalPlate;
					}
				}

				//是否ftp服务端
				temp = setting.getChildNode("FtpServer");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_nFtpServer = xmltoi(strText);
					}
				}

                //系统设置模板编号
                temp = setting.getChildNode("ModelID");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    g_nSettingModelID = xmltoi(strText);
                }
               
                //是否发送图片
                temp = setting.getChildNode("SendImage");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                        g_nSendImage = xmltoi(strText);
                }
                //是否按时间段检测
                temp = setting.getChildNode("DetectByTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                        g_nDetectByTime = xmltoi(strText);
                }
                //开始检测时间
                temp = setting.getChildNode("BeginDetectTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                        g_nBeginDetectTime = xmltoi(strText);
                }
                //结束检测时间
                temp = setting.getChildNode("EndDetectTime");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                        g_nEndDetectTime = xmltoi(strText);
                }
				//检测器工作模式,0:连续模式(默认);1:触发模式;2:DSP模式
				temp = setting.getChildNode("DetectMode");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                        g_nDetectMode = xmltoi(strText);
                }
				
				//是否公交模式
				temp = setting.getChildNode("GongJiaoMode");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
						g_nGongJiaoMode = xmltoi(strText);
				}
            }
			 //Dsp相机服务器主机设置信息
            setting = xml.getChildNode("DspServerHostSetting");
			if (!setting.isEmpty())
            {
                //Server端口
                temp = setting.getChildNode("DspServerPort");
                if(!temp.isEmpty())
                {
                    strText = temp.getText();
                    if(strText)
                    {
                        g_DspServerHostInfo.uDspServerPort = xmltoi(strText);
                    }
                }

                //LogNormal("==Get=uDspServerHost=%d=,uDspServerPort=%d=\n", g_DspServerHostInfo.uDspServerHost, g_DspServerHostInfo.uDspServerPort);
            }

			//SNMP网管中心端信息
			setting = xml.getChildNode("SnmpServerHostSetting");
			{
				if (!setting.isEmpty())
				{
					//ServerHost
					temp = setting.getChildNode("SnmpServerHost");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
						{
							g_strSnmpHost = strText;
						}
					}

					//Server端口
					temp = setting.getChildNode("SnmpServerPort");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
						{
							g_nSnmpPort = xmltoi(strText);
						}
					}
				}
				//LogNormal("SnmpServerHost=%s, SnmpServerPort=%d\n", g_strSnmpHost.c_str(), g_nSnmpPort);
			}
			//电科中心端1.8设置
			setting = xml.getChildNode("DkCenterServerSetting");
			{
				if (!setting.isEmpty())
				{
					//路口号由中心端分配
				/*	temp = setting.getChildNode("CrossingCode");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
						{
							g_dkCrossingCode = strText;
						}
					}*/
					////超速设备限速
					//temp = setting.getChildNode("LimitSpeed");
					//if(!temp.isEmpty())
					//{
					//	strText = temp.getText();
					//	if(strText)
					//	{
					//		g_dkLimitSpeed = xmltoi(strText);
					//	}
					//}
					//历史记录补传方式（1，隔小时补传，2，隔天补传）
					temp = setting.getChildNode("HisRecordTransWay");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
						{
							//g_dkTransWay = xmltoi(strText);
						}

					}
					//补传方式为隔小时补传时，设置每小时的某个分钟时刻开始补传
					temp = setting.getChildNode("TransMinuteMoment");
					if(!temp.isEmpty())
					{
						strText = temp.getText();
						if(strText)
						{
							//g_dkMinute = xmltoi(strText);
						}
					}
				}
			}
        }
		//区间测速，加载上一次的图片计数
		setting = xml.getChildNode("RegionSpeedRecPicIdSetting");
		{
			if (!setting.isEmpty())
			{
				temp = setting.getChildNode("RecPicId");
				if(!temp.isEmpty())
				{
					strText = temp.getText();
					if(strText)
					{
						g_strMvsRecvPicId = xmltoi(strText);
					}

				}
			}
		}
        return true;
    }
    return false;
}

//更新系统设置
bool CXmlParaUtil::UpdateSystemSetting(std::string strSetting,std::string strNode)
{
    std::string strSystemXml = "./system.xml";

    printf("strSetting.c_str()=%s,strNode.c_str()=%s\n",strSetting.c_str(),strNode.c_str());

    char buf[64];
	memset(buf, 0, sizeof(buf));

    XMLNode xml,setting,com,temp;
    XMLCSTR strText;

    if(access(strSystemXml.c_str(),F_OK) != 0)//不存在
    {
        xml = XMLNode::createXMLTopNode("SystemSetting");
    }
    else
    {
        xml = XMLNode::parseFile(strSystemXml.c_str()).getChildNode("SystemSetting");
        if(xml.isEmpty())
        {
            xml = XMLNode::createXMLTopNode("SystemSetting");
        }
    }

    if( strSetting == "DBSetting")//更新数据库设置
    {
        setting = xml.getChildNode("DBSetting");
        if(setting.isEmpty())
        {
            setting = xml.addChild("DBSetting");
        }
        if(strNode == "DiskDay")
        {
            temp = setting.getChildNode("DiskDay");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_uDiskDay);

            if(temp.isEmpty())
            {
                temp = setting.addChild("DiskDay");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
    }
    else if(strSetting == "OtherSetting")
    {
        setting = xml.getChildNode("OtherSetting");
        if(setting.isEmpty())
        {
            setting = xml.addChild("OtherSetting");
        }
        if(strNode == "SynClockHost")
        {
            temp = setting.getChildNode("SynClockHost");
            if(temp.isEmpty())
            {
                temp = setting.addChild("SynClockHost");
                temp.addText(g_strSynClockHost.c_str());
            }
            else
            {
                temp.updateText(g_strSynClockHost.c_str());
            }
        }
        else if(strNode == "AuthenticationHost")
        {
            temp = setting.getChildNode("AuthenticationHost");
            if(temp.isEmpty())
            {
                temp = setting.addChild("AuthenticationHost");
                temp.addText(g_strAuthenticationHost.c_str());
            }
            else
            {
                temp.updateText(g_strAuthenticationHost.c_str());
            }
        }
        else if(strNode == "ControlServerHost")
        {
            temp = setting.getChildNode("ControlServerHost");
            if(temp.isEmpty())
            {
                temp = setting.addChild("ControlServerHost");
                temp.addText(g_strControlServerHost.c_str());
            }
            else
            {
                temp.updateText(g_strControlServerHost.c_str());
            }
        }
        else if(strNode == "ControlServerPort")
        {
            temp = setting.getChildNode("ControlServerPort");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nControlServerPort);
            if(temp.isEmpty())
            {
                temp = setting.addChild("ControlServerPort");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "CenterServerHost")
        {
            temp = setting.getChildNode("CenterServerHost");
            if(temp.isEmpty())
            {
                temp = setting.addChild("CenterServerHost");
                temp.addText(g_strCenterServerHost.c_str());
            }
            else
            {
                temp.updateText(g_strCenterServerHost.c_str());
            }
        }
        else if(strNode == "CenterServerPort")
        {
            temp = setting.getChildNode("CenterServerPort");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nCenterServerPort);

            if(temp.isEmpty())
            {
                temp = setting.addChild("CenterServerPort");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "FtpServerHost")
        {
            temp = setting.getChildNode("FtpServerHost");
            if(temp.isEmpty())
            {
                temp = setting.addChild("FtpServerHost");
                temp.addText(g_strFtpServerHost.c_str());
            }
            else
            {
                temp.updateText(g_strFtpServerHost.c_str());
            }
        }
        else if(strNode == "FtpUserName")
        {
            temp = setting.getChildNode("FtpUserName");
            if(temp.isEmpty())
            {
                temp = setting.addChild("FtpUserName");
                temp.addText(g_strFtpUserName.c_str());
            }
            else
            {
                temp.updateText(g_strFtpUserName.c_str());
            }
        }
        else if(strNode == "FtpPassWord")
        {
            temp = setting.getChildNode("FtpPassWord");
            if(temp.isEmpty())
            {
                temp = setting.addChild("FtpPassWord");
                temp.addText(g_strFtpPassWord.c_str());
            }
            else
            {
                temp.updateText(g_strFtpPassWord.c_str());
            }
        }
        else if(strNode == "FtpPort")
        {
            //ftp端口
            temp = setting.getChildNode("FtpPort");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nFtpPort);

            if(temp.isEmpty())
            {
                temp = setting.addChild("FtpPort");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "FtpRemotePath")
        {
            //ftp存储路径
            temp = setting.getChildNode("FtpRemotePath");
            if(temp.isEmpty())
            {
                temp = setting.addChild("FtpRemotePath");
                temp.addText(g_ftpRemotePath);
            }
            else
            {
                temp.updateText(g_ftpRemotePath);
            }
        }
        else if(strNode == "DetectorID")
        {
            temp = setting.getChildNode("DetectorID");
            if(temp.isEmpty())
            {
                temp = setting.addChild("DetectorID");
                temp.addText(g_strDetectorID.c_str());
            }
            else
            {
                temp.updateText(g_strDetectorID.c_str());
            }
            g_CenterServer.mvSetDetectorId(g_strDetectorID.c_str()); //设置检测设备编号
        }
		else if(strNode == "FlashControl")
        {
			 //视频爆闪控制
            temp = setting.getChildNode("FlashControl");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nFlashControl);
            if(temp.isEmpty())
            {
                temp = setting.addChild("FlashControl");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "ChanWayType")
        {
            temp = setting.getChildNode("ChanWayType");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nRoadType);
            if(temp.isEmpty())
            {
                temp = setting.addChild("ChanWayType");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "CommunicationMode")//旅行时间通讯方式0:ftp;1:socket
        {
            temp = setting.getChildNode("CommunicationMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nCommunicationMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("CommunicationMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
        }
        else if(strNode == "")//所有
        {
            //是否有中心端
            temp = setting.getChildNode("HasCenterServer");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nHasCenterServer);
            if(temp.isEmpty())
            {
                temp = setting.addChild("HasCenterServer");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //中心端类型
            temp = setting.getChildNode("ServerType");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nServerType);
            if(temp.isEmpty())
            {
                temp = setting.addChild("ServerType");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否只发送违章数据
            temp = setting.getChildNode("SendViolationOnly");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSendViolationOnly);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SendViolationOnly");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //回写等待时间
            temp = setting.getChildNode("WriteBackTime");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nWriteBackTime);
            if(temp.isEmpty())
            {
                temp = setting.addChild("WriteBackTime");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否相机同步
            temp = setting.getChildNode("DoSynProcess");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nDoSynProcess);
            if(temp.isEmpty())
            {
                temp = setting.addChild("DoSynProcess");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否抖动检测
            temp = setting.getChildNode("CheckShake");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nCheckShake);
            if(temp.isEmpty())
            {
                temp = setting.addChild("CheckShake");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //存图数量
            temp = setting.getChildNode("SaveImageCount");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSaveImageCount);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SaveImageCount");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否布控报警
            temp = setting.getChildNode("DetectSpecialCarNum");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nDetectSpecialCarNum);
            if(temp.isEmpty())
            {
                temp = setting.addChild("DetectSpecialCarNum");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否记录相机日志
            temp = setting.getChildNode("DspLog");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nDspLog);
            if(temp.isEmpty())
            {
                temp = setting.addChild("DspLog");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否需要切换相机
            temp = setting.getChildNode("SwitchCamera");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSwitchCamera);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SwitchCamera");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //工作方式(是否全帧率输出图像)
            temp = setting.getChildNode("WorkMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nWorkMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("WorkMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //视频爆闪控制
            temp = setting.getChildNode("FlashControl");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nFlashControl);
            if(temp.isEmpty())
            {
                temp = setting.addChild("FlashControl");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //卡口图片组合方式
            temp = setting.getChildNode("PicMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nPicMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("PicMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //闯红灯图片组合方式
            temp = setting.getChildNode("VtsPicMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nVtsPicMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("VtsPicMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //历史视频播放方式
            temp = setting.getChildNode("HistoryPlayMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nHistoryPlayMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("HistoryPlayMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否有智能控制器
            temp = setting.getChildNode("HasExpoMonitor");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nHasExpoMonitor);
            if(temp.isEmpty())
            {
                temp = setting.addChild("HasExpoMonitor");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否有爆闪灯
            temp = setting.getChildNode("HasHighLight");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nHasHighLight);
            if(temp.isEmpty())
            {
                temp = setting.addChild("HasHighLight");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否需要发送历史记录
            temp = setting.getChildNode("SendHistoryRecord");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSendHistoryRecord);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SendHistoryRecord");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否有ntp-server
            temp = setting.getChildNode("NtpServer");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nNtpServer);
            if(temp.isEmpty())
            {
                temp = setting.addChild("NtpServer");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //校时方式
            temp = setting.getChildNode("ClockMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nClockMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("ClockMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

			//载入违章车牌数据
			temp = setting.getChildNode("LoadBasePlateInfo");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_nLoadBasePlateInfo);
			if(temp.isEmpty())
			{
				temp = setting.addChild("LoadBasePlateInfo");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			//本地车牌
			temp = setting.getChildNode("LocalPlate");
			if(temp.isEmpty())
			{
				temp = setting.addChild("LocalPlate");
				temp.addText(g_strLocalPlate.c_str());
			}
			else
			{
				temp.updateText(g_strLocalPlate.c_str());
			}

			//是否ftp服务端
			temp = setting.getChildNode("FtpServer");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_nFtpServer);
			if(temp.isEmpty())
			{
				temp = setting.addChild("FtpServer");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

            //系统设置模板编号
            temp = setting.getChildNode("ModelID");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSettingModelID);
            if(temp.isEmpty())
            {
                temp = setting.addChild("ModelID");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否发送图片
            temp = setting.getChildNode("SendImage");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSendImage);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SendImage");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
            //是否按时间段检测
            temp = setting.getChildNode("DetectByTime");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nDetectByTime);
            if(temp.isEmpty())
            {
                temp = setting.addChild("DetectByTime");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
            //开始检测时间
            temp = setting.getChildNode("BeginDetectTime");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nBeginDetectTime);
            if(temp.isEmpty())
            {
                temp = setting.addChild("BeginDetectTime");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }
            //结束检测时间
            temp = setting.getChildNode("EndDetectTime");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nEndDetectTime);
            if(temp.isEmpty())
            {
                temp = setting.addChild("EndDetectTime");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

			 //设置检测器工作模式
            temp = setting.getChildNode("DetectMode");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nDetectMode);
            if(temp.isEmpty())
            {
                temp = setting.addChild("DetectMode");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

			//是否公交模式
			temp = setting.getChildNode("GongJiaoMode");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_nGongJiaoMode);
			if(temp.isEmpty())
			{
				temp = setting.addChild("GongJiaoMode");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
        }
    }
    else if(strSetting == "ComSetting")
    {
        setting = xml.getChildNode("ComSetting");

            if(!setting.isEmpty())
            setting.deleteNodeContent();

            setting = xml.addChild("ComSetting");

            COM_PARAMETER_MAP::iterator it_b = g_mapComSetting.begin();
            COM_PARAMETER_MAP::iterator it_e = g_mapComSetting.end();

            while(it_b != it_e)
            {
                COM_PARAMETER com_para = it_b->second;
                com = setting.addChild("Com");

                temp = com.addChild("ComID");
                sprintf(buf,"%d",com_para.nComPort);
                temp.addText(buf);

                temp = com.addChild("ComUse");
                if(com_para.nComUse == 1)
                {
                    temp.addText("Camera");
                }
                else if(com_para.nComUse == 2)
                {
                    temp.addText("RedLight");
                }
                else if(com_para.nComUse == 3)
                {
                    temp.addText("DetectVehicle");
                }
                else if(com_para.nComUse == 4)
                {
                    temp.addText("Light");
                }
                else if(com_para.nComUse == 5)
                {
                    temp.addText("VIS");
                }
                else if(com_para.nComUse == 6)
                {
                    temp.addText("GPS");
                }
                else if(com_para.nComUse == 7)
                {
                    temp.addText("Radar");
                }
                else if(com_para.nComUse == 8)
                {
                    temp.addText("Flash");
                }
				else if(com_para.nComUse == 9)
                {
                    temp.addText("Door");
                }
				else if(com_para.nComUse == 10)  // dio
				{
					temp.addText("Dio");
				}
				else if(com_para.nComUse == 11)  //集和诚液晶
				{
					temp.addText("JHCtechScreen");
				}
				else if(com_para.nComUse == 12)  //威强液晶
				{
					temp.addText("WeiQiangScreen");
				}
				else if(com_para.nComUse == 13)  //慧昌雷达
				{
					temp.addText("RadarHuiChang");
				}
				/*
				else if(com_para.nComUse == 13)  // dio2
				{
					temp.addText("Dio2");
				}
				*/
				else if(com_para.nComUse == 14)
				{
					temp.addText("RedLightA");
				}
                temp = com.addChild("BaudRate");
                sprintf(buf,"%d",com_para.nBaud);
                temp.addText(buf);

                temp = com.addChild("DataBit");
                sprintf(buf,"%d",com_para.nDataBits);
                temp.addText(buf);

                temp = com.addChild("StopBit");
                sprintf(buf,"%d",com_para.nStopBits);
                temp.addText(buf);

                temp = com.addChild("Parity");
                sprintf(buf,"%d",com_para.nParity);
                temp.addText(buf);

                it_b++;
            }
    }
    else if(strSetting == "YunTaiSetting")
    {
        setting = xml.getChildNode("YunTaiSetting");
        if(setting.isEmpty())
        setting = xml.addChild("YunTaiSetting");

        //是否需要进行控制
        temp = setting.getChildNode("NeedControl");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_ytControlSetting.nNeedControl);
        if(temp.isEmpty())
        {
            temp = setting.addChild("NeedControl");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //控制方式
        temp = setting.getChildNode("ControlMode");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_nControlMode);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ControlMode");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//图片的格式  
		//LogNormal("-------1.\n");
		temp = setting.getChildNode("PicComPoseMode");
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", g_ytControlSetting.nPicComPoseMode);
		if (temp.isEmpty())
		{
			temp = setting.addChild("PicComPoseMode");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf); 
		}

        //键盘编号
        temp = setting.getChildNode("KeyBoardID");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_nKeyBoardID);
        if(temp.isEmpty())
        {
            temp = setting.addChild("KeyBoardID");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //vis服务地址
        temp = setting.getChildNode("VisHost");
        if(temp.isEmpty())
        {
            temp = setting.addChild("VisHost");
            temp.addText(g_strVisHost.c_str());
        }
        else
        {
            temp.updateText(g_strVisHost.c_str());
        }

        //vis服务端口
        temp = setting.getChildNode("VisPort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_nVisPort);
        if(temp.isEmpty())
        {
            temp = setting.addChild("VisPort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //协议类型
        temp = setting.getChildNode("Protocal");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_ytControlSetting.nProtocalType);
        if(temp.isEmpty())
        {
            temp = setting.addChild("Protocal");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //是否存在多个预置位
        temp = setting.getChildNode("MultiPreSet");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_nMultiPreSet);
        if(temp.isEmpty())
        {
            temp = setting.addChild("MultiPreSet");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //是否存在近景预置位
        temp = setting.getChildNode("HasLocalPreSet");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_ytControlSetting.nHasLocalPreSet);
        if(temp.isEmpty())
        {
            temp = setting.addChild("HasLocalPreSet");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//Serial服务地址
		temp = setting.getChildNode("SerialHost");
		if(temp.isEmpty())
		{
			temp = setting.addChild("SerialHost");
			temp.addText(g_ytControlSetting.szSerialHost);
		}
		else
		{
			temp.updateText(g_ytControlSetting.szSerialHost);
		}

		//Serial服务端口
		temp = setting.getChildNode("SerialPort");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nSerialPort);
		if(temp.isEmpty())
		{
			temp = setting.addChild("SerialPort");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//地址码
		temp = setting.getChildNode("AddressCode");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nAddressCode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("AddressCode");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}
		
		//违停抓拍时间
		temp = setting.getChildNode("StopInterval");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nStopInterval);
		if(temp.isEmpty())
		{
			temp = setting.addChild("StopInterval");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//远景图片时间间隔
		temp = setting.getChildNode("RemotePicInterval");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nRemotePicInterval);
		if(temp.isEmpty())
		{
			temp = setting.addChild("RemotePicInterval");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}
		
		//预置位模式设置
		temp = setting.getChildNode("PreSetMode");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_nPreSetMode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("PreSetMode");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//是否使用3D智能抓拍方式
		temp = setting.getChildNode("CameraAutoMode");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nCameraAutoMode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("CameraAutoMode");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//识别车牌是否入库
		temp = setting.getChildNode("CarnumToDBMode");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nCarnumToDBMode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("CarnumToDBMode");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}


		//手动抓拍时间间隔
		temp = setting.getChildNode("HandCatchTime");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_ytControlSetting.nHandCatchTime);
		if(temp.isEmpty())
		{
			temp = setting.addChild("HandCatchTime");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}


    }
    else if(strSetting == "MonitorHostSetting")
    {

        setting = xml.getChildNode("MonitorHostSetting");
        if(setting.isEmpty())
        setting = xml.addChild("MonitorHostSetting");

        //监控主机地址
        temp = setting.getChildNode("MonitorHost");
        string strMonitorHost(g_monitorHostInfo.chMonitorHost);
        if(temp.isEmpty())
        {
            temp = setting.addChild("MonitorHost");
            temp.addText(strMonitorHost.c_str());
        }
        else
        {
            temp.updateText(strMonitorHost.c_str());
        }

        //监控主机端口
        temp = setting.getChildNode("MonitorPort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_monitorHostInfo.uMonitorPort);
        if(temp.isEmpty())
        {
            temp = setting.addChild("MonitorPort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //监控主机用户名
        temp = setting.getChildNode("UserName");
        string strUserName(g_monitorHostInfo.chUserName);
        if(temp.isEmpty())
        {
            temp = setting.addChild("UserName");
            temp.addText(strUserName.c_str());
        }
        else
        {
            temp.updateText(strUserName.c_str());
        }

        //监控主机密码
        temp = setting.getChildNode("PassWord");
        string strPassWord(g_monitorHostInfo.chPassWord);
        printf("strPassWord=%s\n",strPassWord.c_str());
        if(temp.isEmpty())
        {
            temp = setting.addChild("PassWord");
            temp.addText(strPassWord.c_str());
        }
        else
        {
            temp.updateText(strPassWord.c_str());
        }

		//sip服务器编号
		temp = setting.getChildNode("SipServerCode");
		string strSipServerCode(g_monitorHostInfo.chSipServerCode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("SipServerCode");
			temp.addText(strSipServerCode.c_str());
		}
		else
		{
			temp.updateText(strSipServerCode.c_str());
		}

		//sip客户端编号
		temp = setting.getChildNode("SipClientCode");
		string strSipClientCode(g_monitorHostInfo.chSipClientCode);
		if(temp.isEmpty())
		{
			temp = setting.addChild("SipClientCode");
			temp.addText(strSipClientCode.c_str());
		}
		else
		{
			temp.updateText(strSipClientCode.c_str());
		}
    }
    else if(strSetting == "ExpoMonitorSetting")
    {

        setting = xml.getChildNode("ExpoMonitorSetting");
        if(setting.isEmpty())
        setting = xml.addChild("ExpoMonitorSetting");

        //监控主机地址
        temp = setting.getChildNode("ExpoMonitorHost");
        string strExpoMonitorHost(g_ExpoMonitorInfo.chExpoMonitorHost);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ExpoMonitorHost");
            temp.addText(strExpoMonitorHost.c_str());
        }
        else
        {
            temp.updateText(strExpoMonitorHost.c_str());
        }

        //监控主机端口
        temp = setting.getChildNode("ExpoMonitorPort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_ExpoMonitorInfo.uExpoMonitorPort);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ExpoMonitorPort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
    }
	else if(strSetting == "NtpTimeInfo")
	{
		setting = xml.getChildNode("NtpTimeInfo");
		if(setting.isEmpty())
		{
			setting = xml.addChild("NtpTimeInfo");
		}

		if (strNode.compare("ALL") == 0)
		{
			temp = setting.getChildNode("Time1");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Ntp_Time.nTime1);
			if(temp.isEmpty())
			{
				temp = setting.addChild("Time1");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("Time2");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Ntp_Time.nTime2);
			if(temp.isEmpty())
			{
				temp = setting.addChild("Time2");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("Time3");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Ntp_Time.nTime3);
			if(temp.isEmpty())
			{
				temp = setting.addChild("Time3");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("NtpServerIp");
			if(temp.isEmpty())
			{
				temp = setting.addChild("NtpServerIp");
				temp.addText(g_Ntp_Time.chHost);
			}
			else
			{
				temp.updateText(g_Ntp_Time.chHost);
			}
		}
	}
	else if(strSetting == "KafkaSet")
	{
		setting = xml.getChildNode("KafkaSet");
		if(setting.isEmpty())
		{
			setting = xml.addChild("KafkaSet");
		}
		if (strNode.compare("ALL") == 0)
		{
			temp = setting.getChildNode("SwitchUploading");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Kafka.uSwitchUploading);
			if(temp.isEmpty())
			{
				temp = setting.addChild("SwitchUploading");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("Item");
			if(temp.isEmpty())
			{
				temp = setting.addChild("Item");
				temp.addText(g_Kafka.chItem);
			}
			else
			{
				temp.updateText(g_Kafka.chItem);
			}

			temp = setting.getChildNode("Brand");
			if(temp.isEmpty())
			{
				temp = setting.addChild("Brand");
				temp.addText(g_Kafka.chBrand);
			}
			else
			{
				temp.updateText(g_Kafka.chBrand);
			}

			temp = setting.getChildNode("Version");
			if(temp.isEmpty())
			{
				temp = setting.addChild("Version");
				temp.addText(g_Kafka.chVersion);
			}
			else
			{
				temp.updateText(g_Kafka.chVersion);
			}

			temp = setting.getChildNode("Topic");
			if(temp.isEmpty())
			{
				temp = setting.addChild("Topic");
				temp.addText(g_Kafka.chTopic);
			}
			else
			{
				temp.updateText(g_Kafka.chTopic);
			}

			temp = setting.getChildNode("UpdateType");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Kafka.uUpdateType);
			if(temp.isEmpty())
			{
				temp = setting.addChild("UpdateType");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
			temp = setting.getChildNode("CheckModal");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Kafka.uCheckModal);
			if(temp.isEmpty())
			{
				temp = setting.addChild("CheckModal");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
		}
	}
	else if(strSetting == "CheckTime")
	{
		setting = xml.getChildNode("CheckTime");
		if(setting.isEmpty())
		{
			setting = xml.addChild("CheckTime");
		}

		if (strNode.compare("ALL") == 0)
		{
			temp = setting.getChildNode("MornFastigiumBegin");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nMornFastigiumBegin);
			if(temp.isEmpty())
			{
				temp = setting.addChild("MornFastigiumBegin");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("MornFastigiumEnd");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nMornFastigiumEnd);
			if(temp.isEmpty())
			{
				temp = setting.addChild("MornFastigiumEnd");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("NightFastigiumBegin");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nNightFastigiumBegin);
			if(temp.isEmpty())
			{
				temp = setting.addChild("NightFastigiumBegin");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("NightFastigiumEnd");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nNightFastigiumEnd);
			if(temp.isEmpty())
			{
				temp = setting.addChild("NightFastigiumEnd");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("OrdinaryMornBegin");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nOrdinaryMornBegin);
			if(temp.isEmpty())
			{
				temp = setting.addChild("OrdinaryMornBegin");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("OrdinaryMornEnd");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nOrdinaryMornEnd);
			if(temp.isEmpty())
			{
				temp = setting.addChild("OrdinaryMornEnd");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("OrdinaryNightBegin");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nOrdinaryNightBegin);
			if(temp.isEmpty())
			{
				temp = setting.addChild("OrdinaryNightBegin");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			temp = setting.getChildNode("OrdinaryNightEnd");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_CheckTime.nOrdinaryNightEnd);
			if(temp.isEmpty())
			{
				temp = setting.addChild("OrdinaryNightEnd");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
		}
	}
	else if(strSetting == "FtpPathInfo")
	{
		setting = xml.getChildNode("FtpPathInfo");
		if(setting.isEmpty())
		{
			setting = xml.addChild("FtpPathInfo");
		}

		if (strNode.compare("ALL") == 0)
		{
			//设置UseFlag
			temp = setting.getChildNode("UseFlag");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Ftp_Path.nUseFlag);
			if(temp.isEmpty())
			{
				temp = setting.addChild("UseFlag");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			//设置nServerType
			temp = setting.getChildNode("ServerType");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_Ftp_Path.nServerType);
			if(temp.isEmpty())
			{
				temp = setting.addChild("ServerType");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}

			//设置strVideoPath
			temp = setting.getChildNode("VideoPath");
			if(temp.isEmpty())
			{
				temp = setting.addChild("VideoPath");
				temp.addText(g_Ftp_Path.strVideoPath);
			}
			else
			{
				temp.updateText(g_Ftp_Path.strVideoPath);
			}

			//设置strWfPicPath
			temp = setting.getChildNode("WfPicPath");
			if(temp.isEmpty())
			{
				temp = setting.addChild("WfPicPath");
				temp.addText(g_Ftp_Path.strWfPicPath);
			}
			else
			{
				temp.updateText(g_Ftp_Path.strWfPicPath);
			}

			//设置strWfTxtPath
			temp = setting.getChildNode("WfTxtPath");
			if(temp.isEmpty())
			{
				temp = setting.addChild("WfTxtPath");
				temp.addText(g_Ftp_Path.strWfTxtPath);
			}
			else
			{
				temp.updateText(g_Ftp_Path.strWfTxtPath);
			}
			//设置strKkPicPath
			temp = setting.getChildNode("KkPicPath");
			if(temp.isEmpty())
			{
				temp = setting.addChild("KkPicPath");
				temp.addText(g_Ftp_Path.strKkPicPath);
			}
			else
			{
				temp.updateText(g_Ftp_Path.strKkPicPath);
			}
			//设置strFsTxtPath
			temp = setting.getChildNode("FsTxtPath");
			if(temp.isEmpty())
			{
				temp = setting.addChild("FsTxtPath");
				temp.addText(g_Ftp_Path.strFsTxtPath);
			}
			else
			{
				temp.updateText(g_Ftp_Path.strFsTxtPath);
			}
		}
	}
	else if(strSetting == "3GInfo")
	{
		setting = xml.getChildNode("3GInfo");
        if(setting.isEmpty())
        setting = xml.addChild("3GInfo");

		if (strNode.compare("3G") == 0)
		{
			//是否存在3G
			temp = setting.getChildNode("Exist3G");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_nExist3G);
			if(temp.isEmpty())
			{
				temp = setting.addChild("Exist3G");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
		}
		else if (strNode.compare("3GIP") == 0)
		{
			//设置3G IP
			temp = setting.getChildNode("3GIP");
			if(temp.isEmpty())
			{
				temp = setting.addChild("3GIP");
				temp.addText(g_str3GIp.c_str());
			}
			else
			{
				temp.updateText(g_str3GIp.c_str());
			}
		}
		else if (strNode.compare("3GTYPE") == 0)
		{
			//3G类型
			temp = setting.getChildNode("3GTYPE");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_n3GTYPE);
			if(temp.isEmpty())
			{
				temp = setting.addChild("3GTYPE");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
		}
	}
    else if(strSetting == "MatchHostSetting")
    {
        setting = xml.getChildNode("MatchHostSetting");
        if(setting.isEmpty())
        setting = xml.addChild("MatchHostSetting");

        //比对主机地址
        temp = setting.getChildNode("MatchHost");
        string strMatchHost(g_MatchHostInfo.chMatchHost);
        if(temp.isEmpty())
        {
            temp = setting.addChild("MatchHost");
            temp.addText(strMatchHost.c_str());
        }
        else
        {
            temp.updateText(strMatchHost.c_str());
        }

        //比对主机端口
        temp = setting.getChildNode("MatchPort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_MatchHostInfo.uMatchPort);
        if(temp.isEmpty())
        {
            temp = setting.addChild("MatchPort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //是否有比对服务器
        temp = setting.getChildNode("HasMatchHost");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_MatchHostInfo.uHasMatchHost);
        if(temp.isEmpty())
        {
            temp = setting.addChild("HasMatchHost");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
    }
	else if(strSetting == "PlateLimit")
    {
        setting = xml.getChildNode("PlateLimit");
        if(setting.isEmpty())
        setting = xml.addChild("PlateLimit");

        //是否尾号限行
        temp = setting.getChildNode("IsPlateLimit");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PlateLimit.nIsPlateLimit);
        if(temp.isEmpty())
        {
            temp = setting.addChild("IsPlateLimit");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//限行尾号
        temp = setting.getChildNode("PlateNumber");
        string strPlateNumber(g_PlateLimit.chPlateNumber);
        if(temp.isEmpty())
        {
            temp = setting.addChild("PlateNumber");
            temp.addText(strPlateNumber.c_str());
        }
        else
        {
            temp.updateText(strPlateNumber.c_str());
        }
    }
	else if(strSetting == "GpsSetting")//GPS设置
	{
		setting = xml.getChildNode("GpsSetting");
        if(setting.isEmpty())
        setting = xml.addChild("GpsSetting");

		//GPS类型
        temp = setting.getChildNode("GpsType");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_GpsSetInfo.nType);
        if(temp.isEmpty())
        {
            temp = setting.addChild("GpsType");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
	}
	else if(strSetting == "SignalSetting")//信号机设置
	{
		setting = xml.getChildNode("SignalSetting");
        if(setting.isEmpty())
        setting = xml.addChild("SignalSetting");

		//是否存在信号机
		 temp = setting.getChildNode("SignalExist");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_SignalSetInfo.nExist);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SignalExist");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//信号机类型
        temp = setting.getChildNode("SignalType");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_SignalSetInfo.nType);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SignalType");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//信号机模式
		 temp = setting.getChildNode("SignalMode");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_SignalSetInfo.nMode);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SignalMode");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
	}
	else if(strSetting == "DistanceHostSetting")
    {
        setting = xml.getChildNode("DistanceHostSetting");
        if(setting.isEmpty())
        setting = xml.addChild("DistanceHostSetting");

        //区间测速主机地址
        temp = setting.getChildNode("DistanceHost");
        string strMatchHost(g_DistanceHostInfo.chDistanceHost);
		g_MvsNextHostIp = strMatchHost;
        if(temp.isEmpty())
        {
            temp = setting.addChild("DistanceHost");
            temp.addText(strMatchHost.c_str());
        }
        else
        {
            temp.updateText(strMatchHost.c_str());
        }

        //区间测速主机端口
        temp = setting.getChildNode("DistancePort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_DistanceHostInfo.uDistancePort);
		g_MvsNextRecPort = g_DistanceHostInfo.uDistancePort;
        if(temp.isEmpty())
        {
            temp = setting.addChild("DistancePort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//区间测速主机距离
        temp = setting.getChildNode("Distance");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_DistanceHostInfo.uDistance);
		g_MvsDistance = g_DistanceHostInfo.uDistance;
        if(temp.isEmpty())
        {
            temp = setting.addChild("Distance");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//是否区间测速
		temp = setting.getChildNode("IsDistanceCalculate");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_DistanceHostInfo.bDistanceCalculate);
		g_bMvsDistanceCal = g_DistanceHostInfo.bDistanceCalculate;
		if (temp.isEmpty())
		{
			temp = setting.addChild("IsDistanceCalculate");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//u盘拷贝开始时间
		temp = setting.getChildNode("UsbCopyTime");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_DistanceHostInfo.uUsbCopyTime);
		if (temp.isEmpty())
		{
			temp = setting.addChild("UsbCopyTime");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//是否拷贝录像
		temp = setting.getChildNode("CopyVideo");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_DistanceHostInfo.bCopyVideo);
		if (temp.isEmpty())
		{
			temp = setting.addChild("CopyVideo");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}
    }
	else if(strSetting == "AmsHostSetting")
	{
		setting = xml.getChildNode("AmsHostSetting");
		if(setting.isEmpty())
			setting = xml.addChild("AmsHostSetting");

		//应用管理服务器地址
		temp = setting.getChildNode("AmsHost");
		string strAmsHost(g_AmsHostInfo.chAmsHost);
		if(temp.isEmpty())
		{
			temp = setting.addChild("AmsHost");
			temp.addText(strAmsHost.c_str());
		}
		else
		{
			temp.updateText(strAmsHost.c_str());
		}

		//应用管理服务器端口
		temp = setting.getChildNode("AmsPort");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_AmsHostInfo.uAmsPort);
		if(temp.isEmpty())
		{
			temp = setting.addChild("AmsPort");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//是否有应用管理服务器
		temp = setting.getChildNode("HasAmsHost");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_AmsHostInfo.uHasAmsHost);
		if(temp.isEmpty())
		{
			temp = setting.addChild("HasAmsHost");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//是否备份检测器
		temp = setting.getChildNode("BakType");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_AmsHostInfo.uBakType);
		if(temp.isEmpty())
		{
			temp = setting.addChild("BakType");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

#ifdef MVSBAK
		//更新AMS,DSP通道信息
		g_AMSCommunication.mvSendChannelListXml();
#endif
	}
    else if(strSetting == "LightTimeSetting")
    {
        setting = xml.getChildNode("LightTimeSetting");
        if(setting.isEmpty())
        setting = xml.addChild("LightTimeSetting");

        //是否定时开光灯
        temp = setting.getChildNode("LightTimeControl");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_LightTimeInfo.nLightTimeControl);
        if(temp.isEmpty())
        {
            temp = setting.addChild("LightTimeControl");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //春季开光灯时间
        temp = setting.getChildNode("SpringLightTime");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_LightTimeInfo.nSpringLightTime);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SpringLightTime");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //夏季开光灯时间
        temp = setting.getChildNode("SummerLightTime");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_LightTimeInfo.nSummerLightTime);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SummerLightTime");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //秋季开光灯时间
        temp = setting.getChildNode("AutumnLightTime");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_LightTimeInfo.nAutumnLightTime);
        if(temp.isEmpty())
        {
            temp = setting.addChild("AutumnLightTime");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //冬季开光灯时间
        temp = setting.getChildNode("WinterLightTime");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_LightTimeInfo.nWinterLightTime);
        if(temp.isEmpty())
        {
            temp = setting.addChild("WinterLightTime");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
    }
    else if(strSetting == "PicFormatInfo")
    {

        setting = xml.getChildNode("PicFormatInfo");
        if(setting.isEmpty())
        setting = xml.addChild("PicFormatInfo");

        //字图位置
        temp = setting.getChildNode("WordPos");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nWordPos);
        if(temp.isEmpty())
        {
            temp = setting.addChild("WordPos");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		temp = setting.getChildNode("PicFlag");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_PicFormatInfo.nPicFlag);
		if(temp.isEmpty())
		{
			temp = setting.addChild("PicFlag");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

        //前景颜色
        temp = setting.getChildNode("ForeColor");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nForeColor);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ForeColor");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //背景颜色
        temp = setting.getChildNode("BackColor");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nBackColor);
        if(temp.isEmpty())
        {
            temp = setting.addChild("BackColor");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //字体
        temp = setting.getChildNode("Font");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nFont);
        if(temp.isEmpty())
        {
            temp = setting.addChild("Font");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //车身颜色
        temp = setting.getChildNode("CarColor");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nCarColor);
        if(temp.isEmpty())
        {
            temp = setting.addChild("CarColor");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //车辆类型
        temp = setting.getChildNode("CarType");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nCarType);
        if(temp.isEmpty())
        {
            temp = setting.addChild("CarType");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //车标
        temp = setting.getChildNode("CarBrand");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nCarBrand);
        if(temp.isEmpty())
        {
            temp = setting.addChild("CarBrand");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //车速
        temp = setting.getChildNode("CarSpeed");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nCarSpeed);
        if(temp.isEmpty())
        {
            temp = setting.addChild("CarSpeed");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		 //限速值
        temp = setting.getChildNode("SpeedLimit");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nSpeedLimit);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SpeedLimit");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		 //是否存在空白区域
        temp = setting.getChildNode("SpaceRegion");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nSpaceRegion);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SpaceRegion");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //违章类型
        temp = setting.getChildNode("ViolationType");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nViolationType);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ViolationType");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }


        ////车牌号码
        temp = setting.getChildNode("CarNum");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nCarNum);
        if(temp.isEmpty())
        {
            temp = setting.addChild("CarNum");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

         //车道编号
        temp = setting.getChildNode("RoadIndex");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nRoadIndex);
        if(temp.isEmpty())
        {
            temp = setting.addChild("RoadIndex");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //字体大小
        temp = setting.getChildNode("FontSize");
        memset(buf, 0, sizeof(buf));
        if(g_PicFormatInfo.nFontSize <= 0)
        {
            g_PicFormatInfo.nFontSize = 25;
        }
        else if(g_PicFormatInfo.nFontSize >= 300)
        {
            g_PicFormatInfo.nFontSize = 300;
        }
        sprintf(buf,"%d",g_PicFormatInfo.nFontSize);
        if(temp.isEmpty())
        {
            temp = setting.addChild("FontSize");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //文字区域高度
        temp = setting.getChildNode("ExtentHeight");
        memset(buf, 0, sizeof(buf));
        if(g_PicFormatInfo.nExtentHeight < 0)
        {
            g_PicFormatInfo.nExtentHeight = 60;
        }
        else if(g_PicFormatInfo.nExtentHeight >= 300)
        {
            g_PicFormatInfo.nExtentHeight = 300;
        }
        sprintf(buf,"%d",g_PicFormatInfo.nExtentHeight);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ExtentHeight");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //文字加在图片上
        temp = setting.getChildNode("WordOnPic");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nWordOnPic);
        if(temp.isEmpty())
        {
            temp = setting.addChild("WordOnPic");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

        //存储小图
        temp = setting.getChildNode("SmallPic");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nSmallPic);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SmallPic");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		//jpg压缩比
		temp = setting.getChildNode("JpgQuality");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_PicFormatInfo.nJpgQuality);
		if(temp.isEmpty())
		{
			temp = setting.addChild("JpgQuality");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}


		//违章图片是否需要叠加小图
		temp = setting.getChildNode("SmallViolationPic");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_PicFormatInfo.nSmallViolationPic);
		if(temp.isEmpty())
		{
			temp = setting.addChild("SmallViolationPic");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

		//图片上叠加文字的偏移量
		temp = setting.getChildNode("WordOffSetX");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_PicFormatInfo.nOffsetX);
		if(temp.isEmpty())
		{
			temp = setting.addChild("WordOffSetX");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		} 

		temp = setting.getChildNode("WordOffSetY");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_PicFormatInfo.nOffsetY);
		if(temp.isEmpty())
		{
			temp = setting.addChild("WordOffSetY");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		} 

		 //第二车身颜色
        temp = setting.getChildNode("SecondCarColor");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nSecondCarColor);
        if(temp.isEmpty())
        {
            temp = setting.addChild("SecondCarColor");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		 //缩放比例
        temp = setting.getChildNode("ResizeScale");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nResizeScale);
        if(temp.isEmpty())
        {
            temp = setting.addChild("ResizeScale");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }

		 //文字行数
        temp = setting.getChildNode("WordLine");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_PicFormatInfo.nWordLine);
        if(temp.isEmpty())
        {
            temp = setting.addChild("WordLine");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
    }
    else if(strSetting == "VideoFormatInfo")
    {

        setting = xml.getChildNode("VideoFormatInfo");
        if(setting.isEmpty())
        setting = xml.addChild("VideoFormatInfo");

            //帧率
            temp = setting.getChildNode("FrameRate");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_VideoFormatInfo.nFrameRate);
            if(temp.isEmpty())
            {
                temp = setting.addChild("FrameRate");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //分辨率
            temp = setting.getChildNode("Resolution");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_VideoFormatInfo.nResolution);
            if(temp.isEmpty())
            {
                temp = setting.addChild("Resolution");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //编码方式
            temp = setting.getChildNode("EncodeFormat");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nEncodeFormat);
            if(temp.isEmpty())
            {
                temp = setting.addChild("EncodeFormat");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //录像时长
            temp = setting.getChildNode("TimeLength");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_VideoFormatInfo.nTimeLength);
            if(temp.isEmpty())
            {
                temp = setting.addChild("TimeLength");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //是否需要发送rtsp
            temp = setting.getChildNode("SendRtsp");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nSendRTSP);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SendRtsp");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

            //扩展avi头
            temp = setting.getChildNode("AviHeaderEx");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d",g_nAviHeaderEx);
            if(temp.isEmpty())
            {
                temp = setting.addChild("AviHeaderEx");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

			//Sip视频服务
            temp = setting.getChildNode("SipService");
            memset(buf, 0, sizeof(buf));
            sprintf(buf,"%d", g_VideoFormatInfo.nSip);
            if(temp.isEmpty())
            {
                temp = setting.addChild("SipService");
                temp.addText(buf);
            }
            else
            {
                temp.updateText(buf);
            }

			//是否需要发送h264
			temp = setting.getChildNode("SendH264");
			memset(buf, 0, sizeof(buf));
			sprintf(buf,"%d",g_VideoFormatInfo.nSendH264);
			if(temp.isEmpty())
			{
				temp = setting.addChild("SendH264");
				temp.addText(buf);
			}
			else
			{
				temp.updateText(buf);
			}
    }
	else if(strSetting == "DspServerHostSetting")
    {
        setting = xml.getChildNode("DspServerHostSetting");
        if(setting.isEmpty())
        setting = xml.addChild("DspServerHostSetting");
        //地址
        /*temp = setting.getChildNode("DspServerHost");
        string strDspServerHost(g_DspServerHostInfo.chDspServerHost);
        if(temp.isEmpty())
        {
            temp = setting.addChild("DspServerHost");
            temp.addText(strDspServerHost.c_str());
        }
        else
        {
            temp.updateText(strDspServerHost.c_str());
        }*/

        //端口
        temp = setting.getChildNode("DspServerPort");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d",g_DspServerHostInfo.uDspServerPort);
        if(temp.isEmpty())
        {
            temp = setting.addChild("DspServerPort");
            temp.addText(buf);
        }
        else
        {
            temp.updateText(buf);
        }
    }
	else if(strSetting == "SnmpServerHostSetting")
	{
		setting = xml.getChildNode("SnmpServerHostSetting");
		if(setting.isEmpty())
			setting = xml.addChild("SnmpServerHostSetting");

		//Ip
		temp = setting.getChildNode("SnmpServerHost");
		if(temp.isEmpty())
		{
			temp = setting.addChild("SnmpServerHost");
			temp.addText(g_strSnmpHost.c_str());
		}
		else
		{
			temp.updateText(g_strSnmpHost.c_str());
		}

		//端口
		temp = setting.getChildNode("SnmpServerPort");
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"%d",g_nSnmpPort);
		if(temp.isEmpty())
		{
			temp = setting.addChild("SnmpServerPort");
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}
	}
	else if(strSetting == "DkCenterServerSetting")//电科中心端限速设置
	{
		setting = xml.getChildNode("DkCenterServerSetting");
		if(setting.isEmpty())
			setting = xml.addChild("DkCenterServerSetting");

	/*	///路口号设置
		temp = setting.getChildNode("CrossingCode");
		if(temp.isEmpty())
		{
			temp = setting.addChild("CrossingCode");	
			temp.addText(g_dkCrossingCode.c_str());
		}
		else
		{
			temp.updateText(g_dkCrossingCode.c_str());
		}*/
	/*	///限速
		temp = setting.getChildNode("LimitSpeed");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_dkLimitSpeed);
		if(temp.isEmpty())
		{
			temp = setting.addChild("LimitSpeed");	
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}*/

		//补传方式
		temp = setting.getChildNode("HisRecordTransWay");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_dkTransWay);
		if(temp.isEmpty())
		{
			temp = setting.addChild("HisRecordTransWay");	
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
			
		}

		//补传分钟时刻
		temp = setting.getChildNode("TransMinuteMoment");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_dkMinute);
		if(temp.isEmpty())
		{
			temp = setting.addChild("TransMinuteMoment");	
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}
		
	}

	//区间测速，对接收到的图片计数，并及时更新计数值
	else if (strSetting == "RegionSpeedRecPicIdSetting")
	{
		setting = xml.getChildNode("RegionSpeedRecPicIdSetting");
		if(setting.isEmpty())
			setting = xml.addChild("RegionSpeedRecPicIdSetting");

		temp = setting.getChildNode("RecPicId");
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",g_strMvsRecvPicId);
		if(temp.isEmpty())
		{
			temp = setting.addChild("RecPicId");	
			temp.addText(buf);
		}
		else
		{
			temp.updateText(buf);
		}

	}
    return xml.writeToFile(strSystemXml.c_str());
}
//生成设备车道坐标参数
bool CXmlParaUtil::AddDeviceSettingInfo(LIST_CHANNEL_INFO& list_channel_info, int nDeviceID,int nPreSet)
{
    //创建文件夹
    char buf[256]={0};
    sprintf(buf,"./config/Camera%d",nDeviceID);
    std::string strPath(buf);

    if(access(strPath.c_str(),0) != 0)
    mkdir(strPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);

    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

    if(g_nMultiPreSet == 0)
    sprintf(buf,"%s/RoadSettingInfo.xml",strPath.c_str());
    else
    sprintf(buf,"%s/RoadSettingInfo-PreSet%d.xml",strPath.c_str(),nPreSet);
    std::string strRoadSettingInfoXml(buf);
    XmlNode = XMLNode::createXMLTopNode("RoadSettingInfo");
    ChannelsNode  = XmlNode.addChild("Channels");
    std::list<CHANNEL_INFO>::iterator it_channel_b = list_channel_info.begin();
    std::list<CHANNEL_INFO>::iterator it_channel_e = list_channel_info.end();
    while (it_channel_b != it_channel_e)
    {
        CHANNEL_INFO channel_info;
        channel_info = *it_channel_b;

        AddRoadSettingInfo(ChannelsNode,channel_info, nDeviceID);

        it_channel_b++;
    }//End of while it_channel_b != it_channel_e)

    bool bRet = XmlNode.writeToFile(strRoadSettingInfoXml.c_str());

    if(bRet)
    {
        if(g_nServerType == 1)
        g_AMSCommunication.SendSettingsToCS(nDeviceID,nPreSet,0);
    }

    return bRet;
}

//生成车道参数模板1
bool CXmlParaUtil::AddRoadSettingInfoByList(LIST_CHANNEL_INFO& list_channel_info, int nChannel)
{
    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

    std::string strRoadSettingInfoXml = "./config/RoadSettingInfo.xml";

    //////////////////////////////////////////////////////--新建xml文件--end
    #ifdef _DEBUG
        printf("AddRoadSettingInfo========nChannel=%d======\n\r", nChannel);
    #endif

    //判断RoadSettingInfo.xml是否存在
    if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    {
        #ifdef _DEBUG
            printf("=======Create the RoadSettingInfo xml file!\n\r");
        #endif

        XmlNode = XMLNode::createXMLTopNode("RoadSettingInfo");
        ChannelsNode  = XmlNode.addChild("Channels");
    } //End of if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    else
    {
        XmlNode = XMLNode::parseFile(strRoadSettingInfoXml.c_str()).getChildNode("RoadSettingInfo");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(ChannelsNode.isEmpty())
        {
            XmlNode = XMLNode::createXMLTopNode("RoadSettingInfo");
            ChannelsNode  = XmlNode.addChild("Channels");
        }
        else
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }
    //////////////////////////////////////////////////////--新建xml文件--end
    //printf("===========list_channel_info.size()=%d\n",list_channel_info.size());

    std::list<CHANNEL_INFO>::iterator it_channel_b = list_channel_info.begin();
    std::list<CHANNEL_INFO>::iterator it_channel_e = list_channel_info.end();
    while (it_channel_b != it_channel_e)
    {
        CHANNEL_INFO channel_info;
        channel_info = *it_channel_b;

        AddRoadSettingInfo(ChannelsNode,channel_info, nChannel);

        printf("===========carnumRegion.size()=%d\n",channel_info.carnumRegion.listRegionProp.size());

        it_channel_b++;
    }//End of while it_channel_b != it_channel_e)
    return XmlNode.writeToFile(strRoadSettingInfoXml.c_str());
}

//生成车道参数模板2
bool CXmlParaUtil::AddRoadSettingInfo(XMLNode& ChannelsNode,CHANNEL_INFO& channel_info, int nChannel)
{
    char buf[64];
    memset(buf, 0, sizeof(buf));
    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode, RoadWaysNode, ChannelWaysNode, PointsNode, GlobalNode;
    XMLNode TempNode, ChannelNode, RoadWayNode, ChannelWayNode, PointNode;
    XMLNode TempNode1, TempNode2;
    XMLCSTR strTag;

    int iTemp = 0;

    int nPoints;

    ////////////////////////////////插入车道进xml文件--begin
    {
        //通道个数
        int nChannels = ChannelsNode.nChildNode();
        #ifdef _DEBUG
            printf("=========nChannels=%d=\n\r", nChannels);
        #endif
        /////////////////////////////////////找到当前节点--begin
        int i = 0;
        bool bFindChannel = false;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        /////////////////////////////////////找到当前节点--end
        if(!bFindChannel)
        {
            ChannelNode = ChannelsNode.addChild("Channel");
        }

        {
            strTag = "ChannelId";
            CheckXMLNode(ChannelNode, TempNode, 0, strTag);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%d", nChannel);
            TempNode.updateText(buf);

        #ifdef _DEBUG
            printf("ChannelNode===============\n\r");
        #endif
            //if(xmltoi(strText) > 0)
            {
                //GlobalNode = ChannelNode.getChildNode("Global");
                strTag = "Global";
                CheckXMLNode(ChannelNode, GlobalNode, 0, strTag);

                //TempNode = GlobalNode.getChildNode("Calibration");
                strTag = "Calibration";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);

                //TempNode1 = TempNode.getChildNode("Length");
                strTag = "Length";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%f",channel_info.calibration.length);
                TempNode1.updateText(buf);

                //TempNode1 = TempNode.getChildNode("Width");
                strTag = "Width";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%f",channel_info.calibration.width);
                TempNode1.updateText(buf);

                //TempNode1 = TempNode.getChildNode("CameraHeight");
                strTag = "CameraHeight";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%f",channel_info.calibration.cameraHeight);
                TempNode1.updateText(buf);

                //TempNode1 = TempNode.getChildNode("CalibrationArea");
                strTag = "CalibrationArea";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                //标定区域点
                AddListPointsToNode(TempNode1, channel_info.calibration.region.listPT);

                //标定辅助点
                //TempNode1 = TempNode.getChildNode("CalibrationPoint");
                strTag = "CalibrationPoint";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddListPointsToNode(TempNode1, channel_info.calibration.listPT);

                //标定区域世界坐标
                //TempNode1 = TempNode.getChildNode("CalibrationWorld");
                strTag = "CalibrationWorld";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddListPoints32ToNode(TempNode1, channel_info.calibration.list32fPT);

                //车牌检测区域
                //TempNode = GlobalNode.getChildNode("CardArea");
                strTag = "CardArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.carnumRegion);

                //屏蔽区域
                //TempNode = GlobalNode.getChildNode("SkipArea");
                strTag = "SkipArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.eliminateRegion);

                //稳像背景区
                //TempNode = GlobalNode.getChildNode("StabBackArea");
                //TempNode1 = TempNode.getChildNode("StabBackElemArea");
                strTag = "StabBackArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                //strTag = "StabBackElemArea";
                //CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.StabBackRegion);

                //车牌区域
                //TempNode = GlobalNode.getChildNode("CardnumberArea");
                strTag = "CardnumberArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.CardnumberRegion);

                //虚拟线圈区域
                //TempNode = GlobalNode.getChildNode("CardnumberArea");
                strTag = "VirtualLoop";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.VirtualLoopRegion);

                //远处行人框
                strTag = "RemotePerson";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.RemotePersonRegion);

                //近处行人框
                strTag = "LocalPerson";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.LocalPersonRegion);

                //同步区域
                //TempNode = GlobalNode.getChildNode("SynchArea");
                strTag = "SynchArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                //TempNode1 = TempNode.getChildNode("SynchLeftArea"); //左同步区域
                strTag = "SynchLeftArea";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.SynchLeftRegion);
                //TempNode1 = TempNode.getChildNode("SynchRightArea"); //右同步区域
                strTag = "SynchRightArea";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.SynchRightRegion);

                //违章检测区域
                //TempNode = GlobalNode.getChildNode("Violation");
                strTag = "Violation";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);

                //////////////////////违章检测区域--begin
                //TempNode1 = TempNode.getChildNode("ViolationArea"); //违章检测区域
                strTag = "ViolationArea";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.ViolationRegion);
                //TempNode1 = TempNode.getChildNode("StopLine"); //停止线
                strTag = "StopLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.StopLine);
                //TempNode1 = TempNode.getChildNode("StraightLine"); //直行线
                strTag = "StraightLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.StraightLine);
                //TempNode1 = TempNode.getChildNode("TurnLeftLine"); //左转线
                strTag = "TurnLeftLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.TurnLeftLine);
                //TempNode1 = TempNode.getChildNode("TurnRightLine"); //右转线
                strTag = "TurnRightLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.TurnRightLine);

				 strTag = "ViolationFirstLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.ViolationFirstLine);

				 strTag = "ViolationSecondLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.ViolationSecondLine);

				 strTag = "RightFirstLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.RightFirstLine);

				 strTag = "LeftFirstLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.LeftFirstLine);

				 strTag = "ForeFirstLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.ForeFirstLine);

                strTag = "YellowLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.YellowLine);
				strTag = "WhiteLine";
				CheckXMLNode(TempNode, TempNode1, 0, strTag);
				AddRegionPointsToNode(TempNode1, channel_info.WhiteLine);
				strTag = "LeadStreamLine";
                CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode1, channel_info.LeadStreamLine);
                //////////////////////违章检测区域--end

                //事件检测区域
                //TempNode = GlobalNode.getChildNode("EventArea");
                strTag = "EventArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.EventRegion);

                //红灯检测区域--可多个
                //TempNode = GlobalNode.getChildNode("TrafficSignalArea");
                strTag = "TrafficSignalArea";
                CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                //TempNode1 = TempNode.getChildNode("TrafficSignalElemArea");
                //strTag = "TrafficSignalElemArea";
                //CheckXMLNode(TempNode, TempNode1, 0, strTag);
                AddRegionPointsToNode(TempNode, channel_info.TrafficSignalRegion);

        #ifdef _DEBUG
            printf("RoadWaysNode===============\n\r");
        #endif
            //道路
                //RoadWaysNode = ChannelNode.getChildNode("RoadWays");
                strTag = "RoadWays";
                CheckXMLNode(ChannelNode, RoadWaysNode, 0, strTag);

                //添加道路
                int nRoadWaysCount = RoadWaysNode.nChildNode();

            #ifdef _DEBUG
                printf("RoadWaysNode======nRoadWaysCount=%d========\n\r", nRoadWaysCount);
            #endif


                //////////判断车道是否在已经存在的道路内，如果不存在则创建新的道路节点，并将该车道
                //////////添加到其中，如果存在则直接把车道插入到该道路内
                bool bExistRoad = false;
                int j=0;
                for(j=0; j<nRoadWaysCount; j++)
                {
                    RoadWayNode = RoadWaysNode.getChildNode(j);
                    GlobalNode = RoadWayNode.getChildNode("Global");
                    TempNode = GlobalNode.getChildNode("RoadWayArea");

                    Point32fList ptList;
                    GetListPointsFromNode(ptList, TempNode);

                    //现有道路区域中心
                    CPoint32f ptCenter;
                    GetCenterPoint(ptList,ptCenter);

                    //当前道路区域中心
                    CPoint32f ptCenterCur;
                    GetCenterPoint(channel_info.roadRegion.listPT,ptCenterCur);

                    //if(memcmp(&ptCenter,&ptCenterCur,sizeof(CPoint32f))==0)
                    double distance = sqrt((ptCenter.x-ptCenterCur.x)*(ptCenter.x-ptCenterCur.x)+(ptCenter.y-ptCenterCur.y)*(ptCenter.y-ptCenterCur.y));

                    printf("==AddRoadSettingInfo=distance=%f\n",distance);
                    if( distance <= 5)
                    {
                        bExistRoad = true;
                        break;
                    }
                }
                //////////
                {
            #ifdef _DEBUG
                printf("RoadWayNode====== j= %d =========\n\r", j);
            #endif
                    //RoadWayNode = RoadWaysNode.getChildNode(j); //道路节点
                    strTag = "RoadWay";
                    if(!bExistRoad)
                    CheckXMLNode(RoadWaysNode, RoadWayNode, j, strTag);

                    //道路全局
                    //GlobalNode = RoadWayNode.getChildNode("Global");
                    strTag = "Global";
                    CheckXMLNode(RoadWayNode, GlobalNode, 0, strTag);

                    //道路
                    //TempNode = GlobalNode.getChildNode("RoadWayArea");
                    strTag = "RoadWayArea";
                    CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                    AddListPointsToNode(TempNode, channel_info.roadRegion.listPT);

                    //方向
                    //TempNode = GlobalNode.getChildNode("Direction");
                    strTag = "Direction";
                    CheckXMLNode(GlobalNode, TempNode, 0, strTag);

                    //TempNode1 = TempNode.getChildNode("Degree"); //道路方向角度
                    strTag = "Degree";
                    CheckXMLNode(TempNode, TempNode1, 0, strTag);
                    memset(buf, 0, sizeof(buf));
                    sprintf(buf, "%d", channel_info.chProp_direction.value.nValue);
                    TempNode1.updateText(buf);

                    //TempNode1 = TempNode.getChildNode("Points");
                    strTag = "Points";
                    CheckXMLNode(TempNode, TempNode1, 0, strTag);
                    printf("-=============before=======CheckXMLNode direction\n");
                    AddPointToNode(TempNode1, 0, channel_info.chProp_direction.ptBegin); //方向起点
                    AddPointToNode(TempNode1, 1, channel_info.chProp_direction.ptEnd); //方向终点
                    printf("-=============after=======CheckXMLNode direction\n");

                    //流量背景区域
                    //TempNode = GlobalNode.getChildNode("FlowBackArea");
                    strTag = "FlowBackArea";
                    CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                    AddRegionPointsToNode(TempNode, channel_info.FlowBackRegion);

                    //流量线
                    //TempNode = GlobalNode.getChildNode("FlowLine");
                    strTag = "FlowLine";
                    CheckXMLNode(GlobalNode, TempNode, 0, strTag);
                    AddRegionPointsToNode(TempNode, channel_info.FlowLine);

                    //车道
                    //ChannelWaysNode = RoadWayNode.getChildNode("ChannelWays");
                    strTag = "ChannelWays";
                    CheckXMLNode(RoadWayNode, ChannelWaysNode, 0, strTag);
                    ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");
                    int nChannelWays = ChannelWaysNode.nChildNode();

                    {
                        //ChannelWayNode = ChannelWaysNode.getChildNode("ChannelWay");
                        strTag = "ChannelWay";
                        CheckXMLNode(ChannelWaysNode, ChannelWayNode,nChannelWays-1,strTag);

                        //TempNode = ChannelWayNode.getChildNode("ChannelWayId");//车道序号
                        strTag = "ChannelWayId";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "%d", channel_info.chProp_index.value.nValue);
                        TempNode.updateText(buf);

                        //TempNode = ChannelWayNode.getChildNode("ChannelWayName"); //车道名称--长度是否足够？
                        strTag = "ChannelWayName";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        memset(buf, 0, sizeof(buf));
                        memcpy(buf, channel_info.chProp_name.value.strValue, 32);
                        TempNode.updateText(buf);


                        //事件车道参数序号
                       /* strTag = "EventParamId";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "%d", channel_info.chRegion.chProperty.value.nValue);
                        TempNode.updateText(buf);*/


                        //车道区域
                        //TempNode = ChannelWayNode.getChildNode("ChannelWayArea");
                        strTag = "ChannelWayArea";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddListPointsToNode(TempNode, channel_info.chRegion.listPT);

                        //停车区域
                        //TempNode = ChannelWayNode.getChildNode("ParkArea");
                        strTag = "ParkArea";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("ParkElemArea");--
                        AddRegionPointsToNode(TempNode, channel_info.stopRegion);

                        //丢弃物区域
                        //TempNode = ChannelWayNode.getChildNode("TrashArea");
                        strTag = "TrashArea";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("TrashElemArea");--
                        printf("AddRegionPointsToNode(TempNode, channel_info.dropRegion\n");
                        AddRegionPointsToNode(TempNode, channel_info.dropRegion);

                        //行人区域
                        //TempNode = ChannelWayNode.getChildNode("PersonArea");
                        strTag = "PersonArea";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("PersonElemArea");--
                        AddRegionPointsToNode(TempNode, channel_info.personRegion);

                        //闯入区域
                        strTag = "BargeInRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("ParkElemArea");--
                        AddRegionPointsToNode(TempNode, channel_info.BargeInRegion);

                        //越界区域
                        strTag = "BeyondMarkRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("ParkElemArea");--
                        AddRegionPointsToNode(TempNode, channel_info.BeyondMarkRegion);


                        //雷达区域
                        strTag = "RadarRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        //TempNode1 = TempNode.getChildNode("ParkElemArea");--
                        AddRegionPointsToNode(TempNode, channel_info.RadarRegion);

                        //流量检测区域
                        //TempNode = ChannelWayNode.getChildNode("AmountLine");
                        strTag = "AmountLine";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.AmountLine);

                        //参考线
                        //TempNode = ChannelWayNode.getChildNode("RefLine");
                        strTag = "RefLine";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.RefLine);

                        //变道线
                        strTag = "TurnRoadLine";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.TurnRoadLine);

                        //流量取景框区域
                        //TempNode = ChannelWayNode.getChildNode("FlowFramegetArea");
                        strTag = "FlowFramegetArea";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.FlowFramegetRegion);

                        //测速线圈
                        strTag = "LoopLine";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LoopRegion);

                        //单车道停止线
                        strTag = "LineStop";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LineStop);

                        //红灯区域
                        strTag = "RedLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.RedLightRegion);

                        //绿灯区域
                        strTag = "GreenLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.GreenLightRegion);

                        //左转灯区域
                        strTag = "LeftLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LeftLightRegion);

                        //右转灯区域
                        strTag = "RightLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.RightLightRegion);

                        //直行灯区域
                        strTag = "StraightLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.StraightLightRegion);

                        //禁止转向灯区域
                        strTag = "TurnAroundLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.TurnAroundLightRegion);

                        //左转红灯区域
                        strTag = "LeftRedLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LeftRedLightRegion);

                        //左转绿灯区域
                        strTag = "LeftGreenLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LeftGreenLightRegion);

                        //右转红灯区域
                        strTag = "RightRedLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.RightRedLightRegion);

                        //右转绿灯区域
                        strTag = "RightGreenLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.RightGreenLightRegion);

                        //直行红灯区域
                        strTag = "StraightRedLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.StraightRedLightRegion);

                        //直行绿灯区域
                        strTag = "StraightGreenLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.StraightGreenLightRegion);

                        //禁止转向红灯区域
                        strTag = "TurnAroundRedLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.TurnAroundRedLightRegion);

                        //禁止转向绿灯区域
                        strTag = "TurnAroundGreenLightRegion";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.TurnAroundGreenLightRegion);

                        //单车道直行线
                        strTag = "LineStraight";
                        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
                        AddRegionPointsToNode(TempNode, channel_info.LineStraight);

						//密度区域
						strTag = "DensityRegion";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.DensityRegion);

						//抓图区域
						strTag = "GetPhotoRegion";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.GetPhotoRegion);

						//待转区第一前行线
						strTag = "HoldForeFirstLine";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.HoldForeLineFirst);

						//待转区第二前行线
						strTag = "HoldForeSecondLine";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.HoldForeLineSecond);

						//待转区第一停止线
						strTag = "HoldStopFirstLine";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.HoldStopLineFirst);

						//待转区第二停止线
						strTag = "HoldStopSecondLine";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.HoldStopLineSecond);

						//黄网格区域						
						strTag = "NoParkingArea";
						CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
						AddRegionPointsToNode(TempNode, channel_info.YelGridRgn);						
                    } //End of for k
                } //End of for j
            } //End of if(xmltoi(strText) > 0)
        }//End of for i
    }//End of if(access(strRoadSettingInfoXml.c_str(),F_OK) == 0)//存在

    return true;
}

//获取区域中心点
void CXmlParaUtil::GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter)
{
    if(ptList.size()>0)
    {
        Point32fList::iterator it_b = ptList.begin();
        Point32fList::iterator it_e = ptList.end();

        ptCenter.x = 0;
        ptCenter.y = 0;

        while(it_b != it_e)
        {
            CPoint32f point;

            point = *it_b;
            ptCenter.x += point.x;
            ptCenter.y += point.y;

            it_b++;
        }

        ptCenter.x = ptCenter.x/ptList.size();
        ptCenter.y = ptCenter.y/ptList.size();
    }
}

//载入车道参数模板
bool CXmlParaUtil::LoadRoadSettingInfo(LIST_CHANNEL_INFO& list_channel_info, int nChannel,bool bModel)
{
    #ifdef _DEBUG
        printf("LoadRoadSettingInfo========nChannel=%d======\n\r", nChannel);
    #endif
    if(nChannel < 0)
    {
        printf("=========LoadRoadSettingInfo wrong nChannel number ERROR!!!\n\r");
        return false;
    }
    std::string strRoadSettingInfoXml;

    if(bModel)//模板获取
    {
        strRoadSettingInfoXml = "./profile/RoadSettingModelInfo.xml";
    }
    else
    {
        if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
        strRoadSettingInfoXml = "./config/RoadSettingInfo.xml";
        else
        {
            //如果需要切换相机
            int nCameraID = g_skpDB.GetCameraID(nChannel);
            char buf[256]={0};
            if(g_nMultiPreSet == 0)
            sprintf(buf,"./config/Camera%d/RoadSettingInfo.xml",nCameraID);
            else//如果存在多个预置位
            {
                int nPreSet = g_skpDB.GetPreSet(nChannel);
                sprintf(buf,"./config/Camera%d/RoadSettingInfo-PreSet%d.xml",nCameraID,nPreSet);
            }
            std::string strPath(buf);

            if(access(strPath.c_str(),0) == 0)
            {
                nChannel = nCameraID;
                strRoadSettingInfoXml = strPath;
            }
            else
            {
                return false;
            }
        }
    }


    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode, ChannelsNode, RoadWaysNode, ChannelWaysNode, PointsNode, GlobalNode;
	XMLNode TempNode, ChannelNode, RoadWayNode, ChannelWayNode, PointNode;
	XMLNode TempNode1;
	XMLCSTR strText;

    int iTemp = 0;
    int iCurChannel; //记录当前通道编号
    int nPoints;

	printf("strRoadSettingInfoXml.c_str()=%s\n",strRoadSettingInfoXml.c_str());

    if (access(strRoadSettingInfoXml.c_str(), F_OK) == 0) //存在
    {
		printf("ok strRoadSettingInfoXml.c_str()=%s\n",strRoadSettingInfoXml.c_str());
        //xml文件
        XmlNode = XMLNode::parseFile(strRoadSettingInfoXml.c_str()).getChildNode("RoadSettingInfo");
        //通道
        ChannelsNode = XmlNode.getChildNode("Channels");

        //通道个数
        int nChannels = ChannelsNode.nChildNode();

        CHANNEL_INFO channel_info;
        /////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }//End of while
        if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
        {
            return false;
        }
        /////////////////////////////////////

        #ifdef _DEBUG
            printf("ChannelNode===============\n\r");
        #endif

        GlobalNode = ChannelNode.getChildNode("Global");
        TempNode = GlobalNode.getChildNode("Calibration");

        TempNode1 = TempNode.getChildNode("Length");
        if(!TempNode1.isEmpty())
        {
            strText = TempNode1.getText();
            channel_info.calibration.length = xmltof(strText);
        }


        TempNode1 = TempNode.getChildNode("Width");
        if(!TempNode1.isEmpty())
        {
            strText = TempNode1.getText();
            channel_info.calibration.width = xmltof(strText);
        }

        TempNode1 = TempNode.getChildNode("CameraHeight");
        if(!TempNode1.isEmpty())
        {
            strText = TempNode1.getText();
            channel_info.calibration.cameraHeight = xmltof(strText);
        }

        TempNode1 = TempNode.getChildNode("CalibrationArea");
        GetListPointsFromNode(channel_info.calibration.region.listPT, TempNode1);
        TempNode1 = TempNode.getChildNode("CalibrationPoint");
        GetListPointsFromNode(channel_info.calibration.listPT, TempNode1);
        TempNode1 = TempNode.getChildNode("CalibrationWorld");
        GetListPointsFromNode(channel_info.calibration.list32fPT, TempNode1);

        TempNode = GlobalNode.getChildNode("CardArea");
        GetRegionPointsFromNode(channel_info.carnumRegion, TempNode);

        TempNode = GlobalNode.getChildNode("VirtualLoop");
        GetRegionPointsFromNode(channel_info.VirtualLoopRegion, TempNode);

        TempNode = GlobalNode.getChildNode("RemotePerson");
        GetRegionPointsFromNode(channel_info.RemotePersonRegion, TempNode);

        TempNode = GlobalNode.getChildNode("LocalPerson");
        GetRegionPointsFromNode(channel_info.LocalPersonRegion, TempNode);


        TempNode = GlobalNode.getChildNode("SkipArea");
        GetRegionPointsFromNode(channel_info.eliminateRegion, TempNode);

        TempNode = GlobalNode.getChildNode("StabBackArea");
        GetRegionPointsFromNode(channel_info.StabBackRegion, TempNode);

        TempNode = GlobalNode.getChildNode("CardnumberArea");
        GetRegionPointsFromNode(channel_info.CardnumberRegion, TempNode);

        TempNode = GlobalNode.getChildNode("SynchArea");
        TempNode1 = TempNode.getChildNode("SynchLeftArea");
        GetRegionPointsFromNode(channel_info.SynchLeftRegion, TempNode1);
        //printf("channel_info.SynchLeftRegion.size=%d\n",channel_info.SynchLeftRegion.listRegionProp.size());

        TempNode1 = TempNode.getChildNode("SynchRightArea");
        GetRegionPointsFromNode(channel_info.SynchRightRegion, TempNode1);
        //printf("channel_info.SynchRightRegion.size=%d\n",channel_info.SynchRightRegion.listRegionProp.size());

        TempNode = GlobalNode.getChildNode("Violation");
        TempNode1 = TempNode.getChildNode("ViolationArea");
        GetRegionPointsFromNode(channel_info.ViolationRegion, TempNode1);
        //printf("channel_info.ViolationRegion.size=%d\n",channel_info.ViolationRegion.listRegionProp.size());

        TempNode1 = TempNode.getChildNode("StopLine");
        GetRegionPointsFromNode(channel_info.StopLine, TempNode1);

        TempNode1 = TempNode.getChildNode("StraightLine");
        GetRegionPointsFromNode(channel_info.StraightLine, TempNode1);

        TempNode1 = TempNode.getChildNode("TurnLeftLine");
        GetRegionPointsFromNode(channel_info.TurnLeftLine, TempNode1);

        TempNode1 = TempNode.getChildNode("TurnRightLine");
        GetRegionPointsFromNode(channel_info.TurnRightLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("ViolationFirstLine");
        GetRegionPointsFromNode(channel_info.ViolationFirstLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("ViolationSecondLine");
        GetRegionPointsFromNode(channel_info.ViolationSecondLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("RightFirstLine");
        GetRegionPointsFromNode(channel_info.RightFirstLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("LeftFirstLine");
        GetRegionPointsFromNode(channel_info.LeftFirstLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("ForeFirstLine");
        GetRegionPointsFromNode(channel_info.ForeFirstLine, TempNode1);

        TempNode1 = TempNode.getChildNode("YellowLine");
        GetRegionPointsFromNode(channel_info.YellowLine, TempNode1);

		TempNode1 = TempNode.getChildNode("WhiteLine");
		GetRegionPointsFromNode(channel_info.WhiteLine, TempNode1);

		 TempNode1 = TempNode.getChildNode("LeadStreamLine");
        GetRegionPointsFromNode(channel_info.LeadStreamLine, TempNode1);

        TempNode = GlobalNode.getChildNode("EventArea");
        GetRegionPointsFromNode(channel_info.EventRegion, TempNode);

        TempNode = GlobalNode.getChildNode("TrafficSignalArea");
        GetRegionPointsFromNode(channel_info.TrafficSignalRegion, TempNode);

        RoadWaysNode = ChannelNode.getChildNode("RoadWays");
        int nRoadWaysCount = RoadWaysNode.nChildNode();
        for(int j=0; j<nRoadWaysCount; j++)
        {
            RoadWayNode = RoadWaysNode.getChildNode(j);
            GlobalNode = RoadWayNode.getChildNode("Global");
            channel_info.roadRegion.chProperty.value.nValue = j;//道路节点序号

            TempNode = GlobalNode.getChildNode("RoadWayArea");
            channel_info.roadRegion.listPT.clear();
            GetListPointsFromNode(channel_info.roadRegion.listPT, TempNode);

            TempNode = GlobalNode.getChildNode("Direction");
            TempNode1 = TempNode.getChildNode("Degree");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                channel_info.chProp_direction.value.nValue = xmltoi(strText);
            }
            TempNode1 = TempNode.getChildNode("Points");
            GetPointFromNode(channel_info.chProp_direction.ptBegin, 0, TempNode1);//方向起点
            GetPointFromNode(channel_info.chProp_direction.ptEnd, 1, TempNode1);//方向终点
            channel_info.chProp_direction.point.x = (channel_info.chProp_direction.ptBegin.x+channel_info.chProp_direction.ptEnd.x)*0.5;
            channel_info.chProp_direction.point.y = (channel_info.chProp_direction.ptBegin.y+channel_info.chProp_direction.ptEnd.y)*0.5;

            TempNode = GlobalNode.getChildNode("FlowBackArea");
            channel_info.FlowBackRegion.listRegionProp.clear();
            GetRegionPointsFromNode(channel_info.FlowBackRegion, TempNode);

            TempNode = GlobalNode.getChildNode("FlowLine");
            channel_info.FlowLine.listRegionProp.clear();
            GetRegionPointsFromNode(channel_info.FlowLine, TempNode);

            ChannelWaysNode = RoadWayNode.getChildNode("ChannelWays");
            int nChannelWaysCount = ChannelWaysNode.nChildNode();
            for (int k=0; k<nChannelWaysCount; k++)
            {
                ChannelWayNode = ChannelWaysNode.getChildNode(k); //车道节点
                TempNode = ChannelWayNode.getChildNode("ChannelWayId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    if(strText)
                    channel_info.chProp_index.value.nValue = xmltoi(strText);
                }

                TempNode = ChannelWayNode.getChildNode("ChannelWayName");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    if(strText)
                    {
                        channel_info.chProp_name.value.nValue = xmltoi(strText);
                        string strRoadName(strText);
                        strcpy(channel_info.chProp_name.value.strValue , strRoadName.c_str());
                    }
                }

               /* TempNode = ChannelWayNode.getChildNode("EventParamId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    channel_info.chRegion.chProperty.value.nValue = xmltoi(strText);//存储EventParamId信息
                }*/

                TempNode = ChannelWayNode.getChildNode("ChannelWayArea");
                channel_info.chRegion.listPT.clear();
                GetListPointsFromNode(channel_info.chRegion.listPT, TempNode);

                TempNode = ChannelWayNode.getChildNode("ParkArea");
                channel_info.stopRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.stopRegion, TempNode);
                //printf("channel_info.stopRegion.size=%d\n",channel_info.stopRegion.listRegionProp.size());

                TempNode = ChannelWayNode.getChildNode("TrashArea");
                channel_info.dropRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.dropRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("PersonArea");
                channel_info.personRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.personRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("BargeInRegion");
                channel_info.BargeInRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.BargeInRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("BeyondMarkRegion");
                channel_info.BeyondMarkRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.BeyondMarkRegion, TempNode);

				TempNode = ChannelWayNode.getChildNode("DensityRegion");
                channel_info.DensityRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.DensityRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("RadarRegion");
                channel_info.RadarRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RadarRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("AmountLine");
                channel_info.AmountLine.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.AmountLine, TempNode);

                TempNode = ChannelWayNode.getChildNode("RefLine");
                channel_info.RefLine.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RefLine, TempNode);

                TempNode = ChannelWayNode.getChildNode("TurnRoadLine");
                channel_info.TurnRoadLine.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.TurnRoadLine, TempNode);

                TempNode = ChannelWayNode.getChildNode("FlowFramegetArea");
                channel_info.FlowFramegetRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.FlowFramegetRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LoopLine");
                channel_info.LoopRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LoopRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LineStop");
                channel_info.LineStop.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LineStop, TempNode);

                TempNode = ChannelWayNode.getChildNode("RedLightRegion");
                channel_info.RedLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RedLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("GreenLightRegion");
                channel_info.GreenLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.GreenLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LeftLightRegion");
                channel_info.LeftLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LeftLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("RightLightRegion");
                channel_info.RightLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RightLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("StraightLightRegion");
                channel_info.StraightLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.StraightLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("TurnAroundLightRegion");
                channel_info.TurnAroundLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.TurnAroundLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LeftRedLightRegion");
                channel_info.LeftRedLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LeftRedLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LeftGreenLightRegion");
                channel_info.LeftGreenLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LeftGreenLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("RightRedLightRegion");
                channel_info.RightRedLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RightRedLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("RightGreenLightRegion");
                channel_info.RightGreenLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.RightGreenLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("StraightRedLightRegion");
                channel_info.StraightRedLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.StraightRedLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("StraightGreenLightRegion");
                channel_info.StraightGreenLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.StraightGreenLightRegion, TempNode);

                 TempNode = ChannelWayNode.getChildNode("TurnAroundRedLightRegion");
                channel_info.TurnAroundRedLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.TurnAroundRedLightRegion, TempNode);

                 TempNode = ChannelWayNode.getChildNode("TurnAroundGreenLightRegion");
                channel_info.TurnAroundGreenLightRegion.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.TurnAroundGreenLightRegion, TempNode);

                TempNode = ChannelWayNode.getChildNode("LineStraight");
                channel_info.LineStraight.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.LineStraight, TempNode);

				//抓图区域
				TempNode = ChannelWayNode.getChildNode("GetPhotoRegion");
				channel_info.GetPhotoRegion.listRegionProp.clear();
				GetRegionPointsFromNode(channel_info.GetPhotoRegion, TempNode);

				TempNode = ChannelWayNode.getChildNode("HoldForeFirstLine");
                channel_info.HoldForeLineFirst.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.HoldForeLineFirst, TempNode);

				TempNode = ChannelWayNode.getChildNode("HoldForeSecondLine");
                channel_info.HoldForeLineSecond.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.HoldForeLineSecond, TempNode);

				TempNode = ChannelWayNode.getChildNode("HoldStopFirstLine");
                channel_info.HoldStopLineFirst.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.HoldStopLineFirst, TempNode);

				TempNode = ChannelWayNode.getChildNode("HoldStopSecondLine");
                channel_info.HoldStopLineSecond.listRegionProp.clear();
                GetRegionPointsFromNode(channel_info.HoldStopLineSecond, TempNode);

				TempNode = ChannelWayNode.getChildNode("NoParkingArea");
				channel_info.YelGridRgn.listRegionProp.clear();
				GetRegionPointsFromNode(channel_info.YelGridRgn, TempNode);

                list_channel_info.push_back(channel_info); //添加到d车道链表里面
            }//End of for k
        }//End of for j

        return true;
    }
    else
    {
		printf("error strRoadSettingInfoXml.c_str()=%s\n",strRoadSettingInfoXml.c_str());
        return false;
    }
}

//给指定节点添加链表内的点
void CXmlParaUtil::AddListPointsToNode(XMLNode &pNode, Point32fList &list )
{
    printf("==================Befor AddListPointsToNode=========\n");

    char buf[64];
	memset(buf, 0, sizeof(buf));
    XMLNode TempNode1, TempNode2, TempNode3;
    XMLCSTR strTag;

    std::list<CPoint32f>::iterator it_32fb;
    std::list<CPoint32f>::iterator it_32fe;

    it_32fb = list.begin();
    it_32fe = list.end();

    int nPointsCount;
    //TempNode1 = pNode.getChildNode("Points");
    strTag = "Points";
    CheckXMLNode(pNode, TempNode1, 0, strTag);

    nPointsCount = 0;
    while (it_32fb!=it_32fe)
    {
        //TempNode2 = TempNode1.getChildNode("Point");
        strTag = "Point";
        CheckXMLNode(TempNode1, TempNode2, nPointsCount, strTag);

        strTag = "x";
        CheckXMLNode(TempNode2, TempNode3, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%f", it_32fb->x);
        TempNode3.updateText(buf);

        strTag = "y";
        CheckXMLNode(TempNode2, TempNode3, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%f", it_32fb->y);
        TempNode3.updateText(buf);

        it_32fb++;
        nPointsCount ++;
    }

    printf("==================After AddListPointsToNode=========\n");
}

//给指定节点添加链表内的点，3维坐标
void CXmlParaUtil::AddListPoints32ToNode(XMLNode &pNode, Point32fList &list )
{
    printf("==================Befor AddListPointsToNode=========\n");

    char buf[64];
	memset(buf, 0, sizeof(buf));
    XMLNode TempNode1, TempNode2, TempNode3;
    XMLCSTR strTag;

    std::list<CPoint32f>::iterator it_32fb;
    std::list<CPoint32f>::iterator it_32fe;

    it_32fb = list.begin();
    it_32fe = list.end();

    int nPointsCount;
    //TempNode1 = pNode.getChildNode("Points");
    strTag = "Points";
    CheckXMLNode(pNode, TempNode1, 0, strTag);

    nPointsCount = 0;
    while (it_32fb!=it_32fe)
    {
        //TempNode2 = TempNode1.getChildNode("Point");
        strTag = "Point";
        CheckXMLNode(TempNode1, TempNode2, nPointsCount, strTag);

        strTag = "x";
        CheckXMLNode(TempNode2, TempNode3, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%f", it_32fb->x);
        TempNode3.updateText(buf);

        strTag = "y";
        CheckXMLNode(TempNode2, TempNode3, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%f", it_32fb->y);
        TempNode3.updateText(buf);

        strTag = "z";
        CheckXMLNode(TempNode2, TempNode3, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "0.0");
        TempNode3.updateText(buf);

        it_32fb++;
        nPointsCount ++;
    }

    printf("==================After AddListPointsToNode=========\n");
}

//给指定节点添加区域内的点
void CXmlParaUtil::AddRegionPointsToNode(XMLNode &pNode, COMMON_REGION  & common_region)
{
    printf("==================Befor AddRegionPointsToNode=========\n");

    printf("==============Region %s====common_region.size=%d===\n", common_region.chProperty.strName,common_region.listRegionProp.size());

    if(common_region.listRegionProp.size() < 1)
    {
        return;
    }

    char buf[64];
	memset(buf, 0, sizeof(buf));
    XMLNode TempNode1, TempNode2, TempNode3;
    XMLCSTR strTag;
    int iAreasCount, iPointsCount;

    std::list<REGION_PROPERTY>::iterator it_region_b = common_region.listRegionProp.begin();
    std::list<REGION_PROPERTY>::iterator it_region_e = common_region.listRegionProp.end();

    std::list<CPoint32f>::iterator it_32fb;
    std::list<CPoint32f>::iterator it_32fe;

    iAreasCount = 0;
    while(it_region_b != it_region_e)
    {
        it_32fb = it_region_b->listPt.begin();
        it_32fe = it_region_b->listPt.end();

        strTag = "Points";
        CheckXMLNode(pNode, TempNode1, iAreasCount, strTag);

        iPointsCount = 0;
        while (it_32fb != it_32fe)
        {
            strTag = "Point";
            CheckXMLNode(TempNode1, TempNode2, iPointsCount, strTag);
            //TempNode2 = TempNode1.getChildNode("Point");

            strTag = "x";
            CheckXMLNode(TempNode2, TempNode3, 0, strTag);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%f", it_32fb->x);
            TempNode3.updateText(buf);

            strTag = "y";
            CheckXMLNode(TempNode2, TempNode3, 0, strTag);
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%f", it_32fb->y);
            TempNode3.updateText(buf);

            it_32fb++;
            iPointsCount ++;
        }//End of while (it_32fb!=it_32fe)

		//add by Gaoxiang
		string name(common_region.chProperty.strName);
		if (name == "BEYONDMARK_REGION" || name == "BARGEIN_REGION" || name == "AMOUNT_LINE" || name == "TURNROAD_LINE")
		{
			strTag = "DirectionPoints";
			CheckXMLNode(pNode, TempNode1, iAreasCount, strTag);

			std::list<CPoint32f>::iterator it_directionBegin;
			std::list<CPoint32f>::iterator it_directionEnd;
			it_directionBegin = it_region_b->directionListPt.begin();
			it_directionEnd = it_region_b->directionListPt.end();

			for(int i = 0; it_directionBegin!=it_directionEnd; it_directionBegin++, i++)
			{
				strTag = "Point";
				CheckXMLNode(TempNode1, TempNode2, i, strTag);
				//TempNode2 = TempNode1.getChildNode("Point");
				strTag = "x";
				CheckXMLNode(TempNode2, TempNode3, 0, strTag);
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%f", it_directionBegin->x);
				TempNode3.updateText(buf);
				strTag = "y";
				CheckXMLNode(TempNode2, TempNode3, 0, strTag);
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%f", it_directionBegin->y);
				TempNode3.updateText(buf);
			}
		}

		it_region_b++;
		iAreasCount ++;

    }//End of while(it_region_b != it_region_e)

    printf("========common_region.listRegionProp.size()=%d=====iAreasCount=%d==========\n\r", common_region.listRegionProp.size(), iAreasCount);

    printf("==================After AddRegionPointsToNode=========\n");

}

//给指定节点添加nPoints个点
void CXmlParaUtil::AddPointsToNode(XMLNode &pNode, int nPoints)
{
    printf("==================Befor AddPointsToNode=========\n");

	XMLNode PointsNode, PointNode;
	XMLNode TempNode;
	PointsNode = pNode.addChild("Points");
	char buf[64];

	for(int m=0; m<nPoints; m++)
	{
		PointNode = PointsNode.addChild("Point");

	//x
		TempNode = PointNode.addChild("x");
		//int nX = GetXCoordinate(); //取得x坐标
		int nX = 0;
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", nX);
		TempNode.addText(buf);

	//y
		TempNode = PointNode.addChild("y");
		//int nY = GetYCoordinate(); //取得Y坐标
		int nY = 0;
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", nY);
		TempNode.addText(buf);

	}//End of m

	printf("==================After AddPointsToNode=========\n");
}

//给指定节点添加1个点
//nIndex:点的序号--base 0
void CXmlParaUtil::AddPointToNode(XMLNode &pPointsNode, int nIndex, const CPoint32f &point)
{
    printf("channel_info.chProp_direction point.x=%f,point.y=%f\n",point.x,point.y);
    XMLNode PointNode, TempNode;

    char buf[64];
    XMLCSTR strTag;

    //PointNode = pPointsNode.getChildNode(nIndex);
    strTag = "Point";
    CheckXMLNode(pPointsNode, PointNode, nIndex, strTag);

    //TempNode = PointNode.getChildNode("x");
    strTag = "x";
    CheckXMLNode(PointNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%f", point.x);
    TempNode.updateText(buf);

    //TempNode = PointNode.getChildNode("y");
    strTag = "y";
    CheckXMLNode(PointNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%f", point.y);
    TempNode.updateText(buf);

}

//从指定节点获得点坐标--nIndex:点的序号--base 0
void CXmlParaUtil::GetPointFromNode(CPoint32f &point, int nIndex, XMLNode &pPointsNode)
{
    XMLNode PointNode, TempNode;
    XMLCSTR strText;
    PointNode = pPointsNode.getChildNode(nIndex);

    TempNode = PointNode.getChildNode("x");
    if(!TempNode.isEmpty())
    {
        strText = TempNode.getText();
        point.x = xmltof(strText);
    }

    TempNode = PointNode.getChildNode("y");
    if(!TempNode.isEmpty())
    {
        strText = TempNode.getText();
        point.y = xmltof(strText);
    }
}

////生成违章检测参数list
bool CXmlParaUtil::AddTrafficParameterByList(VTSParaMap &mapVTSPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara)
{
    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

	std::string strVTSParameterXml = "./config/VTSParameterInfo.xml";
	if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strVTSParameterXml = "./config/VTSParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/VTSParameterInfo.xml",nCameraID);
		else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/VTSParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }
		strVTSParameterXml = buf;

		nChannel = nCameraID;
	 }

    //////////////////////////////////////////////////////--新建xml文件--end
    #ifdef _DEBUG
        printf("AddTrafficParameterByList========nChannel=%d======\n\r", nChannel);
    #endif

    //判断RoadSettingInfo.xml是否存在
    if(access(strVTSParameterXml.c_str(),F_OK) != 0) //不存在
    {
        #ifdef _DEBUG
            printf("=======Create the RoadSettingInfo xml file!\n\r");
        #endif

        XmlNode = XMLNode::createXMLTopNode("VTSParameter");
        ChannelsNode = XmlNode.addChild("Channels");

    } //End of if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    else
    {
        XmlNode = XMLNode::parseFile(strVTSParameterXml.c_str()).getChildNode("VTSParameter");
        ChannelsNode = XmlNode.getChildNode("Channels");
        //如果存在则删除该通道下的所有节点
        DeleteChannelNode(ChannelsNode,nChannel);
    }
    //////////////////////////////////////////////////////--新建xml文件--end


    VTSParaMap::iterator it_channel_b = mapVTSPara.begin();
    VTSParaMap::iterator it_channel_e = mapVTSPara.end();
    while (it_channel_b != it_channel_e)
    {
        PARAMETER_VTS vtsPara;
        vtsPara = it_channel_b->second;

        AddTrafficParameter(ChannelsNode,vtsPara, nChannel,vtsGlobalPara);

        it_channel_b++;
    }//End of while it_channel_b != it_channel_e)

    return XmlNode.writeToFile(strVTSParameterXml.c_str());

}

//生成违章检测参数
bool CXmlParaUtil::AddTrafficParameter(XMLNode& ChannelsNode,PARAMETER_VTS &vtsPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara)
{
#ifdef _DEBUG
    printf("====Before===AddTrafficParameter================\n\r");
#endif

	char buf[64];
    int iTemp = 0;
	//通道，车道，参数
	XMLNode XmlNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strTag;

	bool bExist = false;
    //std::string strTrafficParameterXml = "./config/VTSParameter.xml";
	//else //存在
	//if(access(strTrafficParameterXml.c_str(), F_OK) == 0) //存在
	{
		printf("===============insert para to xml file============\n");

        //xml文件
		//XmlNode = XMLNode::parseFile(strTrafficParameterXml.c_str()).getChildNode("VTSParameter");
        //通道
		//ChannelsNode = XmlNode.getChildNode("Channels");

		/////////////////////////////////////找到当前节点
		//通道个数
        int nChannels = ChannelsNode.nChildNode();
        #ifdef _DEBUG
            printf("=========nChannels=%d=\n\r", nChannels);
        #endif
        /////////////////////////////////////找到当前节点--begin
        int i = 0;
        bool bFindChannel = false;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        /////////////////////////////////////找到当前节点--end
        printf("before ChannelsNode.addChild bFindChannel=%d\n",bFindChannel);
        if(!bFindChannel)
        {
            bool bEmpt = ChannelsNode.isEmpty();
            printf("ChannelsNode = %d\n",bEmpt);

            printf("==========before ChannelsNode.addChild\n");
            ChannelNode = ChannelsNode.addChild("Channel");
            printf("end ChannelsNode.addChild\n");
        }

        strTag = "ChannelId";
        CheckXMLNode(ChannelNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", nChannel);
        TempNode.updateText(buf);

        strTag = "Global";
        CheckXMLNode(ChannelNode, GlobalNode, 0, strTag);

        strTag = "StrongLight";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsGlobalPara.bStrongLight);
        TempNode.updateText(buf);

        strTag = "GlemdLight";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsGlobalPara.bGlemdLight);
        TempNode.updateText(buf);

        strTag = "CheckLightByImage";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsGlobalPara.bCheckLightByImage);
        TempNode.updateText(buf);

		strTag = "RedLightTime";
		CheckXMLNode(GlobalNode, TempNode, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsGlobalPara.nRedLightTime);
		TempNode.updateText(buf);

		strTag = "StrongEnhance";
		CheckXMLNode(GlobalNode, TempNode, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsGlobalPara.nStrongEnhance);
		TempNode.updateText(buf);

		strTag = "StopTime";
		CheckXMLNode(GlobalNode, TempNode, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsGlobalPara.nStopTime);
		TempNode.updateText(buf);

		strTag = "RealLimitSpeed";
		CheckXMLNode(GlobalNode, TempNode, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsGlobalPara.nSpeedVal);
		TempNode.updateText(buf);
		
		strTag = "StrongSignal";
		CheckXMLNode(GlobalNode, TempNode, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsGlobalPara.bStrongSignal);
		TempNode.updateText(buf);

        printf("bStrongLight=%d,bGlemdLight=%d,bCheckLightByImage=%d\n",vtsGlobalPara.bStrongLight,vtsGlobalPara.bGlemdLight,vtsGlobalPara.bCheckLightByImage);
        /////////////////////////////////////
        strTag = "ChannelWays";
        CheckXMLNode(ChannelNode, ChannelWaysNode, 0, strTag);
        ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");
        int nChannelWays = ChannelWaysNode.nChildNode();

        strTag = "ChannelWay";
        CheckXMLNode(ChannelWaysNode, ChannelWayNode, nChannelWays-1, strTag);

        strTag = "ChannelWayId";
        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nRoadIndex);
        TempNode.updateText(buf);

        strTag = "TrafficSignal";
        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
        //TempNode1 = TempNode.getChildNode("LeftControl");
        strTag = "LeftControl";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nLeftControl);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("StraightControl");
        strTag = "StraightControl";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nStraightControl);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("RightControl");
        strTag = "RightControl";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nRightControl);
        TempNode1.updateText(buf);

        //TempNode = ChannelWayNode.getChildNode("Parameter");
        //strTag = "Parameter";
        //CheckXMLNode(ChannelWayNode, TempNode1, 0, strTag);
        //TempNode1 = TempNode.getChildNode("bForbidLeft");
        strTag = "IsForbitLeft";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bForbidLeft);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsForbitRight");
        strTag = "IsForbitRight";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bForbidRight);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsForbitRun");
        strTag = "IsForbitRun";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bForbidRun);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsForbitStop");
        strTag = "IsForbitStop";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bForbidStop);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsAgainst");
        strTag = "IsAgainst";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bAgainst);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsChangeRoad");
        strTag = "IsChangeRoad";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bChangeRoad);
        TempNode1.updateText(buf);

        //TempNode1 = TempNode.getChildNode("IsPressLine");
        strTag = "IsPressLine";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.bPressLine);
        TempNode1.updateText(buf);

        strTag = "ForbidVehicleType";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nForbidType);
        TempNode1.updateText(buf);

        strTag = "ForbidBeginTime";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nForbidBeginTime);
        TempNode1.updateText(buf);

        strTag = "ForbidEndTime";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nForbidEndTime);
        TempNode1.updateText(buf);

        strTag = "RoadType";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nRoadType);
        TempNode1.updateText(buf);

        strTag = "RoadDirection";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nRoadDirection);
        TempNode1.updateText(buf);

		strTag = "FlagHoldStopReg";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nFlagHoldStopReg);
        TempNode1.updateText(buf);

		strTag = "RedLightTime";
        CheckXMLNode(TempNode, TempNode1, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", vtsPara.nRedLightTime);
        TempNode1.updateText(buf);		

		strTag = "RetrogradeItem";
		CheckXMLNode(TempNode, TempNode1, 0, strTag);
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", vtsPara.nRetrogradeItem);
		TempNode1.updateText(buf);

	} //End of else

    //XmlNode.writeToFile(strTrafficParameterXml.c_str());

#ifdef _DEBUG
    printf("====After===AddTrafficParameter================\n\r");
#endif
	return true;
}

/*****************************************************
* 函数简介：载入闯红灯参数--从xml文件到参数结构
* 输入参数:
*			vtsPara:参数结构
*			nChannel:通道号
*			nChannelWay: 车道号
* 返回值： bool:是否载入参数成功
*****************************************************/
bool CXmlParaUtil::LoadVTSParameter(VTSParaMap &mapVTSPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara)
{
    mapVTSPara.clear();
#ifdef _DEBUG
    printf("====Before===LoadTrafficParameter================\n\r");
#endif
    std::string strChannelWays;
    GetChannelWaysFromRoadSettingInfo(nChannel,strChannelWays);
    int nSize = strChannelWays.size()/sizeof(int);
    for(int i =0;i<nSize;i++)
    {
        PARAMETER_VTS vtsPara;
        vtsPara.nRoadIndex = *((int*)(strChannelWays.c_str()+sizeof(int)*i));
        mapVTSPara.insert(VTSParaMap::value_type(vtsPara.nRoadIndex,vtsPara));
        printf("=======nSize==%d=======vtsPara.nRoadIndex==%d=====\n\r",nSize,vtsPara.nRoadIndex);
    }

	//通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

	std::string strTrafficParameterXml = "./config/VTSParameterInfo.xml";
	//如果需要切换相机
    if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strTrafficParameterXml = "./config/VTSParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/VTSParameterInfo.xml",nCameraID);
        else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/VTSParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        {
            nChannel = nCameraID;
            strTrafficParameterXml = strPath;
        }
        else
        {
                return false;
        }
    }
	if(access(strTrafficParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strTrafficParameterXml.c_str()).getChildNode("VTSParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
               strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        //全局参数
        GlobalNode = ChannelNode.getChildNode("Global");
        TempNode = GlobalNode.getChildNode("StrongLight");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            vtsGlobalPara.bStrongLight = xmltoi(strText);
        }
        TempNode = GlobalNode.getChildNode("GlemdLight");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            vtsGlobalPara.bGlemdLight = xmltoi(strText);
        }
        TempNode = GlobalNode.getChildNode("CheckLightByImage");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            vtsGlobalPara.bCheckLightByImage = xmltoi(strText);
        }
		TempNode = GlobalNode.getChildNode("RedLightTime");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			vtsGlobalPara.nRedLightTime = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("StrongEnhance");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			vtsGlobalPara.nStrongEnhance = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("StopTime");//黄网格区域停车时间
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			vtsGlobalPara.nStopTime = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("RealLimitSpeed");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			vtsGlobalPara.nSpeedVal = xmltoi(strText);
		}
		
		TempNode = GlobalNode.getChildNode("StrongSignal");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			vtsGlobalPara.bStrongSignal = xmltoi(strText);
		}
        /////////////////////////////////////
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            PARAMETER_VTS vtsPara;

            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            //GlobalNode = ChannelWayNode.getChildNode("Global");
            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                vtsPara.nRoadIndex = xmltoi(strText);
            }

            TempNode = ChannelWayNode.getChildNode("TrafficSignal");
            TempNode1 = TempNode.getChildNode("LeftControl");

            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nLeftControl = (TRAFFIC_SIGNAL_DIRECTION)xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("StraightControl");
            if(!TempNode1.isEmpty())
            {
            strText = TempNode1.getText();
            vtsPara.nStraightControl = (TRAFFIC_SIGNAL_DIRECTION)xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("RightControl");
            if(!TempNode1.isEmpty())
            {
            strText = TempNode1.getText();
            vtsPara.nRightControl = (TRAFFIC_SIGNAL_DIRECTION)xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsForbitLeft");
            if(!TempNode1.isEmpty())
            {
            strText = TempNode1.getText();
            vtsPara.bForbidLeft = xmltoi(strText);
            printf("=========vtsPara.bForbidLeft=%d\n",vtsPara.bForbidLeft);
            }

            TempNode1 = TempNode.getChildNode("IsForbitRight");
            if(!TempNode1.isEmpty())
            {
            strText = TempNode1.getText();
            vtsPara.bForbidRight = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsForbitRun");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.bForbidRun = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsForbitStop");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.bForbidStop = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsAgainst");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.bAgainst = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsChangeRoad");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.bChangeRoad = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("IsPressLine");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.bPressLine = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("ForbidVehicleType");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nForbidType = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("ForbidBeginTime");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nForbidBeginTime = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("ForbidEndTime");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nForbidEndTime = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("RoadType");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nRoadType = xmltoi(strText);
            }

            TempNode1 = TempNode.getChildNode("RoadDirection");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nRoadDirection = xmltoi(strText);
            }

			TempNode1 = TempNode.getChildNode("FlagHoldStopReg");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nFlagHoldStopReg = xmltoi(strText);
            }

			TempNode1 = TempNode.getChildNode("RedLightTime");
            if(!TempNode1.isEmpty())
            {
                strText = TempNode1.getText();
                vtsPara.nRedLightTime = xmltoi(strText);
            }		

			TempNode1 = TempNode.getChildNode("RetrogradeItem");
			if(!TempNode1.isEmpty())
			{
				strText = TempNode1.getText();
				vtsPara.nRetrogradeItem = xmltoi(strText);
				printf("=========vtsPara.nRetrogradeItem=%d\n",vtsPara.nRetrogradeItem);
			}

            printf("vtsPara.nRoadIndex=%d,vtsPara.nLeftControl=%d,vtsPara.nStraightControl=%d,vtsPara.nRightControl=%d\n",vtsPara.nRoadIndex,vtsPara.nLeftControl,vtsPara.nStraightControl,vtsPara.nRightControl);
            printf("vtsPara.bForbidLeft=%d,vtsPara.bForbidRight=%d,vtsPara.bForbidRun=%d,vtsPara.bForbidStop=%d\n",vtsPara.bForbidLeft,vtsPara.bForbidRight,vtsPara.bForbidRun,vtsPara.bForbidStop);
            printf("vtsPara.bAgainst=%d,vtsPara.bChangeRoad=%d,vtsPara.bPressLine=%d\n",vtsPara.bAgainst,vtsPara.bChangeRoad,vtsPara.bPressLine);

            VTSParaMap::iterator it = mapVTSPara.find(vtsPara.nRoadIndex);
            if(it != mapVTSPara.end())
			{
				it->second = vtsPara;
			}
			else
			{
				mapVTSPara.insert(VTSParaMap::value_type(vtsPara.nRoadIndex,vtsPara));
			}
        }
        return true;
	}
#ifdef _DEBUG
    printf("====After===LoadTrafficParameter================\n\r");
#endif

    return false;
}

/*****************************************************
* 函数简介：核查节点的有效性，若不存在则新建一个此节点
* 输入参数:
*			ParentNode:父节点
*			SonNode:子节点
*			iIndex: 父节点的第几个子节点
*			strTag:节点字符串
* 返回值： bool:是否存在该子节点
*****************************************************/
bool CXmlParaUtil::CheckXMLNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag)
{
	//是否存在此Tag标志
	bool bHaveTag = false;

	SonNode = ParentNode.getChildNode(strTag, iIndex);
	if(SonNode.isEmpty())
	{
		bHaveTag = false;
		SonNode = ParentNode.addChild(strTag);
	}
	else
	{
		bHaveTag = true;
	}

	return bHaveTag;
}

/****************************************************************
* 函数简介：更新节点内部信息，若不存在则新建一个此节点，并更新其内容
* 输入参数:
*			ParentNode:父节点
*			SonNode:子节点
*			iIndex: 父节点的第几个子节点
*			strTag:节点字符串
*          nValue:节点内部需要更新的内容
* 返回值： bool:是否存在该孩子节点
*****************************************************************/
bool CXmlParaUtil::UpdateDataToNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag, double nValue,int nValueType)
{
    char buf[64];
    //是否存在此Tag标志
	bool bHaveTag = false;

	SonNode = ParentNode.getChildNode(strTag, iIndex);
	if(SonNode.isEmpty())
	{
		bHaveTag = false;
		SonNode = ParentNode.addChild(strTag);
	}
	else
	{
		bHaveTag = true;
	}

//更新内容
	memset(buf, 0, sizeof(buf));
	if(nValueType == 0)
	sprintf(buf, "%d", (int)nValue);
	else
	sprintf(buf, "%.2f", nValue);
	SonNode.updateText(buf);

	return bHaveTag;
}

/****************************************************************
* 函数简介：更新节点内部信息，若不存在则新建一个此节点，并更新其内容
* 输入参数:
*			ParentNode:父节点
*			SonNode:子节点
*			iIndex: 父节点的第几个子节点
*			strTag:节点字符串
*          strValue:节点内部需要更新的内容
* 返回值： bool:是否存在该孩子节点
*****************************************************************/
bool CXmlParaUtil::UpdateStrDataToNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag, XMLCSTR strValue)
{
    //是否存在此Tag标志
	bool bHaveTag = false;

	SonNode = ParentNode.getChildNode(strTag, iIndex);
	if(SonNode.isEmpty())
	{
		bHaveTag = false;
		SonNode = ParentNode.addChild(strTag);
	}
	else
	{
		bHaveTag = true;
	}

//更新内容
	SonNode.updateText(strValue);

	return bHaveTag;
}


//从指定节点获取到链表内的点
void CXmlParaUtil::GetListPointsFromNode(Point32fList &list, XMLNode &pNode)
{
    //printf("==================Befor GetListPointsFromNode=========\n");

    XMLNode TempNode1, TempNode2, TempNode3;
    XMLCSTR strText;
    CPoint32f ptTemp;

    if (list.size() > 0)
    {
        list.clear();
    }

    int nPointsCount;
    TempNode1 = pNode.getChildNode("Points");

    nPointsCount = TempNode1.nChildNode();

    for(int i =0; i<nPointsCount; i++)
    {
        //ptTemp._POINT();
        TempNode2 = TempNode1.getChildNode(i);

        TempNode3 = TempNode2.getChildNode("x");
        if(!TempNode3.isEmpty())
        {
            strText = TempNode3.getText();
            ptTemp.x = xmltof(strText);
        }

        TempNode3 = TempNode2.getChildNode("y");
        if(!TempNode3.isEmpty())
        {
            strText = TempNode3.getText();
            ptTemp.y = xmltof(strText);
        }


        TempNode3 = TempNode2.getChildNode("z");
        if(!TempNode3.isEmpty())
        {
            strText = TempNode3.getText();
            ptTemp.z = xmltof(strText);
        }
        //printf("ptTemp.x=%f,ptTemp.y=%f,ptTemp.z=%f\r\n",ptTemp.x,ptTemp.y,ptTemp.z);

		LogTrace("GetFromNode.log", "=GetListPointsFromNode==ptTemp=[x,y,z]=[%f,%f,%f]=", \
			ptTemp.x, ptTemp.y, ptTemp.z);

        list.push_back(ptTemp);
    }

    //printf("==================After GetListPointsFromNode=========\n");
}


//获取指定节点，到区域内的点
void CXmlParaUtil::GetRegionPointsFromNode(COMMON_REGION  & common_region, XMLNode &pNode)
{
    //printf("==================Befor GetRegionPointsFromNode=========\n");

    //printf("==============Region %s========\n", common_region.chProperty.strName);

    XMLNode TempNode1, TempNode2, TempNode3;
    XMLCSTR strText;
    int iAreasCount, iPointsCount;

    CPoint32f ptTemp;

    iAreasCount = pNode.nChildNode("Points");

    for(int j=0;j<iAreasCount;j++)
    {
        REGION_PROPERTY region;

        TempNode1 = pNode.getChildNode("Points", j);
        iPointsCount = TempNode1.nChildNode();

        for (int i=0;i<iPointsCount;i++)
        {
            TempNode2 = TempNode1.getChildNode(i);

            TempNode3 = TempNode2.getChildNode("x");
            if(!TempNode3.isEmpty())
            {
                strText = TempNode3.getText();
                ptTemp.x = xmltof(strText);
            }


            TempNode3 = TempNode2.getChildNode("y");
            if(!TempNode3.isEmpty())
            {
                strText = TempNode3.getText();
                ptTemp.y = xmltof(strText);
            }

            //printf("===iAreasCount=%d===iPointsCount=%d==,ptTemp.x=%f,ptTemp.y=%f\n",iAreasCount,iPointsCount,ptTemp.x,ptTemp.y);

            region.listPt.push_back(ptTemp);
        }

		//读入方向
		TempNode1 = pNode.getChildNode("DirectionPoints", j);
        iPointsCount = TempNode1.nChildNode();

		for (int i=0;i<iPointsCount;i++)
        {
			TempNode2 = TempNode1.getChildNode(i);

            TempNode3 = TempNode2.getChildNode("x");
            if(!TempNode3.isEmpty())
            {
                strText = TempNode3.getText();
                ptTemp.x = xmltof(strText);
            }


            TempNode3 = TempNode2.getChildNode("y");
            if(!TempNode3.isEmpty())
            {
                strText = TempNode3.getText();
                ptTemp.y = xmltof(strText);
            }
			region.directionListPt.push_back(ptTemp);
		}

        common_region.listRegionProp.push_back(region);
    }

    //printf("==================After GetRegionPointsFromNode=========\n");

}


/****************************************************************
* 函数简介：由车道检测参数列表,生成车道检测参数
* 输入参数:
*			roadParamInlist:车道检测参数列表
*			nModel:模板编号
* 返回值： bool:是否生成模板成功
*****************************************************************/

//获取模板编号
int CXmlParaUtil::GetMaxModelId(XMLNode& ModelsNode)
{
    int nModelId = 0;

    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
	XMLNode TempNode, ModelNode;
	XMLCSTR strText;

    int iTemp = 0;
    int iCurChannel; //记录当前通道编号
    int nPoints;
    {
        //通道个数
        int nModels = ModelsNode.nChildNode();
        /////////////////////////////////////找到最大的节点
        int i = 0;
        while(i < nModels)
        {
            ModelNode = ModelsNode.getChildNode(i); //通道节点

            TempNode = ModelNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nModelId < iTemp)
                {
                    nModelId = iTemp;
                }
            }


            i++;
        }//End of while
    }
    nModelId++;
    return nModelId;
}

//生成设备检测参数
bool CXmlParaUtil::AddDeviceParaMeterInfo(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT sChannel,int nDeviceID,int nPreSet)
{
    //创建文件夹
    char buf[256]={0};
    sprintf(buf,"./config/Camera%d",nDeviceID);
    std::string strPath(buf);

    if(access(strPath.c_str(),0) != 0)
    mkdir(strPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);

    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

    if(g_nMultiPreSet == 0)
    sprintf(buf,"%s/RoadParameter.xml",strPath.c_str());
    else
    sprintf(buf,"%s/RoadParameter-PreSet%d.xml",strPath.c_str(),nPreSet);
    std::string strRoadParameterXml(buf);
    XmlNode = XMLNode::createXMLTopNode("RoadParameter");
    ChannelsNode  = XmlNode.addChild("Channels");

    std::list<VEHICLE_PARAM_FOR_EVERY_FRAME>::iterator it_roadpara_b = roadParamInlist.begin();
    std::list<VEHICLE_PARAM_FOR_EVERY_FRAME>::iterator it_roadpara_e = roadParamInlist.end();
    while (it_roadpara_b != it_roadpara_e)
    {
        VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;
        roadParamIn = *it_roadpara_b;

        if(roadParamIn.nModelId!=0)//如果使用模板需要将模板中的设置信息修改到当前车道配置中来
        {
            GetModelParameter(roadParamIn);
        }
        sChannel.uId = nDeviceID;
        AddRoadParameter(ChannelsNode,roadParamIn, sChannel);

        it_roadpara_b++;
    }//End of while it_channel_b != it_channel_e)
    bool bRet =  XmlNode.writeToFile(strRoadParameterXml.c_str());

    if(bRet)
    {
        if(g_nServerType == 1)
        g_AMSCommunication.SendSettingsToCS(nDeviceID,nPreSet,1);
    }

    return bRet;
}

//生成事件检测参数及其模板
bool CXmlParaUtil::AddRoadParameterByList(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel)
{
      //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

    std::string strRoadParameterXml = "./config/RoadParameter.xml";

    int nChannel = sChannel.uId;
    //////////////////////////////////////////////////////--新建xml文件--end
    #ifdef _DEBUG
        printf("RoadParameter========nChannel=%d======\n\r", nChannel);
    #endif

    //RoadParameter.xml是否存在
    if(access(strRoadParameterXml.c_str(),F_OK) != 0) //不存在
    {
        #ifdef _DEBUG
            printf("=======Create the strRoadParameterXml xml file!\n\r");
        #endif

        XmlNode = XMLNode::createXMLTopNode("RoadParameter");
        ChannelsNode = XmlNode.addChild("Channels");

    } //End of if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    else
    {
        XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");

        ChannelsNode = XmlNode.getChildNode("Channels");

        if(ChannelsNode.isEmpty())
        {
            XmlNode = XMLNode::createXMLTopNode("RoadParameter");
            ChannelsNode = XmlNode.addChild("Channels");
        }
        else
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }
    //////////////////////////////////////////////////////--新建xml文件--end

    std::list<VEHICLE_PARAM_FOR_EVERY_FRAME>::iterator it_roadpara_b = roadParamInlist.begin();
    std::list<VEHICLE_PARAM_FOR_EVERY_FRAME>::iterator it_roadpara_e = roadParamInlist.end();

    while (it_roadpara_b != it_roadpara_e)
    {
        VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;
        roadParamIn = *it_roadpara_b;

        if(roadParamIn.nModelId!=0)//如果使用模板需要将模板中的设置信息修改到当前车道配置中来
        {
            GetModelParameter(roadParamIn);
        }
        AddRoadParameter(ChannelsNode,roadParamIn, sChannel);

        it_roadpara_b++;
    }//End of while it_roadpara_b != it_roadpara_e)

    return XmlNode.writeToFile(strRoadParameterXml.c_str());
}

//获取事件模板检测参数
void CXmlParaUtil::GetModelParameter(VEHICLE_PARAM_FOR_EVERY_FRAME& roadParamIn)
{
    //通道，车道，参数
	XMLNode XmlNode, ModelsNode;
	XMLNode ModelNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;


	std::string strRoadParameterXml = "./profile/RoadParameterModel.xml";
	if(access(strRoadParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");
        //通道
		ModelsNode = XmlNode.getChildNode("Models");

		int nModels = ModelsNode.nChildNode();

        for(int i =0; i<nModels ;i++)
        {
            ModelNode = ModelsNode.getChildNode(i);

            TempNode = ModelNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                int nModelId = -1;
                if(strText)
                nModelId = xmltoi(strText);

                if(nModelId == roadParamIn.nModelId)
                {
                    LoadRoadParameter(ModelNode,roadParamIn);
                    break;
                }
            }
            else
            {
                printf("==========The ModelNode does't have ChildNode of ModelId========!!!\n");
            }
        }
	}
}

//更新事件检测参数
void CXmlParaUtil::UpdateRoadParameterNode(XMLNode& ParentNode,VEHICLE_PARAM_FOR_EVERY_FRAME & roadParamIn)
{
            XMLNode TempNode,TempNode1,TempNode2;
            XMLCSTR strTag;

            strTag = "IsNixing";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bNixing);

			strTag = "NixingIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nNixingIgn);

			strTag = "IsBianDao";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bBianDao);

			strTag = "BiaoDaoIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nBianDaoIgn);

			strTag = "ChangeDetectMod";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nChangeDetectMod);

			strTag = "IsPressLine";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bPressYellowLine);

			strTag = "PressLineIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nPressYellowLineIgn);

			strTag = "IsPressLeadStreamLine";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bPressLeadStreamLine);

			strTag = "PressLeadStreamLineIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nPressLeadStreamLineIgn);

			strTag = "IsStop";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStop);

			strTag = "StopIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nStopIgn);

			strTag = "IsDusai";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bDusai);

			strTag = "DusaiIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nDusaiIgn);

			strTag = "StatQueueLen";                          //nahs
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatQueueLen);

			strTag = "VeloStartStat4QL";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_fVeloStartStat4QL);	

			strTag = "LengthStartStat4QL";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_fLengthStartStat4QL);

			strTag = "QLStatIntervalTime";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_fQLStatIntervalTime);
			//strTag = "WayRate";
			//UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nWayRate);

			//strTag = "DusaiSpeed";
			//UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nDusaiSpeed);

			strTag = "WayRate";
			CheckXMLNode(ParentNode, TempNode, 0, strTag);
			for(int j=0; j<5; j++)
			{
			    strTag = "Value";
			    CheckXMLNode(TempNode, TempNode1, j, strTag);

                strTag = "QueueLength";
			    //CheckXMLNode(TempNode, TempNode1, j, strTag);
			    UpdateDataToNode(TempNode1, TempNode2, 0, strTag, roadParamIn.m_nWayRate[j]);

			    strTag = "DusaiSpeed";
			    UpdateDataToNode(TempNode1, TempNode2, 0, strTag, roadParamIn.m_nDusaiSpeed[j]);
			}
    #ifdef _DEBUG
            printf("=========3333333 =\n\r");
    #endif
			strTag = "OnlyOverSpeed";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bOnlyOverSped);

			strTag = "OnlyOverSpeedIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nOnlyOverSpedIgn);

			strTag = "OnlyOverSpeedMax";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nOnlyOverSpedMax);

			strTag = "IsDiuqi";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bDiuQi);

			strTag = "DiuQiIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nDiuQiIgn);

			strTag = "DiuQiIDam";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nDiuQiIDam);

			strTag = "IsAvgSpeed";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bAvgSpeed);

			strTag = "AvgSpeedIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAvgSpeedIgn);

			strTag = "AvgSpeedMax";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAvgSpeedMax);

			strTag = "AvgSpeedMin";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAvgSpeedMin);

			strTag = "IsPerson";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bCross);

			strTag = "PersonIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nCrossIgn);

			strTag = "FlowStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatFlux);

			strTag = "AvgSpeedStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatSpeedAvg);
			printf("===========roadParamIn.m_bStatSpeedAvg=%d\n",roadParamIn.m_bStatSpeedAvg);

			strTag = "ZylStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatZyl);

			strTag = "QueueStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatQueue);

			strTag = "CtjjStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatCtjj);

			strTag = "CarTypeStat";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bStatCarType);

			strTag = "BargeIn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bBargeIn);

			strTag = "IsFj";
			//UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.is_person_channel);
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.nChannelMod);

			strTag = "BeyondMark"; //是否混行检测
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bBeyondMark);

			//strTag = "HxIgn";//混行检测忽略
			//UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nWrongChanIgn);

			strTag = "AppearIgn";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAppearIgn);

			strTag = "AlarmLevel";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.alert_level);

			strTag = "IgnStop";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nStopIgnJam);

			strTag = "IgnStopNotJam";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nStopIgnAlert);

			strTag = "Jam";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nDusaiIgnAlert);

			strTag = "IsRight";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.is_right_side);

			strTag = "IsHalfMaxSize";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.is_half_maxsize);

			strTag = "IsPersonAgainst";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAgainstDetectMod);

			strTag = "IsAppear";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bObjectAppear);

			strTag = "IsCross";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nCrossDetectMod);

			strTag = "IsCarAppear";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nObjeceDetectMod);

			strTag = "Angle";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAngle);

			strTag = "ForbidType";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nForbidType);

			printf("==============roadParamIn.m_nForbidType=%d\n", roadParamIn.m_nForbidType);

			strTag = "AllowBigBeginTime";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAllowBigBeginTime);
			printf("==============roadParamIn.m_nAllowBigBeginTime=%d\n", roadParamIn.m_nAllowBigBeginTime);

			strTag = "AllowBigEndTime";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAllowBigEndTime);
			printf("==============roadParamIn.m_nAllowBigEndTime=%d\n", roadParamIn.m_nAllowBigEndTime);

			strTag = "Crowd";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bCrowd);

			strTag = "PersonCount";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nPersonCount);

			strTag = "AreaPercent";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_nAreaPercent);

			strTag = "PersonRun";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_bPersonRun);

			strTag = "MaxRunSpeed";
			UpdateDataToNode(ParentNode, TempNode, 0, strTag, roadParamIn.m_fMaxRunSpeed,1);

			strTag = "ModelName";
			XMLCSTR strModelName(roadParamIn.chModelName);
            UpdateStrDataToNode(ParentNode, TempNode, 0, strTag, strModelName);
			printf("==============roadParamIn.strModelName=%s\n", strModelName);
}

//事件检测参数模板
bool CXmlParaUtil::AddRoadParameterModel(VEHICLE_PARAM_FOR_EVERY_FRAME& roadParamIn,int nModelKind,int& nModelID)
{
    //通道，车道，参数
	XMLNode XmlNode,ModelsNode,ModelNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strTag;
	XMLCSTR strText;

    std::string strRoadParameterXml = "./profile/RoadParameterModel.xml";
    if(access(strRoadParameterXml.c_str(),F_OK) != 0) //不存在
    {
        #ifdef _DEBUG
            printf("=======Create the strRoadParameterXml xml file!\n\r");
        #endif

        XmlNode = XMLNode::createXMLTopNode("RoadParameter");
        ModelsNode = XmlNode.addChild("Models");

    } //End of if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    else
    {
        XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");
        ModelsNode = XmlNode.getChildNode("Models");

        if(nModelKind == 2)
        {
           DeleteModelNode(ModelsNode,roadParamIn.nModelId);
           return XmlNode.writeToFile(strRoadParameterXml.c_str());
        }
    }

    char buf[64]={0};
    int iTemp = 0;

	if(nModelKind == 1)//修改模板
    {
        sprintf(buf, "%d", nModelID);

         //模板个数
        int nModels = ModelsNode.nChildNode();
        int i = 0;
        while(i < nModels)
        {
            ModelNode = ModelsNode.getChildNode(i); //节点

            TempNode = ModelNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nModelID == iTemp)
                {
                    break;
                }
            }

            i++;
        }
    }
    else if(nModelKind == 0)//添加模板
    {
        nModelID = GetMaxModelId(ModelsNode);
        sprintf(buf, "%d", nModelID);

        ModelNode = ModelsNode.addChild("Model");
    }

    strTag = "ModelId";
    CheckXMLNode(ModelNode, TempNode, 0, strTag);
    TempNode.updateText(buf);

    strTag = "IsModel";
    CheckXMLNode(ModelNode, TempNode, 0, strTag);
    TempNode.updateText("1"); //非模板

    //更新模板节点
    UpdateRoadParameterNode(ModelNode,roadParamIn);

    return XmlNode.writeToFile(strRoadParameterXml.c_str());

}

//生成车道检测参数2
/****************************************************************
* 函数简介：由模板参数结构，生成车道检测参数
* 输入参数:
*          ModelsNode:模板组节点
*			roadParamIn:车道检测参数
*			nModel:模板编号
* 返回值： bool:是否生成模板成功
*****************************************************************/
//事件检测参数
bool CXmlParaUtil::AddRoadParameter(XMLNode& ChannelsNode, VEHICLE_PARAM_FOR_EVERY_FRAME & roadParamIn,SRIP_CHANNEL_EXT& sChannel)
{
        int nChannel = sChannel.uId;
        char buf[64];
        int iTemp = 0;
        //通道，车道，参数
        XMLNode XmlNode, ChannelWaysNode;
        XMLNode ChannelNode, ChannelWayNode, GlobalNode;
        XMLNode TempNode, TempNode1, TempNode2;
        XMLCSTR strTag;

        //通道个数
        int nChannels = ChannelsNode.nChildNode();
        #ifdef _DEBUG
            printf("=========nChannels=%d=\n\r", nChannels);
        #endif
        /////////////////////////////////////找到当前节点--begin
        int i = 0;
        bool bFindChannel = false;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            XMLCSTR strText = TempNode.getText();
            iTemp = xmltoi(strText);

            if( nChannel == iTemp)
            {
                bFindChannel = true;
                break; //找到这个通道节点
            }
            i++;
        }
        /////////////////////////////////////找到当前节点--end
        if(!bFindChannel)
        {
            ChannelNode = ChannelsNode.addChild("Channel");
        }

        strTag = "ChannelId";
        CheckXMLNode(ChannelNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", nChannel);
        TempNode.updateText(buf);


        strTag = "Global";
        CheckXMLNode(ChannelNode, GlobalNode, 0, strTag);

        strTag = "TrafficStatTime";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.uTrafficStatTime);
        TempNode.updateText(buf);

        strTag = "EventDetectDelay";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.uEventDetectDelay);
        TempNode.updateText(buf);

        strTag = "RmShade";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.bRmShade);
        TempNode.updateText(buf);

        strTag = "RmTingle";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.bRmTingle);
        TempNode.updateText(buf);

        strTag = "Sensitive";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.bSensitive);
        TempNode.updateText(buf);

        strTag = "ShowTime";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nShowTime);
        TempNode.updateText(buf);

        strTag = "HolizonMoveWeight";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nHolizonMoveWeight);
        TempNode.updateText(buf);

        strTag = "VerticalMoveWeight";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nVerticalMoveWeight);
        TempNode.updateText(buf);

        strTag = "ZoomScale";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nZoomScale);
        TempNode.updateText(buf);

        strTag = "ZoomScale2";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nZoomScale2);
        TempNode.updateText(buf);

        strTag = "ZoomScale3";
        CheckXMLNode(GlobalNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", sChannel.nZoomScale3);
        TempNode.updateText(buf);

        /////////////////////////////////////
        strTag = "ChannelWays";
        CheckXMLNode(ChannelNode, ChannelWaysNode, 0, strTag);
        ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");
        int nChannelWays = ChannelWaysNode.nChildNode();

        strTag = "ChannelWay";
        CheckXMLNode(ChannelWaysNode, ChannelWayNode, nChannelWays-1, strTag);

        strTag = "ChannelWayId";
        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", roadParamIn.nChannelID);
        TempNode.updateText(buf);

        strTag = "IsModel";
        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
        bool bModel = false;
        if(roadParamIn.nModelId!=0) //模板
        bModel = true;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", bModel);
        TempNode.updateText(buf);


        strTag = "ModelId";
        CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
        memset(buf, 0, sizeof(buf));
        if(!bModel)
        roadParamIn.nModelId = 1000+roadParamIn.nChannelID;//特殊的模板号（表示该车道未使用模板）
        sprintf(buf, "%d", roadParamIn.nModelId);
        TempNode.updateText(buf); //模板编号

#ifdef _DEBUG
    printf("=======================Before UpdateRoadParameterNode(ChannelWayNode,roadParamIn);=================\n\r");
#endif
        if(!bModel)
        UpdateRoadParameterNode(ChannelWayNode,roadParamIn);

#ifdef _DEBUG
    printf("=======================After UpdateRoadParameterNode(ChannelWayNode,roadParamIn);=================\n\r");
#endif
	    return true;
}


//清空车道
bool CXmlParaUtil::DeleteRoadSettingInfo(int nChannel)
{
     std::string strRoadSettingInfoXml;

     if((g_nSwitchCamera == 0)&&(g_nMultiPreSet == 0))
     {
         strRoadSettingInfoXml = "./config/RoadSettingInfo.xml";
     }
     else
     {
        {
            int nCameraID = g_skpDB.GetCameraID(nChannel);
            char buf[256]={0};

            if(g_nMultiPreSet == 0)
            sprintf(buf,"./config/Camera%d/RoadSettingInfo.xml",nCameraID);
            else
            {
                int nPreSet = g_skpDB.GetPreSet(nChannel);
                sprintf(buf,"./config/Camera%d/RoadSettingInfo-PreSet%d.xml",nCameraID,nPreSet);
            }
            std::string strPath(buf);

            if(access(strPath.c_str(),0) == 0)
            {
                nChannel = nCameraID;
                strRoadSettingInfoXml = strPath;
            }
            else
            {
                    return false;
            }
        }
     }
    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode, ChannelsNode, RoadWaysNode, ChannelWaysNode, PointsNode, GlobalNode;
	XMLNode TempNode, ChannelNode, RoadWayNode, ChannelWayNode, PointNode;
	XMLCSTR strText;

    int iTemp = 0;

    if (access(strRoadSettingInfoXml.c_str(), F_OK) == 0) //存在
    {
        //xml文件
        XmlNode = XMLNode::parseFile(strRoadSettingInfoXml.c_str()).getChildNode("RoadSettingInfo");
        //通道
        ChannelsNode = XmlNode.getChildNode("Channels");

        //通道个数
        int nChannels = ChannelsNode.nChildNode();
        /////////////////////////////////////找到当前节点
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    printf("DeleteRoadSettingInfo nChannel=%d,i=%d\n",nChannel,i);
                    ChannelNode.deleteNodeContent();
                    return XmlNode.writeToFile(strRoadSettingInfoXml.c_str());
                    //XmlNode.writeToFile("temp.xml");
                }
            }

            i++;
        }
    }
    return false;
}

//删除通道节点
bool CXmlParaUtil::DeleteChannelNode(XMLNode ChannelsNode,int nChannel)
{
	XMLCSTR strText;
    XMLNode ChannelNode,TempNode;
    int iTemp = 0;

    //
    {
        //通道个数
        int nChannels = ChannelsNode.nChildNode();
        /////////////////////////////////////找到当前节点
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    printf("DeleteChannelsNode nChannel=%d,i=%d\n",nChannel,i);
                    ChannelNode.deleteNodeContent();
                    return true;
                }
            }
            else
            {
                printf("==========The ChannelsNode does't have ChildNode of ChannelId========!!!\n");
            }

            i++;
        }
    }
    return false;
}

//删除模板节点
bool CXmlParaUtil::DeleteModelNode(XMLNode ModelsNode,int nModel)
{
	XMLCSTR strText;
    XMLNode ModelNode,TempNode;
    int iTemp = 0;
printf("==========DeleteModelNode====nModel=%d===!!!\n",nModel);

    //
    {
        //通道个数
        int nModels = ModelsNode.nChildNode();
        /////////////////////////////////////找到当前节点
        int i = 0;
        while(i < nModels)
        {
            ModelNode = ModelsNode.getChildNode(i); //通道节点

            TempNode = ModelNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                XMLCSTR strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nModel == iTemp)
                {
                    ModelNode.deleteNodeContent();
                    return true;
                }
            }
            else
            {
                printf("==========The ModelsNode does't have ChildNode of ModelId========!!!\n");
            }

            i++;
        }
    }
    return false;
}


//客户端获取车道坐标模板
/////nModel = 0;//不使用模板(实际设置)
//// nModel = 1;1600x1200模板
//// nModel = 2;1600x1080模板
std::string CXmlParaUtil::GetRoadSettingInfo(int nChannel,int nModel)
{
  CSkpRoadXmlValue xml;
    //读取车道坐标模板
    LIST_CHANNEL_INFO list_channel_info;
    int nModelChannel = nModel;
    CXmlParaUtil xmlModel;

    #ifdef _DEBUG
            printf("=========Before=xmlModel.LoadRoadSettingInfo =\n\r");
    #endif
    if(nModel>0)
    {
        //取模板
        if(!xmlModel.LoadRoadSettingInfo(list_channel_info,nModelChannel,true))
        {

            return "";
        }
    }
    else
    {
        //取实际设置
        if(!xmlModel.LoadRoadSettingInfo(list_channel_info,nChannel,false))
        {

            return "";
        }
    }
    #ifdef _DEBUG
            printf("=========After=xmlModel.LoadRoadSettingInfo =\n\r");
    #endif

	char buf[256]={0};
	int i=0,j=0,k=0;

	//将读取的车道坐标模板转换为客户端所需要的xml格式
	LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
	LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

    std::list<REGION_PROPERTY>::iterator it_rb;
    std::list<REGION_PROPERTY>::iterator it_re;

    std::list<CPoint32f>::iterator it_begin;
    std::list<CPoint32f>::iterator it_end;

    std::list<CPoint32f>::iterator it_32fb;
    std::list<CPoint32f>::iterator it_32fe;

    Point32fList::iterator item_b;
    Point32fList::iterator item_e;

    CPoint32f pt32f;

	while(it_b!=it_e)
	{
	    CHANNEL_INFO channel_info = *it_b;

		xml[i]["Channel"] = nChannel;			/* 通道号		*/
		xml[i]["Index"] = channel_info.chProp_index.value.nValue;			/* 车道的序号		*/
		sprintf(buf,"%d",channel_info.chProp_name.value.nValue);
		xml[i]["RoadName"] = buf;		/* 车道的名称		*/
		xml[i]["Direction"] = channel_info.chProp_direction.value.nValue;		/* 车道路的方向	*/
//		xml[i]["ModelID"] = channel_info.chRegion.chProperty.value.nValue;   //事件参数模板号码

        printf("channel_info.chProp_name.value.nValue=%d\n",channel_info.chProp_name.value.nValue);

        /* 道路检测区域 */
        it_begin = channel_info.roadRegion.listPT.begin();
        it_end = channel_info.roadRegion.listPT.end();
        j=0;
        for(; it_begin != it_end; it_begin++)
        {
            pt32f.x = it_begin->x;
            pt32f.y = it_begin->y;

            xml[i]["Roadway"][j]["x"] = pt32f.x;
			xml[i]["Roadway"][j]["y"] = pt32f.y;

            printf("roadRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
            j++;
        }

        /* 车道方向坐标		*/
		xml[i]["DirectionCoordin"][0]["x"] = channel_info.chProp_direction.ptBegin.x;
        xml[i]["DirectionCoordin"][0]["y"] = channel_info.chProp_direction.ptBegin.y;
        xml[i]["DirectionCoordin"][1]["x"] = channel_info.chProp_direction.ptEnd.x;
        xml[i]["DirectionCoordin"][1]["y"] = channel_info.chProp_direction.ptEnd.y;

		/* 车道检测区域 */
		it_begin = channel_info.chRegion.listPT.begin();
        it_end = channel_info.chRegion.listPT.end();
        j=0;
        for(; it_begin != it_end; it_begin++)
        {
            pt32f.x = it_begin->x;
            pt32f.y = it_begin->y;

            xml[i]["Channelway"][j]["x"] = pt32f.x;
			xml[i]["Channelway"][j]["y"] = pt32f.y;

            printf("chRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
            j++;
        }

		/* 停车检测区域		*/
        it_rb = channel_info.stopRegion.listRegionProp.begin();
        it_re = channel_info.stopRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
            item_b = it_rb->listPt.begin();
            item_e = it_rb->listPt.end();
            k=0;
            for( ; item_b!=item_e; item_b++)
            {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["ParkArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["ParkArea"][j]["Coordin"][k]["y"] = pt32f.y;

                    printf("stopRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
            }
            j++;
        }

		/* 行人检测区域		*/
		it_rb = channel_info.personRegion.listRegionProp.begin();
        it_re = channel_info.personRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
            item_b = it_rb->listPt.begin();
            item_e = it_rb->listPt.end();
            k=0;
            for( ; item_b!=item_e; item_b++)
            {
                pt32f.x = (item_b->x);
                pt32f.y = (item_b->y);
                xml[i]["PersonArea"][j]["Coordin"][k]["x"] = pt32f.x;
				xml[i]["PersonArea"][j]["Coordin"][k]["y"] = pt32f.y;
                printf("personRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                k++;
            }
            j++;
        }

		/* 丢弃物检测区域		*/
		it_rb = channel_info.dropRegion.listRegionProp.begin();
        it_re = channel_info.dropRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["TrashArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["TrashArea"][j]["Coordin"][k]["y"] = pt32f.y;

                    printf("dropRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        /* 闯入区域		*/
        it_rb = channel_info.BargeInRegion.listRegionProp.begin();
        it_re = channel_info.BargeInRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
            item_b = it_rb->listPt.begin();
            item_e = it_rb->listPt.end();
            k=0;
            for( ; item_b!=item_e; item_b++)
            {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["BargeInRegion"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["BargeInRegion"][j]["Coordin"][k]["y"] = pt32f.y;

                    printf("BargeInRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
            }

			item_b = it_rb->directionListPt.begin();
			item_e = it_rb->directionListPt.end();
			for(k=1; item_b!=item_e; item_b++)
			{
				char c;
				sprintf(&c, "%d", k);
				pt32f.x = (item_b->x);
				pt32f.y = (item_b->y);
				xml[i]["BargeInRegion"][j]["direction"]["x" + string(&c)] = pt32f.x;
				xml[i]["BargeInRegion"][j]["direction"]["y" + string(&c)] = pt32f.y;

				printf("BargeInRegion direction pt32f.x%d=%f,pt32f.y%d=%f\n",k, pt32f.x, k, pt32f.y);
				k++;
			}
            j++;
        }

        /* 越界区域		*/
        it_rb = channel_info.BeyondMarkRegion.listRegionProp.begin();
        it_re = channel_info.BeyondMarkRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
            item_b = it_rb->listPt.begin();
            item_e = it_rb->listPt.end();
            k=0;
            for( ; item_b!=item_e; item_b++)
            {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["BeyondMarkRegion"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["BeyondMarkRegion"][j]["Coordin"][k]["y"] = pt32f.y;

                    printf("BeyondMarkRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
            }

			item_b = it_rb->directionListPt.begin();
			item_e = it_rb->directionListPt.end();
			for(k=1; item_b!=item_e; item_b++)
			{
				char c;
				sprintf(&c, "%d", k);
				pt32f.x = (item_b->x);
				pt32f.y = (item_b->y);
				xml[i]["BeyondMarkRegion"][j]["direction"]["x" + string(&c)] = pt32f.x;
				xml[i]["BeyondMarkRegion"][j]["direction"]["y" + string(&c)] = pt32f.y;

				printf("BeyondMarkRegion direction pt32f.x%d=%f,pt32f.y%d=%f\n",k, pt32f.x, k, pt32f.y);
				k++;
			}
            j++;
        }

		/* 密度检测区域		*/
		it_rb = channel_info.DensityRegion.listRegionProp.begin();
		it_re = channel_info.DensityRegion.listRegionProp.end();
		j=0;
		for(; it_rb != it_re; it_rb++)
		{
			item_b = it_rb->listPt.begin();
			item_e = it_rb->listPt.end();
			k=0;
			for( ; item_b!=item_e; item_b++)
			{
				pt32f.x = (item_b->x);
				pt32f.y = (item_b->y);
				xml[i]["Density"][j]["Coordin"][k]["x"] = pt32f.x;
				xml[i]["Density"][j]["Coordin"][k]["y"] = pt32f.y;
				printf("Density pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
				k++;
			}
			j++;
		}

        /* 雷达区域		*/
        it_rb = channel_info.RadarRegion.listRegionProp.begin();
        it_re = channel_info.RadarRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
            item_b = it_rb->listPt.begin();
            item_e = it_rb->listPt.end();
            k=0;
            for( ; item_b!=item_e; item_b++)
            {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RadarRegion"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RadarRegion"][j]["Coordin"][k]["y"] = pt32f.y;

                    printf("RadarRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
            }
            j++;
        }


		/* 流量线检测区域		*/
		it_rb = channel_info.AmountLine.listRegionProp.begin();
        it_re = channel_info.AmountLine.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["AmountLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["AmountLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("AmountLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }

				item_b = it_rb->directionListPt.begin();
				item_e = it_rb->directionListPt.end();
				for(k=1; item_b!=item_e; item_b++)
				{
					char c;
					sprintf(&c, "%d", k);
					pt32f.x = (item_b->x);
					pt32f.y = (item_b->y);
					xml[i]["AmountLine"][j]["direction"]["x" + string(&c)] = pt32f.x;
					xml[i]["AmountLine"][j]["direction"]["y" + string(&c)] = pt32f.y;

					printf("AmountLine direction pt32f.x%d=%f,pt32f.y%d=%f\n",k, pt32f.x, k, pt32f.y);
					k++;
				}
                j++;
        }

		/* 参考线检测区域		*/
		it_rb = channel_info.RefLine.listRegionProp.begin();
        it_re = channel_info.RefLine.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RefLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RefLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("RefLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }
        /* 变道区域集合		*/
        it_rb = channel_info.TurnRoadLine.listRegionProp.begin();
        it_re = channel_info.TurnRoadLine.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["TurnRoad"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["TurnRoad"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("TurnRoadLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }

				item_b = it_rb->directionListPt.begin();
				item_e = it_rb->directionListPt.end();
				for(k=1; item_b!=item_e; item_b++)
				{
					char c;
					sprintf(&c, "%d", k);
					pt32f.x = (item_b->x);
					pt32f.y = (item_b->y);
					xml[i]["TurnRoad"][j]["direction"]["x" + string(&c)] = pt32f.x;
					xml[i]["TurnRoad"][j]["direction"]["y" + string(&c)] = pt32f.y;

					printf("TurnRoad direction pt32f.x%d=%f,pt32f.y%d=%f\n",k, pt32f.x, k, pt32f.y);
					k++;
				}

                j++;
        }

        //测速线圈
        it_rb = channel_info.LoopRegion.listRegionProp.begin();
        it_re = channel_info.LoopRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LoopLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LoopLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LoopLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //单车道停止线
        it_rb = channel_info.LineStop.listRegionProp.begin();
        it_re = channel_info.LineStop.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LineStop"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LineStop"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LineStop pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //红灯区域
        it_rb = channel_info.RedLightRegion.listRegionProp.begin();
        it_re = channel_info.RedLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RedLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RedLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("RedLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //绿灯区域
        it_rb = channel_info.GreenLightRegion.listRegionProp.begin();
        it_re = channel_info.GreenLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["GreenLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["GreenLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("GreenLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //左转灯区域
        it_rb = channel_info.LeftLightRegion.listRegionProp.begin();
        it_re = channel_info.LeftLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LeftLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LeftLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LeftLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //右转灯区域
        it_rb = channel_info.RightLightRegion.listRegionProp.begin();
        it_re = channel_info.RightLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RightLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RightLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("RightLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //直行灯区域
        it_rb = channel_info.StraightLightRegion.listRegionProp.begin();
        it_re = channel_info.StraightLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["StraightLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["StraightLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("StraightLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

         //禁止转向灯区域
        it_rb = channel_info.TurnAroundLightRegion.listRegionProp.begin();
        it_re = channel_info.TurnAroundLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["TurnAroundLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["TurnAroundLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("TurnAroundLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }


         //左转红灯区域
        it_rb = channel_info.LeftRedLightRegion.listRegionProp.begin();
        it_re = channel_info.LeftRedLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LeftRedLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LeftRedLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LeftRedLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //左转绿灯区域
        it_rb = channel_info.LeftGreenLightRegion.listRegionProp.begin();
        it_re = channel_info.LeftGreenLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LeftGreenLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LeftGreenLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LeftGreenLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //右转红灯区域
        it_rb = channel_info.RightRedLightRegion.listRegionProp.begin();
        it_re = channel_info.RightRedLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RightRedLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RightRedLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("RightRedLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

         //右转绿灯区域
        it_rb = channel_info.RightGreenLightRegion.listRegionProp.begin();
        it_re = channel_info.RightGreenLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["RightGreenLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["RightGreenLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("RightGreenLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //直行红灯区域
        it_rb = channel_info.StraightRedLightRegion.listRegionProp.begin();
        it_re = channel_info.StraightRedLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["StraightRedLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["StraightRedLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("StraightRedLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }


        //直行绿灯区域
        it_rb = channel_info.StraightGreenLightRegion.listRegionProp.begin();
        it_re = channel_info.StraightGreenLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["StraightGreenLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["StraightGreenLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("StraightGreenLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //禁止转向红灯区域
        it_rb = channel_info.TurnAroundRedLightRegion.listRegionProp.begin();
        it_re = channel_info.TurnAroundRedLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["TurnAroundRedLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["TurnAroundRedLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("TurnAroundRedLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }


        //禁止转向绿灯区域
        it_rb = channel_info.TurnAroundGreenLightRegion.listRegionProp.begin();
        it_re = channel_info.TurnAroundGreenLightRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["TurnAroundGreenLightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["TurnAroundGreenLightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("TurnAroundGreenLightArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //单车道前行线
        it_rb = channel_info.LineStraight.listRegionProp.begin();
        it_re = channel_info.LineStraight.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["LineStraight"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["LineStraight"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("LineStraight pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

		//待转区第一前行线
        it_rb = channel_info.HoldForeLineFirst.listRegionProp.begin();
        it_re = channel_info.HoldForeLineFirst.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["HoldForeFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["HoldForeFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("HoldForeFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

		//待转区第二前行线
        it_rb = channel_info.HoldForeLineSecond.listRegionProp.begin();
        it_re = channel_info.HoldForeLineSecond.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["HoldForeSecondLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["HoldForeSecondLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("HoldForeSecondLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

		//待转区第一停止线
        it_rb = channel_info.HoldStopLineFirst.listRegionProp.begin();
        it_re = channel_info.HoldStopLineFirst.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["HoldStopFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["HoldStopFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("HoldStopFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

		//待转区第二停止线
        it_rb = channel_info.HoldStopLineSecond.listRegionProp.begin();
        it_re = channel_info.HoldStopLineSecond.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["HoldStopSecondLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["HoldStopSecondLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("HoldStopSecondLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        if(i == 0)
        {
            /* 车牌区域集合		*/
            it_rb = channel_info.carnumRegion.listRegionProp.begin();
            it_re = channel_info.carnumRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["CardArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["CardArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("carnumRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //屏蔽区域
            it_rb = channel_info.eliminateRegion.listRegionProp.begin();
            it_re = channel_info.eliminateRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["SkipArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["SkipArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("eliminateRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }


            //违章检测区域
            it_rb = channel_info.ViolationRegion.listRegionProp.begin();
            it_re = channel_info.ViolationRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["ViolationArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["ViolationArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("ViolationRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //事件检测区域
            it_rb = channel_info.EventRegion.listRegionProp.begin();
            it_re = channel_info.EventRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["EventArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["EventArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("EventRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

             //红灯检测区域
            it_rb = channel_info.TrafficSignalRegion.listRegionProp.begin();
            it_re = channel_info.TrafficSignalRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["TrafficSignalArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["TrafficSignalArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("TrafficSignalRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //停止检测线
            it_rb = channel_info.StopLine.listRegionProp.begin();
            it_re = channel_info.StopLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["StopLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["StopLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("StopLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }


            //直行检测线
            it_rb = channel_info.StraightLine.listRegionProp.begin();
            it_re = channel_info.StraightLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["StraightLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["StraightLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("StraightLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //左转检测线
            it_rb = channel_info.TurnLeftLine.listRegionProp.begin();
            it_re = channel_info.TurnLeftLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["TurnLeftLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["TurnLeftLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("TurnLeftLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //右转检测线
            it_rb = channel_info.TurnRightLine.listRegionProp.begin();
            it_re = channel_info.TurnRightLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["TurnRightLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["TurnRightLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("TurnRightLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//电警第一触发线
            it_rb = channel_info.ViolationFirstLine.listRegionProp.begin();
            it_re = channel_info.ViolationFirstLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["ViolationFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["ViolationFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("ViolationFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//电警第一触发线
            it_rb = channel_info.ViolationSecondLine.listRegionProp.begin();
            it_re = channel_info.ViolationSecondLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["ViolationSecondLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["ViolationSecondLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("ViolationSecondLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//禁右初始触发线
            it_rb = channel_info.RightFirstLine.listRegionProp.begin();
            it_re = channel_info.RightFirstLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["RightFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["RightFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("RightFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//禁左初始触发线
            it_rb = channel_info.LeftFirstLine.listRegionProp.begin();
            it_re = channel_info.LeftFirstLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["LeftFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["LeftFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("LeftFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//禁前初始触发线
            it_rb = channel_info.ForeFirstLine.listRegionProp.begin();
            it_re = channel_info.ForeFirstLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["ForeFirstLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["ForeFirstLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("ForeFirstLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //黄线
            it_rb = channel_info.YellowLine.listRegionProp.begin();
            it_re = channel_info.YellowLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["YellowLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["YellowLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("YellowLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//白线
			it_rb = channel_info.WhiteLine.listRegionProp.begin();
			it_re = channel_info.WhiteLine.listRegionProp.end();
			j=0;
			for(; it_rb != it_re; it_rb++)
			{
				item_b = it_rb->listPt.begin();
				item_e = it_rb->listPt.end();
				k=0;
				for( ; item_b!=item_e; item_b++)
				{
					pt32f.x = (item_b->x);
					pt32f.y = (item_b->y);
					xml[i]["WhiteLine"][j]["Coordin"][k]["x"] = pt32f.x;
					xml[i]["WhiteLine"][j]["Coordin"][k]["y"] = pt32f.y;
					printf("WhiteLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
					k++;
				}
				j++;
			}
			
			 //导流线
            it_rb = channel_info.LeadStreamLine.listRegionProp.begin();
            it_re = channel_info.LeadStreamLine.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["LeadStreamLine"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["LeadStreamLine"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("LeadStreamLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //稳像背景区域
            it_rb = channel_info.StabBackRegion.listRegionProp.begin();
            it_re = channel_info.StabBackRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["StabBackArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["StabBackArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("StabBackArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }


            //同步标志点区域Left
            it_rb = channel_info.SynchLeftRegion.listRegionProp.begin();
            it_re = channel_info.SynchLeftRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["SynchLeftArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["SynchLeftArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("SynchLeftRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }
            //同步标志点区域Right
            it_rb = channel_info.SynchRightRegion.listRegionProp.begin();
            it_re = channel_info.SynchRightRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["SynchRightArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["SynchRightArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("SynchRightRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }
            //车牌区域2
            it_rb = channel_info.CardnumberRegion.listRegionProp.begin();
            it_re = channel_info.CardnumberRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["CardnumberArea"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["CardnumberArea"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("CardnumberRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //虚拟线圈
            it_rb = channel_info.VirtualLoopRegion.listRegionProp.begin();
            it_re = channel_info.VirtualLoopRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["VirtualLoop"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["VirtualLoop"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("VirtualLoop pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //远处行人框
            it_rb = channel_info.RemotePersonRegion.listRegionProp.begin();
            it_re = channel_info.RemotePersonRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["RemotePerson"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["RemotePerson"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("RemotePerson pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

            //近处行人框
            it_rb = channel_info.LocalPersonRegion.listRegionProp.begin();
            it_re = channel_info.LocalPersonRegion.listRegionProp.end();
            j=0;
            for(; it_rb != it_re; it_rb++)
            {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    k=0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        xml[i]["LocalPerson"][j]["Coordin"][k]["x"] = pt32f.x;
                        xml[i]["LocalPerson"][j]["Coordin"][k]["y"] = pt32f.y;
                        printf("LocalPerson pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                        k++;
                    }
                    j++;
            }

			//黄网格区域
			it_rb = channel_info.YelGridRgn.listRegionProp.begin();
			it_re = channel_info.YelGridRgn.listRegionProp.end();
			j=0;
			for(; it_rb != it_re; it_rb++)
			{
				item_b = it_rb->listPt.begin();
				item_e = it_rb->listPt.end();
				k=0;
				for( ; item_b!=item_e; item_b++)
				{
					pt32f.x = (item_b->x);
					pt32f.y = (item_b->y);
					xml[i]["NoParkingArea"][j]["Coordin"][k]["x"] = pt32f.x;
					xml[i]["NoParkingArea"][j]["Coordin"][k]["y"] = pt32f.y;
					printf("NoParkingArea pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
					k++;
				}
				j++;
			}
        }

		/* 车道标定区域			*/
        //矩形区域（4个点）
        it_begin = channel_info.calibration.region.listPT.begin();
        it_end = channel_info.calibration.region.listPT.end();
        j=0;
        while(it_begin!=it_end)
        {
            //image cor
            pt32f.x = it_begin->x;
            pt32f.y = it_begin->y;

            xml[i]["Calibration"][j]["x"] = pt32f.x;
            xml[i]["Calibration"][j]["y"] = pt32f.y;


            printf("calib pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
            it_begin++;
            j++;
        }

        //辅助标定点
        it_begin = channel_info.calibration.listPT.begin();
        it_end = channel_info.calibration.listPT.end();
        while(it_begin!=it_end)
        {
                //image cor
                pt32f.x = it_begin->x;
                pt32f.y = it_begin->y;
                xml[i]["CalibrationPoint"][j]["x"] = pt32f.x;
                xml[i]["CalibrationPoint"][j]["y"] = pt32f.y;
                xml[i]["CalibrationPoint"][j]["z"] = 0.0;

                printf("calib pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                it_begin++;
                j++;
        }

		/* 车道标定区域参数		*/
		xml[i]["Calibration-Length"]		= channel_info.calibration.length;
		xml[i]["Calibration-Width"]			= channel_info.calibration.width;
		xml[i]["Calibration-CameraHeight"]    = channel_info.calibration.cameraHeight;
		it_32fb = channel_info.calibration.list32fPT.begin();
        it_32fe = channel_info.calibration.list32fPT.end();
        j=0;
        while(it_32fb!=it_32fe)
        {
            //world cor
            pt32f.x = it_32fb->x;
            pt32f.y = it_32fb->y;
            xml[i]["Calibration32fPoint"][j]["x"] = pt32f.x;
            xml[i]["Calibration32fPoint"][j]["y"] = pt32f.y;
            xml[i]["Calibration32fPoint"][j]["z"] = 0.0;

            printf("calib pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
            it_32fb++;
            j++;
        }


        //线圈背景区域
        it_rb = channel_info.FlowBackRegion.listRegionProp.begin();
        it_re = channel_info.FlowBackRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["FlowBackArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["FlowBackArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("FlowBackRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //线圈流量检测线
        it_rb = channel_info.FlowLine.listRegionProp.begin();
        it_re = channel_info.FlowLine.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["FlowLine"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["FlowLine"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("FlowLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

        //线圈取景区域
        it_rb = channel_info.FlowFramegetRegion.listRegionProp.begin();
        it_re = channel_info.FlowFramegetRegion.listRegionProp.end();
        j=0;
        for(; it_rb != it_re; it_rb++)
        {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                k=0;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x);
                    pt32f.y = (item_b->y);
                    xml[i]["FlowFramegetArea"][j]["Coordin"][k]["x"] = pt32f.x;
                    xml[i]["FlowFramegetArea"][j]["Coordin"][k]["y"] = pt32f.y;
                    printf("FlowFramegetRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    k++;
                }
                j++;
        }

		//抓图区域
		it_rb = channel_info.GetPhotoRegion.listRegionProp.begin();
		it_re = channel_info.GetPhotoRegion.listRegionProp.end();
		j=0;
		for(; it_rb != it_re; it_rb++)
		{
			item_b = it_rb->listPt.begin();
			item_e = it_rb->listPt.end();
			k=0;
			for( ; item_b!=item_e; item_b++)
			{
				pt32f.x = (item_b->x);
				pt32f.y = (item_b->y);
				xml[i]["GetPhotoRegion"][j]["Coordin"][k]["x"] = pt32f.x;
				xml[i]["GetPhotoRegion"][j]["Coordin"][k]["y"] = pt32f.y;
				printf("GetPhotoRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
				k++;
			}
			j++;
		}

        it_b++;
		i++;
	}// End of while(it_b)
	return xml.toXml();
}

//载入全部车道检测参数及其模板
bool CXmlParaUtil::LoadAllRoadParameter(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel)
{
    #ifdef _DEBUG
        printf("LoadAllRoadParameter=============\n\r");
    #endif
	//载入车道参数模板
	LoadRoadParameterModel(roadParamInlist);

	//载入车道参数
	LoadRoadParameter(roadParamInlist,sChannel);

#ifdef _DEBUG
    printf("====After===LoadAllRoadParameter================\n\r");
#endif
    return true;
}

////载入车道检测参数模板
bool CXmlParaUtil::LoadRoadParameterModel(paraDetectList &roadParamInlist)
{
    //通道，车道，参数
	XMLNode XmlNode, ModelsNode;
	XMLNode ModelNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;


	std::string strRoadParameterXml = "./profile/RoadParameterModel.xml";
	if(access(strRoadParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");
        //通道
		ModelsNode = XmlNode.getChildNode("Models");
		int nModels = ModelsNode.nChildNode();

        for(int i =0; i<nModels ;i++)
        {
            ModelNode = ModelsNode.getChildNode(i);

            VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;
            TempNode = ModelNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                roadParamIn.nModelId = xmltoi(strText);

                LoadRoadParameter(ModelNode,roadParamIn);

                roadParamInlist.push_back(roadParamIn);
            }
            else
            {
                printf("==========The ModelNode does't have ChildNode of ModelId========!!!\n");
            }
        }
        return true;
	}
    return false;
}


//载入车道检测参数
bool CXmlParaUtil::LoadRoadParameter(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel)
{
    int nChannel = sChannel.uId;
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

    std::string strChannelWays;
    GetChannelWaysFromRoadSettingInfo(nChannel,strChannelWays);
    int nSize = strChannelWays.size()/sizeof(int);
    for(int i =0;i<nSize;i++)
    {
        VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;
        roadParamIn.nChannelID = *((int*)(strChannelWays.c_str()+sizeof(int)*i));

        printf("=================LoadRoadParameter roadParamIn.nChannelID=%d,roadParamIn.nChannelMod=%d\n",roadParamIn.nChannelID,roadParamIn.nChannelMod);
        roadParamInlist.push_back(roadParamIn);
    }

    std::string strRoadParameterXml;
    //如果需要切换相机
    if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strRoadParameterXml = "./config/RoadParameter.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/RoadParameter.xml",nCameraID);
        else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/RoadParameter-PreSet%d.xml",nCameraID,nPreSet);
        }

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        {
            nChannel = nCameraID;
            strRoadParameterXml = strPath;
        }
        else
        {
                return false;
        }
    }

    printf("=========LoadRoadParameter=======strRoadParameterXml.c_str()=%s\n",strRoadParameterXml.c_str());
	if(access(strRoadParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
               strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }
            else
            {
                printf("======RoadParameter.xml:====The ChannelNode does't have ChildNode of ChannelId========!!!\n");
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        //全局参数
        GlobalNode = ChannelNode.getChildNode("Global");
        TempNode = GlobalNode.getChildNode("TrafficStatTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.uTrafficStatTime = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("EventDetectDelay");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.uEventDetectDelay = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("RmShade");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.bRmShade = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("RmTingle");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.bRmTingle = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("Sensitive");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.bSensitive = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("ShowTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nShowTime = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("HolizonMoveWeight");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nHolizonMoveWeight = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("VerticalMoveWeight");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nVerticalMoveWeight = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("ZoomScale");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nZoomScale = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("ZoomScale2");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nZoomScale2 = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("ZoomScale3");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nZoomScale3 = xmltoi(strText);
        }

        /////////////////////////////////////
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            ChannelWayNode = ChannelWaysNode.getChildNode(i);
            VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                roadParamIn.nChannelID = xmltoi(strText);
            }

            TempNode = ChannelWayNode.getChildNode("ModelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                roadParamIn.nModelId = xmltoi(strText);

                roadParamIn.m_nStatFluxTime = sChannel.uTrafficStatTime;//统计周期

                if(roadParamIn.nModelId<1000)//模板
                GetModelParameter(roadParamIn);
                else
                LoadRoadParameter(ChannelWayNode,roadParamIn);//非模板
                printf("roadParamInlist.size=%d,roadParamIn.nChannelID=========%d\n",roadParamInlist.size(),roadParamIn.nChannelID);

                paraDetectList::iterator it_b = roadParamInlist.begin();
                paraDetectList::iterator it_e = roadParamInlist.end();
                while(it_b != it_e)
                {
                    if((it_b->nChannelID>0)&&(it_b->nChannelID == roadParamIn.nChannelID))
                    {
                        printf("roadParamIn.nModelId=%d,roadParamIn.nChannelID=========%d,is_person_channel=%d\n",roadParamIn.nModelId,roadParamIn.nChannelID,roadParamIn.is_person_channel);
                        *it_b = roadParamIn;
                         printf("roadParamIn.nModelId=%d,roadParamIn.nChannelID=========%d,is_person_channel=%d\n",it_b->nModelId,it_b->nChannelID,it_b->is_person_channel);
                        break;
                    }
                    it_b++;
                }
            }
        }
        return true;
	}
	return false;
}

//载入车道检测参数
bool CXmlParaUtil::LoadRoadParameter(XMLNode &ParentNode,VEHICLE_PARAM_FOR_EVERY_FRAME &roadParamIn)
{
    XMLNode XmlNode;
	XMLNode TempNode;
	XMLNode TempNode1, TempNode2;
	XMLCSTR strText;
    {
        TempNode = ParentNode.getChildNode("IsNixing");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bNixing = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("NixingIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nNixingIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsBianDao");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bBianDao = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("BiaoDaoIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nBianDaoIgn = xmltoi(strText);
        }

		TempNode = ParentNode.getChildNode("ChangeDetectMod");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nChangeDetectMod = xmltoi(strText);
        }

		TempNode = ParentNode.getChildNode("IsPressLine");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bPressYellowLine = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PressLineIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nPressYellowLineIgn = xmltoi(strText);
        }

		TempNode = ParentNode.getChildNode("IsPressLeadStreamLine");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bPressLeadStreamLine = xmltoi(strText);
        }

		TempNode = ParentNode.getChildNode("StatQueueLen");    //nahs
		if (!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			roadParamIn.m_bStatQueueLen = xmltoi(strText);
		}

		TempNode = ParentNode.getChildNode("VeloStartStat4QL");  //nahs
		if (!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			roadParamIn.m_fVeloStartStat4QL = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("LengthStartStat4QL");  // nhs ////////
		if (!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			roadParamIn.m_fLengthStartStat4QL = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("QLStatIntervalTime");  //nahs
		if (!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			roadParamIn.m_fQLStatIntervalTime = xmltof(strText);
		}


        TempNode = ParentNode.getChildNode("PressLeadStreamLineIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nPressLeadStreamLineIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsStop");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStop = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("StopIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nStopIgn = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("IsDusai");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bDusai = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("DusaiIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nDusaiIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("WayRate");
        if(!TempNode.isEmpty())
        {
            for(int j=0; j<5; j++)
            {
                TempNode1 = TempNode.getChildNode(j); //Value
                if(!TempNode1.isEmpty())
                {
                    TempNode2 = TempNode1.getChildNode("QueueLength");
                    if(!TempNode2.isEmpty())
                    {
                        strText = TempNode2.getText();
                        roadParamIn.m_nWayRate[j] = xmltoi(strText);
                    }

                    TempNode2 = TempNode1.getChildNode("DusaiSpeed");
                    if(!TempNode2.isEmpty())
                    {
                        strText = TempNode2.getText();
                        roadParamIn.m_nDusaiSpeed[j] = xmltof(strText);
                    }
                }
            }
        }

        TempNode = ParentNode.getChildNode("OnlyOverSpeed");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bOnlyOverSped = xmltoi(strText);
            //printf("====++=roadParamIn.m_bOnlyOverSped=%d\n",roadParamIn.m_bOnlyOverSped);
        }

        TempNode = ParentNode.getChildNode("OnlyOverSpeedIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nOnlyOverSpedIgn = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("OnlyOverSpeedMax");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nOnlyOverSpedMax = xmltof(strText);
            //printf("====++=roadParamIn.m_nOnlyOverSpedMax=%d\n",roadParamIn.m_nOnlyOverSpedMax);
        }

        TempNode = ParentNode.getChildNode("IsDiuqi");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bDiuQi = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("DiuQiIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nDiuQiIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("DiuQiIDam");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nDiuQiIDam = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsAvgSpeed");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bAvgSpeed = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AvgSpeedIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAvgSpeedIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AvgSpeedMax");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAvgSpeedMax = xmltof(strText);
        }

        TempNode = ParentNode.getChildNode("AvgSpeedMin");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAvgSpeedMin = xmltof(strText);
        }

        TempNode = ParentNode.getChildNode("IsPerson");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bCross = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nCrossIgn = xmltoi(strText);
           // printf("====++=roadParamIn.m_nCrossIgn=%d\n",roadParamIn.m_nCrossIgn);
        }

        TempNode = ParentNode.getChildNode("FlowStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatFlux = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AvgSpeedStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatSpeedAvg = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("ZylStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatZyl = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("QueueStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatQueue = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("CtjjStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatCtjj = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("CarTypeStat");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bStatCarType = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("BargeIn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bBargeIn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsFj");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.is_person_channel = xmltoi(strText);
            roadParamIn.nChannelMod = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("BeyondMark");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bBeyondMark = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AppearIgn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAppearIgn = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AlarmLevel");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.alert_level = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IgnStop");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nStopIgnJam = xmltoi(strText);
            printf("====++=roadParamIn.m_nStopIgnJam=%d\n",roadParamIn.m_nStopIgnJam);
        }

        TempNode = ParentNode.getChildNode("IgnStopNotJam");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nStopIgnAlert = xmltoi(strText);
            printf("====++=roadParamIn.m_nStopIgnAlert=%d\n",roadParamIn.m_nStopIgnAlert);
        }

        TempNode = ParentNode.getChildNode("Jam");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nDusaiIgnAlert = xmltoi(strText);
            printf("====++=roadParamIn.m_nDusaiIgnAlert=%d\n",roadParamIn.m_nDusaiIgnAlert);
        }

        TempNode = ParentNode.getChildNode("IsRight");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.is_right_side = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsHalfMaxSize");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.is_half_maxsize = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsPersonAgainst");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAgainstDetectMod = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsAppear");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bObjectAppear = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("IsCross");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nCrossDetectMod = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("IsCarAppear");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nObjeceDetectMod = xmltoi(strText);
            printf("roadParamIn.m_bCarAppear=%d\n",roadParamIn.m_bCarAppear);
        }

        TempNode = ParentNode.getChildNode("Angle");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAngle = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("ForbidType");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nForbidType = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AllowBigBeginTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAllowBigBeginTime = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AllowBigEndTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAllowBigEndTime = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("Crowd");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bCrowd = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonCount");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nPersonCount = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("AreaPercent");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_nAreaPercent = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonRun");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_bPersonRun = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("MaxRunSpeed");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            roadParamIn.m_fMaxRunSpeed = xmltof(strText);
        }

        TempNode = ParentNode.getChildNode("ModelName");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            {
                string strModelName = strText;
                if(strModelName.size() > 0 && strModelName.size() < 16)
                {
                    memcpy(roadParamIn.chModelName, strModelName.c_str(), strModelName.size());
                }
            }
        }
        /*
        else
        {
            printf("========Can't find: ModelName=========!!!\n");
        }
        */

        return true;
    } //End of if

}

////生成线圈参数list
bool CXmlParaUtil::AddLoopParameterByList(LoopParaMap &MapLoopPara, int nChannel)
{
    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode;
    XMLNode ChannelsNode;

    std::string strLoopParameterXml = "./config/LoopParameterInfo.xml";

    //////////////////////////////////////////////////////--新建xml文件--end
    #ifdef _DEBUG
        printf("AddLoopParameterByList========nChannel=%d======\n\r", nChannel);
    #endif

    //判断RoadSettingInfo.xml是否存在
    if(access(strLoopParameterXml.c_str(),F_OK) != 0) //不存在
    {
        //#ifdef _DEBUG
            printf("=======Create the strLoopParameterXml xml file!\n\r");
        //#endif

        XmlNode = XMLNode::createXMLTopNode("LoopParameter");
        ChannelsNode = XmlNode.addChild("Channels");

    } //End of if(access(strRoadSettingInfoXml.c_str(),F_OK) != 0) //不存在
    else
    {
        XmlNode = XMLNode::parseFile(strLoopParameterXml.c_str()).getChildNode("LoopParameter");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(!ChannelsNode.isEmpty())
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
        else
        {
            printf("====LoopParameterInfo.xml:===The ChannelsNode is empty!!!\n");
        }
    }
    //////////////////////////////////////////////////////--新建xml文件--end


    LoopParaMap::iterator it_channel_b = MapLoopPara.begin();
    LoopParaMap::iterator it_channel_e = MapLoopPara.end();
    while (it_channel_b != it_channel_e)
    {
        PARAMETER_LOOP loopPara;
        loopPara = it_channel_b->second;

        AddLoopParameter(ChannelsNode,loopPara, nChannel);

        it_channel_b++;
    }//End of while it_channel_b != it_channel_e)

    return XmlNode.writeToFile(strLoopParameterXml.c_str());
}

bool CXmlParaUtil::AddLoopParameter(XMLNode& ChannelsNode,PARAMETER_LOOP &loopPara, int nChannel)
{
#ifdef _DEBUG
    printf("====Before===AddLoopParameter================\n\r");
#endif

	char buf[64];
    int iTemp = 0;
	//通道，车道，参数
	XMLNode XmlNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strTag;

    printf("===============insert para to xml file============\n");
    /////////////////////////////////////找到当前节点
    //通道个数
    int nChannels = ChannelsNode.nChildNode();
    #ifdef _DEBUG
        printf("=========nChannels=%d=\n\r", nChannels);
    #endif
    /////////////////////////////////////找到当前节点--begin
    int i = 0;
    bool bFindChannel = false;
    while(i < nChannels)
    {
        ChannelNode = ChannelsNode.getChildNode(i); //通道节点

        TempNode = ChannelNode.getChildNode("ChannelId");
        if(!TempNode.isEmpty())
        {
           XMLCSTR strText = TempNode.getText();
            iTemp = xmltoi(strText);

            if( nChannel == iTemp)
            {
                bFindChannel = true;
                break; //找到这个通道节点
            }
        }
        else
        {
            printf("======LoopParameterInfo.xml:====The ChannelsNode does't have ChildNode of ChannelId========!!!\n");
        }

        i++;
    }
    /////////////////////////////////////找到当前节点--end

    printf("before ChannelsNode.addChild bFindChannel=%d\n",bFindChannel);
    if(!bFindChannel)
    {
        bool bEmpt = ChannelsNode.isEmpty();
        printf("ChannelsNode = %d\n",bEmpt);

        printf("==========before ChannelsNode.addChild\n");
        ChannelNode = ChannelsNode.addChild("Channel");
        printf("end ChannelsNode.addChild\n");
    }

    strTag = "ChannelId";
    CheckXMLNode(ChannelNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", nChannel);
    TempNode.updateText(buf);

    /////////////////////////////////////
    strTag = "ChannelWays";
    CheckXMLNode(ChannelNode, ChannelWaysNode, 0, strTag);
    ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");
    int nChannelWays = ChannelWaysNode.nChildNode();

    strTag = "ChannelWay";
    CheckXMLNode(ChannelWaysNode, ChannelWayNode, nChannelWays-1, strTag);

    strTag = "ChannelWayId";
    CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", loopPara.nRoadIndex);
    TempNode.updateText(buf);

    strTag = "LoopIndex";
    CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", loopPara.nLoopIndex);
    TempNode.updateText(buf);

    strTag = "LoopDistance";
    CheckXMLNode(ChannelWayNode, TempNode, 0, strTag);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%f", loopPara.fDistance);
    TempNode.updateText(buf);


#ifdef _DEBUG
    printf("====After===AddLoopParameter================\n\r");
#endif
	return true;
}

bool CXmlParaUtil::LoadLoopParameter(LoopParaMap& MapLoopPara,int nChannel)
{
    MapLoopPara.clear();
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

	std::string strChannelWays;
    GetChannelWaysFromRoadSettingInfo(nChannel,strChannelWays);
    int nSize = strChannelWays.size()/sizeof(int);
    for(int i =0;i<nSize;i++)
    {
        PARAMETER_LOOP loopPara;
        loopPara.nRoadIndex = *((int*)(strChannelWays.c_str()+sizeof(int)*i));
        loopPara.nLoopIndex = 0;
        MapLoopPara.insert(LoopParaMap::value_type(loopPara.nRoadIndex,loopPara));
        printf("=======nSize==%d=======loopPara.nRoadIndex==%d=====\n\r",nSize,loopPara.nRoadIndex);
    }

	std::string strLoopParameterXml = "./config/LoopParameterInfo.xml";
	if(access(strLoopParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strLoopParameterXml.c_str()).getChildNode("LoopParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }
            else
            {
                printf("====LoopParameterInfo.xml:======The ChannelsNode does't have ChildNode of ChannelId========!!!\n");
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }
        /////////////////////////////////////
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            PARAMETER_LOOP loopPara;

            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            //GlobalNode = ChannelWayNode.getChildNode("Global");
            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                loopPara.nRoadIndex = xmltoi(strText);
            }


            TempNode = ChannelWayNode.getChildNode("LoopIndex");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                loopPara.nLoopIndex = xmltoi(strText);
            }

            TempNode = ChannelWayNode.getChildNode("LoopDistance");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                loopPara.fDistance = xmltof(strText);
            }

            LoopParaMap::iterator it = MapLoopPara.find(loopPara.nRoadIndex);
            if(it != MapLoopPara.end())
            it->second = loopPara;
        }
        return true;
	}

#ifdef _DEBUG
    printf("====After===LoadLoopParameter================\n\r");
#endif

    return false;
}

//从车道设置文件中获取车道列表
bool CXmlParaUtil::GetChannelWaysFromRoadSettingInfo(int nChannel,std::string& strChannelWays)
{
    std::string strRoadSettingInfoXml;

    if((g_nSwitchCamera == 0) &&(g_nMultiPreSet == 0))
    {
        strRoadSettingInfoXml = "./config/RoadSettingInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        {
            char buf[256]={0};

            if(g_nMultiPreSet == 0)
            sprintf(buf,"./config/Camera%d/RoadSettingInfo.xml",nCameraID);
            else
            {
                int nPreSet = g_skpDB.GetPreSet(nChannel);
                sprintf(buf,"./config/Camera%d/RoadSettingInfo-PreSet%d.xml",nCameraID,nPreSet);
            }
            std::string strPath(buf);

            if(access(strPath.c_str(),0) == 0)
            {
                nChannel = nCameraID;
                strRoadSettingInfoXml = strPath;
            }
            else
            {
                return false;
            }
        }
    }

    printf("====GetChannelWaysFromRoadSettingInfo=========nChannel=%d======\n\r",nChannel);

    //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode, ChannelsNode, RoadWaysNode, ChannelWaysNode, PointsNode, GlobalNode;
	XMLNode TempNode, ChannelNode, RoadWayNode, ChannelWayNode, PointNode;
	XMLNode TempNode1;
	XMLCSTR strText;

    int iTemp = 0;
    int iCurChannel; //记录当前通道编号
    int nPoints;

    if (access(strRoadSettingInfoXml.c_str(), F_OK) == 0) //存在
    {
        //xml文件
        XmlNode = XMLNode::parseFile(strRoadSettingInfoXml.c_str()).getChildNode("RoadSettingInfo");
        //通道
        ChannelsNode = XmlNode.getChildNode("Channels");

        //通道个数
        int nChannels = ChannelsNode.nChildNode();

        CHANNEL_INFO channel_info;
        /////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }//End of while
        if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
        {
            return false;
        }

        RoadWaysNode = ChannelNode.getChildNode("RoadWays");
        int nRoadWaysCount = RoadWaysNode.nChildNode();
        for(int j=0; j<nRoadWaysCount; j++)
        {
            RoadWayNode = RoadWaysNode.getChildNode(j);
            ChannelWaysNode = RoadWayNode.getChildNode("ChannelWays");
            int nChannelWaysCount = ChannelWaysNode.nChildNode();

            printf("====nRoadWaysCount==%d=======nChannelWaysCount=%d======\n\r",nRoadWaysCount,nChannelWaysCount);
            int k =0;
            for (k=0; k<nChannelWaysCount; k++)
            {
                ChannelWayNode = ChannelWaysNode.getChildNode(k); //车道节点
                TempNode = ChannelWayNode.getChildNode("ChannelWayId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    int nChannelWayId = xmltoi(strText);
                    strChannelWays.append((char*)&nChannelWayId,sizeof(int));
                }
            }
        }
        return true;
    }
    return false;
}

//更新参数设置文件(闯红灯，线圈，事件等)
void CXmlParaUtil::UpdateParaSettingInfo(int nChannel,std::string strChannelWays,int nPreSet)
{
    printf("UpdateParaSettingInfo  nChannel=%d,strChannelWays=%s\n",nChannel,strChannelWays.c_str());
     //通道，道路，车道，区域，区域元素，点
    XMLNode XmlNode, ChannelsNode, RoadWaysNode, ChannelWaysNode, PointsNode, GlobalNode;
	XMLNode TempNode, ChannelNode, RoadWayNode, ChannelWayNode, PointNode;
	XMLNode TempNode1;
	XMLCSTR strText;

    bool bDeleteRoadIndex = false;
    int iTemp = 0;
    int iCurChannel; //记录当前通道编号
    int nPoints;
    //更新线圈参数
    std::string strLoopParameterXml;

    if((g_nMultiPreSet == 0) && (g_nSwitchCamera == 0))
    strLoopParameterXml = "./config/LoopParameterInfo.xml";
    else
    {
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/LoopParameterInfo.xml",nChannel);
        else
        sprintf(buf,"./config/Camera%d/LoopParameterInfo-PreSet%d.xml",nChannel,nPreSet);

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        strLoopParameterXml = strPath;
    }

	if(access(strLoopParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strLoopParameterXml.c_str()).getChildNode("LoopParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }


            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return;
        }

        bDeleteRoadIndex = false;
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                int nChannelWayId = xmltoi(strText);
                int nSize = strChannelWays.size()/sizeof(int);
                bool bExist = false;
                for(int j=0;j<nSize;j++)
                {
                    int nRoadIndex = *((int*)(strChannelWays.c_str()+j*sizeof(int)));
                    if(nRoadIndex==nChannelWayId)
                    {
                        bExist = true;
                        break;
                    }
                }
                if(!bExist)
                {
                    ChannelWayNode.deleteNodeContent();
                    bDeleteRoadIndex = true;
                }
            }
        }
        if(bDeleteRoadIndex)
        {
            XmlNode.writeToFile(strLoopParameterXml.c_str());
        }
	}

    //更新闯红灯参数
    std::string strVTSParameterXml;

    if((g_nMultiPreSet == 0) && (g_nSwitchCamera == 0))
    strVTSParameterXml = "./config/VTSParameterInfo.xml";
    else
    {
        char buf[256]={0};
        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/VTSParameterInfo.xml",nChannel);
        else
        sprintf(buf,"./config/Camera%d/VTSParameterInfo-PreSet%d.xml",nChannel,nPreSet);

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        strVTSParameterXml = strPath;
    }

	if(access(strVTSParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strVTSParameterXml.c_str()).getChildNode("VTSParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return;
        }

        bDeleteRoadIndex = false;
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                int nChannelWayId = xmltoi(strText);
                int nSize = strChannelWays.size()/sizeof(int);
                bool bExist = false;
                for(int j=0;j<nSize;j++)
                {
                    int nRoadIndex = *((int*)(strChannelWays.c_str()+j*sizeof(int)));
                    if(nRoadIndex==nChannelWayId)
                    {
                        bExist = true;
                        break;
                    }
                }
                if(!bExist)
                {
                    ChannelWayNode.deleteNodeContent();
                    bDeleteRoadIndex = true;
                }
            }
        }

        if(bDeleteRoadIndex)
        {
            XmlNode.writeToFile(strVTSParameterXml.c_str());
        }
	}

	//更新事件检测参数
    std::string strRoadParameterXml;

    if((g_nMultiPreSet == 0) && (g_nSwitchCamera == 0))
    strRoadParameterXml = "./config/RoadParameter.xml";
    else
    {
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/RoadParameter.xml",nChannel);
        else
        sprintf(buf,"./config/Camera%d/RoadParameter-PreSet%d.xml",nChannel,nPreSet);

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        {
            strRoadParameterXml = strPath;
        }
    }

	if(access(strRoadParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strRoadParameterXml.c_str()).getChildNode("RoadParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return;
        }

        bDeleteRoadIndex = false;
        ChannelWaysNode = ChannelNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                int nChannelWayId = xmltoi(strText);
                int nSize = strChannelWays.size()/sizeof(int);
                bool bExist = false;
                for(int j=0;j<nSize;j++)
                {
                    int nRoadIndex = *((int*)(strChannelWays.c_str()+j*sizeof(int)));
                    if(nRoadIndex==nChannelWayId)
                    {
                        bExist = true;
                        break;
                    }
                }
                if(!bExist)
                {
                    ChannelWayNode.deleteNodeContent();
                    bDeleteRoadIndex = true;
                }
            }
        }
        if(bDeleteRoadIndex)
        {
            if(XmlNode.writeToFile(strRoadParameterXml.c_str()))
            {
          /*      if((g_nMultiPreSet == 1) || (g_nSwitchCamera == 1))
                {
                    if(g_nServerType == 1)
                    g_AMSCommunication.SendSettingsToCS(nChannel,nPreSet,1);
                }*/
            }
        }
	}
}

//获取裁剪区域坐标(数据库中存放的是jpg图像坐标)
RegionCoordinate CXmlParaUtil::GetClipRegionCoordinate(int nChannel)
{
    RegionCoordinate ord;

    LIST_CHANNEL_INFO list_channel_info;
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        bool bLoadViolationArea = false;
        bool bLoadEventArea = false;
        bool bLoadTrafficSignalArea = false;
        int i = 0;

        while(it_b != it_e)
        {
                CHANNEL_INFO channel_info = *it_b;
                //违章检测区域
                if(!bLoadViolationArea)
                {
                    it_rb = channel_info.ViolationRegion.listRegionProp.begin();
                    it_re = channel_info.ViolationRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            if(i==0)
                            {
                                ord.uViolationX = item_b->x;
                                ord.uViolationY = item_b->y;
                            }
                            else if(i == 2)
                            {
                                 ord.uViolationWidth  = item_b->x - ord.uViolationX;
                                 ord.uViolationHeight = item_b->y - ord.uViolationY;
                            }
                            i++;
                        }
                    }
                    bLoadViolationArea = true;
                }
                //事件检测区域
                if(!bLoadEventArea)
                {
                    it_rb = channel_info.EventRegion.listRegionProp.begin();
                    it_re = channel_info.EventRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            if(i==0)
                            {
                                ord.uEventX = item_b->x;
                                ord.uEventY = item_b->y;
                            }
                            else if(i==2)
                            {
                                ord.uEventWidth  = item_b->x - ord.uEventX;
                                ord.uEventHeight = item_b->y - ord.uEventY;
                            }
                            i++;
                        }
                    }
                    bLoadEventArea = true;
                }
                //红灯检测区域
                if(!bLoadTrafficSignalArea)
                {
                    it_rb = channel_info.TrafficSignalRegion.listRegionProp.begin();
                    it_re = channel_info.TrafficSignalRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            if(i==0)
                            {
                                ord.uTrafficSignalX = item_b->x;
                                ord.uTrafficSignalY = item_b->y;
                            }
                            else if(i==2)
                            {
                                ord.uTrafficSignalWidth  = item_b->x - ord.uTrafficSignalX;
                                ord.uTrafficSignalHeight = item_b->y - ord.uTrafficSignalY;
                            }
                            i++;
                        }
                     }
                    bLoadTrafficSignalArea = true;
                }
                it_b++;
        }
    }
    return ord;
}

//获取稳像区域
void CXmlParaUtil::GetStabBackRegion(int nChannel,RegionList& listStabBack)
{
    listStabBack.clear();
    LIST_CHANNEL_INFO list_channel_info;
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        bool bStabBackArea = false;
        CPoint32f pt32f;

        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;
            //稳像背景区域
            if(!bStabBackArea)
            {
                it_rb = channel_info.StabBackRegion.listRegionProp.begin();
                it_re = channel_info.StabBackRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    Rgn	region;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x);
                        pt32f.y = (item_b->y);
                        region.pt32fList.push_back(pt32f);
                        printf("StabBackRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    }
                    listStabBack.push_back(region);
                }
                bStabBackArea = true;
            }
            it_b++;
        }
    }
}


//获取车道区域,虚拟检测区域以及车牌区域宽高
void CXmlParaUtil::GetVirtualLoopRegion(int nChannel,vector<mvvideorgnstru>& mChannelReg,CvRect& VideoRect,CvRect& rtCarnumber)
{
    mChannelReg.clear();
    VideoRect.width = 0;
    VideoRect.height = 0;
    rtCarnumber.width = 0;
    rtCarnumber.height = 0;

    LIST_CHANNEL_INFO list_channel_info;
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        CvPoint pt,pt1,pt2;
        CvPoint2D32f ptImage;

        bool bVirtualLoop = false;
        bool bLoadCarnumber = false;

        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;

            if(!bVirtualLoop)
            {
                it_rb = channel_info.VirtualLoopRegion.listRegionProp.begin();
                it_re = channel_info.VirtualLoopRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();

                    if(it_rb->listPt.size() >= 4)
                    {
                        int i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x);
                            pt.y = (int) (item_b->y);

                            if(i == 0)
                            {
                                pt1.x = pt.x;
                                pt1.y = pt.y;
                                pt2.x = pt.x;
                                pt2.y = pt.y;
                            }
                            else
                            {
                                if(pt1.x>pt.x)
                                {
                                    pt1.x=pt.x;
                                }
                                if(pt1.y>pt.y)
                                {
                                    pt1.y=pt.y;
                                }
                                if(pt2.x<pt.x)
                                {
                                    pt2.x=pt.x;
                                }
                                if(pt2.y<pt.y)
                                {
                                    pt2.y=pt.y;
                                }
                            }
                           i++;
                        }

                        VideoRect.x = (pt1.x);
                        VideoRect.y = (pt1.y);
                        VideoRect.width = pt2.x - pt1.x;
                        VideoRect.height = pt2.y - pt1.y;
                    }
                }
                bVirtualLoop = true;
            }

            if(!bLoadCarnumber)
            {
                it_rb = channel_info.CardnumberRegion.listRegionProp.begin();
                it_re = channel_info.CardnumberRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();

                    if(it_rb->listPt.size() >= 4)
                    {
                        int i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x);
                            pt.y = (int) (item_b->y);

                            if(i == 0)
                            {
                                pt1.x = pt.x;
                                pt1.y = pt.y;
                                pt2.x = pt.x;
                                pt2.y = pt.y;
                            }
                            else
                            {
                                if(pt1.x>pt.x)
                                {
                                    pt1.x=pt.x;
                                }
                                if(pt1.y>pt.y)
                                {
                                    pt1.y=pt.y;
                                }
                                if(pt2.x<pt.x)
                                {
                                    pt2.x=pt.x;
                                }
                                if(pt2.y<pt.y)
                                {
                                    pt2.y=pt.y;
                                }
                            }
                           i++;
                        }

                        rtCarnumber.x = (pt1.x);
                        rtCarnumber.y = (pt1.y);
                        rtCarnumber.width = pt2.x - pt1.x;
                        rtCarnumber.height = pt2.y - pt1.y;
                    }
                }
                bLoadCarnumber = true;
            }

            //车道区域
            mvvideorgnstru videorgnstru;
            item_b = channel_info.chRegion.listPT.begin();
            item_e = channel_info.chRegion.listPT.end();
            for(; item_b != item_e; item_b++)
            {
                ptImage.x = item_b->x;
                ptImage.y = item_b->y;
                videorgnstru.pPoints.push_back(ptImage);
            }
            videorgnstru.nChannel = channel_info.chProp_index.value.nValue;
            mChannelReg.push_back(videorgnstru);

            it_b++;
        }
    }
}

//获取同步区域
void CXmlParaUtil::GetSynchRegion(TRACK_GROUP& sLeftTrack,TRACK_GROUP& sRightTrack,CvRect& rtDetectArea)
{
    sLeftTrack.clear();
    sRightTrack.clear();
    //nDetectAreaCenter = 0;
    LIST_CHANNEL_INFO list_channel_info;
    if(LoadRoadSettingInfo(list_channel_info,1))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        bool bSynchLeftArea = false;
        bool bSynchRightArea = false;
        bool bLoadCardArea = false;
        CvPoint pt,pt1,pt2;

        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;
            //左同步区域
            if(!bSynchLeftArea)
            {
                it_rb = channel_info.SynchLeftRegion.listRegionProp.begin();
                it_re = channel_info.SynchLeftRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();

                    for( ; item_b!=item_e; item_b++)
                    {
                        pt.x = (item_b->x);
                        pt.y = (item_b->y);
                        sLeftTrack.push_back(pt);
                    }
                }
                bSynchLeftArea = true;
            }
            //右同步区域
            if(!bSynchRightArea)
            {
                it_rb = channel_info.SynchRightRegion.listRegionProp.begin();
                it_re = channel_info.SynchRightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();

                    for( ; item_b!=item_e; item_b++)
                    {
                        pt.x = (item_b->x);
                        pt.y = (item_b->y);
                        sRightTrack.push_back(pt);
                    }
                }
                bSynchRightArea = true;
            }
            //车牌区域
            if(!bLoadCardArea)
            {
                it_rb = channel_info.carnumRegion.listRegionProp.begin();
                it_re = channel_info.carnumRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    int i = 0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt.x = (item_b->x);
                        pt.y = (item_b->y);
                        if(i==0)
                        {
                            pt1.x = pt.x;
                            pt1.y = pt.y;
                            pt2.x = pt.x;
                            pt2.y = pt.y;
                        }
                        else
                        {
                            if(pt1.x>pt.x)
                            {
                                pt1.x=pt.x;
                            }
                            if(pt1.y>pt.y)
                            {
                                pt1.y=pt.y;
                            }
                            if(pt2.x<pt.x)
                            {
                                pt2.x=pt.x;
                            }
                            if(pt2.y<pt.y)
                            {
                                pt2.y=pt.y;
                            }
                        }
                        i++;
                    }
                    //nDetectAreaCenter = (pt1.x+pt2.x)/2.0;
                    rtDetectArea.x = pt1.x;
                    rtDetectArea.y = pt1.y;
                    rtDetectArea.width = pt2.x - pt1.x;
                    rtDetectArea.height = pt2.y - pt1.y;
                }
                bLoadCardArea = true;
            }
            it_b++;
        }
    }
}


//获取车道标定信息
bool CXmlParaUtil::GetCalibrationAndDirectionInfo(int nChannel,CALIBRATION& calibration,CHANNEL_PROPERTY& chDirection)
{
    LIST_CHANNEL_INFO list_channel_info;
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        bool bCalibration = false;

        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;
            //标定信息
            if(!bCalibration)
            {
                calibration =  channel_info.calibration;
                chDirection = channel_info.chProp_direction;
                bCalibration = true;
                return true;
                break;
            }
            it_b++;
        }
    }
    return false;
}

//设置停车检测区域
bool CXmlParaUtil::SetStopRegion(int nChannel,string strStopRegion)
{
    int nSize = strStopRegion.size();

    LIST_CHANNEL_INFO list_channel_info;//车道坐标设置
    paraDetectList sFrameList; //车道参数设置

    SRIP_CHANNEL_EXT sChannel;
    sChannel.uId = nChannel;
     //检测控制参数
    paraDetectList sParamInList;
    LoadRoadParameter(sParamInList,sChannel);

    int i = 0;
    int nIndex = 1;
    char buf[128] = {0};

    //标点区域
    CALIBRATION calibration;
    CHANNEL_PROPERTY chDirection;
    bool bRet = GetCalibrationAndDirectionInfo(nChannel,calibration,chDirection);
    if(!bRet)
    {
        return false;
    }

    while(i < nSize)
    {
        //////////////////先读每个检测区域的交点个数
        UINT32 nCoordinNum = *((UINT32*)(strStopRegion.c_str()+i));
        i += sizeof(UINT32);

        if(nCoordinNum < 4)
        return false;

        CHANNEL_INFO Channel_info;
        //车道的序号
        strcpy(Channel_info.chProp_index.strName, "CHANNEL_INDEX");
        Channel_info.chProp_index.value.nValue = nIndex;

        //车道名称
        strcpy(Channel_info.chProp_name.strName, "CHANNEL_NAME");
        sprintf(buf,"%d",nIndex);
        string strRoadName(buf);
        strcpy(Channel_info.chProp_name.value.strValue , strRoadName.c_str());

        //标点区域
        Channel_info.calibration = calibration;

        //方向区域
        Channel_info.chProp_direction = chDirection;

        ///////////////////////////再读每个交点的坐标
        //道路区域
        strcpy(Channel_info.roadRegion.chProperty.strName, "ROAD_REGION");
        Channel_info.roadRegion.chProperty.value.nValue = nCoordinNum;

        //车道区域
        strcpy(Channel_info.chRegion.chProperty.strName, "CHANNEL_REGION");
        Channel_info.chRegion.chProperty.value.nValue = nCoordinNum;

        //停车区域
        strcpy(Channel_info.stopRegion.chProperty.strName, "STOP_REGION");
        Channel_info.stopRegion.chProperty.value.nValue = nCoordinNum;

        REGION_PROPERTY region_property;
        region_property.nValue = nCoordinNum;//至少3个点
        for(int j = 0; j < nCoordinNum; j++)
        {
            CPoint32f pt;
            pt.x = *((UINT32*)(strStopRegion.c_str()+i));
            pt.y = *((UINT32*)(strStopRegion.c_str()+i+sizeof(UINT32)));
            i += 2*sizeof(UINT32);

            Channel_info.roadRegion.listPT.push_back(pt);
            Channel_info.chRegion.listPT.push_back(pt);
            region_property.listPt.push_back(pt);
        }
        Channel_info.stopRegion.listRegionProp.push_back(region_property);

        list_channel_info.push_back(Channel_info);


        /* 车道检测参数		*/
        VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;
        sFrame.nChannelID = nIndex;
        sFrame.m_bStop = true; //停车检测
        sFrame.nModelId = 0;
        sFrameList.push_back(sFrame);

        nIndex++;
    }

    //如果存在多个预置位
    if(g_nMultiPreSet == 1)
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        int nPreSet = g_skpDB.GetPreSet(nChannel);
        if(nPreSet >= 0)
        {
            if(AddDeviceSettingInfo(list_channel_info,nCameraID,nPreSet))
            {
                AddDeviceParaMeterInfo(sFrameList,sChannel,nCameraID,nPreSet);
            }
        }
    }
    else
    {
        if(AddRoadSettingInfoByList(list_channel_info, nChannel))
        {
            AddRoadParameterByList(sFrameList,sChannel);
        }
    }

    //要求检测重新读取配置
    g_skpChannelCenter.ReloadDetect(nChannel);

    return true;
}

//获取停车检测区域
bool CXmlParaUtil::GetStopRegion(int nChannel,string& strStopRegion)
{
    LIST_CHANNEL_INFO list_channel_info;
    //载入车道参数
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;

             //停车检测区域
            it_rb = channel_info.stopRegion.listRegionProp.begin();
            it_re = channel_info.stopRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();

                UINT32 nSize = it_rb->listPt.size();
                strStopRegion.append((char*)&nSize,sizeof(UINT32));
                for( ; item_b!=item_e; item_b++)
                {
                    UINT32 x = (item_b->x);
                    UINT32 y = (item_b->y);
                    printf("stopRegion pt.x=%d,pt.y=%d\n",x,y);
                    strStopRegion.append((char*)&x,sizeof(UINT32));
                    strStopRegion.append((char*)&y,sizeof(UINT32));
                }
            }

            it_b++;
        }
        return true;
    }
    return false;
}

//设置行为分析参数
bool CXmlParaUtil::SetBehaviorParameter(int nChannel,string strBehaviorPara)
{
    XMLNode XmlNode,ChannelsNode,ChannelNode,TempNode,SettingInfoNode;
    XMLCSTR strTag;

    std::string strBehaviorParameterXml = "./config/BehaviorParameterInfo.xml";
	if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strBehaviorParameterXml = "./config/BehaviorParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/BehaviorParameterInfo.xml",nCameraID);
		else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/BehaviorParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }
		strBehaviorParameterXml = buf;
	 }


    if(access(strBehaviorParameterXml.c_str(),F_OK) != 0)
    {
        XmlNode = XMLNode::createXMLTopNode("BehaviorParameter");
        ChannelsNode = XmlNode.addChild("Channels");
    }
    else
    {
        XmlNode = XMLNode::parseFile(strBehaviorParameterXml.c_str()).getChildNode("BehaviorParameter");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(!ChannelsNode.isEmpty())
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }

    char buf[64] = {0};
    ChannelNode = ChannelsNode.addChild("Channel");
    TempNode = ChannelNode.addChild("ChannelId");
    sprintf(buf, "%d", nChannel);
    TempNode.addText(buf);

    SettingInfoNode = XMLNode::parseString(strBehaviorPara.c_str());
    ChannelNode.addChild(SettingInfoNode);

    return XmlNode.writeToFile(strBehaviorParameterXml.c_str());
}

//获取行为分析参数设置
bool CXmlParaUtil::GetBehaviorParameter(int nChannel,string& strBehaviorPara)
{
    char buf[64] = {0};

    SRIP_CHANNEL_EXT sChannel;
    sChannel.uId = nChannel;

    paraBehaviorList listParaBehavior;

    if(LoadBehaviorParameter(listParaBehavior,sChannel))
    {
        XMLNode SettingInfoNode,GlobalNode,ChannelWaysNode,ChannelWayNode,TempNode;
        XMLCSTR strText;

        SettingInfoNode = XMLNode::createXMLTopNode("Setting");

        GlobalNode = SettingInfoNode.addChild("Global");

        TempNode = GlobalNode.addChild("ShowTime");
        sprintf(buf,"%d",sChannel.nShowTime);
        TempNode.addText(buf);

        TempNode = GlobalNode.addChild("TrafficStatTime");
        sprintf(buf,"%d",sChannel.uTrafficStatTime);
        TempNode.addText(buf);


        ChannelWaysNode = SettingInfoNode.addChild("ChannelWays");

        paraBehaviorList::iterator it_b = listParaBehavior.begin();
        paraBehaviorList::iterator it_e = listParaBehavior.end();

        while(it_b != it_e)
        {
            BEHAVIOR_PARAM ParaBehavior;

            ParaBehavior = *it_b;

            ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");

            TempNode = ChannelWayNode.addChild("ChannelWayId");
            sprintf(buf,"%d",ParaBehavior.nIndex);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("BargeIn");
            sprintf(buf,"%d",ParaBehavior.bInside_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("BeyondMark");
            sprintf(buf,"%d",ParaBehavior.bOutside_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("Crowd");
            sprintf(buf,"%d",ParaBehavior.bJam_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("PersonCount");
            sprintf(buf,"%d",ParaBehavior.nPeopleNumberJam);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("Run");
            sprintf(buf,"%d",ParaBehavior.bRun_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("MaxSpeedRun");
            sprintf(buf,"%.2f",ParaBehavior.fRunThreshold);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("Passerby");
            sprintf(buf,"%d",ParaBehavior.bPasserby_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("Derelict");
            sprintf(buf,"%d",ParaBehavior.bDerelict_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("PersonStop");
            sprintf(buf,"%d",ParaBehavior.bStoop_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("PersonAgainst");
            sprintf(buf,"%d",ParaBehavior.bAgainst_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("PersonAppear");
            sprintf(buf,"%d",ParaBehavior.bAppear_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("StatFlux");
            sprintf(buf,"%d",ParaBehavior.bStatFlux_event);
            TempNode.addText(buf);


             TempNode = ChannelWayNode.addChild("ContextDetect");
            sprintf(buf,"%d",ParaBehavior.bContext_event);
            TempNode.addText(buf);


             TempNode = ChannelWayNode.addChild("SmokeDetect");
            sprintf(buf,"%d",ParaBehavior.bSmoke_event);
            TempNode.addText(buf);


             TempNode = ChannelWayNode.addChild("FireDetect");
            sprintf(buf,"%d",ParaBehavior.bFire_event);
            TempNode.addText(buf);


             TempNode = ChannelWayNode.addChild("FightDetect");
            sprintf(buf,"%d",ParaBehavior.bFight_event);
            TempNode.addText(buf);

             TempNode = ChannelWayNode.addChild("LeafletDetect");
            sprintf(buf,"%d",ParaBehavior.bLeafLet_event);
            TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("DensityDetect");
            sprintf(buf,"%d",ParaBehavior.bDensity_event);
            TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("LoiteringDetect");
			sprintf(buf,"%d",ParaBehavior.bLoitering_event);
			TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("RemovedItemDetect");
			sprintf(buf,"%d",ParaBehavior.bRemovedItem_event);
			TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("ImpropriateDetect");
			sprintf(buf,"%d",ParaBehavior.bImpropriate_event);
			TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("SeeperDetect");
			sprintf(buf,"%d",ParaBehavior.bSeeper_event);
			TempNode.addText(buf);

			TempNode = ChannelWayNode.addChild("TailgatingDetect");
			sprintf(buf,"%d",ParaBehavior.bTailgating_event);
			TempNode.addText(buf);


            it_b++;
        }


        int nSize;
        XMLSTR strData = SettingInfoNode.createXMLString(1, &nSize);
        if(strData)
        {
            strBehaviorPara.append(strData, sizeof(XMLCHAR)*nSize);
            freeXMLString(strData);
        }
        return true;
    }
    return false;
}

//载入行为分析参数
bool CXmlParaUtil::LoadBehaviorParameter(paraBehaviorList &listParaBehavior,SRIP_CHANNEL_EXT& sChannel)
{
    int nChannel = sChannel.uId;
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, SettingNode,GlobalNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

    std::string strChannelWays;
    GetChannelWaysFromRoadSettingInfo(nChannel,strChannelWays);
    int nSize = strChannelWays.size()/sizeof(int);
    for(int i =0;i<nSize;i++)
    {
        BEHAVIOR_PARAM ParaBehavior;
        ParaBehavior.nIndex = *((int*)(strChannelWays.c_str()+sizeof(int)*i));

        listParaBehavior.push_back(ParaBehavior);
    }

    std::string strBehaviorParameterXml;
    //如果需要切换相机
    if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strBehaviorParameterXml = "./config/BehaviorParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/BehaviorParameterInfo.xml",nCameraID);
        else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/BehaviorParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        {
            nChannel = nCameraID;
            strBehaviorParameterXml = strPath;
        }
        else
        {
                return false;
        }
    }

    printf("strBehaviorParameterXml.c_str()=%s,nChannel=%d\n",strBehaviorParameterXml.c_str(),nChannel);
	if(access(strBehaviorParameterXml.c_str(), F_OK) == 0) //存在
	{
	    printf("11 strBehaviorParameterXml.c_str()=%s,nChannel=%d\n",strBehaviorParameterXml.c_str(),nChannel);
        //xml文件
		XmlNode = XMLNode::parseFile(strBehaviorParameterXml.c_str()).getChildNode("BehaviorParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
               strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }
            else
            {
                printf("======RoadParameter.xml:====The ChannelNode does't have ChildNode of ChannelId========!!!\n");
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        SettingNode = ChannelNode.getChildNode("Setting");

        //全局参数
        GlobalNode = SettingNode.getChildNode("Global");
        TempNode = GlobalNode.getChildNode("TrafficStatTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.uTrafficStatTime = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("ShowTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            sChannel.nShowTime = xmltoi(strText);
        }

        printf("sChannel.nShowTime=%d\n",sChannel.nShowTime);
        /////////////////////////////////////
        ChannelWaysNode = SettingNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            BEHAVIOR_PARAM ParaBehavior;

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                ParaBehavior.nIndex = xmltoi(strText);

                LoadBehaviorParameter(ChannelWayNode,ParaBehavior);

                paraBehaviorList::iterator it_b = listParaBehavior.begin();
                paraBehaviorList::iterator it_e = listParaBehavior.end();
                while(it_b != it_e)
                {
                    if((it_b->nIndex>0)&&(it_b->nIndex == ParaBehavior.nIndex))
                    {
                        *it_b = ParaBehavior;
                        break;
                    }
                    it_b++;
                }
            }
        }
        return true;
	}
	return false;
}

//载入车道检测参数
bool CXmlParaUtil::LoadBehaviorParameter(XMLNode &ParentNode,BEHAVIOR_PARAM &ParaBehavior)
{
    XMLNode XmlNode;
	XMLNode TempNode;
	XMLNode TempNode1, TempNode2;
	XMLCSTR strText;
    {
        TempNode = ParentNode.getChildNode("BargeIn");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bInside_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("BeyondMark");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bOutside_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("Crowd");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bJam_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonCount");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.nPeopleNumberJam = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("Run");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bRun_event = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("MaxSpeedRun");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.fRunThreshold = xmltof(strText);
        }

        TempNode = ParentNode.getChildNode("Passerby");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bPasserby_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("Derelict");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bDerelict_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonStop");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bStoop_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonAgainst");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bAgainst_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("PersonAppear");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bAppear_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("StatFlux");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bStatFlux_event = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("ContextDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bContext_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("SmokeDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bSmoke_event = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("FireDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bFire_event = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("FightDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bFight_event = xmltoi(strText);
        }


        TempNode = ParentNode.getChildNode("LeafletDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bLeafLet_event = xmltof(strText);
        }

		TempNode = ParentNode.getChildNode("DensityDetect");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            ParaBehavior.bDensity_event = xmltof(strText);
        }

		TempNode = ParentNode.getChildNode("LoiteringDetect");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			ParaBehavior.bLoitering_event = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("RemovedItemDetect");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			ParaBehavior.bRemovedItem_event = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("ImpropriateDetect");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			ParaBehavior.bImpropriate_event = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("SeeperDetect");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			ParaBehavior.bSeeper_event = xmltof(strText);
		}

		TempNode = ParentNode.getChildNode("TailgatingDetect");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			ParaBehavior.bTailgating_event = xmltof(strText);
		}


        return true;
    } //End of if

}

//设置目标参数
bool CXmlParaUtil::SetObjectParameter(int nChannel,string strObjectPara)
{
    XMLNode XmlNode,ChannelsNode,ChannelNode,TempNode,SettingInfoNode;
    XMLCSTR strTag;

    std::string strObjectParameterXml = "./config/ObjectParameterInfo.xml";
	if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strObjectParameterXml = "./config/ObjectParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/ObjectParameterInfo.xml",nCameraID);
		else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/ObjectParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }
		strObjectParameterXml = buf;
	 }

    if(access(strObjectParameterXml.c_str(),F_OK) != 0)
    {
        XmlNode = XMLNode::createXMLTopNode("ObjectParameter");
        ChannelsNode = XmlNode.addChild("Channels");
    }
    else
    {
        XmlNode = XMLNode::parseFile(strObjectParameterXml.c_str()).getChildNode("ObjectParameter");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(!ChannelsNode.isEmpty())
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }

    char buf[64] = {0};
    ChannelNode = ChannelsNode.addChild("Channel");
    TempNode = ChannelNode.addChild("ChannelId");
    sprintf(buf, "%d", nChannel);
    TempNode.addText(buf);

    SettingInfoNode = XMLNode::parseString(strObjectPara.c_str());
    ChannelNode.addChild(SettingInfoNode);

    return XmlNode.writeToFile(strObjectParameterXml.c_str());
}

//客户端获取目标检测参数
bool CXmlParaUtil::GetObjectParameter(int nChannel,string& strObjectPara)
{
    char buf[64] = {0};

    _ObjectPara ObjectPara;
    if(LoadObjectParameter(nChannel,ObjectPara))
    {
        XMLNode SettingInfoNode,GlobalNode,ChannelWaysNode,ChannelWayNode,TempNode;
        XMLCSTR strText;

        SettingInfoNode = XMLNode::createXMLTopNode("Setting");

        GlobalNode = SettingInfoNode.addChild("Global");

        TempNode = GlobalNode.addChild("DropNonAutoMobile");
        sprintf(buf,"%d",ObjectPara.bOutPutNonAutoMobile);
        TempNode.addText(buf);

        TempNode = GlobalNode.addChild("DetectNonVehicle");
        sprintf(buf,"%d",ObjectPara.bDetectNonVehicle);
        TempNode.addText(buf);

        TempNode = GlobalNode.addChild("FilterSideVehicle");
        sprintf(buf,"%d",ObjectPara.bFilterSideVehicle);
        TempNode.addText(buf);

        TempNode = GlobalNode.addChild("FilterSideVlpObj");
        sprintf(buf,"%d",ObjectPara.bFilterSideVlpObj);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("DetectNonPlate");
        sprintf(buf,"%d",ObjectPara.bDetectNonPlate);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("DetectShield");
        sprintf(buf,"%d",ObjectPara.bDetectShield);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("ImageEnhance");
        sprintf(buf,"%d",ObjectPara.bImageEnhance);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("ImageEnhanceFactor");
        sprintf(buf,"%d",ObjectPara.nImageEnhanceFactor);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("DoMotorCar");
        sprintf(buf,"%d",ObjectPara.bDoMotorCar);
        TempNode.addText(buf);

		TempNode = GlobalNode.addChild("RedLightViolationAllowance");
		sprintf(buf,"%d",ObjectPara.nRedLightViolationAllowance);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("FramePlus");
		sprintf(buf,"%d",ObjectPara.nFramePlus);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("LoopVehicleLenFix");
		sprintf(buf,"%f",ObjectPara.fLoopVehicleLenFix);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("FarRedLightVioPic");
		sprintf(buf,"%f",ObjectPara.fFarRedLightVioPic);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("NearRedLightVioPic");
		sprintf(buf,"%f",ObjectPara.fNearRedLightVioPic);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("DelayFrame");
		sprintf(buf,"%d",ObjectPara.nDelayFrame);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("PressLineScale");
		sprintf(buf,"%f",ObjectPara.fPressLineScale);
		TempNode.addText(buf);

		TempNode = GlobalNode.addChild("StrongEnhance");
		sprintf(buf,"%d",ObjectPara.nStrongEnhance);
		TempNode.addText(buf);

        int nSize;
        XMLSTR strData = SettingInfoNode.createXMLString(1, &nSize);
        if(strData)
        {
            strObjectPara.append(strData, sizeof(XMLCHAR)*nSize);
            freeXMLString(strData);
        }

        return true;
    }
    return false;
}

//获取目标检测参数
bool CXmlParaUtil::LoadObjectParameter(int nChannel,_ObjectPara& ObjectPara)
{
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode,SettingNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

	std::string strObjectParameterXml = "./config/ObjectParameterInfo.xml";
	//如果需要切换相机
    if( (g_nSwitchCamera == 0) && (g_nMultiPreSet == 0))
    {
        strObjectParameterXml = "./config/ObjectParameterInfo.xml";
    }
    else
    {
        int nCameraID = g_skpDB.GetCameraID(nChannel);
        char buf[256]={0};

        if(g_nMultiPreSet == 0)
        sprintf(buf,"./config/Camera%d/ObjectParameterInfo.xml",nCameraID);
        else//如果存在多个预置位
        {
            int nPreSet = g_skpDB.GetPreSet(nChannel);
            sprintf(buf,"./config/Camera%d/ObjectParameterInfo-PreSet%d.xml",nCameraID,nPreSet);
        }

        std::string strPath(buf);

        if(access(strPath.c_str(),0) == 0)
        {
            nChannel = nCameraID;
            strObjectParameterXml = strPath;
        }
        else
        {
                return false;
        }
    }
	if(access(strObjectParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strObjectParameterXml.c_str()).getChildNode("ObjectParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        SettingNode = ChannelNode.getChildNode("Setting");
        /////////////////////////////////////
        GlobalNode = SettingNode.getChildNode("Global");

        TempNode = GlobalNode.getChildNode("DropNonAutoMobile");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bOutPutNonAutoMobile = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("DetectNonVehicle");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bDetectNonVehicle = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("FilterSideVehicle");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bFilterSideVehicle = xmltoi(strText);
        }

        TempNode = GlobalNode.getChildNode("FilterSideVlpObj");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bFilterSideVlpObj = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("DetectNonPlate");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bDetectNonPlate = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("DetectShield");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bDetectShield = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("ImageEnhance");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bImageEnhance = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("ImageEnhanceFactor");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.nImageEnhanceFactor = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("DoMotorCar");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            ObjectPara.bDoMotorCar = xmltoi(strText);
        }

		TempNode = GlobalNode.getChildNode("RedLightViolationAllowance");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.nRedLightViolationAllowance = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("FramePlus");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.nFramePlus = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("LoopVehicleLenFix");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.fLoopVehicleLenFix = xmltof(strText);
		}

		TempNode = GlobalNode.getChildNode("FarRedLightVioPic");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.fFarRedLightVioPic = xmltof(strText);
		}

		TempNode = GlobalNode.getChildNode("NearRedLightVioPic");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.fNearRedLightVioPic = xmltof(strText);
		}

		TempNode = GlobalNode.getChildNode("DelayFrame");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.nDelayFrame = xmltoi(strText);
		}

		TempNode = GlobalNode.getChildNode("PressLineScale");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.fPressLineScale = xmltof(strText);
		}

		TempNode = GlobalNode.getChildNode("StrongEnhance");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
				ObjectPara.nStrongEnhance = xmltoi(strText);
		}

        return true;
	}
    return false;
}

//设置雷达参数
bool CXmlParaUtil::SetRadarParameter(int nChannel,string strRadarPara)
{
    XMLNode XmlNode,ChannelsNode,ChannelNode,TempNode,SettingInfoNode;
    XMLCSTR strTag;

    std::string strRadarParameterXml = "./config/RadarParameterInfo.xml";

    if(access(strRadarParameterXml.c_str(),F_OK) != 0)
    {
        XmlNode = XMLNode::createXMLTopNode("RadarParameter");
        ChannelsNode = XmlNode.addChild("Channels");
    }
    else
    {
        XmlNode = XMLNode::parseFile(strRadarParameterXml.c_str()).getChildNode("RadarParameter");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(!ChannelsNode.isEmpty())
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }

    char buf[64] = {0};
    ChannelNode = ChannelsNode.addChild("Channel");
    TempNode = ChannelNode.addChild("ChannelId");
    sprintf(buf, "%d", nChannel);
    TempNode.addText(buf);

    SettingInfoNode = XMLNode::parseString(strRadarPara.c_str());
    ChannelNode.addChild(SettingInfoNode);

    return XmlNode.writeToFile(strRadarParameterXml.c_str());
}


//客户端获取雷达检测参数
bool CXmlParaUtil::GetRadarParameter(int nChannel,string& strRadarPara)
{
    char buf[64] = {0};
    RadarParaMap MapRadarPara;
    if(LoadRadarParameter(nChannel,MapRadarPara))
    {
        XMLNode SettingInfoNode,GlobalNode,ChannelWaysNode,ChannelWayNode,TempNode;
        XMLCSTR strText;

        SettingInfoNode = XMLNode::createXMLTopNode("Setting");

        GlobalNode = SettingInfoNode.addChild("Global");
        ChannelWaysNode = SettingInfoNode.addChild("ChannelWays");

        int nCoverRoadIndex = 0;
        float fSpeedFactor = 0.0;

        RadarParaMap::iterator it_b = MapRadarPara.begin();
        RadarParaMap::iterator it_e = MapRadarPara.end();

        while(it_b != it_e)
        {
            RADAR_PARAMETER radarPara;

            radarPara = it_b->second;
            nCoverRoadIndex = radarPara.nCoverRoadIndex;
            printf("=======nCoverRoadIndex=%d\n",nCoverRoadIndex);

            fSpeedFactor = radarPara.fSpeedFactor;
            printf("=======fSpeedFactor=%f\n",fSpeedFactor);

            ChannelWayNode = ChannelWaysNode.addChild("ChannelWay");

            TempNode = ChannelWayNode.addChild("ChannelWayId");
            sprintf(buf,"%d",radarPara.nRoadIndex);
            TempNode.addText(buf);

            it_b++;
        }

        TempNode = GlobalNode.addChild("CoverRoadIndex");
        sprintf(buf,"%d",nCoverRoadIndex);
        TempNode.addText(buf);

        TempNode = GlobalNode.addChild("SpeedFactor");
        sprintf(buf,"%f",fSpeedFactor);
        TempNode.addText(buf);

        int nSize;
        XMLSTR strData = SettingInfoNode.createXMLString(1, &nSize);
        if(strData)
        {
            strRadarPara.append(strData, sizeof(XMLCHAR)*nSize);
            freeXMLString(strData);
        }

        return true;
    }
    return false;
}

//设置图片格式参数
bool CXmlParaUtil::SetPicFormatInfo(int nChannel,string strPicFormatInfo)
{
    XMLNode XmlNode,ChannelsNode,ChannelNode,TempNode,SettingInfoNode;
    XMLCSTR strTag;

    std::string strPicFormatInfoXml = "./config/PicFormatInfo.xml";

    if(access(strPicFormatInfoXml.c_str(),F_OK) != 0)
    {
        XmlNode = XMLNode::createXMLTopNode("PicFormatInfo");
        ChannelsNode = XmlNode.addChild("Channels");
    }
    else
    {
        XmlNode = XMLNode::parseFile(strPicFormatInfoXml.c_str()).getChildNode("PicFormatInfo");
        ChannelsNode = XmlNode.getChildNode("Channels");

        if(!ChannelsNode.isEmpty())
        {
            //如果存在则删除该通道下的所有节点
            DeleteChannelNode(ChannelsNode,nChannel);
        }
    }

    char buf[64] = {0};
    ChannelNode = ChannelsNode.addChild("Channel");
    TempNode = ChannelNode.addChild("ChannelId");
    sprintf(buf, "%d", nChannel);
    TempNode.addText(buf);

    SettingInfoNode = XMLNode::parseString(strPicFormatInfo.c_str());
    ChannelNode.addChild(SettingInfoNode);

    return XmlNode.writeToFile(strPicFormatInfoXml.c_str());
}


//客户端获取图片格式参数
bool CXmlParaUtil::GetPicFormatInfo(int nChannel,string& strPicFormatInfo)
{
    char buf[64] = {0};
    REGION_ROAD_CODE_INFO picFormatInfo;
    if(LoadPicFormatInfo(nChannel,picFormatInfo))
    {
        XMLNode SettingInfoNode,TempNode;
        XMLCSTR strText;

        SettingInfoNode = XMLNode::createXMLTopNode("Setting");

        TempNode = SettingInfoNode.addChild("WordOffSetX");
		sprintf(buf,"%d",picFormatInfo.nOffsetX);
        TempNode.addText(buf);

        TempNode = SettingInfoNode.addChild("WordOffSetY");
		sprintf(buf,"%d",picFormatInfo.nOffsetY);
        TempNode.addText(buf);


		TempNode = SettingInfoNode.addChild("RoadID");
		sprintf(buf,"%s",picFormatInfo.nRegionRoadCode);
		TempNode.addText(buf);

		TempNode = SettingInfoNode.addChild("RoadName");
		sprintf(buf,"%s",picFormatInfo.chRegionName);
		TempNode.addText(buf);

		TempNode = SettingInfoNode.addChild("BlackFrameWidth");
		sprintf(buf,"%d",picFormatInfo.nBlackFrameWidth);
		TempNode.addText(buf);

		TempNode = SettingInfoNode.addChild("FontSize");
		sprintf(buf,"%d",picFormatInfo.nFontSize);
		TempNode.addText(buf);


        int nSize;
        XMLSTR strData = SettingInfoNode.createXMLString(1, &nSize);
        if(strData)
        {
            strPicFormatInfo.append(strData, sizeof(XMLCHAR)*nSize);
            freeXMLString(strData);
        }

        return true;
    }
    return false;
}

//获取图片格式参数
bool CXmlParaUtil::LoadPicFormatInfo(int nChannel,REGION_ROAD_CODE_INFO& picFormatInfo)
{
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode,SettingNode;
	XMLNode TempNode;
	XMLCSTR strText;

	std::string strPicFormatInfoXml = "./config/PicFormatInfo.xml";
	if(access(strPicFormatInfoXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strPicFormatInfoXml.c_str()).getChildNode("PicFormatInfo");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        SettingNode = ChannelNode.getChildNode("Setting");
        /////////////////////////////////////

        TempNode = SettingNode.getChildNode("WordOffSetX");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            {
                picFormatInfo.nOffsetX = xmltof(strText);
            }
        }

		TempNode = SettingNode.getChildNode("WordOffSetY");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            {
                picFormatInfo.nOffsetY = xmltof(strText);
            }
        }

		TempNode = SettingNode.getChildNode("RoadID");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				string nStrCode = strText;
				memcpy(picFormatInfo.nRegionRoadCode,nStrCode.c_str(),nStrCode.size());
			}
		}

		TempNode = SettingNode.getChildNode("RoadName");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				string nStrCode = strText;
				memcpy(picFormatInfo.chRegionName,nStrCode.c_str(),nStrCode.size());
			}
		}

		TempNode = SettingNode.getChildNode("BlackFrameWidth");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				picFormatInfo.nBlackFrameWidth = xmltof(strText);
			}
		}

		TempNode = SettingNode.getChildNode("FontSize");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				picFormatInfo.nFontSize = xmltof(strText);
			}
		}

        return true;
	}
    return false;
}

//获取雷达检测参数
bool CXmlParaUtil::LoadRadarParameter(int nChannel,RadarParaMap& MapRadarPara)
{
    MapRadarPara.clear();
    //通道，车道，参数
	XMLNode XmlNode, ChannelsNode, ChannelWaysNode;
	XMLNode ChannelNode, ChannelWayNode, GlobalNode,SettingNode;
	XMLNode TempNode, TempNode1, TempNode2;
	XMLCSTR strText;

	std::string strChannelWays;
    GetChannelWaysFromRoadSettingInfo(nChannel,strChannelWays);
    int nSize = strChannelWays.size()/sizeof(int);
    for(int i =0;i<nSize;i++)
    {
        RADAR_PARAMETER radarPara;
        radarPara.nRoadIndex = *((int*)(strChannelWays.c_str()+sizeof(int)*i));
        radarPara.nCoverRoadIndex = 0;
        radarPara.fSpeedFactor = 0.0;
        MapRadarPara.insert(RadarParaMap::value_type(radarPara.nRoadIndex,radarPara));
    }

	std::string strRadarParameterXml = "./config/RadarParameterInfo.xml";
	if(access(strRadarParameterXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strRadarParameterXml.c_str()).getChildNode("RadarParameter");
        //通道
		ChannelsNode = XmlNode.getChildNode("Channels");
		int nChannels = ChannelsNode.nChildNode();

		/////////////////////////////////////找到当前节点
        bool bFindChannel = false;
        int iTemp = 0;
        int i = 0;
        while(i < nChannels)
        {
            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

            TempNode = ChannelNode.getChildNode("ChannelId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
                iTemp = xmltoi(strText);

                if( nChannel == iTemp)
                {
                    bFindChannel = true;
                    break; //找到这个通道节点
                }
            }

            i++;
        }
        if(!bFindChannel) //若未找到这个通道节点
        {
			return false;
        }

        SettingNode = ChannelNode.getChildNode("Setting");
        /////////////////////////////////////
        GlobalNode = SettingNode.getChildNode("Global");

        int nCoverRoadIndex = 0;
        TempNode = GlobalNode.getChildNode("CoverRoadIndex");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();

            if(strText)
            nCoverRoadIndex = xmltoi(strText);

            printf("===LoadRadarParameter====nCoverRoadIndex=%d\n",nCoverRoadIndex);
        }

        float fSpeedFactor = 0.0;
        TempNode = GlobalNode.getChildNode("SpeedFactor");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            {
                fSpeedFactor = xmltof(strText);
                printf("===LoadRadarParameter====fSpeedFactor=%f==\n",fSpeedFactor);
            }
        }

        ChannelWaysNode = SettingNode.getChildNode("ChannelWays");
        int nChannelWays = ChannelWaysNode.nChildNode();
        for(i =0; i<nChannelWays ;i++)
        {
            RADAR_PARAMETER radarPara;

            ChannelWayNode = ChannelWaysNode.getChildNode(i);

            TempNode = ChannelWayNode.getChildNode("ChannelWayId");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();

                if(strText)
                radarPara.nRoadIndex = xmltoi(strText);

                radarPara.nCoverRoadIndex = nCoverRoadIndex;
                radarPara.fSpeedFactor = fSpeedFactor;
            }

            RadarParaMap::iterator it = MapRadarPara.find(radarPara.nRoadIndex);
            if(it != MapRadarPara.end())
            it->second = radarPara;
        }
        return true;
	}
    return false;
}

//获取区域
bool CXmlParaUtil::GetRegion(int nChannel,string& strRegion)
{
    XMLNode RegionInfoNode;
    XMLNode TempNode, TempNode1, TempNode2,PointNode,ChannelNode;
    XMLCSTR strTag;
    char buf[64];
    memset(buf, 0, sizeof(buf));

    int nMaxWidth = 0;
    int nMaxHeight = 0;
    g_skpChannelCenter.GetImageSize(nChannel,nMaxWidth,nMaxHeight);

    if(nMaxWidth <= 0 || nMaxHeight <= 0)
    {
        return false;
    }
    float fscale = 1.0;
    if(nMaxWidth > 2000)
    {
        fscale = 1.0/6;
    }
    else if(nMaxWidth > 1000)
    {
        fscale = 0.25;
    }

    LIST_CHANNEL_INFO list_channel_info;
    //载入车道参数(注意坐标需要转换)
    if(LoadRoadSettingInfo(list_channel_info,nChannel))
    {
        RegionInfoNode = XMLNode::createXMLTopNode("RegionInfo");

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        ChannelNode = RegionInfoNode.addChild("ChannelId");
        sprintf(buf,"%d",nChannel);
        ChannelNode.addText(buf);

        TempNode = RegionInfoNode.addChild("BargeInRegions");
        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;

             //闯入区域
            it_rb = channel_info.BargeInRegion.listRegionProp.begin();
            it_re = channel_info.BargeInRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                TempNode1 = TempNode.addChild("BargeInRegion");
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();

                for( ; item_b!=item_e; item_b++)
                {
                    PointNode = TempNode1.addChild("Point");

                    TempNode2 = PointNode.addChild("x");
                    sprintf(buf,"%f",(item_b->x*fscale));
                    TempNode2.addText(buf);

                    TempNode2 = PointNode.addChild("y");
                    sprintf(buf,"%f",(item_b->y*fscale));
                    TempNode2.addText(buf);

                }
            }
            it_b++;
        }

        TempNode = RegionInfoNode.addChild("BeyondMarkRegions");
        it_b = list_channel_info.begin();
        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;
            //越界区域
            it_rb = channel_info.BeyondMarkRegion.listRegionProp.begin();
            it_re = channel_info.BeyondMarkRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                TempNode1 = TempNode.addChild("BeyondMarkRegion");
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();

                for( ; item_b!=item_e; item_b++)
                {
                    PointNode = TempNode1.addChild("Point");

                    TempNode2 = PointNode.addChild("x");
                    sprintf(buf,"%f",(item_b->x*fscale));
                    TempNode2.addText(buf);

                    TempNode2 = PointNode.addChild("y");
                    sprintf(buf,"%f",(item_b->y*fscale));
                    TempNode2.addText(buf);
                }
            }
            it_b++;
        }
        int nSize;
        XMLSTR strData = RegionInfoNode.createXMLString(1, &nSize);
        if(strData)
        {
            strRegion.append(strData, sizeof(XMLCHAR)*nSize);
            freeXMLString(strData);
        }

        return true;
    }
    return false;
}

//设置区域
bool CXmlParaUtil::SetRegion(string& strRegion)
{
    //需要删除不存在的车道
    XMLNode TempNode, TempNode1, TempNode2,PointNode;
    XMLNode RegionNode = XMLNode::parseString(strRegion.c_str());
    XMLNode ChannelNode = RegionNode.getChildNode("ChannelId");
    XMLCSTR strText = ChannelNode.getText();
    if(strText)
    {
        int nChannel = xmltoi(strText);

        if(nChannel <= 0)
        {
            return false;
        }

        LIST_CHANNEL_INFO list_channel_info;//车道坐标设置
        //载入车道参数(注意坐标需要转换)
        if(!LoadRoadSettingInfo(list_channel_info,nChannel))
        {
            return false;
        }

        if(list_channel_info.size() <= 0)
        {
             return false;
        }

        paraDetectList sFrameList; //车道参数设置
        SRIP_CHANNEL_EXT sChannel;
        sChannel.uId = nChannel;
         //检测控制参数
        if(!LoadRoadParameter(sFrameList,sChannel))
        {
            return false;
        }

        if(sFrameList.size() <= 0)
        {
             return false;
        }

        paraDetectMap roadParamInMap;
        paraDetectList::iterator it_r = sFrameList.begin();
        while(it_r!= sFrameList.end())
        {
            roadParamInMap.insert(paraDetectMap::value_type(it_r->nChannelID,*it_r));
            it_r++;
        }

        //获取缩放比
        int nMaxWidth = 0;
        int nMaxHeight = 0;
        g_skpChannelCenter.GetImageSize(nChannel,nMaxWidth,nMaxHeight);

        if(nMaxWidth <= 0 || nMaxHeight <= 0)
        {
            return false;
        }
        float fscale = 1.0;
        if(nMaxWidth > 2000)
        {
            fscale = 6;
        }
        else if(nMaxWidth > 1000)
        {
            fscale = 4;
        }

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<CPoint32f>::iterator it_begin;
        std::list<CPoint32f>::iterator it_end;

        LIST_CHANNEL_INFO list_channel_info_new;//新车道坐标设置
        paraDetectList sFrameList_new; //新车道参数设置
        CALIBRATION calibration;       //标定
        CHANNEL_PROPERTY chDirection;  //方向
        COMMON_REGION carnumRegion;    //车牌区域
        COMMON_REGION eliminateRegion; //屏蔽区域

        int nMaxIndex = 0; //最大车道编号

        CPoint32f pt32f;

        //获取标定和方向
        while(it_b != it_e)
        {
            CHANNEL_INFO channel_info = *it_b;

            if(it_b == list_channel_info.begin())
            {
                calibration =  channel_info.calibration;
                chDirection = channel_info.chProp_direction;
                carnumRegion = channel_info.carnumRegion;
                eliminateRegion = channel_info.eliminateRegion;

                break;
            }

            //选择需要保留的车道
            if(channel_info.stopRegion.listRegionProp.size() > 0 ||
               channel_info.personRegion.listRegionProp.size() > 0 ||
               channel_info.dropRegion.listRegionProp.size() > 0 ||
               channel_info.AmountLine.listRegionProp.size() > 0 ||
               channel_info.TurnRoadLine.listRegionProp.size() > 0)
            {

                 if(channel_info.BargeInRegion.listRegionProp.size() > 0)
                 {
                    channel_info.BargeInRegion.listRegionProp.clear();
                 }

                 if(channel_info.BeyondMarkRegion.listRegionProp.size() > 0)
                 {
                    channel_info.BeyondMarkRegion.listRegionProp.clear();
                 }

                 list_channel_info_new.push_back(channel_info);

                 printf("stopRegion.size=%d\n",channel_info.stopRegion.listRegionProp.size());

                 paraDetectMap::iterator it_rp = roadParamInMap.find(channel_info.chProp_index.value.nValue);

                 if(it_rp != roadParamInMap.end())
                 {
                    sFrameList_new.push_back(it_rp->second);
                 }
                 else
                 {
                    VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;
                    sFrame.nChannelID = channel_info.chProp_index.value.nValue;
                    sFrameList_new.push_back(sFrame);
                 }

                if(nMaxIndex < channel_info.chProp_index.value.nValue)
                {
                    nMaxIndex = channel_info.chProp_index.value.nValue;
                }
            }

            it_b++;
        }

        int i =0,j=0;
        char buf[64]={0};
        ////闯入区域
        TempNode = RegionNode.getChildNode("BargeInRegions");
        int nRegionCount = TempNode.nChildNode("BargeInRegion");
        for(i =0;i < nRegionCount;i++)
        {
            CHANNEL_INFO channel_info;
            TempNode1 = TempNode.getChildNode("BargeInRegion", i);

            Point32fList pt32fList;
            int nPointCount = TempNode1.nChildNode("Point");


            //道路区域
            strcpy(channel_info.roadRegion.chProperty.strName, "ROAD_REGION");
            channel_info.roadRegion.chProperty.value.nValue = nPointCount;

            //车道区域
            strcpy(channel_info.chRegion.chProperty.strName, "CHANNEL_REGION");
            channel_info.chRegion.chProperty.value.nValue = nPointCount;

            //闯入区域
            strcpy(channel_info.BargeInRegion.chProperty.strName, "BARGEIN_REGION");
            channel_info.BargeInRegion.chProperty.value.nValue = nPointCount;

            REGION_PROPERTY region_property;
            region_property.nValue = nPointCount;//至少3个点

            for(j =0;j < nPointCount;j++)
            {
                CPoint32f point;
                PointNode = TempNode1.getChildNode("Point", j);

                TempNode2 = PointNode.getChildNode("x");

                strText = TempNode2.getText();
                if(strText)
                {
                    point.x = xmltof(strText)*fscale;
                }

                TempNode2 = PointNode.getChildNode("y");

                strText = TempNode2.getText();
                if(strText)
                {
                    point.y = xmltof(strText)*fscale;
                }
                pt32fList.push_back(point);

                channel_info.roadRegion.listPT.push_back(point);
                channel_info.chRegion.listPT.push_back(point);
                region_property.listPt.push_back(point);
            }
            channel_info.BargeInRegion.listRegionProp.push_back(region_property);

            //需要生成新的区域
            {
                //车道的序号
                strcpy(channel_info.chProp_index.strName, "CHANNEL_INDEX");
                channel_info.chProp_index.value.nValue = nMaxIndex+1;
                nMaxIndex++;
                //车道名称
                strcpy(channel_info.chProp_name.strName, "CHANNEL_NAME");
                sprintf(buf,"%d",channel_info.chProp_index.value.nValue);
                string strRoadName(buf);
                strcpy(channel_info.chProp_name.value.strValue , strRoadName.c_str());

                //标点区域
                channel_info.calibration = calibration;
                //方向区域
                channel_info.chProp_direction = chDirection;
                list_channel_info_new.push_back(channel_info);

                //车道检测参数
                VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;
                sFrame.nChannelID = channel_info.chProp_index.value.nValue;
                sFrame.m_bBargeIn = true; //闯入检测
                sFrame.nModelId = 0;
                sFrameList_new.push_back(sFrame);
            }
        }

        //越界区域
        TempNode = RegionNode.getChildNode("BeyondMarkRegions");
        nRegionCount = TempNode.nChildNode("BeyondMarkRegion");
        for(i =0;i < nRegionCount;i++)
        {
            CHANNEL_INFO channel_info;
            TempNode1 = TempNode.getChildNode("BeyondMarkRegion", i);

            Point32fList pt32fList;
            int nPointCount = TempNode1.nChildNode("Point");

            //越界区域
            strcpy(channel_info.BeyondMarkRegion.chProperty.strName, "BEYONDMARK_REGION");
            channel_info.BeyondMarkRegion.chProperty.value.nValue = nPointCount;

            REGION_PROPERTY region_property;
            region_property.nValue = nPointCount;//至少2个点

            for(j =0;j < nPointCount;j++)
            {
                CPoint32f point;
                PointNode = TempNode1.getChildNode("Point", j);

                TempNode2 = PointNode.getChildNode("x");

                strText = TempNode2.getText();
                if(strText)
                {
                    point.x = xmltof(strText)*fscale;
                }

                TempNode2 = PointNode.getChildNode("y");

                strText = TempNode2.getText();
                if(strText)
                {
                    point.y = xmltof(strText)*fscale;
                }
                pt32fList.push_back(point);

                region_property.listPt.push_back(point);
            }
            channel_info.BeyondMarkRegion.listRegionProp.push_back(region_property);

            //道路区域需要扩充
            GetBeyondMarkRegion(pt32fList,nMaxWidth,nMaxHeight,calibration);

            nPointCount = pt32fList.size();
            //道路区域
            strcpy(channel_info.roadRegion.chProperty.strName, "ROAD_REGION");
            channel_info.roadRegion.chProperty.value.nValue = nPointCount;

            //车道区域
            strcpy(channel_info.chRegion.chProperty.strName, "CHANNEL_REGION");
            channel_info.chRegion.chProperty.value.nValue = nPointCount;

            it_begin = pt32fList.begin();
            it_end = pt32fList.end();
            while(it_begin != it_end)
            {
                CPoint32f point = *it_begin;
                channel_info.roadRegion.listPT.push_back(point);
                channel_info.chRegion.listPT.push_back(point);
                it_begin++;
            }

            //需要生成新的区域
            {
                //车道的序号
                strcpy(channel_info.chProp_index.strName, "CHANNEL_INDEX");
                channel_info.chProp_index.value.nValue = nMaxIndex+1;
                nMaxIndex++;
                //车道名称
                strcpy(channel_info.chProp_name.strName, "CHANNEL_NAME");
                sprintf(buf,"%d",channel_info.chProp_index.value.nValue);
                string strRoadName(buf);
                strcpy(channel_info.chProp_name.value.strValue , strRoadName.c_str());

                //标点区域
                channel_info.calibration = calibration;
                //方向区域
                channel_info.chProp_direction = chDirection;
                list_channel_info_new.push_back(channel_info);

                //车道检测参数
                VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;
                sFrame.nChannelID = channel_info.chProp_index.value.nValue;
                sFrame.m_bBeyondMark = true; //越界检测
                sFrame.nModelId = 0;
                sFrameList_new.push_back(sFrame);
            }
        }

        //如果存在多个预置位
        if(g_nSwitchCamera == 1)
        {
            int nCameraID = g_skpDB.GetCameraID(nChannel);
            if(nCameraID > 0)
            {
                if(AddDeviceSettingInfo(list_channel_info_new,nCameraID))
                {
                    AddDeviceParaMeterInfo(sFrameList_new,sChannel,nCameraID);
                }
            }
        }
        else
        {
            if(AddRoadSettingInfoByList(list_channel_info_new, nChannel))
            {
                AddRoadParameterByList(sFrameList_new,sChannel);
            }
        }

        //要求检测重新读取配置
        g_skpChannelCenter.ReloadDetect(nChannel);

        return true;
    }
    return false;
}

//判断点是否在多边形内
bool CXmlParaUtil::PointInRec(Point32fList& vList,CPoint32f point)
{
	int i,j;
	float k;
	bool bIsInside=false;
	CPoint32f pt = point;
	const int nPolySize=vList.size();

	CPoint32f* pts=NULL;
	pts=new CPoint32f[nPolySize];

	if(!pts)
	{
		return false;
	}
	Point32fList::iterator it_b=vList.begin();
	Point32fList::iterator it_e=vList.end();

	for(i=0;it_b!=it_e;it_b++,i++)
	{
		CPoint32f pp=*it_b;
		//		TRACE2("%d,%d \r\n",pp.x,pp.y);
		pts[i].x = pp.x;
		pts[i].y = pp.y;
	}
	if(nPolySize>2)
	{
		for(i=0,j=nPolySize-1;i<nPolySize;j=i++)
        {
            if ( (pt.y >= pts[i].y && pt.y < pts[j].y) || (pt.y >= pts[j].y && pt.y < pts[i].y) )
            {
                k = (pts[j].x-pts[i].x) * (pt.y-pts[i].y) / (pts[j].y-pts[i].y) + pts[i].x;
                if ( pt.x < k)
                {
                    bIsInside=!bIsInside;
                }
                else if ( fabs(pt.x - k)<0.1 )//点在线上则直接返回
                {
                    bIsInside = true;
                    break;
                }
            }
            else if ( fabs(pt.y - pts[i].y) <0.1 && fabs(pt.y - pts[j].y)<0.1)//点在水平线上则直接返回
            {
                if ((pt.x >= pts[i].x && pt.x <= pts[j].x) || (pt.x >= pts[j].x && pt.x <= pts[i].x) )
                 {
                     bIsInside = true;
                    break;
                 }
            }
            else if ( (fabs(pt.y - pts[i].y)<0.1 && fabs(pt.x - pts[i].x)<0.1 ) || ( fabs(pt.y - pts[j].y)<0.1 && fabs(pt.x - pts[j].x)<0.1 ) )//与顶点重合则直接返回
            {
                   bIsInside = true;
                    break;
            }
        }
	}

	delete[] pts;
	pts=NULL;

	return bIsInside;
}

//扩充越界区域
bool CXmlParaUtil::GetBeyondMarkRegion(Point32fList& ptList,int nMaxWidth,int nMaxHeight,CALIBRATION calibration)
{
    if(ptList.size() == 2)
    {
        CPoint32f point1,point2,point3,point4;

        Point32fList::iterator it_b = ptList.begin();
        Point32fList::iterator it_e = ptList.end();

        int i = 0;
        while(it_b != it_e)
        {
            if(i == 0)
            {
                point1 = *it_b;
            }
            else
            {
                point2 = *it_b;
            }
            it_b++;
            i++;
        }

        int nMax1 = GetCarSize(point1,calibration);
        int nMax2 = GetCarSize(point2,calibration);

        printf("nMax1=%d,nMax2=%d\n",nMax1,nMax2);

        //根据两个点的位置关系进行扩充
        if( (point1.x <= point2.x) && (point1.y <= point2.y) )
        {
            point1.x = point1.x - nMax1;
            point1.y = point1.y - nMax1;
            if(point1.x <0)
            point1.x = 0;
            if(point1.y <0)
            point1.y = 0;

            point3.x = point2.x + nMax2;
            point3.y = point2.y + nMax2;

            if(point3.x > nMaxWidth)
                point3.x = nMaxWidth;
            if(point3.y > nMaxHeight)
                point3.y = nMaxHeight;

            point2.x = point3.x;
            point2.y = point1.y;

            point4.x = point1.x;
            point4.y = point3.y;
        }
        else if( (point1.x < point2.x) && (point1.y > point2.y) )
        {
            point4.x = point1.x - nMax1;
            point4.y = point1.y + nMax1;
            if(point4.x <0)
            point4.x = 0;
            if(point4.y > nMaxHeight)
                point4.y = nMaxHeight;

            point2.x = point2.x + nMax2;
            point2.y = point2.y - nMax2;

            if(point2.y <0)
            point2.y = 0;
            if(point2.x > nMaxWidth)
                point2.x = nMaxWidth;

            point3.x = point2.x;
            point3.y = point4.y;

            point1.x = point4.x;
            point1.y = point2.y;
        }
        else if( (point1.x > point2.x) && (point1.y > point2.y) )
        {
            point3.x = point1.x + nMax1;
            point3.y = point1.y + nMax1;
            if(point3.x > nMaxWidth)
                point3.x = nMaxWidth;
            if(point3.y > nMaxHeight)
                point3.y = nMaxHeight;

            point1.x = point2.x - nMax2;
            point1.y = point2.y - nMax2;

            if(point1.x <0)
            point1.x = 0;

            if(point1.y <0)
            point1.y = 0;

            point2.x = point3.x;
            point2.y = point1.y;

            point4.x = point1.x;
            point4.y = point3.y;
        }
        else if( (point1.x > point2.x) && (point1.y < point2.y) )
        {
            point4.x = point2.x - nMax2;
            point4.y = point2.y + nMax2;
            if(point4.x <0)
            point4.x = 0;
            if(point4.y > nMaxHeight)
                point4.y = nMaxHeight;

            point2.x = point1.x + nMax1;
            point2.y = point1.y - nMax1;
            if(point2.y <0)
            point2.y = 0;
            if(point2.x > nMaxWidth)
                point2.x = nMaxWidth;

            point1.x = point4.x;
            point1.y = point2.y;

            point3.x = point2.x;
            point3.y = point4.y;
        }

        ptList.clear();

        //扩充后的区域
        ptList.push_back(point1);
        ptList.push_back(point2);
        ptList.push_back(point3);
        ptList.push_back(point4);

        return true;
    }
    return false;
}

//获取车宽度
int CXmlParaUtil::GetCarSize( CPoint32f pt,CALIBRATION calibration)
{
    CvPoint2D32f point;//图像点的坐标
    point.x = pt.x;
    point.y = pt.y;

    float homography_src[12] = {0};//标定点的世界坐标
    float homography_dst[12] = {0};//标定点的图像坐标

    Point32fList::iterator it_begin,it_32fb;
    Point32fList::iterator it_end,it_32fe;

    it_begin = calibration.region.listPT.begin();
    it_end = calibration.region.listPT.end();
    int i = 0;
    while(it_begin!=it_end)
    {
        homography_dst[2*i] = (it_begin->x);
        homography_dst[2*i+1] = (it_begin->y);

        if(i==0)
        {
            homography_src[2*i] = 0;
            homography_src[2*i+1] = 0;
        }
        else if(i==1)
        {
            homography_src[2*i] = calibration.length;
            homography_src[2*i+1] = 0;
        }
        else if(i==2)
        {
            homography_src[2*i] = calibration.length;
            homography_src[2*i+1] = calibration.width;
        }
        else if(i==3)
        {
            homography_src[2*i] = 0;
            homography_src[2*i+1] = calibration.width;
        }

        it_begin++;
        i++;
    }

    //辅助标定点
    it_begin = calibration.listPT.begin();
    it_end = calibration.listPT.end();

    it_32fb = calibration.list32fPT.begin();
    it_32fe = calibration.list32fPT.end();
    while(it_begin!=it_end&&it_32fb!=it_32fe)
    {
        //image cor
        homography_dst[2*i] = (it_begin->x);
        homography_dst[2*i+1] = (it_begin->y);

        //world cor
        homography_src[2*i] = it_32fb->x;
        homography_src[2*i+1] = it_32fb->y;

        it_32fb++;
        it_begin++;
        i++;
    }
	
	int nMaxsize = 0;
	#ifndef NOEVENT
    MvDemarcate   demarcate;
    nMaxsize = demarcate.mvgetMaxSizeOfPoint( 6,homography_src,homography_dst,point);
	#endif
    return  nMaxsize;
}


bool CXmlParaUtil::GetCalibration(float image_cord[12], float world_cord[12], int channelID)
{
        int i = 0;
        int j = 0;
        int m_nDeinterlace = 1;
        float m_ratio_x = 1.0;
        float m_ratio_y = 1.0;

        CALIBRATION calibration;
        CHANNEL_PROPERTY chDirection;
        bool bRet = GetCalibrationAndDirectionInfo(channelID,calibration,chDirection);
        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;
        Point32fList::iterator it_32fb;
        Point32fList::iterator it_32fe;
        it_begin = calibration.region.listPT.begin();
        it_end = calibration.region.listPT.end();
        for(; it_begin != it_end; it_begin++)
        {
            image_cord[2*i] = (it_begin->x)*m_ratio_x;
            image_cord[2*i+1] = (it_begin->y)*m_ratio_y*m_nDeinterlace;

            if(i==0)
            {
                world_cord[2*i] = 0;
                world_cord[2*i+1] = 0;
            }
            else if(i==1)
            {
                world_cord[2*i] = calibration.length;
                world_cord[2*i+1] = 0;
            }
            else if(i==2)
            {
                world_cord[2*i] = calibration.length;
                world_cord[2*i+1] = calibration.width;
            }
            else if(i==3)
            {
                world_cord[2*i] = 0;
                world_cord[2*i+1] = calibration.width;
            }
            printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);
            i++;
        }
                //printf("channel_info.calibration.length=%f,channel_info.calibration.width=%f\n",channel_info.calibration.length,channel_info.calibration.width);
                //辅助标定点
        it_begin = calibration.listPT.begin();
        it_end = calibration.listPT.end();

        it_32fb = calibration.list32fPT.begin();
        it_32fe = calibration.list32fPT.end();
        while(it_begin!=it_end&&it_32fb!=it_32fe)
        {
            //image cor
            image_cord[2*i] = (it_begin->x)*m_ratio_x;
            image_cord[2*i+1] = (it_begin->y)*m_ratio_y*m_nDeinterlace;

            //world cor
            world_cord[2*i] = it_32fb->x;
            world_cord[2*i+1] = it_32fb->y;

            printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);


            it_32fb++;
            it_begin++;
            i++;
        }
    return bRet;
}

//获取远景预置位列表
bool CXmlParaUtil::LoadRemotePreSet(int nChannel,vector<int>& vRemotePreSet)
{
	vRemotePreSet.clear();
	std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
    if(access(strPreSetInfoXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,LocalPreSetsNode,LocalPreSetNode,LocalPreSetAreaNode,PointsNode,TempNode;
        XMLCSTR strText;
        int iTemp = 0;
        int iCurChannel; //记录当前通道编号
        int nPoints;

        XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
        if(!XmlNode.isEmpty())
        {
             //通道
            ChannelsNode = XmlNode.getChildNode("Channels");

             //通道个数
            int nChannels = ChannelsNode.nChildNode();

             /////////////////////////////////////找到当前节点
            bool bFindChannel = false;
            int i = 0;
            while(i < nChannels)
            {
                ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                TempNode = ChannelNode.getChildNode("ChannelId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nChannel == iTemp)
                    {
                        bFindChannel = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
            {
                return false;
            }

            RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

            bool bFindPreSet = false;
            i = 0;
            int nRemotePreSets = RemotePreSetsNode.nChildNode();
            while(i < nRemotePreSets)
            {
                RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);
					vRemotePreSet.push_back(iTemp);
                }

                i++;
            }//End of while


            return true;
        }
    }
    return false;
}

//加载预置位信息
bool CXmlParaUtil::LoadPreSetInfo(int nChannel,int nPreSet,PreSetInfoList& ListPreSetInfo)
{
    std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
    if(access(strPreSetInfoXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,LocalPreSetsNode,LocalPreSetNode,LocalPreSetAreaNode,PointsNode,TempNode;
        XMLCSTR strText;
        int iTemp = 0;
        int iCurChannel; //记录当前通道编号
        int nPoints;

        XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
        if(!XmlNode.isEmpty())
        {
             //通道
            ChannelsNode = XmlNode.getChildNode("Channels");

             //通道个数
            int nChannels = ChannelsNode.nChildNode();

             /////////////////////////////////////找到当前节点
            bool bFindChannel = false;
            int i = 0;
            while(i < nChannels)
            {
                ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                TempNode = ChannelNode.getChildNode("ChannelId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nChannel == iTemp)
                    {
                        bFindChannel = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
            {
                return false;
            }

            RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

            bool bFindPreSet = false;
            i = 0;
            int nRemotePreSets = RemotePreSetsNode.nChildNode();
            while(i < nRemotePreSets)
            {
                RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nPreSet == iTemp)
                    {
                        bFindPreSet = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindPreSet) //若未找到这个通道节点，则返回载入失败
            {
                return false;
            }

            LocalPreSetsNode = RemotePreSetNode.getChildNode("LocalPreSets");

            i = 0;
            int nLocalPreSets = LocalPreSetsNode.nChildNode();
            while(i < nLocalPreSets)
            {
                LocalPreSetNode = LocalPreSetsNode.getChildNode(i); //通道节点

                PreSetInfo info;

                TempNode = LocalPreSetNode.getChildNode("LocalPreSetID");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();

                    if(strText)
                    {
                        iTemp = xmltoi(strText);
                        info.nPreSetID = iTemp;
                    }
                }

                //
                LocalPreSetAreaNode =  LocalPreSetNode.getChildNode("LocalPreSetArea");

                //PointsNode = LocalPreSetAreaNode.getChildNode("Points");
                GetListPointsFromNode(info.listRegion,LocalPreSetAreaNode);

                ListPreSetInfo.push_back(info);
                printf("info.nPreSetID=%d,info.listRegion.size=%d\n",info.nPreSetID,info.listRegion.size());

                i++;
            }//End of while

            return true;
        }
    }
    return false;
}

//获取预置位检测类型
int CXmlParaUtil::LoadPreSetDetectKind(int nChannel,int nPreSet)
{
    std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
    if(access(strPreSetInfoXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,TempNode;
        XMLCSTR strText;
        int iTemp = 0;
        int iCurChannel; //记录当前通道编号

        XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
        if(!XmlNode.isEmpty())
        {
             //通道
            ChannelsNode = XmlNode.getChildNode("Channels");

             //通道个数
            int nChannels = ChannelsNode.nChildNode();

             /////////////////////////////////////找到当前节点
            bool bFindChannel = false;
            int i = 0;
            while(i < nChannels)
            {
                ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                TempNode = ChannelNode.getChildNode("ChannelId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nChannel == iTemp)
                    {
                        bFindChannel = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
            {
                return 0;
            }

            RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

            bool bFindPreSet = false;
            i = 0;
            int nRemotePreSets = RemotePreSetsNode.nChildNode();
            while(i < nRemotePreSets)
            {
                RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nPreSet == iTemp)
                    {
                        bFindPreSet = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindPreSet) //若未找到这个通道节点，则返回载入失败
            {
                return 0;
            }

            TempNode = RemotePreSetNode.getChildNode("DetectKind");
			 if(!TempNode.isEmpty())
             {
                    strText = TempNode.getText();

                    if(strText)
                    {
                        iTemp = xmltoi(strText);
                    }
              }

            return iTemp;
        }
    }
    return 0;
}

//获取预置位
bool CXmlParaUtil::GetPreSetInfo(string& strPreset,string& response)
{
    XMLNode XmlNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,RemotePreSetPicNode,LocalPreSetsNode,LocalPreSetNode,LocalPreSetAreaNode,PointsNode,PointNode;

    char buf[64];
	memset(buf, 0, sizeof(buf));

    ChannelNode = XMLNode::parseString(strPreset.c_str());
    XMLNode TempNode = ChannelNode.getChildNode("ChannelId");
    XMLCSTR strText = TempNode.getText();
    if(strText)
    {
          int nChannel = xmltoi(strText);
          printf("OnPreSetInfo ======nChannel=%d\n",nChannel);

          RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

          RemotePreSetNode = RemotePreSetsNode.getChildNode("RemotePreSet");

          TempNode =  RemotePreSetNode.getChildNode("RemotePreSetID");

          RemotePreSetPicNode = RemotePreSetNode.addChild("RemotePreSetPic");

          //获取图片信息
          ImageRegion imgRegion;
          string strPic;
          g_skpChannelCenter.CaptureOneFrame(strPic,nChannel,imgRegion);

        /*  FILE* fp = fopen("CaptureOneFrame.jpg","wb");
        fwrite(strPic.c_str(),strPic.size(),1,fp);
        fclose(fp);*/


          string strEncodedPic;
          EncodeBase64(strEncodedPic,(unsigned char*)strPic.c_str(),strPic.size());
          RemotePreSetPicNode.addText(strEncodedPic.c_str());

          strText = TempNode.getText();

          if(strText)
          {
            int nPreSet = xmltoi(strText);

            PreSetInfoList ListPreSetInfo;
            //载入预置位信息
            if(LoadPreSetInfo(nChannel,nPreSet,ListPreSetInfo))
            {
                LocalPreSetsNode = RemotePreSetNode.addChild("LocalPreSets");

                PreSetInfoList::iterator it_b = ListPreSetInfo.begin();
                PreSetInfoList::iterator it_e = ListPreSetInfo.end();

                while(it_b != it_e)
                {
                    PreSetInfo info = *it_b;

                    LocalPreSetNode = LocalPreSetsNode.addChild("LocalPreSet");

                    TempNode = LocalPreSetNode.addChild("LocalPreSetID");

                    sprintf(buf,"%d",info.nPreSetID);
                    TempNode.addText(buf);

                    LocalPreSetAreaNode = LocalPreSetNode.addChild("LocalPreSetArea");

                    PointsNode = LocalPreSetAreaNode.addChild("Points");

                    Point32fList::iterator it = info.listRegion.begin();
                    while(it != info.listRegion.end())
                    {
                        PointNode = PointsNode.addChild("Point");

                        TempNode = PointNode.addChild("x");
                        sprintf(buf,"%f",it->x);
                        TempNode.addText(buf);

                        TempNode = PointNode.addChild("y");
                        sprintf(buf,"%f",it->y);
                        TempNode.addText(buf);

                        it++;
                    }

                    it_b++;
                }
            }

            int nSize;
            XMLSTR strData = ChannelNode.createXMLString(1, &nSize);
            if(strData)
            {
                response.append(strData, sizeof(XMLCHAR)*nSize);
                freeXMLString(strData);
            }

            return true;
          }
    }
    return false;
}

//更新预置位检测类型
bool CXmlParaUtil::UpdatePreSetDetectKind(int nChannel,int nPreSet,int nDetectKind)
{
	nPreSet = g_skpDB.GetPreSet(nChannel);
	std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
    if(access(strPreSetInfoXml.c_str(),F_OK) == 0)//存在
    {
        XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,DetectKindNode,PointsNode,TempNode;
        XMLCSTR strText;
        int iTemp = 0;
        int iCurChannel; //记录当前通道编号
        int nPoints;
		char buf[256] = {0};

        XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
        if(!XmlNode.isEmpty())
        {
             //通道
            ChannelsNode = XmlNode.getChildNode("Channels");

             //通道个数
            int nChannels = ChannelsNode.nChildNode();

             /////////////////////////////////////找到当前节点
            bool bFindChannel = false;
            int i = 0;
            while(i < nChannels)
            {
                ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                TempNode = ChannelNode.getChildNode("ChannelId");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nChannel == iTemp)
                    {
                        bFindChannel = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
            {
                return false;
            }

            RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

            bool bFindPreSet = false;
            i = 0;
            int nRemotePreSets = RemotePreSetsNode.nChildNode();
            while(i < nRemotePreSets)
            {
                RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                if(!TempNode.isEmpty())
                {
                    strText = TempNode.getText();
                    iTemp = xmltoi(strText);

                    if( nPreSet == iTemp)
                    {
                        bFindPreSet = true;
                        break; //找到这个通道节点
                    }
                }

                i++;
            }//End of while
            if(!bFindPreSet) //若未找到这个通道节点，则返回载入失败
            {
                return false;
            }

            DetectKindNode = RemotePreSetNode.getChildNode("DetectKind");

            if(DetectKindNode.isEmpty())
            {
               DetectKindNode = RemotePreSetNode.addChild("DetectKind"); 
			   sprintf(buf,"%d",nDetectKind);
               DetectKindNode.addText(buf);
            }
			else
			{
			   sprintf(buf,"%d",nDetectKind);
               DetectKindNode.updateText(buf);
			}

			 XmlNode.writeToFile(strPreSetInfoXml.c_str());
			
            return true;
        }
    }
    return false;
}

//更新预置位信息
bool CXmlParaUtil::UpdatePreSetInfo(string& strPreset)
{
    XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,PreSetNode;

    char buf[64];
	memset(buf, 0, sizeof(buf));

    ChannelNode = XMLNode::parseString(strPreset.c_str());
    XMLNode TempNode = ChannelNode.getChildNode("ChannelId");
    XMLCSTR strText = TempNode.getText();
    if(strText)
    {
        int nChannel = xmltoi(strText);
        printf("OnPreSetInfo ======nChannel=%d\n",nChannel);

         RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

         PreSetNode = RemotePreSetsNode.getChildNode("RemotePreSet");

         TempNode =  PreSetNode.getChildNode("RemotePreSetID");

          strText = TempNode.getText();

          int nPreSet = 0;
          if(strText)
          nPreSet = xmltoi(strText);

          if(nPreSet > 0)
          {
                   std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
                   if(access(strPreSetInfoXml.c_str(),F_OK) != 0)//不存在需要重新创建
                   {
                       XmlNode = XMLNode::createXMLTopNode("PreSetSetting");
                   }
                   else
                   {
                       XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
                       if(XmlNode.isEmpty())
                       {
                            XmlNode = XMLNode::createXMLTopNode("PreSetSetting");
                       }
                   }

                    if(!XmlNode.isEmpty())
                    {
                        int iTemp = 0;
                                     //通道
                        ChannelsNode = XmlNode.getChildNode("Channels");
                        if(ChannelsNode.isEmpty())
                        {
                            ChannelsNode = XmlNode.addChild("Channels");
                        }

                         //通道个数
                        int nChannels = ChannelsNode.nChildNode();

                         /////////////////////////////////////找到当前节点
                        bool bFindChannel = false;
                        int i = 0;
                        while(i < nChannels)
                        {
                            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                            TempNode = ChannelNode.getChildNode("ChannelId");
                            if(!TempNode.isEmpty())
                            {
                                strText = TempNode.getText();
                                iTemp = xmltoi(strText);

                                if( nChannel == iTemp)
                                {
                                    bFindChannel = true;
                                    break; //找到这个通道节点
                                }
                            }

                            i++;
                        }//End of while
                        if(!bFindChannel) //若未找到这个通道节点
                        {
                            ChannelNode = ChannelsNode.addChild("Channel");

                            TempNode = ChannelNode.addChild("ChannelId");
                            sprintf(buf,"%d",nChannel);
                            TempNode.addText(buf);
                        }

                        RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");
                        if(RemotePreSetsNode.isEmpty())
                        {
                            RemotePreSetsNode = ChannelNode.addChild("RemotePreSets");
                        }

                        bool bFindPreSet = false;
                        i = 0;
                        int nRemotePreSets = RemotePreSetsNode.nChildNode();
                        while(i < nRemotePreSets)
                        {
                            RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                            TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                            if(!TempNode.isEmpty())
                            {
                                strText = TempNode.getText();
                                iTemp = xmltoi(strText);

                                if( nPreSet == iTemp)
                                {
                                    bFindPreSet = true;
                                    break; //找到这个通道节点
                                }
                            }

                            i++;
                        }//End of while

                        if(bFindPreSet) //若未找到这个通道节点，则返回载入失败
                        {
                            RemotePreSetNode.deleteNodeContent();
                        }

                        RemotePreSetsNode.addChild(PreSetNode);

                        if(XmlNode.writeToFile(strPreSetInfoXml.c_str()))
                        {
                             //要求检测重新读取配置
                            g_skpChannelCenter.ReloadDetect(nChannel);
                            return true;
                        }
                    }

          }
    }
    return false;
}

//删除预置位
bool CXmlParaUtil::DeletePreSetInfo(string& strPreset)
{
     XMLNode XmlNode,ChannelsNode,ChannelNode,RemotePreSetsNode,RemotePreSetNode,RemotePreSetPicNode,LocalPreSetsNode,LocalPreSetNode;

    ChannelNode = XMLNode::parseString(strPreset.c_str());
    XMLNode TempNode = ChannelNode.getChildNode("ChannelId");
    XMLCSTR strText = TempNode.getText();
    if(strText)
    {
          int nChannel = xmltoi(strText);
          printf("OnPreSetInfo ======nChannel=%d\n",nChannel);

          RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

          RemotePreSetNode = RemotePreSetsNode.getChildNode("RemotePreSet");

          TempNode =  RemotePreSetNode.getChildNode("RemotePreSetID");

          strText = TempNode.getText();

          int nPreSet = 0;
          if(strText)
          nPreSet = xmltoi(strText);

          int nLocalPreSet = 0;

          LocalPreSetsNode = RemotePreSetNode.getChildNode("LocalPreSets");

          if(!LocalPreSetsNode.isEmpty())
          {
              LocalPreSetNode =  LocalPreSetsNode.getChildNode("LocalPreSet");

              TempNode = LocalPreSetNode.getChildNode("LocalPreSetID");

              strText = TempNode.getText();

              if(strText)
              nLocalPreSet = xmltoi(strText);
          }

          if(nPreSet > 0)
          {
               std::string strPreSetInfoXml = "./config/PreSetSettingInfo.xml";
               if(access(strPreSetInfoXml.c_str(),F_OK) == 0)//存在
               {
                    XmlNode = XMLNode::parseFile(strPreSetInfoXml.c_str()).getChildNode("PreSetSetting");
                    if(!XmlNode.isEmpty())
                    {
                        int iTemp = 0;
                        //通道
                        ChannelsNode = XmlNode.getChildNode("Channels");

                         //通道个数
                        int nChannels = ChannelsNode.nChildNode();

                         /////////////////////////////////////找到当前节点
                        bool bFindChannel = false;
                        int i = 0;
                        while(i < nChannels)
                        {
                            ChannelNode = ChannelsNode.getChildNode(i); //通道节点

                            TempNode = ChannelNode.getChildNode("ChannelId");
                            if(!TempNode.isEmpty())
                            {
                                strText = TempNode.getText();
                                iTemp = xmltoi(strText);

                                if( nChannel == iTemp)
                                {
                                    bFindChannel = true;
                                    break; //找到这个通道节点
                                }
                            }

                            i++;
                        }//End of while
                        if(!bFindChannel) //若未找到这个通道节点，则返回载入失败
                        {
                            return false;
                        }

                        RemotePreSetsNode = ChannelNode.getChildNode("RemotePreSets");

                        bool bFindPreSet = false;
                        i = 0;
                        int nRemotePreSets = RemotePreSetsNode.nChildNode();
                        while(i < nRemotePreSets)
                        {
                            RemotePreSetNode = RemotePreSetsNode.getChildNode(i); //通道节点

                            TempNode = RemotePreSetNode.getChildNode("RemotePreSetID");
                            if(!TempNode.isEmpty())
                            {
                                strText = TempNode.getText();
                                iTemp = xmltoi(strText);

                                if( nPreSet == iTemp)
                                {
                                    bFindPreSet = true;
                                    break; //找到这个通道节点
                                }
                            }

                            i++;
                        }//End of while
                        if(!bFindPreSet) //若未找到这个通道节点，则返回载入失败
                        {
                            return false;
                        }

                        LocalPreSetsNode = RemotePreSetNode.getChildNode("LocalPreSets");

                        if(nLocalPreSet > 0)
                        {
                            bool bFindLocalPreSet = false;
                            i = 0;
                            int nLocalPreSets = LocalPreSetsNode.nChildNode();
                            while(i < nLocalPreSets)
                            {
                                LocalPreSetNode = LocalPreSetsNode.getChildNode(i); //通道节点

                                TempNode = LocalPreSetNode.getChildNode("LocalPreSetID");
                                if(!TempNode.isEmpty())
                                {
                                    strText = TempNode.getText();
                                    iTemp = xmltoi(strText);

                                    if( nLocalPreSet == iTemp)
                                    {
                                        bFindLocalPreSet = true;
                                        break; //找到这个通道节点
                                    }
                                }

                                i++;
                            }//End of while

                            if(!bFindLocalPreSet) //若未找到这个通道节点，则返回载入失败
                            {
                                return false;
                            }

                            LocalPreSetNode.deleteNodeContent();
                        }
                        else
                        {
                             RemotePreSetNode.deleteNodeContent();
                        }

                        return XmlNode.writeToFile(strPreSetInfoXml.c_str());
                    }
               }
          }
    }
    return false;
}

//载入违章数据表
bool CXmlParaUtil::LoadBasePlateInfo()
{
	//把柴油车的车牌写入/var/lib/mysql/目录下才能被load进数据库
	//FILE *fp = fopen("/var/lib/mysql/yellowcar.txt","w+");
	FILE *fp1 = fopen("/var/lib/mysql/yellowcar_1.txt","w+");//存储类型是1的车牌（本地黄标车，不是柴油车）
	FILE *fp2 = fopen("/var/lib/mysql/yellowcar_2.txt","w+");//存储类型是2的车牌（本地柴油车，不是黄标车）
	FILE *fp3 = fopen("/var/lib/mysql/yellowcar_3.txt","w+");//存储类型是3的车牌（即时柴油车又是黄标车）
	int nCorrect = -1;//"yellowcar.xml"所读取的那条信息是否真确。0表示真确，1表示不正确，-1表示初始值
	int nPartPrint = 0;//"yellowcar.xml数据格式。-1表示数据格式部分错误，0表示数据格式都真确，1表示数据格式都错误
	std::string strBasePlateInfoXml = "./yellowcar.xml";
	g_nType1 = 0;
	g_nType2 = 0;
	if(access(strBasePlateInfoXml.c_str(),F_OK) == 0)//存在
	{
		//g_skpDB.ClearPlateInfo();
		XMLNode xml,cars,temp,car;
		XMLCSTR strText;
		xml = XMLNode::parseFile(strBasePlateInfoXml.c_str()).getChildNode("root");//获取"./yellowcar.xml"文件中的子节点"root"的句柄

		if(!xml.isEmpty())
		{
			cars = xml.getChildNode("cars");//获取"root"节点的子节点"cars"的句柄

			if(!cars.isEmpty())
			{
				int nCarsCount = cars.nChildNode();//获取"cars"节点的子节点数目
				for(int i=0; i<nCarsCount; i++)
				{
					car = cars.getChildNode(i);//获取"cars"节点的第i个子节点的句柄

					if(!car.isEmpty())
					{
						XMLCSTR XMLText;
						XMLCSTR XMLnVale;

						XMLnVale = car.getAttributeValue();//获取"cars"节点的第i个子节点的<>符号中的值

						XMLText = car.getText();//获取"cars"节点的第i个子节点的内容
						if(XMLText && XMLnVale)
						{
							int nValue = 0;

							string strCarNum = XMLText;

							string strValue = XMLnVale;

							string strinput = "." +strCarNum + "." + "," + "." + strValue + "." + "\n";//车牌前后加点是为了满足Load Data infile的格式，这里用.作为字段分隔符所以要加.
							if(nCorrect == 1)
							{
								nPartPrint = -1;//之前是有错误数据的，现在的是真确的，就可以判断是部分错误
							}
							else if(nPartPrint != -1)//只有当不是部分错误的情况下才可能是都正确
							{
								nPartPrint = 0;//都真确
							}
							nCorrect = 0;
							//string strinput = "." +strCarNum + "." + strValue + "." + "\n";//车牌前后加点是为了满足Load Data infile的格式，这里用.作为字段分隔符所以要加.
							if(XMLnVale)
							{
								char chValue[5] = {0};
								memcpy(chValue, strValue.c_str(), strValue.size());
								//nValue = atoi(&(strValue.c_str()));
								nValue = atoi(chValue);
							}
							if(nValue == 1)
							{
								if(fp1)
								{
									fwrite(strinput.c_str(), strinput.size(), 1, fp1);
									g_nType1++;
								}								
							}
							else if(nValue == 2)
							{
								if(fp2)
								{
									fwrite(strinput.c_str(), strinput.size(), 1, fp2);
									g_nType2++;
								}
							}
							else if(nValue == 3)
							{
								if(fp3)
									fwrite(strinput.c_str(), strinput.size(), 1, fp3);
							}
							//g_skpDB.AddPlateInfo(strText);

						}
						else
						{
							if(!nCorrect)
							{
								nPartPrint = -1;//之前是有真确数据的，现在的是错误的，就可以判断是部分错误
							}
							else if(nPartPrint != -1)//只有当不是部分错误的情况下才可能是都错误
							{
								nPartPrint = 1;//都错误
							}
							nCorrect = 1;
						}
					}
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			LogNormal("无效的yellowcar.xml文件!");
			return false;
		}

		//if(fp)
			//fclose(fp);
		if(fp1)
			fclose(fp1);
		if(fp2)
			fclose(fp2);
		if(fp3)
			fclose(fp3);

		if(nPartPrint != 1)//只要不是都错就去加载
		{
			g_skpDB.ClearPlateInfo();//加载前先清空
			//g_skpDB.LoadPlateInfo("yellowcar.txt", "BASE_PLATE_INFO");
			g_skpDB.LoadPlateInfo("yellowcar_1.txt", "BASE_PLATE_INFO");
			g_skpDB.LoadPlateInfo("yellowcar_2.txt", "BASE_PLATE_INFO");
			g_skpDB.LoadPlateInfo("yellowcar_3.txt", "BASE_PLATE_INFO");
			if(nPartPrint == -1)
			{
				LogNormal("yellowcar.xml中的部分数据的格式有错误!");
			}
			else if(nPartPrint == 0)
			{
				LogNormal("yellowcar.xml载入成功!");
			}
		}
		else if(nPartPrint == 1)
		{
			LogNormal("yellowcar.xml的数据格式错误!");
		}


		return true;
	}
	else
	{
		return false;
	}
}

//取得模板中的系统部分
void CXmlParaUtil::GetSystem(XMLNode setting)
{
	XMLNode temp;
	if (!setting.isEmpty())
	{
		temp = setting.getChildNode("HasCenterServer");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nHasCenterServer = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("ServerType");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nServerType = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SendViolationOnly");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSendViolationOnly = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("WorkMode");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nWorkMode = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("DoSynProcess");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nDoSynProcess = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("WriteBackTime");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nWriteBackTime = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CheckShake");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nCheckShake = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SaveImageCount");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSaveImageCount = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("DetectSpecialCarNum");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nDetectSpecialCarNum = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("DspLog");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nDspLog = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SwitchCamera");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSwitchCamera = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("EncodeFormat");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nEncodeFormat = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("AviHeaderEx");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nAviHeaderEx = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("PicMode");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nPicMode = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("VtsPicMode");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nVtsPicMode = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("HasExpoMonitor");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nHasExpoMonitor = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("HasHighLight");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nHasHighLight = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SendRTSP");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSendRTSP = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SendImage");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSendImage = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SendHistoryRecord");
		if (!temp.isEmpty() && temp.getText())
		{
			g_nSendHistoryRecord = xmltoi(temp.getText());
		}
	}
}

//取得模板中的图片配置部分
void CXmlParaUtil::GetPicFormatInfo(XMLNode setting)
{
	XMLNode temp;
	if (!setting.isEmpty())
	{
		temp = setting.getChildNode("WordPos");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nWordPos = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CarColor");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nCarColor = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CarType");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nCarType = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CarBrand");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nCarBrand = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CarSpeed");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nCarSpeed = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("ViolationType");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nViolationType = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("CarNum");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nCarNum = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("RoadIndex");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nRoadIndex = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("WordOnPic");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nWordOnPic = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SmallPic");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nSmallPic = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("FontSize");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nFontSize = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("ExtentHeight");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nExtentHeight = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("ForeColor");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nForeColor = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("BackColor");
		if (!temp.isEmpty() && temp.getText())
		{
			g_PicFormatInfo.nBackColor = xmltoi(temp.getText());
		}
	}
}

//取得模板中的视频配置部分
void CXmlParaUtil::GetVideoFormatInfo(XMLNode setting)
{
	XMLNode temp;
	if (!setting.isEmpty())
	{
		temp = setting.getChildNode("EncodeFormat");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nEncodeFormat = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("FrameRate");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nFrameRate = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("Resolution");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nResolution = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("AviHeaderEx");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nAviHeaderEx = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("SendRtsp");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nSendRtsp = xmltoi(temp.getText());
		}
		temp = setting.getChildNode("TimeLength");
		if (!temp.isEmpty() && temp.getText())
		{
			g_VideoFormatInfo.nTimeLength = xmltoi(temp.getText());
		}
	}
}

//读入系统设置模板
void CXmlParaUtil::LoadSysModel(int nSysModel)
{
	XMLNode xmlNode, setting,temp;
	string strSysModelXml = "./SystemModel.xml";
	if(access(strSysModelXml.c_str(), F_OK) == 0) //存在
	{
		//xml文件
		xmlNode = XMLNode::parseFile(strSysModelXml.c_str()).getChildNode("SystemModel");
		if (!xmlNode.isEmpty())
		{
			g_nSettingModelID = nSysModel;
			switch (nSysModel)
			{
			case 0:		//出厂默认
				setting = xmlNode.getChildNode("ModelID_0");
				GetSystem(setting);
				break;
			case 1:		//浦东卡口
				setting = xmlNode.getChildNode("ModelID_1");
				GetSystem(setting);
				break;
			case 2:		//浦东智能分析
				setting = xmlNode.getChildNode("ModelID_2");
				GetSystem(setting);
				break;
			case 3:		//旅行时间
				setting = xmlNode.getChildNode("ModelID_3");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				GetVideoFormatInfo(setting.getChildNode("VideoFormatInfo"));
				break;
			case 4:		//国庆阅兵
				setting = xmlNode.getChildNode("ModelID_4");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				break;
			case 5:		//中关村停车
				setting = xmlNode.getChildNode("ModelID_5");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				break;
			case 6:		//交管局综合检测
				setting = xmlNode.getChildNode("ModelID_6");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				break;
			case 7:		//交管局事件检测
				setting = xmlNode.getChildNode("ModelID_7");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				break;
			case 8:		//西安事件检测
				setting = xmlNode.getChildNode("ModelID_8");
				GetSystem(setting);
				break;
			case 9:		//江宁电警
				setting = xmlNode.getChildNode("ModelID_9");
				GetSystem(setting);
				GetPicFormatInfo(setting.getChildNode("PicFormatInfo"));
				break;
			}
		}
	}
	else
	{
		// 文件不存在的情况下 生成空文件
		xmlNode = XMLNode::createXMLTopNode("SystemModel");

		for (int i = 0; i < 10; i++)
		{
			char buf[15] = {0};
			sprintf(buf, "ModelID_%d", i);
			setting = xmlNode.addChild(buf);
			temp = setting.addChild("HasCenterServer");
			temp.addText("");
			temp = setting.addChild("ServerType");
			temp.addText("");
			temp = setting.addChild("HasNeServer");
			temp.addText("");
			temp = setting.addChild("WorkMode");
			temp.addText("");
			temp = setting.addChild("DoSynProcess");
			temp.addText("");
			temp = setting.addChild("WriteBackTime");
			temp.addText("");
			temp = setting.addChild("CheckShake");
			temp.addText("");
			temp = setting.addChild("SaveImageCount");
			temp.addText("");
			temp = setting.addChild("DetectSpecialCarNum");
			temp.addText("");
			temp = setting.addChild("Single");
			temp.addText("");
			temp = setting.addChild("SwitchCamera");
			temp.addText("");
			if (i == 3)
			{
				temp = setting.addChild("EncodeFormat");
				temp.addText("");
				temp = setting.addChild("AviHeaderEx");
				temp.addText("");
				temp = setting.addChild("SendRTSP");
				temp.addText("");
			}

			temp = setting.addChild("PicMode");
			temp.addText("");
			temp = setting.addChild("VtsPicMode");
			temp.addText("");
			temp = setting.addChild("HasExpoMonitor");
			temp.addText("");
			temp = setting.addChild("HasHighLight");
			temp.addText("");
			if (i == 0 || i == 3 || i == 4)
			{
				temp = setting.addChild("SendImage");
				temp.addText("");
				temp = setting.addChild("SendHistoryRecord");
				temp.addText("");
			}
			if (i == 3 || i == 4 || i == 5 || i == 6 || i == 7 || i == 9)//图片格式设置
			{
				XMLNode picFormat = setting.addChild("PicFormatInfo");
				temp = picFormat.addChild("WordPos");
				temp.addText("");
				temp = picFormat.addChild("CarColor");
				temp.addText("");
				temp = picFormat.addChild("CarType");
				temp.addText("");
				temp = picFormat.addChild("CarBrand");
				temp.addText("");
				temp = picFormat.addChild("CarSpeed");
				temp.addText("");
				temp = picFormat.addChild("ViolationType");
				temp.addText("");
				temp = picFormat.addChild("CarNum");
				temp.addText("");
				temp = picFormat.addChild("RoadIndex");
				temp.addText("");
				temp = picFormat.addChild("WordOnPic");
				temp.addText("");
				temp = picFormat.addChild("SmallPic");
				temp.addText("");
				temp = picFormat.addChild("FontSize");
				temp.addText("");
				temp = picFormat.addChild("ExtentHeight");
				temp.addText("");
				temp = picFormat.addChild("ForeColor");
				temp.addText("");
				temp = picFormat.addChild("BackColor");
				temp.addText("");
			}
			if (i == 3) //录像格式设置
			{
				XMLNode videoFormat = setting.addChild("VideoFormatInfo");
				temp = videoFormat.addChild("EncodeFormat");
				temp.addText("");
				temp = videoFormat.addChild("FrameRate");
				temp.addText("");
				temp = videoFormat.addChild("Resolution");
				temp.addText("");
				temp = videoFormat.addChild("AviHeaderEx");
				temp.addText("");
				temp = videoFormat.addChild("SendRtsp");
				temp.addText("");
				temp = videoFormat.addChild("TimeLength");
				temp.addText("");
				temp = videoFormat.addChild("SipService");
				temp.addText("");
			}

		}
		xmlNode.writeToFile(strSysModelXml.c_str());
		LogError("模板文件不存在, 生成空模板文件!\n");
	}
}

/*
//取得配置文件中的限速
void CXmlParaUtil::GetMaxSpeed(map<UINT32,UINT32> &roadWayMap, UINT32 channelId)
{
	XMLNode Setting, ChannelNodes, ChannelNode, TempNode, ChannelWays, ChannelWay;
	XMLCSTR strText;
	string xmlFile;

	roadWayMap.clear();

	//取得xml文件名
	if( (g_nMultiPreSet == 0) && (g_nSwitchCamera == 0) )
	{
		xmlFile = "./config/RoadParameter.xml";
	}
	else
	{
		int nCameraID = g_skpDB.GetCameraID(channelId);
        int nPreSet = 0;
        if(g_nMultiPreSet == 1)
        {
          nPreSet = g_skpDB.GetPreSet(channelId);
        }
		if(nPreSet > 0)
		{
			char buf[256]={0};
			sprintf(buf,"./config/Camera%d", nCameraID);
			string strPath(buf);
			if(access(strPath.c_str(), F_OK) == 0)
			{
				if(g_nMultiPreSet == 0)
					sprintf(buf,"%s/RoadParameter.xml", strPath.c_str());
				else
					sprintf(buf,"%s/RoadParameter-PreSet%d.xml", strPath.c_str(), nPreSet);
			}
			xmlFile = buf;
		}
	}

	if(access(xmlFile.c_str(), F_OK) == 0) //存在
	{
		Setting = XMLNode::parseFile(xmlFile.c_str());
		Setting = Setting.getChildNode("RoadParameter");
		if (!Setting.isEmpty())
		{
			//通道
			ChannelNodes = Setting.getChildNode("Channels");

			//通道个数
			int nChannels = ChannelNodes.nChildNode();
			for(int i = 0; i < nChannels; i++)
			{
				ChannelNode = ChannelNodes.getChildNode(i); //通道节点

				TempNode = ChannelNode.getChildNode("ChannelId");
				if(!TempNode.isEmpty())
				{
					if( channelId == xmltoi(TempNode.getText()) )//匹配通道ID
					{
						ChannelWays = ChannelNode.getChildNode("ChannelWays");
						if (!ChannelWays.isEmpty())
						{
							int roadWayId = 0;
							int waysCount = ChannelWays.nChildNode();
							for (int i = 0; i < waysCount; i++)
							{
								ChannelWay = ChannelWays.getChildNode(i);
								if(!ChannelWay.isEmpty())
								{
									TempNode = ChannelWay.getChildNode("ChannelWayId");
									if(!TempNode.isEmpty())
									{
										roadWayId = xmltoi(TempNode.getText());
										TempNode = ChannelWay.getChildNode("IsModel");
										if(!TempNode.isEmpty())
										{
											UINT32 maxSpeed;
											if(xmltoi(TempNode.getText()) <= 0)//是否载入模板
											{
												TempNode = ChannelWay.getChildNode("AvgSpeedMax");
												if(!TempNode.isEmpty())
												{
													maxSpeed = xmltoi(TempNode.getText());
													roadWayMap[roadWayId] = maxSpeed;
												}
											}
											else
											{
												TempNode = ChannelWay.getChildNode("ModelId");
												if(!TempNode.isEmpty())
												{
													int modelId = xmltoi(TempNode.getText());
													maxSpeed = GetMaxSpeedModel(modelId);
													roadWayMap[roadWayId] = maxSpeed;
												}
											}
										}
									}
								}
							}
						}
						break;
					}
				}
			}
		}
	}
}
*/

//取得配置文件中的限速
void CXmlParaUtil::GetMaxSpeedStr(mapMaxSpeedStr &roadWayMapStr, const UINT32 channelId)
{
	XMLNode Setting, ChannelNodes, ChannelNode, TempNode, ChannelWays, ChannelWay;
	XMLCSTR strText;
	string xmlFile;

	roadWayMapStr.clear();

	//取得xml文件名
	if( (g_nMultiPreSet == 0) && (g_nSwitchCamera == 0) )
	{
		xmlFile = "./config/RoadParameter.xml";
	}
	else
	{
		int nCameraID = g_skpDB.GetCameraID(channelId);
		int nPreSet = 0;
		if(g_nMultiPreSet == 1)
		{
			nPreSet = g_skpDB.GetPreSet(channelId);
		}
		if(nPreSet > 0)
		{
			char buf[256]={0};
			sprintf(buf,"./config/Camera%d", nCameraID);
			string strPath(buf);
			if(access(strPath.c_str(), F_OK) == 0)
			{
				if(g_nMultiPreSet == 0)
					sprintf(buf,"%s/RoadParameter.xml", strPath.c_str());
				else
					sprintf(buf,"%s/RoadParameter-PreSet%d.xml", strPath.c_str(), nPreSet);
			}
			xmlFile = buf;
		}
	}

	if(access(xmlFile.c_str(), F_OK) == 0) //存在
	{
		Setting = XMLNode::parseFile(xmlFile.c_str());
		Setting = Setting.getChildNode("RoadParameter");
		if (!Setting.isEmpty())
		{
			//通道
			ChannelNodes = Setting.getChildNode("Channels");

			//通道个数
			int nChannels = ChannelNodes.nChildNode();
			for(int i = 0; i < nChannels; i++)
			{
				ChannelNode = ChannelNodes.getChildNode(i); //通道节点

				TempNode = ChannelNode.getChildNode("ChannelId");
				if(!TempNode.isEmpty())
				{
					if( channelId == xmltoi(TempNode.getText()) )//匹配通道ID
					{
						ChannelWays = ChannelNode.getChildNode("ChannelWays");
						if (!ChannelWays.isEmpty())
						{
							int roadWayId = 0;
							int waysCount = ChannelWays.nChildNode();
							for (int i = 0; i < waysCount; i++)
							{
								ChannelWay = ChannelWays.getChildNode(i);
								if(!ChannelWay.isEmpty())
								{
									TempNode = ChannelWay.getChildNode("ChannelWayId");
									if(!TempNode.isEmpty())
									{
										roadWayId = xmltoi(TempNode.getText());
										TempNode = ChannelWay.getChildNode("IsModel");
										if(!TempNode.isEmpty())
										{
											MaxSpeedStr maxSpeedStr;
											if(xmltoi(TempNode.getText()) <= 0)//是否载入模板
											{
												TempNode = ChannelWay.getChildNode("AvgSpeedMax");
												if(!TempNode.isEmpty())
												{
													maxSpeedStr.nRadarAlarmVelo = xmltoi(TempNode.getText());													
												}

												TempNode = ChannelWay.getChildNode("AlarmBigMax");
												if(!TempNode.isEmpty())
												{
													maxSpeedStr.uAlarmBig = xmltoi(TempNode.getText());
												}

												TempNode = ChannelWay.getChildNode("AlarmSmallMax");
												if(!TempNode.isEmpty())
												{
													maxSpeedStr.uAlarmSmall = xmltoi(TempNode.getText());
												}	

												//LogNormal("xml--[%d,%d,%d],\n", maxSpeedStr.nRadarAlarmVelo, maxSpeedStr.uAlarmBig, maxSpeedStr.uAlarmSmall);


												roadWayMapStr[roadWayId] = maxSpeedStr;
											}
											else
											{
												TempNode = ChannelWay.getChildNode("ModelId");
												if(!TempNode.isEmpty())
												{
													int modelId = xmltoi(TempNode.getText());
													//maxSpeed = GetMaxSpeedModel(modelId);
													maxSpeedStr = GetMaxSpeedStrModel(modelId);
													roadWayMapStr[roadWayId] = maxSpeedStr;
												}
											}
										}
									}
								}
							}
						}
						break;
					}
				}
			}
		}
	}
}

/*
//取得模板中的限速
UINT32 CXmlParaUtil::GetMaxSpeedModel(int modelId)
{
	XMLNode Setting, Models, Model, TempNode;
	XMLSTR xmlStr;
	string xmlFile = "./profile/RoadParameterModel.xml";
	if(access(xmlFile.c_str(),F_OK) == 0) //存在
	{
		Setting = XMLNode::parseFile(xmlFile.c_str());
		if(!Setting.isEmpty())
		{
			Setting = Setting.getChildNode("RoadParameter");
			if(!Setting.isEmpty())
			{
				Models = Setting.getChildNode("Models");
				if(!Models.isEmpty())
				{
					int modelCount = Models.nChildNode();
					for(int i = 0; i < modelCount; i++)
					{
						Model = Models.getChildNode(i);
						if(!Model.isEmpty())
						{
							TempNode = Model.getChildNode("ModelId");
							if(!TempNode.isEmpty())
							{
								if( xmltoi(TempNode.getText()) == modelId ) //匹配模板ID
								{
									TempNode = Model.getChildNode("AvgSpeedMax");
									if(!TempNode.isEmpty())
									{
										return xmltoi(TempNode.getText());
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
*/

//取得模板中的限速
MaxSpeedStr CXmlParaUtil::GetMaxSpeedStrModel(int modelId)
{
	MaxSpeedStr maxSpeedStrTmp;
	XMLNode Setting, Models, Model, TempNode;
	XMLSTR xmlStr;
	string xmlFile = "./profile/RoadParameterModel.xml";
	if(access(xmlFile.c_str(),F_OK) == 0) //存在
	{
		Setting = XMLNode::parseFile(xmlFile.c_str());
		if(!Setting.isEmpty())
		{
			Setting = Setting.getChildNode("RoadParameter");
			if(!Setting.isEmpty())
			{
				Models = Setting.getChildNode("Models");
				if(!Models.isEmpty())
				{
					int modelCount = Models.nChildNode();
					for(int i = 0; i < modelCount; i++)
					{
						Model = Models.getChildNode(i);
						if(!Model.isEmpty())
						{
							TempNode = Model.getChildNode("ModelId");
							if(!TempNode.isEmpty())
							{
								if( xmltoi(TempNode.getText()) == modelId ) //匹配模板ID
								{
									TempNode = Model.getChildNode("AvgSpeedMax");
									if(!TempNode.isEmpty())
									{
										maxSpeedStrTmp.nRadarAlarmVelo = xmltoi(TempNode.getText());
									}

									TempNode = Model.getChildNode("AlarmBigMax");
									if(!TempNode.isEmpty())
									{
										maxSpeedStrTmp.uAlarmBig = xmltoi(TempNode.getText());
									}

									TempNode = Model.getChildNode("AlarmSmallMax");
									if(!TempNode.isEmpty())
									{
										maxSpeedStrTmp.uAlarmSmall = xmltoi(TempNode.getText());
									}									
								}
							}
						}
					}
				}
			}
		}
	}
	return maxSpeedStrTmp;
}

//北京H264视频传输协议，解析登陆报文
void CXmlParaUtil::ParserLoginXml(const string& xmlstr, string& name, string& passwd)
{
	XMLNode Xml, Setting, Temp;
	Xml = XMLNode::parseString(xmlstr.c_str());
	if(!Xml.isEmpty())
	{
		cerr<<"xml 00"<<endl;
		Setting = Xml.getChildNode("LoginInfo");
		if(!Setting.isEmpty())
		{
			cerr<<"xml 11"<<endl;
			Temp = Setting.getChildNode("Name");
			if(!Temp.isEmpty())
			{
				cerr<<"xml Name"<<endl;
				name = Temp.getText();
			}

			cerr<<"xml 22"<<endl;
			Temp = Setting.getChildNode("Password");
			if(!Temp.isEmpty())
			{
				cerr<<"xml Passwd"<<endl;
				passwd = Temp.getText();
			}
		}
	}
	cerr<<"xml 33"<<endl;
}

//北京H264视频传输协议，解析视频切换报文
bool CXmlParaUtil::ParserSwitchCameraXml(const string& xmlstr, map<string, UINT32>&cameraMap)
{
	cerr<<"ParserSwitchCameraXml() in"<<endl;
	XMLNode Xml, Setting, Temp;
	Xml = XMLNode::parseString(xmlstr.c_str());
	if(!Xml.isEmpty())
	{
		cerr<<"xml 00"<<endl;
		Setting = Xml.getChildNode("SwitchCamera");
		if(!Setting.isEmpty())
		{
			cerr<<"xml 11"<<endl;
			Temp = Setting.getChildNode("OldCameraID");
			if(!Temp.isEmpty() && Temp.getText()!=NULL)
			{
				UINT32 oldCameraId = atoi(Temp.getText());
				cameraMap.insert(make_pair("OldCameraID", oldCameraId));
			}

			cerr<<"xml 22"<<endl;
			Temp = Setting.getChildNode("NewCameraID");
			if(!Temp.isEmpty() && Temp.getText()!=NULL)
			{
				UINT32 newCameraId = atoi(Temp.getText());
				cameraMap.insert(make_pair("NewCameraID", newCameraId));
			}

			Temp = Setting.getChildNode("PreSetID");
			if(!Temp.isEmpty() && Temp.getText()!=NULL)
			{
				UINT32 preSetId = atoi(Temp.getText());
				cameraMap.insert(make_pair("PreSetID", preSetId));
			}
		}
	}
	cerr<<"ParserSwitchCameraXml() out"<<endl;
	if (cameraMap.size() == 3)
		return true;
	return false;
}


//北京H264视频传输协议，封装Event成xml
void CXmlParaUtil::CreateEventToXml(int cameraID, const RECORD_EVENT* sEvent, string& xmlstr)
{
	char buf[20] = {0};
	string timeStr = GetTime(sEvent->uEventBeginTime, 0);

	sprintf(buf, "%d", sEvent->uMiEventBeginTime);
	timeStr += ":" + string(buf);
	
	sprintf(buf, "%d", cameraID);
	string cameraIDstr(buf);

	sprintf(buf, "%d", sEvent->uSeq);
	string seqStr(buf);

	sprintf(buf, "%d", sEvent->uDirection);
	string directionStr(buf);

	//将取得的图片进行base64编码
	string base64Pic = "";
	string isShow = "0";
	if((int)sEvent->chReserved[0] > 0) //是否停留显示为true时
	{
		isShow = "1";
	}
	else
	{
		string strPicPath(sEvent->chPicPath);
		string strPic = GetImageByPath(strPicPath);
		EncodeBase64(base64Pic, (unsigned char*)strPic.c_str(), strPic.size());
	}
	sprintf(buf,"%d",sEvent->uCode);
	string codeStr(buf);

	sprintf(buf,"%d",sEvent->uColor1);
	string colorStr(buf);

	sprintf(buf,"%d",sEvent->uPosX);
	string posXStr(buf);

	sprintf(buf,"%d",sEvent->uPosY);
	string posYStr(buf);

	XMLNode xml, setting, subSetting;
	xml = XMLNode::createXMLTopNode("EventInfo");
    setting = xml.addChild("Time");
    setting.addText(timeStr.c_str());
	setting = xml.addChild("CameraID");
    setting.addText(cameraIDstr.c_str());
	setting = xml.addChild("CameraLoc");
    setting.addText(sEvent->chPlace);
	setting = xml.addChild("Seq");
    setting.addText(seqStr.c_str());
	setting = xml.addChild("EventType");
    setting.addText(codeStr.c_str());
	setting = xml.addChild("EventColor");
    setting.addText(colorStr.c_str());
	setting = xml.addChild("Pic");
    setting.addText(base64Pic.c_str());
	setting = xml.addChild("Direction");
    setting.addText(directionStr.c_str());
	setting = xml.addChild("IsTrace");
    setting.addText(isShow.c_str());

	setting = xml.addChild("Point");
	subSetting = setting.addChild("x");
    subSetting.addText(posXStr.c_str());
	subSetting = setting.addChild("y");
    subSetting.addText(posYStr.c_str());

	xmlstr = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	xmlstr.append(strData, nSize);
}

//生成检测配置获取请求(从AMS获取检测配置)
void CXmlParaUtil::CreateRequestAMS(vector<UINT32>& cameraVec, string& xmlStr)
{
	XMLNode xml, tmp;
	xml = XMLNode::createXMLTopNode("DetectSettingRequestInfo");

	char buf[10] = {0};
	for(int i = 0; i < cameraVec.size(); i++)
	{
		sprintf(buf, "%d", cameraVec[i]);
		tmp = xml.addChild("CameraID");
		tmp.addText(buf);
	}
	xmlStr = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	xmlStr.append(strData, nSize);
}

//解析并从AMS取得的配置文件,并替换本地文件
bool CXmlParaUtil::ParseAndUpdateSetting(string& xmlStr)
{
	XMLNode xml, cameras, camera, presets, preset, tmp;
	xml = XMLNode::parseString(xmlStr.c_str());
	if(!xml.isEmpty())
	{
		tmp = xml.getChildNode("DetectSettingResponseInfo");
		if (!tmp.isEmpty())
		{
			cameras = tmp.getChildNode("Cameras");
			if(!cameras.isEmpty())
			{
				int camerasCount = cameras.nChildNode();
				for (int i = 0; i< camerasCount; i++)
				{
					camera = cameras.getChildNode(i);
					if (!camera.isEmpty())
					{
						string cameraID = "";
						tmp = camera.getChildNode("CameraID");
						if (!tmp.isEmpty())
						{
							cameraID = tmp.getText();
						}

						presets = camera.getChildNode("Presets");
						if (!presets.isEmpty())
						{
							int presetsCount = presets.nChildNode();
							for (int i =0; i<presetsCount; i++)
							{
								preset = presets.getChildNode(i);
								if (!preset.isEmpty())
								{
									int nChildCount = preset.nChildNode();
									string presetID = "";
									tmp = preset.getChildNode("PresetID");
									if (!tmp.isEmpty())
									{
										presetID = tmp.getText();
									}
									if (presetID == "")
										continue;

									char buf[100];
									string path = "";

									//取得配置信息并写入文件
									for (int i = 1; i < nChildCount; i++)
									{
										tmp = preset.getChildNode(i);
										if (!tmp.isEmpty())
										{
											sprintf(buf, "./config/Camera%s", cameraID.c_str());
											path = buf;

											//如果目录不存在,创建目录
											if(access(path.c_str(),0) != 0)
												mkdir(path.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);

											string fileName = tmp.getName();
											if ( strcmp(tmp.getName(), "RoadParameter") != 0 &&
												strcmp(tmp.getName(), "RoadSettingInfo") != 0)
												fileName += "Info";

											if (presetsCount == 1)
												sprintf(buf, "%s/%s.xml", path.c_str(), fileName.c_str());
											else
												sprintf(buf, "%s/%s-PreSet%s.xml", path.c_str(), fileName.c_str(), presetID.c_str());

											cerr<<"WriteFile="<<buf<<endl;
											XMLNode out = XMLNode::parseString(tmp.createXMLString());
											if(!out.writeToFile(buf))
											{
												LogError("生成\"%s\"配置文件失败!\r\n", buf);
												return false;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return true;
}

//解析xml并修改检测类型
int CXmlParaUtil::ParseAndSetDetectKind(string& xmlStr)
{
	//cerr<<xmlStr<<endl;
	XMLNode xml, setting, tmp;
	int detectKind = -1;
	xml = XMLNode::parseString(xmlStr.c_str());
	if(!xml.isEmpty())
	{
		setting = xml.getChildNode("DetectRequestInfo");
		if(!setting.isEmpty())
		{
			tmp = setting.getChildNode("DetectKind");
			if(!tmp.isEmpty())
			{
				detectKind = atoi(tmp.getText());
			}
		}
	}
	return detectKind;
}

//解析xml并替换系统设置
bool CXmlParaUtil::ParseAndSetSystemSetting(string& xmlStr)
{
	XMLNode xml, pic, video, com, light, other, yuntai, monitor, amsHost, setting, tmp;
	xml = XMLNode::parseString(xmlStr.c_str());
	if(!xml.isEmpty())
	{
		setting = xml.getChildNode("SystemRequestInfo");
		if(!setting.isEmpty())
		{
			pic = setting.getChildNode("PicFormatInfo");
			video = setting.getChildNode("VideoFormatInfo");
			com = setting.getChildNode("ComSetting");
			light = setting.getChildNode("LightTimeSetting");
			other = setting.getChildNode("OtherSetting");
			yuntai = setting.getChildNode("YunTaiSetting");
			monitor = setting.getChildNode("MonitorHostSetting");
			amsHost = setting.getChildNode("AmsHostSetting");
		}
	}

	string xmlFile = "./system.xml";
	if(access(xmlFile.c_str(),F_OK) == 0)//存在
	{
		XMLCSTR strText;
		xml = XMLNode::parseFile(xmlFile.c_str());
		setting = xml.getChildNode("SystemSetting");
		if(!setting.isEmpty())
		{
			tmp = setting.getChildNode("PicFormatInfo");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(pic);

			tmp = setting.getChildNode("VideoFormatInfo");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(video);

			tmp = setting.getChildNode("ComSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(com);

			tmp = setting.getChildNode("LightTimeSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(light);

			tmp = setting.getChildNode("OtherSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(other);

			tmp = setting.getChildNode("YunTaiSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(yuntai);

			tmp = setting.getChildNode("MonitorHostSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(monitor);

			tmp = setting.getChildNode("AmsHostSetting");
			if (!tmp.isEmpty())
			{
				tmp.deleteNodeContent();
			}
			setting.addChild(amsHost);

			//写入文件
			if( !xml.writeToFile(xmlFile.c_str()) )
			{
				LogError("生成\"%s\"配置文件失败!\r\n", xmlFile.c_str());
				return false;
			}

			//重读配置文件
			LoadSystemSetting();
			return true;
		}
	}
	return false;
}

//载入打印模块时间信息设置
bool CXmlParaUtil::LoadTimeLogSetting()
{
	std::string strSystemXml = "./timelog.xml";

	if(access(strSystemXml.c_str(),F_OK) == 0)//存在
	{
		g_nPrintfTime = 1;
		return true;
	}
	return false;
}

//解析视频切换请求
bool CXmlParaUtil::ParseSwitchCamera(string& xmlStr)
{
	printf("================= CXmlParaUtil::ParseSwitchCamera  start ! \n");
	SRIP_CHANNEL sChannel;
	int nOldCameraID = 0;

	XMLNode xml, TempNode,RoadSettingInfoNode,RoadParaInfoNode,BehaviorParameterInfoNode;
	xml = XMLNode::parseString(xmlStr.c_str());
	if(!xml.isEmpty())
	{
		XMLCSTR strText;

		/*TempNode = xml.getChildNode("ChannelID");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.uId = xmltoi(strText);
			}
		}*/

		TempNode = xml.getChildNode("OldCameraID");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				nOldCameraID = xmltoi(strText);
				printf("====== CXmlParaUtil::ParseSwitchCamera  nOldCameraID = %d \n",nOldCameraID);
			}
		}

		TempNode = xml.getChildNode("NewCameraID");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.nCameraId = xmltoi(strText);
				printf("====== CXmlParaUtil::ParseSwitchCamera  sChannel.nCameraId = %d \n",sChannel.nCameraId);
			}
		}

		TempNode = xml.getChildNode("Location");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				memcpy(sChannel.chPlace, strText,sizeof(strText));
				//sChannel.chPlace = xmltoi(strText);
				printf("====== CXmlParaUtil::ParseSwitchCamera  sChannel.chPlace = %s \n",sChannel.chPlace);
			}
		}

		TempNode = xml.getChildNode("PreSetID");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.nPreSet = xmltoi(strText);
				printf("====== CXmlParaUtil::ParseSwitchCamera  sChannel.nPreSet = %d \n",sChannel.nPreSet);
			}
		}

		TempNode = xml.getChildNode("VideoType");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				
				//xmltoi(strText);
			}
		}

		TempNode = xml.getChildNode("VideoBeginTime");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.uVideoBeginTime = xmltoi(strText);
			}
		}

		TempNode = xml.getChildNode("VideoEndTime");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.uVideoEndTime = xmltoi(strText);
			}
		}

		TempNode = xml.getChildNode("DetectKind");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				sChannel.uDetectKind = (CHANNEL_DETECT_KIND)xmltoi(strText);
			}
		}

		LogNormal("sChannel.uId=%d,sChannel.nCameraId=%d,sChannel.uDetectKind=%d\n",sChannel.uId,sChannel.nCameraId,sChannel.uDetectKind);

		TempNode = xml.getChildNode("DetectSetting");
		if(!TempNode.isEmpty())
		{
			//车道坐标信息
			RoadSettingInfoNode = TempNode.getChildNode("RoadSettingInfo");

			if(!RoadSettingInfoNode.isEmpty())
			{
				char buf[256] = {0};
				sprintf(buf,"./config/Camera%d/RoadSettingInfo.xml",sChannel.nCameraId);

				RoadSettingInfoNode.writeToFile(buf);
			}

			//车道参数
			RoadParaInfoNode = TempNode.getChildNode("RoadParameter");
			if(!RoadParaInfoNode.isEmpty())
			{
				char buf[256] = {0};
				sprintf(buf,"./config/Camera%d/RoadParameter.xml",sChannel.nCameraId);

				RoadParaInfoNode.writeToFile(buf);
			}

			//行为分析参数
			BehaviorParameterInfoNode = TempNode.getChildNode("BehaviorParameter");
			if(!BehaviorParameterInfoNode.isEmpty())
			{
				char buf[256] = {0};
				sprintf(buf,"./config/Camera%d/BehaviorParameterInfo.xml",sChannel.nCameraId);

				BehaviorParameterInfoNode.writeToFile(buf);
			}
		}
		
		
		int uChannelId = 0;
		if (nOldCameraID > 0 && g_skpDB.UpdateSwitchChannelInfo(sChannel,nOldCameraID,uChannelId))
		{
			printf("g_skpDB.UpdateSwitchChannelInfo ===== SUCCESS !  \n");
			sChannel.uId = uChannelId;
			printf("g_skpDB.UpdateSwitchChannelInfo ===== sChannel.uId = %d  \n",sChannel.uId);
			if (!g_skpChannelCenter.UpdateChannel(sChannel))
			{
					printf("g_skpChannelCenter.UpdateChannel ===== ERROR !  \n");
			}
				//g_skpChannelCenter.UpdateChannel(sChannel);
		}
		
		//else if(g_skpDB.UpdateChannelInfo(sChannel))
		//{
		//	g_skpChannelCenter.UpdateChannel(sChannel);
		//}
		else
		{
			printf("g_skpDB.UpdateSwitchChannelInfo ===== ERROR !  \n");
			sChannel.uId = g_skpDB.CreateChannelId();
			printf("g_skpDB.CreateChannelId ===== SUCCESS !  \n");
			printf("g_skpDB.CreateChannelId =====  sChannel.uId = %d  \n",sChannel.uId);	
			g_skpDB.AddChan(sChannel);
		}

		return true;
	}

	return false;
}

//载入dsp配置文件
bool CXmlParaUtil::LoadDspSettingFile(MvDspGlobalSetting& mvdspGlobalSetting,int nChannel)
{
	bool bReadFileFlag = false;

	UINT32 uDspSettingSize = sizeof(MvDspGlobalSetting);

	//读取本地算法自定义结构文件,二进制流
	int nFileSize = 0;
	char szBuffer[4096] = {0};
	int nReadSum = 0;
	int nBytesRead = 0;
	FILE *fpIn = NULL;
	char file_name[256] = {0};
	sprintf(file_name, "./config/DspSetting%d.aa", nChannel);
	fpIn = fopen(file_name, "rb");
	if(fpIn != NULL)
	{
		fseek(fpIn, SEEK_SET, SEEK_END);
		nFileSize = ftell(fpIn);
		printf("==file==size=%d==\n", nFileSize);

		fseek(fpIn, 0, SEEK_SET);
		if(uDspSettingSize == nFileSize)
		{
			while(nReadSum < nFileSize - 1)
			{
				memset(szBuffer, 0, 4096);
				fseek(fpIn, (int)nReadSum, SEEK_SET);
				nBytesRead = fread((char*)szBuffer, 1, 4096, fpIn);
				memcpy((char*)((MvDspGlobalSetting*)(&mvdspGlobalSetting)) + nReadSum,\
					szBuffer, nBytesRead);
				nReadSum += nBytesRead;
			}
			//printf("=dwParamLen=%d==nBytesRead=%d==nReadSum=%d=nFileSize=%d\r\n", \
				dwParamLen, nBytesRead, nReadSum, nFileSize);

			if(nReadSum == nFileSize)
			{
				bReadFileFlag = true;
			}
			else
			{
				LogNormal("=load--Read file:%x error! nReadSum=%d !\n", file_name, nReadSum);
			}
		}
		else
		{
			bReadFileFlag = false;
		}

		fclose(fpIn);
		fpIn = NULL;
	}

	//LogNormal("-bReadFileFlag=%d--nChannel=%d LoadDspSettingFile\n", bReadFileFlag, nChannel);
	return bReadFileFlag;
}

//载入DspSetting参数
////标定Calibration m_calibration;
bool CXmlParaUtil::LoadDspSettingInfo(MvDspGlobalSetting &dspSettingStrc, CALIBRATION &calibration, int nChannel)
{
    #ifdef _DEBUG
        printf("LoadDspSettingInfo========nChannel=%d======\n\r", nChannel);
    #endif

    if(nChannel < 0)
    {
        printf("=========LoadDspSettingInfo wrong nChannel number ERROR!!!\n\r");
        return false;
    }
    
	char buf[256]={0};
	sprintf(buf,"./config/MvdspGlobalSetting%d.xml", nChannel);
	std::string strDspSettingInfoXml(buf);

    //车道，区域，区域元素，点
    XMLNode XmlNode, ChannelWaysNode, GlobalLineNode, MvDspRgnsNode, CalibrationNode;
	XMLNode TempNode, ChannelWayNode, RectNode, MvDspRgnNode;
	XMLNode TempNode1, TempNode2;
	XMLCSTR strText;

    int iTemp = 0;
	float fTemp = 0.0f;
    int nPoints = 0;

	printf("strDspSettingInfoXml.c_str()=%s\n",strDspSettingInfoXml.c_str());

    if (access(strDspSettingInfoXml.c_str(), F_OK) == 0) //存在
    {
        //xml文件
		//读取到内存在解析
		std::string strXmlDspSetting="";
		char *pXmlData = NULL;
		char szBuffer[4096] = {0};
		FILE *fpIn = NULL;
		fpIn = fopen(strDspSettingInfoXml.c_str(), "rb");
		int nFileSize = 0;
		int nReadSum = 0;
		int nBytesRead = 0;
		if( fpIn != NULL)
		{
			fseek(fpIn, SEEK_SET, SEEK_END);
			nFileSize = ftell(fpIn);

			pXmlData = new char[nFileSize+1];

			printf("==pXmlData==size=%d==\n", nFileSize);

			fseek(fpIn, 0, SEEK_SET);

			while(nReadSum < nFileSize - 1)
			{
				memset(szBuffer, 0, 4096);
				fseek(fpIn, (int)nReadSum, SEEK_SET);
				nBytesRead = fread((char*)szBuffer, 1, 4096, fpIn);
				memcpy(pXmlData + nReadSum, szBuffer, nBytesRead);

				nReadSum += nBytesRead;
			}

			printf("==after read=pXmlData=\n");

			strXmlDspSetting.reserve(102400); //100k
			strXmlDspSetting.append((char*)(pXmlData), nFileSize);

			if(pXmlData != NULL)
			{
				delete pXmlData;
				pXmlData = NULL;
			}			
		}

		if(fpIn != NULL)
		{
			fclose(fpIn);
			fpIn = NULL;
		}

		if(strXmlDspSetting.size() > 10)
		{
			printf("=strXmlDspSetting.size()=%d=\n", strXmlDspSetting.size());
			//文件根节点即为DSP_GLOBAL_SETTING
			XmlNode = XMLNode::parseString(strXmlDspSetting.c_str()); 
		}
        //XmlNode = XMLNode::parseFile(strDspSettingInfoXml.c_str()).getChildNode("DSP_GLOBAL_SETTING");

	//	int nWidth;           //图像的宽度（应用配置）
		TempNode = XmlNode.getChildNode("Width");
		dspSettingStrc.nWidth =  GetIntValFromNode(TempNode);
	//	int nHeight;          //图像的高度（应用配置）
		TempNode = XmlNode.getChildNode("Height");	
		dspSettingStrc.nHeight =  GetIntValFromNode(TempNode);
	//	float m_fScaleX;			//m_nWidth*m_nScaleX==原图的宽-  宽度的缩放比例（应用配置	
		TempNode = XmlNode.getChildNode("ScaleX");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			fTemp = xmltof(strText);
		}
		dspSettingStrc.m_fScaleX = fTemp;
		

	//	float m_fScaleY;            //高度的缩放比例（应用配置）
		TempNode = XmlNode.getChildNode("ScaleY");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			fTemp = xmltof(strText);
		}
		dspSettingStrc.m_fScaleY = fTemp;

		LogTrace("GetFromNode.log", "=ScaleX:%f, ScaleY:%f", dspSettingStrc.m_fScaleX, dspSettingStrc.m_fScaleY);
	//	int nCheckType;			//做检测的类型（应用配置）
		TempNode = XmlNode.getChildNode("CheckType");
		dspSettingStrc.nCheckType =  GetIntValFromNode(TempNode);

	//标定信息
		TempNode = XmlNode.getChildNode("Calibration");		
		TempNode1 = TempNode.getChildNode("Length");
		if(!TempNode1.isEmpty())
		{
			strText = TempNode1.getText();
			calibration.length = xmltof(strText);			
		}

		TempNode1 = TempNode.getChildNode("Width");
		if(!TempNode1.isEmpty())
		{
			strText = TempNode1.getText();
			calibration.width = xmltof(strText);
		}

		TempNode1 = TempNode.getChildNode("CameraHeight");
		if(!TempNode1.isEmpty())
		{
			strText = TempNode1.getText();
			calibration.cameraHeight = xmltof(strText);
		}

		LogTrace("GetFromNode.log", "=calibration:[length, width, cameraHeight]=[%f, %f, %f]", \
			calibration.length, calibration.width, calibration.cameraHeight);

		TempNode1 = TempNode.getChildNode("CalibrationArea");
		GetListPointsFromNode(calibration.region.listPT, TempNode1);
		TempNode1 = TempNode.getChildNode("CalibrationPoint");
		GetListPointsFromNode(calibration.listPT, TempNode1);
		TempNode1 = TempNode.getChildNode("CalibrationWorld");
		GetListPointsFromNode(calibration.list32fPT, TempNode1);

	//	MvRect m_rtCarNum;		//车牌检测区,相对于小图中位置（应用配置）
		RectNode = XmlNode.getChildNode("CarNumRect");
		GetRectFromNode(dspSettingStrc.m_rtCarNum, RectNode);

	//车道信息
		ChannelWaysNode = XmlNode.getChildNode("ChannelRegions");
		
	//	int nChannels;					//车道数目（应用配置）
		int nChannelWaysCount = ChannelWaysNode.nChildNode();
		dspSettingStrc.nChannels = nChannelWaysCount;

	//	DspChannelRegion ChnlRegion[8];    //车道属性（应用配置）		//
		if(nChannelWaysCount > 0)
		{
			for(int i=0; i<nChannelWaysCount; i++)
			{
				ChannelWayNode = ChannelWaysNode.getChildNode(i);

				//DspChannelRegion dspSettingStrc.ChnlRegion[i];

			//int			nRoadIndex;	//车道序号（应用配置）
				TempNode = ChannelWayNode.getChildNode("RoadIndex");		
				dspSettingStrc.ChnlRegion[i].nRoadIndex =  GetIntValFromNode(TempNode);
			//int			nVerRoadIndex; //车道逻辑序号，暂时没有用处。（应用配置）
				TempNode = ChannelWayNode.getChildNode("VerRoadIndex");	
				dspSettingStrc.ChnlRegion[i].nVerRoadIndex =  GetIntValFromNode(TempNode);
			//int			nDirection;    //车道方向（应用配置）
				TempNode = ChannelWayNode.getChildNode("Direction");	
				dspSettingStrc.ChnlRegion[i].nDirection =  GetIntValFromNode(TempNode);
			//DspMvLine		vDirection;    //车道方向线（应用配置）
				TempNode = ChannelWayNode.getChildNode("DirectionLine");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vDirection, TempNode);
			//DspMvPoint		arrListChannel[16];     //车道区域（应用配置）
			//int			nChannlePointNumber;  //构成每个车道区域的顶点个数（应用配置）
				TempNode = ChannelWayNode.getChildNode("ChannelPoints");
				 dspSettingStrc.ChnlRegion[i].nChannlePointNumber = GetMvPointsFromNode(dspSettingStrc.ChnlRegion[i].arrListChannel, TempNode);
			//int			nPhysicalLoop;			//物理线圈个数（应用配置）
				TempNode = ChannelWayNode.getChildNode("PhysicalLoop");	
				dspSettingStrc.ChnlRegion[i].nPhysicalLoop = GetIntValFromNode(TempNode);
			//float		fLoopDist;				//物理线圈之间的距离（应用配置，默认为5m）
				TempNode = ChannelWayNode.getChildNode("LoopDist");
				if(!TempNode.isEmpty())
				{
					strText = TempNode.getText();
					fTemp = xmltof(strText);
				}
				dspSettingStrc.ChnlRegion[i].fLoopDist = fTemp;

				LogTrace("GetFromNode.log", "=LoopDist=:[%d]=[%f]", \
					i, dspSettingStrc.ChnlRegion[i].fLoopDist);
			
			//int			bNoTurnLeft;		//禁左（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoTurnLeft");
				dspSettingStrc.ChnlRegion[i].bNoTurnLeft = GetIntValFromNode(TempNode);
			//int			bNoTurnRight;		//禁右（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoTurnRight");
				dspSettingStrc.ChnlRegion[i].bNoTurnRight = GetIntValFromNode(TempNode);
			//int			bNoForeward;		//禁止前行（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoForeward");
				dspSettingStrc.ChnlRegion[i].bNoForeward = GetIntValFromNode(TempNode);
			//int     bNoReverse;				// 禁止逆行（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoReverse");
				dspSettingStrc.ChnlRegion[i].bNoReverse = GetIntValFromNode(TempNode);
			//int     bNoPressLine;			//禁止压线！（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoPressLine");
				dspSettingStrc.ChnlRegion[i].bNoPressLine = GetIntValFromNode(TempNode);
			//int     bNoChangeChannel;		//禁止变道！（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoChangeChannel");
				dspSettingStrc.ChnlRegion[i].bNoChangeChannel = GetIntValFromNode(TempNode);
			//int     nNoTurnAround;          //禁止调头 ——应用配置
				TempNode = ChannelWayNode.getChildNode("NoTurnAround");
				dspSettingStrc.ChnlRegion[i].nNoTurnAround = GetIntValFromNode(TempNode);
			//int     nNonMotorWay;           // 是否为非机动车道标志符， 1表示非机动车道，非1为其它类型车道 ——应用配置
				TempNode = ChannelWayNode.getChildNode("NonMotorWay");
				dspSettingStrc.ChnlRegion[i].nNonMotorWay = GetIntValFromNode(TempNode);
			//int     bFlagHoldStopReg;  //是否存在待转区（应用配置）
				TempNode = ChannelWayNode.getChildNode("FlagHoldStopReg");
				dspSettingStrc.ChnlRegion[i].bFlagHoldStopReg = GetIntValFromNode(TempNode);
			////车道行驶属性:直行车道、左转车道、右转车道、左转+直行车道、右转+直行车道,等！
			//int      nChannelDriveDir; //（应用配置）
				TempNode = ChannelWayNode.getChildNode("ChannelDriveDir");
				dspSettingStrc.ChnlRegion[i].nChannelDriveDir = GetIntValFromNode(TempNode);

			//DspMvLine   vForeLine;  //每车道一个前行线！（应用配置）
				TempNode = ChannelWayNode.getChildNode("ForeLine");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vForeLine, TempNode);
			////每个车道一个停止线
			//DspMvLine   vStopLine;  //（应用配置）
				TempNode = ChannelWayNode.getChildNode("StopLine");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vStopLine, TempNode);
			////待转区
			
			//DspMvLine   vHoldForeLineFirst;  //待转区第一前行线（应用配置）				
				TempNode = ChannelWayNode.getChildNode("HoldForeLineFirst");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vHoldForeLineFirst, TempNode);
			//DspMvLine   vHoldForeLineSecond;  //待转区第二前行线（应用配置）
				TempNode = ChannelWayNode.getChildNode("HoldForeLineSecond");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vHoldForeLineSecond, TempNode);
			//DspMvLine   vHoldStopLineFirst;    //待转区第一停止线（应用配置）
				TempNode = ChannelWayNode.getChildNode("HoldStopLineFirst");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vHoldStopLineFirst, TempNode);
			//DspMvLine   vHoldStopLineSecond;    //待转区第二停止线（应用配置）
				TempNode = ChannelWayNode.getChildNode("HoldStopLineSecond");
				GetMvLineFromNode(dspSettingStrc.ChnlRegion[i].vHoldStopLineSecond, TempNode);

			//MvRect OnOffRed;      //红灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("OffRedRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].OnOffRed, RectNode);
			//MvRect OnOffGreen;    //绿灯区域（应用配置）				//防闪用
				RectNode = ChannelWayNode.getChildNode("OffGreenRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].OnOffGreen, RectNode);

			//MvRect roiLeftLight;   //左边灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("LeftLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiLeftLight, RectNode);
			//MvRect roiMidLight;    //中间灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("MidLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiMidLight, RectNode);
			//MvRect roiRightLight;  //右边灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("RightLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiRightLight, RectNode);

			//MvRect roiTurnAroundLight; //拐弯灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("TurnAroundLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiTurnAroundLight, RectNode);

			////红灯增强用的
			//MvRect roiLeftLight_red, roiLeftLight_green;  //左边红、绿灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("RedLeftLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiLeftLight_red, RectNode);
				RectNode = ChannelWayNode.getChildNode("GreenLeftLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiLeftLight_green, RectNode);
			//MvRect roiMidLight_red, roiMidLight_green;    //中间红、绿灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("RedMidLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiMidLight_red, RectNode);
				RectNode = ChannelWayNode.getChildNode("GreenMidLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiMidLight_green, RectNode);
			//MvRect roiRightLight_red, roiRightLight_green; //右边红、绿灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("RedRightLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiRightLight_red, RectNode);
				RectNode = ChannelWayNode.getChildNode("GreenRightLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiRightLight_green, RectNode);
			//MvRect roiTurnAroundLight_red, roiTurnAroundLight_green; //拐弯红、绿灯区域（应用配置）
				RectNode = ChannelWayNode.getChildNode("RedTurnAroundLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiTurnAroundLight_red, RectNode);
				RectNode = ChannelWayNode.getChildNode("GreenTurnAroundLightRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].roiTurnAroundLight_green, RectNode);
			//MvRect rectMedianPos;  //闯红灯或电警时，由客户端指定的第二张图的车辆位置
				RectNode = ChannelWayNode.getChildNode("MedianPosRect");
				GetRectFromNode(dspSettingStrc.ChnlRegion[i].rectMedianPos, RectNode);

			//DspNoPassingInfo   vecNoPassingInfo[24]; //禁止通过时间段属性（应用配置）
			//int             nNoPassingInfoNumber;  //禁止通过的时间段个数（应用配置）
				TempNode = ChannelWayNode.getChildNode("NoPassingInfos");
				int nNoPassingInfoNumber = TempNode.nChildNode();

				if(nNoPassingInfoNumber > 0)
				{
					dspSettingStrc.ChnlRegion[i].nNoPassingInfoNumber = nNoPassingInfoNumber;

					for(int j=0; j<nNoPassingInfoNumber; j++)
					{
						TempNode1 = TempNode.getChildNode(j);
						
						//DspNoPassingInfo dspNoPassingInfo;
						//unsigned char ucWeekDay; //禁止通行的周天。可以按位或。 (应用配置)								
						TempNode2 = TempNode1.getChildNode("WeekDay");
						dspSettingStrc.ChnlRegion[i].vecNoPassingInfo[j].ucWeekDay = GetIntValFromNode(TempNode2);//???	
																	
						//int nVehType; // //0表示不禁行；
						TempNode2 = TempNode1.getChildNode("VehType");
						dspSettingStrc.ChnlRegion[i].vecNoPassingInfo[j].nVehType = GetIntValFromNode(TempNode2);

						//int nStart;   // 禁止通行时间断开始。从0时0分0秒到禁止通行时间的秒数。  (应用配置)
						TempNode2 = TempNode1.getChildNode("Start");
						dspSettingStrc.ChnlRegion[i].vecNoPassingInfo[j].nStart = GetIntValFromNode(TempNode2);
					
						//int nEnd;     // 比如从1点到两点是禁行时间，则nStart = 1*60*60, nEnd = 2*60*60     (应用配置)
						TempNode2 = TempNode1.getChildNode("End");
						dspSettingStrc.ChnlRegion[i].vecNoPassingInfo[j].nEnd = GetIntValFromNode(TempNode2);
						
					}//End of for(int j=0; j<nNoPassingInfoNumber; j++)

				}//End of if(nNoPassingInfoNumber > 0)

			//int nRadarAlarmVelo;				//雷达预警速度
				TempNode = ChannelWayNode.getChildNode("RadarAlarmVelo");
				dspSettingStrc.ChnlRegion[i].nRadarAlarmVelo =  GetIntValFromNode(TempNode);

			//红灯延迟时间（秒），每个车道两个时间段，
			//[0]信号延迟接收时间，[1]红灯的持续时间
			//int64          m_pRedLightDelayTime[2];
				TempNode = ChannelWayNode.getChildNode("RedLightDelayTime1");
				dspSettingStrc.ChnlRegion[i].m_pRedLightDelayTime[0] =  GetIntValFromNode(TempNode);

				TempNode = ChannelWayNode.getChildNode("RedLightDelayTime2");
				dspSettingStrc.ChnlRegion[i].m_pRedLightDelayTime[1] =  GetIntValFromNode(TempNode);

			//int  nLeftControl;  //左转控制 (取值范围0-11)
				TempNode = ChannelWayNode.getChildNode("LeftControl");
				dspSettingStrc.ChnlRegion[i].nLeftControl =  GetIntValFromNode(TempNode);
			//int  nStraightControl;  //直行控制(取值范围0-11)
				TempNode = ChannelWayNode.getChildNode("StraightControl");
				dspSettingStrc.ChnlRegion[i].nStraightControl =  GetIntValFromNode(TempNode);
			//int nRightControl;   //右转控制(取值范围0-11)	
				TempNode = ChannelWayNode.getChildNode("RightControl");
				dspSettingStrc.ChnlRegion[i].nRightControl =  GetIntValFromNode(TempNode);

			//int nStopTime;				//停车多久报出(单位秒)
			TempNode = ChannelWayNode.getChildNode("StopTime");
			dspSettingStrc.ChnlRegion[i].nStopTime = GetIntValFromNode(TempNode);
				
			}//End of for(int i=0; i<nChannelWaysCount; i++)
		}//End of if(nChannelWaysCount > 0)
	

	//	GlobeLine gLines;			//全局用到的线（应用配置）
		GlobalLineNode = XmlNode.getChildNode("GlobalLine");
		//GlobeLine gLines;

		//	// 电子警察的辅助线（相对于电警缩小图）
		//DspMvLine           m_stopLine;   //道路停止线（应用配置）
		TempNode = GlobalLineNode.getChildNode("StopLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_stopLine, TempNode);
		//DspMvLine           m_foreLine;   //道路前行线（应用配置）
		TempNode = GlobalLineNode.getChildNode("ForeLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_foreLine, TempNode);
		//DspMvLine           m_rightLine;  //禁右线（应用配置）
		TempNode = GlobalLineNode.getChildNode("RightLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_rightLine, TempNode);
		//DspMvLine           m_leftLine;   //禁左线（应用配置）
		TempNode = GlobalLineNode.getChildNode("LeftLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_leftLine, TempNode);

		//DspMvLine           m_firstLine;		//电警第一触发线（应用配置）
		TempNode = GlobalLineNode.getChildNode("FirstLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_firstLine, TempNode);
		//DspMvLine           m_secondLine;  //	//电警第二触发线（应用配置）
		TempNode = GlobalLineNode.getChildNode("SecondLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_secondLine, TempNode);
		//DspMvLine           m_rightLineOri;	//禁右初始触发线（应用配置）
		TempNode = GlobalLineNode.getChildNode("RightLineOri");
		GetMvLineFromNode(dspSettingStrc.gLines.m_rightLineOri, TempNode);
		//DspMvLine           m_leftLineOri;		//预留,暂时无定义（应用配置）
		TempNode = GlobalLineNode.getChildNode("LeftLineOri");
		GetMvLineFromNode(dspSettingStrc.gLines.m_leftLineOri, TempNode);

		//DspMvLine m_NoTurnAroundLineOri; //禁止调头初始触发线 ——应用配置(2012.12.21 add)
		TempNode = GlobalLineNode.getChildNode("NoTurnAroundLineOri");
		GetMvLineFromNode(dspSettingStrc.gLines.m_NoTurnAroundLineOri, TempNode);

		//DspMvLine           m_foreLineOri;     //禁前初始触发线（应用配置）(2012.12.21 add)
		TempNode = GlobalLineNode.getChildNode("ForeLineOri");
		GetMvLineFromNode(dspSettingStrc.gLines.m_foreLineOri, TempNode);

		//DspMvLine  m_NoTurnAroundLine;     //禁止调头线 ——应用配置
		TempNode = GlobalLineNode.getChildNode("NoTurnAroundLine");
		GetMvLineFromNode(dspSettingStrc.gLines.m_NoTurnAroundLine, TempNode);		

		////可以有多条
		//DspMvLine m_vecYellowLine[8];    //黄线（应用配置）
		//int    m_vecYellowNumber;     //黄线的条数（应用配置）
		TempNode = GlobalLineNode.getChildNode("YellowLines");
		dspSettingStrc.gLines.m_vecYellowNumber = GetMvLinesFromNode(dspSettingStrc.gLines.m_vecYellowLine, TempNode);

		//DspMvLine m_vecWhiteLine[8];     //白线（应用配置）
		//int    m_vecWhiteNumber;      //白线的条数（应用配置）
		TempNode = GlobalLineNode.getChildNode("WhiteLines");
		dspSettingStrc.gLines.m_vecWhiteNumber = GetMvLinesFromNode(dspSettingStrc.gLines.m_vecWhiteLine, TempNode);

		//DspMvLine m_vecBianDaoXian[8];   //变道线（应用配置）			
		//int    m_vecBianDaoXianNumber;  //变道线的条数（应用配置）
		TempNode = GlobalLineNode.getChildNode("BianDaoLines");
		dspSettingStrc.gLines.m_vecBianDaoXianNumber = GetMvLinesFromNode(dspSettingStrc.gLines.m_vecBianDaoXian, TempNode);	

	//	MvRect m_rtRemoteCarNum; //远处车牌大小
		RectNode = XmlNode.getChildNode("RemoteCarNumRect");
		GetRectFromNode(dspSettingStrc.m_rtRemoteCarNum, RectNode);
	//	MvRect m_rtLocalCarNum;  //近处车牌大小
		RectNode = XmlNode.getChildNode("LocalCarNumRect");
		GetRectFromNode(dspSettingStrc.m_rtLocalCarNum, RectNode);

	//	int				m_nMaskRegionCount;				//屏蔽区域数目
	//	MvDspRegion		m_pMaskRegion[8];			//屏蔽区
		MvDspRgnsNode = XmlNode.getChildNode("MaskRegions");
		dspSettingStrc.m_nMaskRegionCount = GetMvDspRgnsFromNode(dspSettingStrc.m_pMaskRegion, MvDspRgnsNode);

	//	int             nNoPutInRegNum;    //禁止驶入区数目——应用配置 	
	//	MvDspRegion     NoPutInReg[8];     //禁止驶入区 ——应用配置		
		MvDspRgnsNode = XmlNode.getChildNode("NoPutInRegs");
		dspSettingStrc.nNoPutInRegNum = GetMvDspRgnsFromNode(dspSettingStrc.NoPutInReg, MvDspRgnsNode);

	//	int             nNoStopRegNum;       //禁停区数目——应用配置
	//	MvDspRegion     NoStopReg[8];       //禁停区 ——应用配置
		MvDspRgnsNode = XmlNode.getChildNode("NoStopRegs");
		dspSettingStrc.nNoStopRegNum = GetMvDspRgnsFromNode(dspSettingStrc.NoStopReg, MvDspRgnsNode);
	
	//	int             nNoResortRegNum;   //禁止滞留区数目——应用配置
	//	MvDspRegion     NoResortReg[8];   //禁止路口滞留区 ——应用配置
		MvDspRgnsNode = XmlNode.getChildNode("NoResortRegs");
		dspSettingStrc.nNoResortRegNum = GetMvDspRgnsFromNode(dspSettingStrc.NoResortReg, MvDspRgnsNode);

		//	MvDspRegion		parkingRegion;			//黄网格区域
		MvDspRgnsNode = XmlNode.getChildNode("YelGridRgn");
		dspSettingStrc.parkingRegion.nPoints = GetMvPointsFromNode(dspSettingStrc.parkingRegion.arrList, MvDspRgnsNode);
		
    }//End of if (access(strDspSettingInfoXml.c_str(), F_OK) == 0) //存在
    else
    {
		printf("error strDspSettingInfoXml.c_str()=%s\n",strDspSettingInfoXml.c_str());
        return false;
    }

	return true;
}

//解析MvDspRect结构
void CXmlParaUtil::GetRectFromNode(MvRect &rect, XMLNode &rectNode)
{
	XMLNode TempNode;
	XMLCSTR strText;
	int fTemp=0;

	TempNode = rectNode.getChildNode("x");
	if(!TempNode.isEmpty())
	{
		strText = TempNode.getText();
		fTemp = xmltof(strText);
	}
	rect.x = (int)(fTemp+0.5f);

	TempNode = rectNode.getChildNode("y");
	if(!TempNode.isEmpty())
	{
		strText = TempNode.getText();
		fTemp = xmltof(strText);
	}
	rect.y = (int)(fTemp+0.5f);

	TempNode = rectNode.getChildNode("w");
	if(!TempNode.isEmpty())
	{
		strText = TempNode.getText();
		fTemp = xmltof(strText);
	}
	rect.width = (int)(fTemp+0.5f);

	TempNode = rectNode.getChildNode("h");
	if(!TempNode.isEmpty())
	{
		strText = TempNode.getText();
		fTemp = xmltof(strText);
	}
	rect.height = (int)(fTemp+0.5f);

	LogTrace("GetFromNode.log", "=rect=[%d, %d, %d, %d]", \
		rect.x, rect.y, rect.width, rect.height );
}

//解析DspMvLine结构
#ifndef ALGORITHM_YUV
void CXmlParaUtil::GetMvLineFromNode(DspMvLine &dspMvLine, XMLNode &pNode)
#else
void CXmlParaUtil::GetMvLineFromNode(MvLine &dspMvLine, XMLNode &pNode)
#endif
{
	XMLNode TempNode, PointsNode;
	XMLCSTR strText;
	int fTemp=0;

	CPoint32f pt32Temp;
	PointsNode = pNode.getChildNode("Points");

	// 图像坐标。
	//DspMvPoint start;  //(应用配置)
	GetPointFromNode(pt32Temp, 0, PointsNode);//从指定节点获得点坐标
	dspMvLine.start.x = (int)(pt32Temp.x + 0.5f);
	dspMvLine.start.y = (int)(pt32Temp.y + 0.5f);	

	//DspMvPoint end;    //（应用配置）
	GetPointFromNode(pt32Temp, 1, PointsNode);
	dspMvLine.end.x = (int)(pt32Temp.x + 0.5f);
	dspMvLine.end.y = (int)(pt32Temp.y + 0.5f);	

	LogTrace("GetFromNode.log", "=dspMvLine.start=[%d, %d],end:[ %d, %d]", \
		dspMvLine.start.x, dspMvLine.start.y,  dspMvLine.end.x, dspMvLine.end.y);
}


//解析DspMvPoint[16]列表结构
//返回点计数
#ifndef ALGORITHM_YUV
int CXmlParaUtil::GetMvPointsFromNode(DspMvPoint DspMvPointsArray [], XMLNode &pNode)
#else
int CXmlParaUtil::GetMvPointsFromNode(MvPoint DspMvPointsArray [], XMLNode &pNode)
#endif
{
	XMLNode TempNode, PointsNode;
	XMLCSTR strText;
	int fTemp=0;

	CPoint32f pt32Temp;
	int nPoints = 0;


	PointsNode = pNode.getChildNode("Points");
	nPoints = PointsNode.nChildNode();


	for(int i=0; i<nPoints; i++)
	{
		GetPointFromNode(pt32Temp, i, PointsNode);
		DspMvPointsArray[i].x = (int)(pt32Temp.x + 0.5f);
		DspMvPointsArray[i].y = (int)(pt32Temp.y + 0.5f);

		LogTrace("GetFromNode.log", "=DspMvPointsArray[%d].x=%d=DspMvPointsArray[%d].y =%d", \
			i, DspMvPointsArray[i].x, i, DspMvPointsArray[i].y );
	}

	return nPoints;
}

//获取解析整型值，返回整型值
//bFloatFlag:是否节点值为浮点型标志,默认为false
int CXmlParaUtil::GetIntValFromNode(XMLNode &TempNode, bool bFloatFlag)
{
	int nValue = 0;
	XMLCSTR strText;

	float fTemp = 0.0f;

	if(bFloatFlag)
	{
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			fTemp = xmltof(strText);
		}

		nValue = (int)(fTemp + 0.5f);
	}
	else
	{
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			nValue = xmltoi(strText);
		}
	}

	LogTrace("GetFromNode.log", "=bFloatFlag=%d=nValue=%d", \
		bFloatFlag, nValue);

	return nValue;
}

//解析mvDspRgn[8]列表结构
//返回列表结构计数
int CXmlParaUtil::GetMvDspRgnsFromNode(MvDspRegion mvDspRgn[], XMLNode &mvDspRgnsNode)
{
	//MvDspRgnsNode = XmlNode.getChildNode("MaskRegions");

	XMLNode MvDspRgnNode;
	int	nRgnsCount = mvDspRgnsNode.nChildNode();

	if(nRgnsCount > 0)
	{
		for(int i=0; i<nRgnsCount; i++)
		{
			MvDspRgnNode = mvDspRgnsNode.getChildNode(i);

			mvDspRgn[i].nPoints = GetMvPointsFromNode(mvDspRgn[i].arrList, MvDspRgnNode);

			LogTrace("GetFromNode.log", "=GetMvDspRgnsFromNode=[%d]=:points:%d", \
				i, mvDspRgn[i].nPoints );
		}//End of for
	}//End of if(nMaskRegionCount > 0)

	return nRgnsCount;
}

//解析DspMvLine[]列表结构,返回列表数目
#ifndef ALGORITHM_YUV
int CXmlParaUtil::GetMvLinesFromNode(DspMvLine dspMvLineArray[], XMLNode &mvLinesNode)
#else
int CXmlParaUtil::GetMvLinesFromNode(MvLine dspMvLineArray[], XMLNode &mvLinesNode)
#endif
{
	XMLNode MvLineNode;
	int nMvLinesCount = mvLinesNode.nChildNode();

	if(nMvLinesCount > 0)
	{
		for(int i=0; i<nMvLinesCount; i++)
		{
			MvLineNode = mvLinesNode.getChildNode(i);
			GetMvLineFromNode(dspMvLineArray[i], MvLineNode);

			LogTrace("GetFromNode.log", "=GetMvLinesFromNode==dspMvLineArray==[%d]=", \
				i);
		}//End of for
	}

	return nMvLinesCount;
}

//生成操作系统信息
bool CXmlParaUtil::WriteOsInfo()
{
    char buf[64];
	memset(buf, 0, sizeof(buf));

    XMLNode xml,OsInfo,CpuInfo;

    std::string strOsInfoXml = "./SysInfo.xml";

	remove(strOsInfoXml.c_str());
 
    xml = XMLNode::createXMLTopNode("SysInfo");

    OsInfo = xml.addChild("OsInfo");	      
	OsInfo.addText(g_KernelRelese.c_str());

	CpuInfo = xml.addChild("CpuInfo");	      
	CpuInfo.addText(g_sysInfo_ex.szCpu);

    if(!xml.writeToFile(strOsInfoXml.c_str()))
    {
        return false;
    }
    //
    return true;
}

//获取检测器程序版本
bool CXmlParaUtil::GetVersion(string& strVersion)
{
	XMLNode XmlNode,ModuleNode;
	XMLNode TempNode;
	XMLCSTR strText;
	string strModuleVersion,strModuleSubVersion;

	std::string strVersionXml = "./version.xml";
	if(access(strVersionXml.c_str(), F_OK) == 0) //存在
	{
        //xml文件
		XmlNode = XMLNode::parseFile(strVersionXml.c_str()).getChildNode("roaddetect-module-list");

		int nItems = XmlNode.nChildNode();

        int iTemp = 0;
        int i = 0;
        if(nItems > 0)
        {
            ModuleNode = XmlNode.getChildNode(i); 

            TempNode = ModuleNode.getChildNode("module-version");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
				if(strText != NULL)
				strModuleVersion = strText;
            }

			TempNode = ModuleNode.getChildNode("module-subversion");
            if(!TempNode.isEmpty())
            {
                strText = TempNode.getText();
				if(strText != NULL)
				strModuleSubVersion = strText;
            }
			printf("===========strModuleVersion=%s\n",strModuleVersion.c_str());
			printf("===========strModuleSubVersion=%s\n",strModuleSubVersion.c_str());

			string strSub("");
			if(strModuleSubVersion.size() > 0)
			{
				int nPos = -1;
				nPos = strModuleSubVersion.rfind(".");
				if(nPos > 0)
				{
					string strTemp = strModuleSubVersion.substr(nPos+1,strModuleSubVersion.size() - nPos -1 );
					strSub = "(" + strTemp + ")";
				}
			}

			if(strSub.size() > 0)
			{
				strVersion = strModuleVersion + strSub;
			}
			else
			{
				strVersion = strModuleVersion;
			}

			return true;
        }
	}
    return false;
}

bool CXmlParaUtil::SetRoadItudeInfo( string strData )
{
	string strRoadNameInfoXml = "./RoadItude.xml";
	XMLNode xmlNode;
	if (access(strRoadNameInfoXml.c_str(),F_OK) != 0)
	{
		xmlNode= XMLNode::parseString(strData.c_str());
		//g_skpDB.GBKToUTF8(strRoadNameInfoXml);
		xmlNode.writeToFile(strRoadNameInfoXml.c_str());
		
	}
	else
	{
		xmlNode = XMLNode::parseFile(strRoadNameInfoXml.c_str());
		xmlNode.deleteNodeContent();//删除原来的数据
		xmlNode = XMLNode::parseString(strData.c_str());//加载新数据

		//g_skpDB.GBKToUTF8(strRoadNameInfoXml);
		xmlNode.writeToFile(strRoadNameInfoXml.c_str());
	}
	//g_skpDB.GBKToUTF8(strRoadNameInfoXml);
	LogNormal("[%s]:successed!\n",__FUNCTION__);
	return true;
}

bool CXmlParaUtil::LoadRoadItudeInfo(RoadNameInfoList &listRoadInfo)
{
	string strRoadNameInfoXml = "./RoadItude.xml";
	listRoadInfo.clear();

	if (access(strRoadNameInfoXml.c_str(),F_OK) == 0)
	{
		XMLNode xmlNode;
		XMLNode tempNode,tempNode2,tempNode3;
		XMLNode ItudeInfosNode;
		XMLNode RoadInfoNode;
		XMLNode StartPosNode,EndPosNode;
		XMLCSTR strText;
		//sRoadNameInfo sRoadInfo;  

		ItudeInfosNode = XMLNode::parseFile(strRoadNameInfoXml.c_str()).getChildNode("ItudeInfos");
		if(ItudeInfosNode.isEmpty())
		{ 
			LogNormal("ItudeInfosNode.isEmpty = %d\n",ItudeInfosNode.isEmpty());
			return false;
		}
		LogNormal("nChildNode = %d\n",ItudeInfosNode.nChildNode());
		for(int i =0;i<ItudeInfosNode.nChildNode();i++)
		{
			sRoadNameInfo sRoadInfo;
			memset(&sRoadInfo,0,sizeof(sRoadNameInfo));
			//路段信息节点
			RoadInfoNode = ItudeInfosNode.getChildNode(i);

			//tempNode = RoadInfoNode.getChildNode("RoadInfo");
			if (!RoadInfoNode.isEmpty())
			{
				//XMLCSTR strText = temp.getText();
				//printf("tempNode.isEmpty() = %d\n",RoadInfoNode.isEmpty());
				tempNode = RoadInfoNode.getChildNode("KaKouItem");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();

					memset(sRoadInfo.chKaKouItem,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chKaKouItem,strTmp.c_str(),strTmp.size());
				}

				tempNode = RoadInfoNode.getChildNode("Item");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();

					memset(sRoadInfo.chPosNumber,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chPosNumber,strTmp.c_str(),strTmp.size());
				}

				tempNode = RoadInfoNode.getChildNode("Name");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();

					memset(sRoadInfo.chRoadName,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chRoadName,strTmp.c_str(),strTmp.size());
				}

				tempNode = RoadInfoNode.getChildNode("Direct");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();
					memset(sRoadInfo.chDirection,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chDirection,strTmp.c_str(),strTmp.size());
				}
				//找到路段起点节点
				StartPosNode = RoadInfoNode.getChildNode("StartPos");
				tempNode = StartPosNode.getChildNode("StartName");
				if(!tempNode.isEmpty())
				{
					
					strText = tempNode.getText();
					memset(sRoadInfo.chStartPos,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chStartPos,strTmp.c_str(),strTmp.size());
				}
				tempNode = StartPosNode.getChildNode("Longitude");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();
					if(strText)
						sRoadInfo.dStartPosX = xmltof(strText);
				}
				tempNode = StartPosNode.getChildNode("Latitude");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();
					if(strText)
						sRoadInfo.dStartPosY = xmltof(strText);
				}
				//找到路段终点节点
				EndPosNode = RoadInfoNode.getChildNode("EndPos");
				tempNode = EndPosNode.getChildNode("EndName");
				if(!tempNode.isEmpty())
				{
					
					strText = tempNode.getText();
					memset(sRoadInfo.chEndPos,0,128);
					string strTmp = strText;
					memcpy(sRoadInfo.chEndPos,strTmp.c_str(),strTmp.size());
				}
				tempNode = EndPosNode.getChildNode("Longitude");
				if(!tempNode.isEmpty())
				{
					
					strText = tempNode.getText();
					if(strText)
						sRoadInfo.dEndPosX = xmltof(strText);
				}
				tempNode = EndPosNode.getChildNode("Latitude");
				if(!tempNode.isEmpty())
				{
					strText = tempNode.getText();
					if(strText)
						sRoadInfo.dEndPosY = xmltof(strText);
				}
			}
			//LogNormal("[%s]:chPosNumber = %s,RoadName = %s,diretion = %s\n",__FUNCTION__,sRoadInfo.chPosNumber,sRoadInfo.chRoadName,\
				sRoadInfo.chDirection);
			//LogNormal("[%s]:dStartPosX = %.8f,dStartPosY = %.8f,\n",__FUNCTION__,sRoadInfo.dStartPosX,sRoadInfo.dStartPosY);
			//LogNormal("[%s]:dEndPosx = %.8f,dEndPosY = %.8f\n",__FUNCTION__,sRoadInfo.dEndPosX,sRoadInfo.dEndPosY);
			//数据保存到m_RoadInfoList
			listRoadInfo.push_back(sRoadInfo);
			//LogNormal("[%s]listRoadInfo size = %d\n",__FUNCTION__,listRoadInfo.size());
		}
		return true;
	}
	LogNormal("LoadRoadItudeInfo fail\n");
	return false;
}


bool CXmlParaUtil::GetRoadItudeInfo(string& strData)
{
	string strRoadNameInfoXml = "./RoadItude.xml";
	XMLNode xmlNode;
	if (access(strRoadNameInfoXml.c_str(),F_OK) == 0)
	{
	
		xmlNode = XMLNode::parseFile(strRoadNameInfoXml.c_str());

		strData = xmlNode.createXMLString();
		return true;
        		
	}
	return false;
}
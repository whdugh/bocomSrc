#include "XmlParaUtil.h"
#include "RoadXmlData.h"
#include "Common.h"
#include "CommonHeader.h"
#include "XmlParaUtil.h"
#include "ximage.h"
#include "BrandSubSection.h"
#include "CarLabel.h"

CRoadXmlData::CRoadXmlData()
{
    m_nCarRoadNum = 0;
}

CRoadXmlData::~CRoadXmlData()
{

}

/* 函数介绍：生成数据包头
 * 输入参数：无
 * 输出参数：PackageHeadNode：数据包头地址
 * 返回值：是否生成数据包头成功
 */
bool CRoadXmlData::SetPackageHead(XMLNode& PackageHeadNode)
{
    char buf[64];
    memset(buf, 0, sizeof(buf));
    XMLNode VersionNode,RecordNode,DescNode;
    XMLCSTR strTag;

    //版本号
    VersionNode = PackageHeadNode.addChild("Version");
	if(g_nServerType == 3)
	{
		VersionNode.addText("3.00");
	}
	else
	{
		VersionNode.addText("1.0");
	}


    //记录数
    RecordNode = PackageHeadNode.addChild("Record");
    RecordNode.addText("1");

    //备注
    DescNode = PackageHeadNode.addChild("Desc");
    DescNode.addText("");

    return true;
}

/* 函数介绍：生成车牌数据
 * 输入参数：strMsg：车牌数据内容，strNewMsg：xml文件方式组织的车牌数据
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成车牌数据包成功
 */
string CRoadXmlData::AddCarNumberData(const string &strMsg,string &strNewMsg,int nUploadType)
{
    RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    printf("=AddCarNumberData=pPlate->uViolationType=%d\n",pPlate->uViolationType);
	if(g_nServerType == 3)
	{
		if(pPlate->uViolationType == DETECT_RESULT_NOCARNUM ||//无牌车
		   pPlate->uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)//大货禁行
		   {
				string strViolationMsg;
				ConvertPlateToEvent(strMsg,strViolationMsg);

				return AddViolationData(strViolationMsg,strNewMsg,nUploadType);
		   }
	}

    string strPath;
    XMLNode PackageNode,PackageHeadNode, DataNode;
    XMLNode TempNode, TempNode1, TempNode2,TempNode3;

    int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);

    //printf("nPicSize=%d, pPlate->uPicSize=%d\n",nPicSize , pPlate->uPicSize);

    int nPicCount = 1;
    if(nPicSize > pPlate->uPicSize)
    {
        nPicCount = 2;

        if(nPicSize > pPlate->uPicSize+pPlate->uSmallPicSize)
        nPicCount = 3;
    }

    char buf[64]={0};

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");
    {
        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
		if(g_nServerType == 3)
		{
			TempNode.addText("C");
		}
		else
		{
			TempNode.addText("C");
		}

        //厂商编码
        TempNode = DataNode.addChild("WorksNo");
		if(g_nServerType == 3)
		{
			TempNode.addText("C02");
		}
		else
		{
			TempNode.addText("01");
		}

        //设备编码
        TempNode = DataNode.addChild("DeviceNo");

		if(g_nServerType == 3)
        TempNode.addText(g_strFtpUserName.c_str());
		else 
		TempNode.addText(g_strDetectorID.c_str());


        //1.带图片 2.不带图片
        TempNode = DataNode.addChild("UploadType");
        memset(buf, 0, sizeof(buf));
        if(nUploadType == 1 || nUploadType == 3)
        sprintf(buf,"%d", 1);
        else
        sprintf(buf,"%d", 2);
        TempNode.addText(buf);

        //数据包类型--见1.13监测数据类型
        char chCode = 'Z';
        if(pPlate->chText[0]=='0')
		{
			if(g_nServerType == 3)
			{
				chCode = 'H';
			}
			else
			{
				chCode = 'X';
			}
		}
        TempNode = DataNode.addChild("Type");
        sprintf(buf,"%c", chCode);
        TempNode.addText(buf);

        //车牌号
        TempNode = DataNode.addChild("CarNo");
        String strCarNum = pPlate->chText;
        if(pPlate->chText[0]=='0')
		{
			if(g_nServerType == 3)
			TempNode.addText("?");//无牌车报警
			else
			TempNode.addText("000000");
		}
		else if(pPlate->chText[0] == '1')
		{
		   TempNode.addText("无牌车");
		}
        else
		{
			TempNode.addText(strCarNum.c_str());
		}

        //车型--1大车，2小车--见1.14车辆类型编码
        TempNode = DataNode.addChild("CarType");
        if(pPlate->uType == BIG_CAR)
        {
            TempNode.addText("1");
        }
        else if(pPlate->uType == SMALL_CAR)
        {
            TempNode.addText("2");
        }
        else
        {
            TempNode.addText("0");
        }

        //号牌颜色--见1.15车辆号牌颜色
        TempNode = DataNode.addChild("PlateColor");
        if(pPlate->uColor == CARNUM_BLUE)
        {
            TempNode.addText("1");
        }
        else if(pPlate->uColor == CARNUM_YELLOW)
        {
            TempNode.addText("2");
        }
        else if(pPlate->uColor == CARNUM_WHITE)
        {
            TempNode.addText("3");
        }
        else if(pPlate->uColor == CARNUM_BLACK)
        {
            TempNode.addText("4");
        }
        else
        {
            TempNode.addText("0");
        }

        //置信度--[0，1]--无计算则为?
        TempNode = DataNode.addChild("Confidence");
        TempNode.addText("0.90");

        //车辆标志--见1.16车辆标志分类
        TempNode = DataNode.addChild("CarLogo");

		if(g_nServerType == 3)
        TempNode.addText("?");
		else
		{
			string strCarLogo = "?";
			if(pPlate->uCarBrand == AUDI)
			{
				strCarLogo = "0001";
			}
			else if(pPlate->uCarBrand == BENZ)
			{
				strCarLogo = "0002";
			}
			else if(pPlate->uCarBrand == BMW)
			{
				strCarLogo = "0003";
			}
			else if(pPlate->uCarBrand == HONDA)
			{
				strCarLogo = "0004";
			}
			else if(pPlate->uCarBrand == BUICK)
			{
				strCarLogo = "0005";
			}
			else if(pPlate->uCarBrand == PEUGEOT)
			{
				strCarLogo = "0006";
			}
			else if(pPlate->uCarBrand == BNTLY)
			{
				strCarLogo = "0007";
			}
			else if(pPlate->uCarBrand == PORSCHE)
			{
				strCarLogo = "0008";
			}
			else if(pPlate->uCarBrand == BYD)
			{
				strCarLogo = "0009";
			}
			else if(pPlate->uCarBrand == BESTURN)
			{
				strCarLogo = "0010";
			}
			else if(pPlate->uCarBrand == CHANA)
			{
				strCarLogo = "0011";
			}
			else if(pPlate->uCarBrand == GREATWALL)
			{
				strCarLogo = "0012";
			}
			else if(pPlate->uCarBrand == CHANGFENAMOTOR)
			{
				strCarLogo = "0013";
			}
			else if(pPlate->uCarBrand == CHANGHE)
			{
				strCarLogo = "0014";
			}
			else if(pPlate->uCarBrand == EMGRANO)
			{
				strCarLogo = "0015";
			}
			else if(pPlate->uCarBrand == VW)
			{
				strCarLogo = "0016";
			}
			else if(pPlate->uCarBrand == SOUEAST)
			{
				strCarLogo = "0017";
			}
			else if(pPlate->uCarBrand == DONGFENG)
			{
				strCarLogo = "0019";
			}
			else if(pPlate->uCarBrand == FORD)
			{
				strCarLogo = "0020";
			}
			else if(pPlate->uCarBrand == FERRARI)
			{
				strCarLogo = "0021";
			}
			else if(pPlate->uCarBrand == FIAT)
			{
				strCarLogo = "0022";
			}
			else if(pPlate->uCarBrand == FOTON)
			{
				strCarLogo = "0023";
			}
			else if(pPlate->uCarBrand == RedFlags)
			{
				strCarLogo = "0024";
			}
			else if(pPlate->uCarBrand == HAIMA)
			{
				strCarLogo = "0025";
			}
			else if(pPlate->uCarBrand == SMA)
			{
				strCarLogo = "0026";
			}
			else if(pPlate->uCarBrand == HAFEI)
			{
				strCarLogo = "0027";
			}
			else if(pPlate->uCarBrand == HAWTAIAUTOMOBILE)
			{
				strCarLogo = "0028";
			}
			else if(pPlate->uCarBrand == HUMMER)
			{
				strCarLogo = "0029";
			}
			else if(pPlate->uCarBrand == JEEP)
			{
				strCarLogo = "0030";
			}
			else if(pPlate->uCarBrand == GEELY)
			{
				strCarLogo = "0031";
			}
			else if(pPlate->uCarBrand == JINBEI)
			{
				strCarLogo = "0032";
			}
			else if(pPlate->uCarBrand == JAGUAR)
			{
				strCarLogo = "0033";
			}
			else if(pPlate->uCarBrand == JAC)
			{
				strCarLogo = "0034";
			}
			else if(pPlate->uCarBrand == CHERY)
			{
				strCarLogo = "0035";
			}
			else if(pPlate->uCarBrand == CHRYSLER)
			{
				strCarLogo = "0036";
			}
			else if(pPlate->uCarBrand == CADILLAC)
			{
				strCarLogo = "0037";
			}
			else if(pPlate->uCarBrand == SUZUKI)
			{
				strCarLogo = "0038";
			}
			else if(pPlate->uCarBrand == CHERY)
			{
				strCarLogo = "0039";
			}
			else if(pPlate->uCarBrand == RENAULT)
			{
				strCarLogo = "0040";
			}
			else if(pPlate->uCarBrand == LIFAN)
			{
				strCarLogo = "0041";
			}
			else if(pPlate->uCarBrand == LANDROVER)
			{
				strCarLogo = "0044";
			}
			else if(pPlate->uCarBrand == MAZDA)
			{
				strCarLogo = "0045";
			}
			else if(pPlate->uCarBrand == MG)
			{
				strCarLogo = "0046";
			}
			else if(pPlate->uCarBrand == KIA)
			{
				strCarLogo = "0052";
			}
			else if(pPlate->uCarBrand == ROWE)
			{
				strCarLogo = "0053";
			}
			else if(pPlate->uCarBrand == UD)
			{
				strCarLogo = "0054";
			}
			else if(pPlate->uCarBrand == MITSUBISHI)
			{
				strCarLogo = "0055";
			}
			else if(pPlate->uCarBrand == SKODA)
			{
				strCarLogo = "0057";
			}
			else if(pPlate->uCarBrand == VOLVO)
			{
				strCarLogo = "0063";
			}
			else if(pPlate->uCarBrand == CHEVROLET)
			{
				strCarLogo = "0064";
			}
			else if(pPlate->uCarBrand == CITERON)
			{
				strCarLogo = "0065";
			}
			else if(pPlate->uCarBrand == HYUNDAI)
			{
				strCarLogo = "0066";
			}
			else if(pPlate->uCarBrand == XIALI)
			{
				strCarLogo = "0067";
			}
			else if(pPlate->uCarBrand == FLYWITHOUT)
			{
				strCarLogo = "0068";
			}
			
			 TempNode.addText(strCarLogo.c_str());
		}

        //行驶方向
        string strDirection,strDesc;
		
		if(g_nServerType == 3)
		{
			ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.find(pPlate->uRoadWayID);
			if(it != g_roadDirectionMap.end())
			{
				strDirection = it->second.chDirection;
				strDesc = it->second.chDeviceDesc;
			}
			else
			{
				strDirection = "1A";
			}
		}
		else
		{
			if(pPlate->uDirection == EAST_TO_WEST)
			{
				strDirection = "01";
			}
			else if(pPlate->uDirection == WEST_TO_EAST)
			{
				strDirection = "02";
			}
			else if(pPlate->uDirection == SOUTH_TO_NORTH)
			{
				strDirection = "03";
			}
			else if(pPlate->uDirection == NORTH_TO_SOUTH)
			{
				strDirection = "04";
			}
			else if(pPlate->uDirection == SOUTHEAST_TO_NORTHWEST)
			{
				strDirection = "05";
			}
			else if(pPlate->uDirection == NORTHWEST_TO_SOUTHEAST)
			{
				strDirection = "06";
			}
			else if(pPlate->uDirection == NORTHEAST_TO_SOUTHWEST)
			{
				strDirection = "07";
			}
			else if(pPlate->uDirection == SOUTHWEST_TO_NORTHEAST)
			{
				strDirection = "08";
			}
			
			string strPlace(pPlate->chPlace);
			g_skpDB.UTF8ToGBK(strPlace);
			strDesc = strPlace;
		}

        TempNode = DataNode.addChild("Direction");
        ///////8个方向解析
        TempNode.addText(strDirection.c_str());

        //车道标识符--见7.2
        TempNode = DataNode.addChild("CarRoad");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d", pPlate->uRoadWayID);
        TempNode.addText(buf);

        //车速--km/h
        TempNode = DataNode.addChild("CarSpeed");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%.2f", (float)pPlate->uSpeed);
        TempNode.addText(buf);

		if(g_nServerType != 3)
		{
			//限速值
			if(pPlate->chText[0] != '0')
			{
				TempNode = DataNode.addChild("LimitedSpeed");
				memset(buf, 0, sizeof(buf));
				sprintf(buf,"%.2f", (float)pPlate->uLimitSpeed);
				TempNode.addText(buf);
			}
		}

        //抓拍时间
        TempNode = DataNode.addChild("WatchTime");
        std::string strTime = GetTime(pPlate->uTime);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pPlate->uMiTime);
        strTime.append(buf,4);
        TempNode.addText(strTime.c_str());

        //过车时间1(与抓拍时间相同)
        TempNode = DataNode.addChild("WatchTime1");
        TempNode.addText(strTime.c_str());

        //过车时间2
		if(g_nServerType == 3)
		{
			TempNode = DataNode.addChild("WatchTime2");
			if(pPlate->uTime2 == 0)
			{
				pPlate->uTime2 = pPlate->uTime;
				pPlate->uMiTime2 = pPlate->uMiTime+333;//
				if(pPlate->uMiTime2 >= 1000)
				{
					pPlate->uMiTime2 = pPlate->uMiTime2 - 1000;
					pPlate->uTime2 +=1;
				}
			}
			strTime = GetTime(pPlate->uTime2);
			memset(buf,0,sizeof(buf));
			sprintf(buf,".%03d", pPlate->uMiTime2);
			strTime.append(buf,4);
			TempNode.addText(strTime.c_str());
		}

        //设备监测地点描述，交管局统一下发
        TempNode = DataNode.addChild("DeviceDesc");
        TempNode.addText(strDesc.c_str());

        //文件加密的校验位，保留，暂时固定为-1
        TempNode = DataNode.addChild("CheckValue");
        TempNode.addText("-1");

        //照片数量
        TempNode = DataNode.addChild("PicNum");
        if(nUploadType == 2)
        {
            nPicCount = 0;
        }
        sprintf(buf,"%d", nPicCount);
        TempNode.addText(buf);

        //判断车牌位置是否需要扩充df
		if(g_nServerType == 3)
		{
			CvRect rect = GetCarPos(*pPlate);
			//特征图片坐标
			TempNode = DataNode.addChild("FeaturePic");
			sprintf(buf, "%d,%d,%d,%d",rect.x, rect.y, rect.width, rect.height);
			TempNode.addText(buf);
		}

        if(nUploadType == 1 || nUploadType == 3)
        {
            //照片明细信息
            TempNode = DataNode.addChild("Picture");

            int nPicSize1 = pPlate->uPicSize;
            TempNode1 = TempNode.addChild("PicName1");
            string strPicName = "远景照片";
			if(g_nServerType != 3)
			{
				strPicName = "图片1";
			}
            g_skpDB.UTF8ToGBK(strPicName);
            TempNode1.addText(strPicName.c_str());
            TempNode1 = TempNode.addChild("Pic1");
            //
            string strPic;
            if(nPicSize1 > 0)
            {
                EncodeBase64(strPic,(unsigned char*)strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE),nPicSize1);
                TempNode1.addText(strPic.c_str());
            }

            if(nPicCount > 1)
            {
                int  nPicSize2 = nPicSize-nPicSize1-pPlate->uSmallPicSize;

                if(nPicSize2 > 0)
                {
                    TempNode2 = TempNode.addChild("PicName2");
                    TempNode2.addText(strPicName.c_str());
                    TempNode2 = TempNode.addChild("Pic2");

                    string strPic2;
                    EncodeBase64(strPic2,(unsigned char*)strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE)+nPicSize1,nPicSize2);
                    TempNode2.addText(strPic2.c_str());
                }

                //特征图片
                int  nPicSize3 = pPlate->uSmallPicSize;
                if(nPicSize3 > 0)
                {
                    TempNode3 = TempNode.addChild("PicName3");
                    TempNode3.addText(strPicName.c_str());
                    TempNode3 = TempNode.addChild("Pic3");

                    string strPic3;
                    EncodeBase64(strPic3,(unsigned char*)strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE)+nPicSize1+nPicSize2,nPicSize3);
                    TempNode3.addText(strPic3.c_str());
                }
            }
            //
        }

        int nSize;
        XMLSTR strData = PackageNode.createXMLString(1,&nSize);
        if(strData)
        {
            strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
            strNewMsg.append(strData,sizeof(XMLCHAR)*nSize);

            strTime = GetTime(pPlate->uTime,2);
            if(nUploadType == 1 || nUploadType == 3)
            {
				if(g_nServerType == 3)
				{
					sprintf(buf,"%8s_%s_%d_%s%03d%c.XML",g_strFtpUserName.c_str(),strDirection.c_str(),pPlate->uRoadWayID,strTime.c_str(),pPlate->uMiTime,chCode);
				}
				else
				{
					sprintf(buf,"%8s_%s_%d_%s%03d%c.XML",g_strDetectorID.c_str(),strDirection.c_str(),pPlate->uRoadWayID,strTime.c_str(),pPlate->uMiTime,chCode);
				}
            }
            else if(nUploadType == 2)
            {
                sprintf(buf,"%8s_%s_%d_%s%03d.XML",g_strFtpUserName.c_str(),strDirection.c_str(),pPlate->uRoadWayID,strTime.c_str(),pPlate->uMiTime);
            }
            strPath = buf;

            /*if(nUploadType == 1)
            {
                FILE* fp = fopen(strPath.c_str(),"wb");
                fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
                fclose(fp);
            }*/

            freeXMLString(strData);
        }
    }
    return strPath;
}

/* 函数介绍：生成违章数据
 * 输入参数：strMsg：车牌数据内容，strNewMsg：xml文件方式组织的车牌数据
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成车牌数据包成功
 */
string CRoadXmlData::AddViolationData(const string &strMsg,string &strNewMsg,int nUploadType)
{
    string strPath;
    XMLNode PackageNode,PackageHeadNode, DataNode;
    XMLNode TempNode, TempNode1, TempNode2;

    RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_EVENT);

    //printf("nPicSize=%d, pEvent->uPicSize=%d\n",nPicSize , pEvent->uPicSize);

    int nPicCount = 1;

    char buf[64]={0};

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");
    {
        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
        TempNode.addText("C");

        //厂商编码
        TempNode = DataNode.addChild("WorksNo");
        TempNode.addText("C02");

        //设备编码
        TempNode = DataNode.addChild("DeviceNo");
        TempNode.addText(g_strFtpUserName.c_str());

        //1.带图片 2.不带图片
        TempNode = DataNode.addChild("UploadType");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d", nUploadType);
        TempNode.addText(buf);

        //数据包类型--见1.13监测数据类型
        char chCode = GetEventType(pEvent->uCode);
        TempNode = DataNode.addChild("Type");
        sprintf(buf,"%c", chCode);
        TempNode.addText(buf);

        //车牌号
        TempNode = DataNode.addChild("CarNo");
        String strCarNum = pEvent->chText;
        if(strCarNum.size()<=0 || pEvent->chText[0] == '0')
        {
           TempNode.addText("?");
        }
		else if(pEvent->chText[0] == '1')
		{
		   TempNode.addText("无牌车");
		}
        else
        {
           TempNode.addText(strCarNum.c_str());
        }

        //车型--1大车，2小车--见1.14车辆类型编码
        TempNode = DataNode.addChild("CarType");
        if(pEvent->uType == BIG_CAR)
        {
            TempNode.addText("1");
        }
        else if(pEvent->uType == SMALL_CAR)
        {
            TempNode.addText("2");
        }
        else
        {
            TempNode.addText("0");
        }

        //号牌颜色--见1.15车辆号牌颜色
        TempNode = DataNode.addChild("PlateColor");
        TempNode.addText("0");

        //置信度--[0，1]--无计算则为?
        TempNode = DataNode.addChild("Confidence");
        TempNode.addText("0.90");

        //车辆标志--见1.16车辆标志分类
        TempNode = DataNode.addChild("CarLogo");
        TempNode.addText("?");

        //行驶方向
        string strDirection,strDesc;
        ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.find(pEvent->uRoadWayID);
        if(it != g_roadDirectionMap.end())
        {
            strDirection = it->second.chDirection;
            strDesc = it->second.chDeviceDesc;
        }
        else
        {
            strDirection = "1A";
        }

        TempNode = DataNode.addChild("Direction");
        ///////8个方向解析
        TempNode.addText(strDirection.c_str());

        //车道标识符--见7.2
        TempNode = DataNode.addChild("CarRoad");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d", pEvent->uRoadWayID);
        TempNode.addText(buf);

        //车速--km/h
        TempNode = DataNode.addChild("CarSpeed");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%.2f", (float)pEvent->uSpeed);
        TempNode.addText(buf);

        //抓拍时间
        TempNode = DataNode.addChild("WatchTime");
        std::string strTime = GetTime(pEvent->uEventBeginTime);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pEvent->uMiEventBeginTime);
        strTime.append(buf,4);
        TempNode.addText(strTime.c_str());

        //过车时间1(与抓拍时间相同)
        TempNode = DataNode.addChild("WatchTime1");
        TempNode.addText(strTime.c_str());

        //过车时间2
        TempNode = DataNode.addChild("WatchTime2");
        if(pEvent->uTime2 == 0)
        {
            pEvent->uTime2 = pEvent->uEventBeginTime;
            pEvent->uMiTime2 = pEvent->uMiEventBeginTime+333;//
            if(pEvent->uMiTime2 >= 1000)
            {
                pEvent->uMiTime2 = pEvent->uMiTime2 - 1000;
                pEvent->uTime2 +=1;
            }
        }
        strTime = GetTime(pEvent->uTime2);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pEvent->uMiTime2);
        strTime.append(buf,4);
        TempNode.addText(strTime.c_str());

        //设备监测地点描述，交管局统一下发
        TempNode = DataNode.addChild("DeviceDesc");
        TempNode.addText(strDesc.c_str());

        //文件加密的校验位，保留，暂时固定为-1
        TempNode = DataNode.addChild("CheckValue");
        TempNode.addText("-1");

        if(nUploadType == 1)
        {
            //照片数量
            TempNode = DataNode.addChild("PicNum");
            sprintf(buf,"%d", 1);
            TempNode.addText(buf);

            //照片明细信息
            TempNode = DataNode.addChild("Picture");

            int nPicSize1 = pEvent->uPicSize;
            TempNode1 = TempNode.addChild("PicName1");
            string strPicName = "远景照片";
            g_skpDB.UTF8ToGBK(strPicName);
            TempNode1.addText(strPicName.c_str());
            TempNode1 = TempNode.addChild("Pic1");
            //
            string strPic;
            {
                EncodeBase64(strPic,(unsigned char*)strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_EVENT),nPicSize1);
            }
            TempNode1.addText(strPic.c_str());
            //
        }

        int nSize;
        XMLSTR strData = PackageNode.createXMLString(1,&nSize);
        if(strData)
        {
            strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
            strNewMsg.append(strData,sizeof(XMLCHAR)*nSize);

            strTime = GetTime(pEvent->uEventBeginTime,2);
            if(nUploadType == 1)
            {
                sprintf(buf,"%8s_%s_%d_%s%03d%c.XML",g_strFtpUserName.c_str(),strDirection.c_str(),pEvent->uRoadWayID,strTime.c_str(),pEvent->uMiEventBeginTime,chCode);
            }
            else if(nUploadType == 2)
            {
                sprintf(buf,"%8s_%s_%d_%s%03d.XML",g_strFtpUserName.c_str(),strDirection.c_str(),pEvent->uRoadWayID,strTime.c_str(),pEvent->uMiEventBeginTime);
            }
            strPath = buf;

           /* if(nUploadType == 1)
            {
                FILE* fp = fopen(strPath.c_str(),"wb");
                fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
                fclose(fp);
            }*/
            freeXMLString(strData);
        }
    }
    return strPath;
}

/* 函数介绍：生成交通事件数据
 * 输入参数：strMsg：事件数据内容，strNewMsg：xml文件方式组织的事件数据
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成交通事件数据包成功
 */
string CRoadXmlData::AddEventData(const string &strMsg, string &strNewMsg,int nUploadType)
{
    RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));


    if(pEvent->uCode == DETECT_RESULT_EVENT_GO_AGAINST ||
       pEvent->uCode == DETECT_RESULT_EVENT_PERSON_AGAINST ||//逆行
       pEvent->uCode == DETECT_RESULT_EVENT_APPEAR||//走非机动车道
       pEvent->uCode == DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD||//不按车道行驶
       pEvent->uCode == DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD)
       {
            return AddViolationData(strMsg,strNewMsg,nUploadType);
       }


    XMLNode PackageNode, PackageHeadNode, DataNode;
    char buf[64];
    memset(buf, 0, sizeof(buf));

    XMLNode TempNode, TempNode1, TempNode2;
    std::string sTemp;
    XMLCSTR strTag;

    int iTemp = 0;
    int nDataRecords = 1;
    std::string strTime,strVideoTime;

    int i=0;

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");


        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
        TempNode.addText("C");

        //厂商编码
        //TempNode = DataNode.addChild("WorksNo");
        //TempNode.addText("C02");

        //设备编码
        TempNode = DataNode.addChild("DeviceNo");
        TempNode.addText(g_strFtpUserName.c_str());

        //过车时间
        TempNode = DataNode.addChild("WatchTime");
        strTime = GetTime(pEvent->uEventBeginTime);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pEvent->uMiEventBeginTime);
        strTime.append(buf,4);
        TempNode.addText(strTime.c_str());


        //事件信息
        TempNode = DataNode.addChild("EventInfo");

        string strDirection,strDesc;
        ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.find(pEvent->uRoadWayID);
        if(it != g_roadDirectionMap.end())
        {
            strDirection = it->second.chDirection;
            strDesc = it->second.chDeviceDesc;
        }
        else
        {
            strDirection = "1A";
        }
        //行驶方向
        TempNode1 = TempNode.addChild("Direction");
        TempNode1.addText(strDirection.c_str());

        //车道号
        TempNode1 = TempNode.addChild("CarRoad");
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d", pEvent->uRoadWayID);
        TempNode1.addText(buf);

        //事件类型
        TempNode1 = TempNode.addChild("EventType");
        sTemp = this->GetEventType(pEvent->uCode);
        TempNode1.addText(sTemp.c_str());

        //事件开始时间
        TempNode1 = TempNode.addChild("EventStart");
        strTime = GetTime(pEvent->uEventBeginTime);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pEvent->uMiEventBeginTime);
        strTime.append(buf,4);
        TempNode1.addText(strTime.c_str());

        //事件结束时间
        TempNode1 = TempNode.addChild("EventEnd");
        strTime = GetTime(pEvent->uEventEndTime);
        memset(buf,0,sizeof(buf));
        sprintf(buf,".%03d", pEvent->uMiEventEndTime);
        strTime.append(buf,4);
        TempNode1.addText(strTime.c_str());

        //事件类型相对应的参数值
        TempNode1 = TempNode.addChild("EventDefine");
        //char chCode = *(sTemp.c_str());
        //iTemp = this->GetEventDefine(chCode, 1, pEvent->uRoadWayID); //获取不同类型事件的参考值
        memset(buf, 0, sizeof(buf));
        //sprintf(buf,"%d", iTemp);
        sprintf(buf,"%d", pEvent->uEventDefine);
        TempNode1.addText(buf);

        //录像名称--地址？//ftp://192.168.60.127/20100901/23/26.mp4
        TempNode1 = TempNode.addChild("EventVideo");
        strVideoTime = GetTime(pEvent->uEventBeginTime,2);
        sprintf(buf,"EVENTVIDEO_%8s_%s_%d_%s%03d%c.mp4",g_strFtpUserName.c_str(),strDirection.c_str(),pEvent->uRoadWayID,strVideoTime.c_str(),pEvent->uMiEventBeginTime,*(sTemp.c_str()));
        TempNode1.addText(buf);

        //事件发生坐标
        TempNode1 = TempNode.addChild("EVideoXY");
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d,%d", pEvent->uPosX , pEvent->uPosY); //?????
        TempNode1.addText(buf);

        //事件发生地点
        TempNode1 = TempNode.addChild("EventPlace");
        TempNode1.addText(strDesc.c_str());

        //检测方向-（检测地点描述）
        TempNode1 = TempNode.addChild("DI");
        TempNode1.addText(strDesc.c_str());

    string strPath;
    int nSize;
    XMLSTR strData = PackageNode.createXMLString(1, &nSize);
    if(strData)
    {
        strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
        strNewMsg.append(strData, sizeof(XMLCHAR)*nSize);

        strTime = GetTime(pEvent->uEventBeginTime,2);
        sprintf(buf,"EVENT_%8s_%s_%d_%s%03d%c.XML",g_strFtpUserName.c_str(),strDirection.c_str(),pEvent->uRoadWayID,strTime.c_str(),pEvent->uMiEventBeginTime,*(sTemp.c_str()));
        strPath = buf;

        /*FILE* fp = fopen(strPath.c_str(),"wb");
        fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
        fclose(fp);*/

        freeXMLString(strData);
    }

    return strPath;
}


/* 函数介绍：生成车流量数据
 * 输入参数：statistic：流量数据，strPackageName:xml文件名称
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成车流量数据包成功
 */
string CRoadXmlData::AddFlowData(const string &strMsg, string &strNewMsg)
{
    XMLNode PackageNode, PackageHeadNode, DataNode;
    char buf[64];
    memset(buf, 0, sizeof(buf));

    RECORD_STATISTIC *pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    XMLNode TempNode, TempNode1, TempNode2;
    std::string strTime;
    string strDirection;
    XMLCSTR strTag;

    int iTemp = 0;
    int i=0;

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");
    {
        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
        TempNode.addText("C");

        //设备编码
        TempNode = DataNode.addChild("DeviceNo");
        TempNode.addText(g_strFtpUserName.c_str());

        //过车时间
        TempNode = DataNode.addChild("DetectDateTime");
        strTime = GetTime(pStatistic->uTime)+".000";
        TempNode.addText(strTime.c_str());

        //实时过车信息
        TempNode = DataNode.addChild("RealTimeInfo");

        int nRoadCount = 0;
        UINT32 uRoadIndex = 0;
        int j = 0;
        while( j< MAX_ROADWAY )
        {
            if(pStatistic->uRoadType[j]!=0xffffffff)
            {
                uRoadIndex = pStatistic->uRoadType[j]>>16;
                //实时过车信息-详细
                TempNode1 = TempNode.addChild("RowInfo");

                //行驶方向
                ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.find(uRoadIndex);
                if(it != g_roadDirectionMap.end())
                {
                    strDirection = it->second.chDirection;
                }
                else
                {
                    strDirection = "1A";
                }
                TempNode2 = TempNode1.addChild("Direction");
                TempNode2.addText(strDirection.c_str());

                //车道号
                TempNode2 = TempNode1.addChild("CarRoad");
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "%d", uRoadIndex);
                TempNode2.addText(buf);

                //样本时间
                TempNode2 = TempNode1.addChild("SampleTime");
                sprintf(buf,"%d",pStatistic->uStatTimeLen/60);
                TempNode2.addText(buf);

                //流量
                TempNode2 = TempNode1.addChild("Flow");
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%.2f", (float)(pStatistic->uFlux[j]&0xffff));
                TempNode2.addText(buf);

                //流速
                TempNode2 = TempNode1.addChild("Velocity");
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%.2f", (float)pStatistic->uSpeed[j]);
                TempNode2.addText(buf);

                //占有率
                TempNode2 = TempNode1.addChild("Share");
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%.2f", (float)pStatistic->uOccupancy[j]*0.01);
                TempNode2.addText(buf);

                //平均车头间距
                TempNode2 = TempNode1.addChild("Distance");
                memset(buf, 0, sizeof(buf));
                sprintf(buf,"%.2f", (float)pStatistic->uSpace[j]);
                TempNode2.addText(buf);

                nRoadCount++;
            }

            j++;
        } //End of while

         //车道数
        TempNode = DataNode.addChild("CarRoadNum");
        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%d", nRoadCount);
        TempNode.addText(buf);
    }

    string strPath;
    int nSize;
    XMLSTR strData = PackageNode.createXMLString(1, &nSize);
    if(strData)
    {
        strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
        strNewMsg.append(strData, sizeof(XMLCHAR)*nSize);

        strTime = GetTime(pStatistic->uTime,2)+"000";
        sprintf(buf,"FLOW_%8s_%s_%d_%s.XML",g_strFtpUserName.c_str(),strDirection.c_str(),0,strTime.c_str());
        strPath = buf;

       /* FILE* fp = fopen(strPath.c_str(),"wb");
        fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
        fclose(fp);*/

        freeXMLString(strData);
    }

    return strPath;
}


/* 函数介绍：生成日志数据
 * 输入参数：log：日志数据，strPackageName:xml文件名称
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成日志数据包成功
 */
string CRoadXmlData::AddLogData(string &strNewMsg)
{
   XMLNode PackageNode, PackageHeadNode, DataNode;
    char buf[64];
    memset(buf, 0, sizeof(buf));

    //RECORD_STATISTIC *pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    XMLNode TempNode, TempNode1, TempNode2;
    std::string sTemp;
    XMLCSTR strTag;

    int iTemp = 0;
    int i=0;

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");

        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
        TempNode.addText("C");

        //厂商编码
       // TempNode = DataNode.addChild("WorksNo");
        //TempNode.addText("C02");

        //设备编码
        TempNode = DataNode.addChild("F_SBBH");
        TempNode.addText(g_strFtpUserName.c_str());

        //设备IP
        TempNode = DataNode.addChild("F_IP");
        TempNode.addText(g_ServerHost.c_str());

        //记录时间
        TempNode = DataNode.addChild("F_JLSJ");
        std::string strTime = GetTimeCurrent();
        TempNode.addText(strTime.c_str());

        //cpu温度，摄氏度
        TempNode = DataNode.addChild("F_CPUWD");
        sprintf(buf,"%d",(int)g_sysInfo.fCpuT);
        TempNode.addText(buf);

        //strTag = "F_CPUFSZS"; //cpu风扇转速,转/分
        //CheckXMLNode(DataNode, TempNode, i, strTag);

        //cpu占有率 ，百分比 96 = 98%
        TempNode = DataNode.addChild("F_CPUZYL");
        sprintf(buf,"%d",(int)g_sysInfo.fCpu);
        TempNode.addText(buf);

        //剩余物理内存,Kbyte
        TempNode = DataNode.addChild("F_SYWLNC");
        sprintf(buf,"%u",(unsigned int)(2*1024*1024*(100-g_sysInfo.fMemory)/100.0));
        TempNode.addText(buf);

        //strTag = "F_XCSL"; //前端设备线程计数
        //CheckXMLNode(DataNode, TempNode, i, strTag);

        //磁盘空间,Kbyte
        TempNode = DataNode.addChild("F_CPKJ");
        sprintf(buf,"%u",(unsigned int)(g_sysInfo_ex.fTotalDisk*1024*1024*(100-g_sysInfo.fDisk)/100.0));
        TempNode.addText(buf);

        //前端保存的未上传违法数据量(需要计算)
        TempNode = DataNode.addChild("F_SJL");
        int nCount = GetUnsendCount();
        sprintf(buf,"%u",nCount);
        TempNode.addText(buf);

        //相机
        TempNode = DataNode.addChild("XJ");
        //相机数目
        TempNode1 = TempNode.addChild("F_XJSM");
        TempNode1.addText("1");
        //相机状态
        TempNode1 = TempNode.addChild("F_XJ1");
        int nCameraState = 1;
        if(g_skpDB.GetCameraState(g_nCameraID)==0)
        {
            nCameraState = 1;
        }
        else
        {
            nCameraState = 2;
        }
        sprintf(buf,"%u",nCameraState);
        TempNode1.addText(buf);

        //视频
        TempNode = DataNode.addChild("SP");
        //视频数目
        TempNode1 = TempNode.addChild("F_SPSM");
        TempNode1.addText("1");
        //视频状态
        TempNode1 = TempNode.addChild("F_SP1");
        TempNode1.addText(buf);

        //触发设备
        TempNode = DataNode.addChild("CF");
        //触发数目
        TempNode1 = TempNode.addChild("F_CFSM");
        TempNode1.addText("0");

        //补光设备
        TempNode = DataNode.addChild("BG");
        //补光数目
        TempNode1 = TempNode.addChild("F_BGSM");
        TempNode1.addText("1");
        //补光状态
        TempNode1 = TempNode.addChild("F_BG1");
        TempNode1.addText("1");

        //记录上一次通信是否正常-1，正常 0，不正常
        TempNode = DataNode.addChild("F_TX");
        TempNode.addText("1");

        //当天时钟同步失败次数
        TempNode = DataNode.addChild("F_SZ");
        TempNode.addText("0");

        //备注信息
        TempNode = DataNode.addChild("F_BZ");
        TempNode.addText("");


    string strPath;
    int nSize;
    XMLSTR strData = PackageNode.createXMLString(1, &nSize);
    if(strData)
    {
        strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
        strNewMsg.append(strData, sizeof(XMLCHAR)*nSize);

        strTime = GetTime(GetTimeStamp(),2);
        strTime += "000";
        sprintf(buf,"LOG_%8s_%s.XML",g_strFtpUserName.c_str(),strTime.c_str());
        strPath = buf;

        /*FILE* fp = fopen(strPath.c_str(),"wb");
        fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
        fclose(fp);*/

        freeXMLString(strData);
    }

    return strPath;
}


/* 函数介绍：生成运行类日志数据
 * 输入参数：log：日志数据，strPackageName:xml文件名称
 * 输出参数：PackageNode：数据包地址
 * 返回值：是否生成日志数据包成功
 */
string CRoadXmlData::AddRunLogData(const string &strMsg, string &strNewMsg)
{
   XMLNode PackageNode, PackageHeadNode, DataNode;
    char buf[64];
    memset(buf, 0, sizeof(buf));

    RECORD_LOG *pLog = (RECORD_LOG *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    XMLNode TempNode, TempNode1, TempNode2;
    std::string sTemp;
    XMLCSTR strTag;

    int iTemp = 0;
    int i=0;

    PackageNode = XMLNode::createXMLTopNode("Package");
    //包头数据
    PackageHeadNode = PackageNode.addChild("PackageHead");
    SetPackageHead(PackageHeadNode);

    //包体数据
    DataNode  = PackageNode.addChild("Data");

        //监测系统编号
        TempNode = DataNode.addChild("XTBH");
        TempNode.addText("C");

        //设备编码
        TempNode = DataNode.addChild("DeviceNo");
        TempNode.addText(g_strFtpUserName.c_str());

        //记录时间
        TempNode = DataNode.addChild("WatchTime");
        std::string strTime = GetTimeCurrent();
        strTime += ".000";
        TempNode.addText(strTime.c_str());

        //检测日期
        TempNode = DataNode.addChild("WatchDay");
        std::string strDay;
        strDay.append(strTime.c_str(),10);
        TempNode.addText(strDay.c_str());

        //记录的日志事件个数
        TempNode = DataNode.addChild("LogNum");
        int nCount = 1;
        sprintf(buf,"%u",nCount);
        TempNode.addText(buf);

        //日志
        TempNode = DataNode.addChild("EventData");
        //
        TempNode1 = TempNode.addChild("Log");
        //事件时间
        TempNode2 = TempNode1.addChild("EventTime");
        strTime = GetTime(pLog->uTime)+".000";
        TempNode2.addText(strTime.c_str());
        //日志内容
        TempNode2 = TempNode1.addChild("EventValue");
        string strLog(pLog->chText);
        TempNode2.addText(strLog.c_str());


    string strPath;
    int nSize;
    XMLSTR strData = PackageNode.createXMLString(1, &nSize);
    if(strData)
    {
        strNewMsg.append("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
        strNewMsg.append(strData, sizeof(XMLCHAR)*nSize);

        strTime = GetTime(GetTimeStamp(),2);
        strTime += "000";
        sprintf(buf,"RUNLOG_%8s_%s.XML",g_strFtpUserName.c_str(),strTime.c_str());
        strPath = buf;

        /*FILE* fp = fopen(strPath.c_str(),"wb");
        fwrite(strNewMsg.c_str(),strNewMsg.size(),1,fp);
        fclose(fp);*/

        freeXMLString(strData);
    }

    return strPath;
}

//获取行驶方向编码--参见1.8 行驶方向编码
/*
车道方向
enum ROAD_DIRECTION
{
    EAST_TO_WEST = 1,   //从东到西 default 1
    WEST_TO_EAST,       //从西到东
    SOUTH_TO_NORTH,     //从南到北
    NORTH_TO_SOUTH     //从北到南
};
*/
std::string CRoadXmlData::GetDirectionStr(std::string strDirection)
{
    std::string sDirect = "";
    if(strDirection == "1A")
    {
        sDirect = "由东向西";
    }
    else if(strDirection == "1B")
    {
        sDirect = "由西向东";
    }
    else if(strDirection == "2A")
    {
        sDirect = "由南向北";
    }
    else if(strDirection == "2B")
    {
        sDirect = "由北向南";
    }
    else if(strDirection == "3A")
    {
        sDirect = "由东南到西北";
    }
    else if(strDirection == "3B")
    {
        sDirect = "由西北到东南";
    }
    else if(strDirection == "4A")
    {
        sDirect = "由东北到西南";
    }
    else if(strDirection == "4B")
    {
        sDirect = "由西南到东北";
    }

    return sDirect;
}

/*
获取事件类型编码
A:交通拥堵：车辆排队长度超过报警设定值；
B:车辆停驶：车辆停止移动10秒（可调）以上认为车辆停驶；
C:遗弃物：车辆行驶后路面出现遗弃物；
D:行人：有人出现在车道中；
E:车辆慢行：车辆行驶速度低于设定的报警速度；
O:其他类型
*/
char CRoadXmlData::GetEventType(int nEvent)
{
    char chEvent;
    DETECT_RESULT_TYPE eventType = (DETECT_RESULT_TYPE)nEvent;

    switch(eventType)
    {
        case DETECT_RESULT_EVENT_JAM:
        {
            chEvent = 'A';
            break;
        }
        case DETECT_RESULT_EVENT_STOP:
        {
            chEvent = 'B';
            break;
        }
        case DETECT_RESULT_EVENT_DERELICT:
        {
            chEvent = 'C';
            break;
        }
        case DETECT_RESULT_EVENT_PERSON_APPEAR:
        {
            chEvent = 'D';
            break;
        }
        case DETECT_RESULT_EVENT_GO_SLOW:
        {
            chEvent = 'E';
            break;
        }
        case DETECT_RESULT_EVENT_GO_AGAINST:
        {
            chEvent = 'C';
            break;
        }
        case DETECT_RESULT_EVENT_PERSON_AGAINST:
        {
            chEvent = 'C';
            break;
        }
        case DETECT_RESULT_EVENT_APPEAR:
        {
            chEvent = 'G';
            break;
        }
        case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD:
        {
            chEvent = 'J';
            break;
        }
        case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD:
        {
            chEvent = 'J';
            break;
        }
        case DETECT_RESULT_NOCARNUM:
        {
            chEvent = 'H';
            break;
        }
        case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME:
        {
            chEvent = 'I';
            break;
        }
        default:
        {
            chEvent = '0';
            break;
        }
    }
    return chEvent;
}

//获取车牌颜色类型编码--见1.15车辆号牌颜色
int CRoadXmlData::GetCarNumColor(int nColor)
{
    CARNUM_COLOR carnumColor = (CARNUM_COLOR)nColor;
    int nValColor = 0;
    switch(carnumColor)
    {
        case CARNUM_BLUE://蓝色 1--》1
        {
            nValColor = 1;
            break;
        }
        case CARNUM_BLACK:  //黑色 2--》4
        {
            nValColor = 4;
            break;
        }
        case CARNUM_YELLOW:  //黄色 3--》2
        {
            nValColor = 2;
            break;
        }
        case CARNUM_WHITE:  //白色 4--》3
        {
            nValColor = 3;
            break;
        }
        default: //其他 5--》0（未识别），或5（绿色）
        {
            nValColor = 0;
            break;
        }
    }

    return nValColor;
}

//获取车牌标志类型编码--见1.16车辆标志分类--（暂保留）
int CRoadXmlData::GetCarLogo(int nLogo)
{
    return 0;
}



//解析业务控制参数内容
bool CRoadXmlData::GetControlInfo(const string &strMsg, TRAVEL_CONTROL_INFO &travelControlInfo)
{
    //保存认证数据
    FILE* fp = fopen("ControlInfo.xml","wb");
	if(fp)
	{
		fwrite(strMsg.c_str(),strMsg.size(),1,fp);
		fclose(fp);
	}

    XMLNode PackageNode, ParentNode;
	XMLNode TempNode;
	XMLNode TempNode1;
	XMLCSTR strText;
	XMLCSTR strTextTemp;

    string strNewMsg = strMsg;

    if(strNewMsg.size() < 36)
    {
        return false;
    }
    strNewMsg.erase(0,36);
	//string->xml
    PackageNode = XMLNode::parseString(strNewMsg.c_str());
    printf("=====strNewMsg=%s,PackageNode.nChildNode()=%d,getName=%s\n",strNewMsg.c_str(),PackageNode.nChildNode(),PackageNode.getName());
    {
        ParentNode = PackageNode.getChildNode("PackageBody");

        printf("=====ParentNode.nChildNode()=%d\n",ParentNode.nChildNode());

        TempNode = ParentNode.getChildNode("UploadRealtime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();

            if(strText)
            {
                printf("=====strText=%s\n",strText);
                if(strncmp(strText,"Start",5)==0)
                {
                    travelControlInfo.m_bIsUploadRealTime  = 1;
                }
                else
                {
                    travelControlInfo.m_bIsUploadRealTime = 0;
                }
            }
        }

        TempNode = ParentNode.getChildNode("UploadOpLogInterval");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            travelControlInfo.m_nUploadOplog = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("UploadDevStateInterval");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            travelControlInfo.m_nUploadDevState = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("SampleTime");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            travelControlInfo.m_nSampleTime = xmltoi(strText);
        }

        TempNode = ParentNode.getChildNode("VideoInterval");
        if(!TempNode.isEmpty())
        {
            strText = TempNode.getText();
            if(strText)
            travelControlInfo.m_nVideo = xmltoi(strText);
        }

        int nCount = ParentNode.nChildNode("VideoEvent");

        for(int i =0;i < nCount;i++)
        {
           TempNode = ParentNode.getChildNode("VideoEvent", i);

           printf("==TempNode.getName()=%s\n",TempNode.getName());
            if(!TempNode.isEmpty())
            {
                TempNode1 = TempNode.getChildNode("EventType");
                strTextTemp = TempNode1.getText();

                TempNode1 = TempNode.getChildNode("EventDefine");
                strText = TempNode1.getText();

                //printf("==strTextTemp=%s==strText=%s\n",strTextTemp,strText);
                if(strTextTemp)
                {
                    if(strncmp(strTextTemp,"A",1)==0)
                    {
                        if(strText)
                        travelControlInfo.m_nTraficLine = xmltoi(strText);
                        printf("==travelControlInfo.m_nTraficLine=%d\n",travelControlInfo.m_nTraficLine);
                    }
                    else if(strncmp(strTextTemp,"B",1)==0)
                    {
                        if(strText)
                        travelControlInfo.m_nCarStopTime = xmltoi(strText);
                        printf("==travelControlInfo.m_nCarStopTime=%d\n",travelControlInfo.m_nCarStopTime);
                    }
                    else if(strncmp(strTextTemp,"E",1)==0)
                    {
                        if(strText)
                        travelControlInfo.m_nCarSpeedSlow = xmltoi(strText);
                        printf("==travelControlInfo.m_nCarSpeedSlow=%d\n",travelControlInfo.m_nCarSpeedSlow);
                    }
                    else
                    {
                    }
                }
            }
        }
    }
}

//获取设备配置信息
bool CRoadXmlData::GetDeviceInfo(const string &strMsg)
{
    //保存认证数据
    FILE* fp = fopen("AuthData.xml","wb");

	if(fp)
	{
		fwrite(strMsg.c_str(),strMsg.size(),1,fp);
		fclose(fp);
	}

    XMLNode PackageNode, ParentNode;
	XMLNode TempNode;
	XMLNode TempNode1;
	XMLCSTR strText;
	XMLCSTR strTextTemp;
	int iTemp1, iTemp2;

    string strNewMsg = strMsg;
    if(strNewMsg.size() < 36)
    {
        return false;
    }
    strNewMsg.erase(0,36);

    string strDirection,strDesc;

    printf("==========GetDeviceInfo=====strNewMsg=%s=======!\n", strNewMsg.c_str());
	//string->xml
    PackageNode = XMLNode::parseString(strNewMsg.c_str());
    {
        ParentNode = PackageNode.getChildNode("PackageBody");

        TempNode = ParentNode.getChildNode("DeviceNo");
        if(!TempNode.isEmpty())
        {
           /* strText = TempNode.getText();
            if(strText)
            g_strFtpUserName = strText;

            printf("===============g_strFtpUserName=%s=======!\n", g_strFtpUserName);

            CXmlParaUtil xml;
            xml.UpdateSystemSetting("OtherSetting", "DetectorID");*///ftp用户名直接通过客户端配置进去
        }

        int nCount = ParentNode.nChildNode("Baseinfo");

        printf("=============================nCount=%d\n",nCount);
        for(int i = 0;i < nCount;i++)
        {
            TempNode = ParentNode.getChildNode("Baseinfo",i);

            TempNode1 = TempNode.getChildNode("CarRoad");
            strText = TempNode1.getText();
            if(strText)
            iTemp1 = xmltoi(strText);

             printf("=============================iTemp1=%d\n",iTemp1);

            ROAD_DESC road_desc;

            TempNode1 = TempNode.getChildNode("Direction");
            strText = TempNode1.getText();
            if(strText)
            {
                strDirection = strText;
                memcpy(road_desc.chDirection,strDirection.c_str(),strDirection.size());
            }

            printf("=============================strDirection.c_str()=%s\n",strDirection.c_str());

            TempNode1 = TempNode.getChildNode("DeviceDesc");
            strText = TempNode1.getText();
            if(strText)
            {
                strDesc = strText;
                memcpy(road_desc.chDeviceDesc,strDesc.c_str(),strDesc.size());
            }
            printf("=============================strDesc.c_str()=%s\n",strDesc.c_str());

            //插入到全局结构中
            g_roadDirectionMap.insert(ROAD_DIRECTION_MAP::value_type(iTemp1, road_desc));
        }
    }
}

//获取未上传违法数据量
int CRoadXmlData::GetUnsendCount()
{
    char szBuff[1024] = {0};
    sprintf(szBuff, "select count(*) from NUMBER_PLATE_INFO where STATUS=0;");

    int count1 = 0;
    MysqlQuery sql1 = g_skpDB.execQuery(string(szBuff));
    if (!sql1.eof())
    {
        count1 = sql1.getUnIntFileds(0);
    }
    sql1.finalize();

    sprintf(szBuff, "select count(*) from TRAFFIC_EVENT_INFO where STATUS=0;");
    int count2 = 0;
    MysqlQuery sql2 = g_skpDB.execQuery(string(szBuff));
    if (!sql2.eof())
    {
        count2 = sql2.getUnIntFileds(0);
    }
    sql2.finalize();

    int count = count1+count2;
    return count;
}


//获取车身位置
CvRect CRoadXmlData::GetCarPos(RECORD_PLATE plate)
{
        int w = plate.uPosRight - plate.uPosLeft;//宽度
        int h = plate.uPosBottom - plate.uPosTop;//高度

        CvRect rtCar;

        int dw = 2*w;
        if(plate.uType == SMALL_CAR)
        {
            dw = 3*w;
            rtCar.width = 7*w;
        }
        else
        {
            dw = 4*w;
            rtCar.width = 9*w;
        }
        rtCar.height = rtCar.width;


        int x = plate.uPosLeft-dw;
        int y = plate.uPosTop-rtCar.height+6*h;

        if(x > 0)
        {
            rtCar.x = x;
        }
        else
        {
            rtCar.x = 0;
        }

        int nExtentHeight = 0;
        if(g_PicFormatInfo.nWordPos == 1)
        {
            nExtentHeight = g_PicFormatInfo.nExtentHeight;
        }

        if(y > nExtentHeight)
        {
            rtCar.y = y;
        }
        else
        {
            rtCar.y = nExtentHeight;
        }

        //printf("0000========%d==%d,%d,%d,%d=============================\n",plate.uPosLeft-3*w,rtCar.x,rtCar.y,rtCar.width,rtCar.height);


        if(rtCar.x+rtCar.width>=plate.uPicWidth)
        {
            rtCar.x = plate.uPicWidth - rtCar.width-1;
        }

        if(rtCar.y+rtCar.height>=plate.uPicHeight)
        {
            rtCar.y = plate.uPicHeight - rtCar.height-1;
        }

       return rtCar;
}

//车牌事件结构转换
void CRoadXmlData::ConvertPlateToEvent(const string &strMsg,string &strViolationMsg)
{
        RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

        RECORD_EVENT  event;
        event.uSeq  = pPlate->uSeq;
        event.uEventBeginTime = pPlate->uTime;
        event.uMiEventBeginTime = pPlate->uMiTime;
        event.uTime2 = pPlate->uTime2;
        event.uMiTime2 = pPlate->uMiTime2;
        event.uEventEndTime = pPlate->uTime+5;
        event.uType = pPlate->uType;
        event.uRoadWayID = pPlate->uRoadWayID;
        event.uPicSize = pPlate->uPicSize;
        event.uPicWidth = pPlate->uPicWidth;
        event.uPicHeight = pPlate->uPicHeight;
        event.uPosX = (pPlate->uPosLeft+pPlate->uPosRight)/2;
        event.uPosY = (pPlate->uPosTop+pPlate->uPosBottom)/2;
        event.uColor1 = pPlate->uCarColor1;
        event.uSpeed = pPlate->uSpeed;
        event.uColor2 = pPlate->uCarColor2;
        event.uWeight1 = pPlate->uWeight1;
        event.uWeight2 = pPlate->uWeight2;
        event.uChannelID = pPlate->uChannelID;
        event.uDirection = pPlate->uDirection;
        memcpy(event.chText,pPlate->chText,sizeof(pPlate->chText));
        memcpy(event.chPlace,pPlate->chPlace,sizeof(pPlate->chPlace));
        memcpy(event.chPicPath,pPlate->chPicPath,sizeof(pPlate->chPicPath));
        event.uCode = pPlate->uViolationType;
        strViolationMsg.append((char*)strMsg.c_str(),sizeof(MIMAX_HEADER));
        strViolationMsg.append((char*)&event,sizeof(RECORD_EVENT));
        strViolationMsg.append((char*)(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE)),strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE));
}

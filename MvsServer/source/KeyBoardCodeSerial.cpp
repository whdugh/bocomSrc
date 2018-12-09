// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "KeyBoardCodeSerial.h"

//
CKeyBoardCodeSerial::CKeyBoardCodeSerial()
{

}
//
CKeyBoardCodeSerial::~CKeyBoardCodeSerial()
{

}

//设置监视器ID
void CKeyBoardCodeSerial::SetMonitorID(int nMonitorID)
{
    m_nMonitorID = nMonitorID;
}

//设置摄像机ID
void CKeyBoardCodeSerial::SetCameraID(int nCameraID)
{
    m_nCameraID = nCameraID;
}

//设置预置位ID
void CKeyBoardCodeSerial::SetPreSetID(int nPreSetID)
{
    m_nPreSetID = nPreSetID;
}

//设置键盘ID
void CKeyBoardCodeSerial::SetKeyBoardID(int nKeyBoardID)
{
    m_nKeyBoardID = nKeyBoardID;
}

//获取键前端码
std::string CKeyBoardCodeSerial::GetKeyHeadCode()
{
    std::string strKeyCode("");
    char chKey[7] = {'*', '8', '0', '0', 'K', '0','\0'};
    if(m_nKeyBoardID < 10)
    {
        chKey[2] = '0';
        chKey[3] = (char)( (m_nKeyBoardID % 10) + 48 );
    }
    else
    {
        chKey[2] = (char)( (m_nKeyBoardID / 10) + 48 );
        chKey[3] = (char)( (m_nKeyBoardID % 10) + 48 );
    }
    strKeyCode = chKey;

    //printf("==========CKeyBoardCodeSerial::GetKeyHeadCode()===m_nKeyBoardID=%d=-====strKeyCode= %s ==========\n", m_nKeyBoardID, strKeyCode.c_str());

    return strKeyCode;
}

//获取单字符键盘码字符串
std::string CKeyBoardCodeSerial::GetKeyCode(const char chInput)
{
    std::string strKeyCode_down, strKeyCode_up;
    std::string strKeyCode, strKeyHeadCode;

    strKeyHeadCode = GetKeyHeadCode();
    switch(chInput)
    {
        case '0':
            {
                strKeyCode_down = ",75,1";
                strKeyCode_up = ",75,0";
                break;
            }
        case '1':
            {
                strKeyCode_down = ",62,1";
                strKeyCode_up = ",62,0";
                break;
            }
        case '2':
            {
                strKeyCode_down = ",63,1";
                strKeyCode_up = ",63,0";
                break;
            }
        case '3':
            {
                strKeyCode_down = ",64,1";
                strKeyCode_up = ",64,0";
                break;
            }
        case '4':
            {
                strKeyCode_down = ",66,1";
                strKeyCode_up = ",66,0";
                break;
            }
        case '5':
            {
                strKeyCode_down = ",67,1";
                strKeyCode_up = ",67,0";
                break;
            }
        case '6':
            {
                strKeyCode_down = ",68,1";
                strKeyCode_up = ",68,0";
                break;
            }
        case '7':
            {
                strKeyCode_down = ",70,1";
                strKeyCode_up = ",70,0";
                break;
            }
        case '8':
            {
                strKeyCode_down = ",71,1";
                strKeyCode_up = ",71,0";
                break;
            }
        case '9':
            {
                strKeyCode_down = ",72,1";
                strKeyCode_up = ",72,0";
                break;
            }
        case 'm': //Monitor
            {
                strKeyCode_down = ",61,1";
                strKeyCode_up = ",61,0";
                break;
            }
        case 'c': //Camera
            {
                strKeyCode_down = ",73,1";
                strKeyCode_up = ",73,0";
                break;
            }
        case 'e': //Enter
            {
                strKeyCode_down = ",76,1";
                strKeyCode_up = ",76,0";
                break;
            }
        case 's': //Set
            {
                strKeyCode_down = ",26,1";
                strKeyCode_up = ",26,0";
                break;
            }
        case 'd': //Del
            {
                strKeyCode_down = ",17,1";
                strKeyCode_up = ",17,0";
                break;
            }
        default:
            break;
    }

    std::string strKeyTemp = "\r";
    strKeyCode = strKeyHeadCode + strKeyCode_down + strKeyTemp + strKeyHeadCode + strKeyCode_up + strKeyTemp;

  //  printf("===============CKeyBoardCodeSerial::GetKeyCode()======strKeyCode.size()=%d===========\n", strKeyCode.size());
  //  printf("===============CKeyBoardCodeSerial::GetKeyCode()==========strKeyCode= %s=============\n", strKeyCode.c_str());
    return strKeyCode;
}

//获取整数字符串码
std::string CKeyBoardCodeSerial::GetNumberCode(int nInputNum)
{
	char buffer[10] = {0};
    //_itoa(nInputNum, buffer, 10);
    sprintf(buffer, "%ld", nInputNum);
   // printf("=========CKeyBoardCodeSerial::GetNumberCode()==number_buffer=%s=========\n", buffer);

    std::string strNumber("");
	std::string strTemp("");

    for(int i=0; i<10; i++)
    {
        if(buffer[i] != '\0')
        {
            strTemp = GetKeyCode(buffer[i]);
        }
        else
        {
            break;
        }

        strNumber += strTemp;
    }

    //printf("=========CKeyBoardCodeSerial::GetNumberCode()===strNumber=%s=, nInputNumber=%d======\n", strNumber.c_str(), nInputNum);

    return strNumber;
}

//获取控制字符串码
//nDownFlag:按键按下状态：0，按下又抬起 1：按下 2：抬起
std::string CKeyBoardCodeSerial::GetControlCode(CAMERA_CONFIG& cfg, bool bSwitchFlag, int nDownFlag,int nSpeed)
{
    std::string strKeyControlCode, strKeyControlCode1, strKeyControlCode2;
    std::string strKeyCode_down, strKeyCode_up;
    std::string strKeyCode;
    std::string strKeyHeadCode = GetKeyHeadCode();
    std::string strMonitorCode, strMonitorIdCode;
    std::string strCameraCode, strCameraIdCode;
    std::string strPreSetCode,strPreSetIdCode;

    std::string strKeyCode_enter = GetKeyCode('e');
    std::string strKeyTemp = "\r";
    int nPreSetFlag = 0; //是否选择预置位
    std::string strKeyCode_set = GetKeyCode('s');
    std::string strKeyCode_del = GetKeyCode('d');

    CAMERA_MESSAGE nMsg = (CAMERA_MESSAGE)cfg.nIndex;
	int ndata = (int)cfg.fValue;
    bool bOnlySwitchFlag = false;
    char buf[256] = {0};
	switch(nMsg)
	{
	    case ZOOM_NEAR:
        {
            //strKeyCode_down = ",86,5";
            sprintf(buf,",86,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",86,0";
            break;
        }
        case ZOOM_FAR:
        {
          //strKeyCode_down = ",87,5";
            sprintf(buf,",87,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",87,0";
            break;
        }
        case FOCUS_NEAR:
        {
           // strKeyCode_down = ",88,5";
           sprintf(buf,",88,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",88,0";
            break;
        }
        case FOCUS_FAR:
        {
           // strKeyCode_down = ",89,5";
            sprintf(buf,",89,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",89,0";
            break;
        }
        case SET_PRESET:
        {
            strPreSetIdCode = GetNumberCode(m_nPreSetID);
            strKeyCode_down = ",74,1";
            strKeyCode_up = ",74,0";

            nPreSetFlag = 1;
            strPreSetCode = strKeyCode_set + strPreSetIdCode;
            break;
        }
        case GOTO_PRESET:
        {
            strPreSetIdCode = GetNumberCode(m_nPreSetID);
            strKeyCode_down = ",74,1";
            strKeyCode_up = ",74,0";

            nPreSetFlag = 2;
            strPreSetCode = strPreSetIdCode;
            break;
        }
        case CLEAR_PRESET:
        {
            strPreSetIdCode = GetNumberCode(m_nPreSetID);
            strKeyCode_down = ",74,1";
            strKeyCode_up = ",74,0";

            nPreSetFlag = 3;
            strPreSetCode = strKeyCode_del + strPreSetIdCode;
            break;
        }
        case UP_DIRECTION:
        {
           // strKeyCode_down = ",96,5";
            sprintf(buf,",96,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",96,0";
            break;
        }
        case DOWN_DIRECTION:
        {
            //strKeyCode_down = ",97,5";
            sprintf(buf,",97,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",97,0";
            break;
        }
        case LEFT_DIRECTION:
        {
            //strKeyCode_down = ",98,5";
            sprintf(buf,",98,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",98,0";
            break;
        }
        case RIGHT_DIRECTION:
        {
            //strKeyCode_down = ",99,5";
            sprintf(buf,",99,%d",nSpeed);
            strKeyCode_down = buf;
            strKeyCode_up = ",99,0";
            break;
        }
        case SWITCH_CAMERA:
        {
            bOnlySwitchFlag = true;
            break;
        }
		default:
			return "";
	}//End of switch

	if(bSwitchFlag) //切换视频(始终为真)
    {
        strMonitorCode = GetKeyCode('m');
        strMonitorIdCode = GetNumberCode(m_nMonitorID);
      //  printf("=========CKeyBoardCodeSerial::GetControlCode======strMonitorIdCode=%s=====\n", strMonitorIdCode.c_str());

        strCameraCode = GetKeyCode('c');
        strCameraIdCode = GetNumberCode(m_nCameraID);
      //  printf("=========CKeyBoardCodeSerial::GetControlCode======strCameraCode=%s=====\n", strCameraCode.c_str());

        //切换Monitor 和 Camera对应
        strKeyControlCode1 = strMonitorCode + strMonitorIdCode + strKeyCode_enter + strCameraCode + strCameraIdCode + strKeyCode_enter;
      //  printf("===============CKeyBoardCodeSerial::GetControlCode()====1==strKeyControlCode1.size()=%d===========\n", strKeyControlCode1.size());

        if(bOnlySwitchFlag)
        return strKeyControlCode1; //返回切换视频码
    }

    //strKeyCode = strKeyHeadCode + strKeyCode_down + strKeyTemp + strKeyHeadCode + strKeyCode_up + strKeyTemp;

    if(1 == nDownFlag)
    {
        strKeyCode = strKeyHeadCode + strKeyCode_down + strKeyTemp;
    }
    else if(2 == nDownFlag)
    {
        strKeyCode = strKeyHeadCode + strKeyCode_up + strKeyTemp;
    }
    else
    {
        strKeyCode = strKeyHeadCode + strKeyCode_down + strKeyTemp + strKeyHeadCode + strKeyCode_up + strKeyTemp;
    }

   // printf("===============CKeyBoardCodeSerial::GetControlCode()======strKeyCode.size()=%d===========\n", strKeyCode.size());

    if(nPreSetFlag != 0)
    {
   //     printf("==============CKeyBoardCodeSerial::GetControlCode()===SetPreSet===\n");
        strKeyCode += strPreSetCode;
    }

    if(1 != nDownFlag)
    {
        strKeyControlCode2 = strKeyCode + strKeyCode_enter;
    }
    else
    {
        strKeyControlCode2 = strKeyCode;
    }

   // printf("nDownFlag=%d,nPreSetFlag=%d,bSwitchFlag=%d\n",nDownFlag,nPreSetFlag,bSwitchFlag);

  //  printf("===============CKeyBoardCodeSerial::GetControlCode()======strKeyControlCode2.size()=%d===========\n", strKeyControlCode2.size());

    strKeyControlCode = strKeyControlCode1 + strKeyControlCode2;

    return strKeyControlCode;//返回键盘控制码
}

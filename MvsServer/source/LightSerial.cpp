#include"LightSerial.h"
#include "Common.h"
#include "CommonHeader.h"
LightSerial L_SerialComm;

LightSerial::LightSerial()
{
    fd_com = -1;
    m_nCount = 999;
}
LightSerial::~LightSerial()
{

}
bool LightSerial::OpenDev()
{
    fd_com = open_port(g_LightComSetting.nComPort,g_LightComSetting.nBaud);
    if(fd_com!=-1)
    return true;
    else
    return false;
}

//使能脉冲间隔
bool LightSerial::SetPulsInter(int nSpace)
{
    if(fd_com!=-1)
    {
    unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
    int nWrite = 0;

    nSpace = nSpace/1000 - 1;
    szCmd[5] = 0x06;
    szCmd[6] = nSpace&0xff;

    printf("InsertPulsSpace:%s\n", szCmd);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));
    usleep(1000*50);
    return (nWrite == sizeof(szCmd));
    }
    return false;
}

//使能脉冲宽度
bool LightSerial::SetPulsWidth(int nWidth)
{
    if(fd_com!=-1)
    {
    unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
    int nWrite = 0;

    nWidth--;
    szCmd[5] = 0x07;
    szCmd[6] = nWidth&0xff;

    printf("InsertPulsWidth:%s\n", szCmd);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));
    usleep(1000*50);
    return (nWrite == sizeof(szCmd));
    }
    return false;
}

//设置开/关灯(主灯/从灯)
bool LightSerial::SetOpenAndHost(bool bOpen, bool bHost)
{
    if(fd_com!=-1)
    {
        unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
        int nWrite = 0;

        szCmd[5] = 0x05;
        if (bOpen)
        {
            if (bHost)
            {
                szCmd[6] = 0x07;
            }
            else
            {
                szCmd[6] = 0x03;
            }
        }
        else
        {
            if (bHost)
            {
                szCmd[6] = 0x05;
            }
            else
            {
                szCmd[6] = 0x01;
            }
        }

        printf("SetLightOpen:%x\n", szCmd[6]);
        nWrite = write(fd_com, szCmd, sizeof(szCmd));
        usleep(1000*50);
        if(nWrite == sizeof(szCmd))
        {
            //LogNormal("SetOpenAndHost==:%x\n",szCmd[6]);
            return true;
        }
    }
    return false;
}

//调节分频计数器
bool LightSerial::AdjustCount(int nFrenCount)
{
    if(fd_com!=-1)
    {
    unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
    int nWrite = 0;

    int nHigh8 = (nFrenCount >> 8) & 0xff;
    int nLow8 = nFrenCount & 0xff;

    szCmd[5] = 0x08;
    szCmd[6] = nLow8&0xff;

    printf("AdjustFreqCount1:%x\n", szCmd[6]);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));
    usleep(1000*100);

    if (nWrite != sizeof(szCmd))
    {
        cout << "调节分频计数器错误" << endl;
        return false;
    }

    szCmd[5] = 0x09;
    szCmd[6] = nHigh8&0xff;

    printf("AdjustFreqCount2:%x\n", szCmd[6]);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));
    usleep(1000*100);
    return (nWrite == sizeof(szCmd));
    }
    return false;
}

//设置频率
bool LightSerial::SetFrequency(int nFren)
{
    if(fd_com!=-1)
    {
    if(nFren == 10)
    {
        nFren = 100000;
    }
    else if(nFren == 15)
    {
        nFren = 66666;
    }
    else if(nFren == 20)
    {
        nFren = 50000;
    }
    else if(nFren == 25)
    {
        nFren = 40000;
    }
     printf("SetFrequency:%d\n", nFren);
    float fMsFren = (float)nFren/1000;
    int nMsFren;
    if(nFren%1000== 0)
    {
        m_nCount = 999;
        nMsFren = (int)fMsFren-1;
    }
    else
    {
        m_nCount = (fMsFren/((int)fMsFren+1)) * 1000 -1;
        nMsFren = (int)fMsFren;
    }
    printf("SetFrequency:m_nCount%d\n", m_nCount);

    if (!AdjustCount(m_nCount)) //设置分频计数器
    {
        return false;
    }

    if (!AdjustFrequency(nMsFren)) //设置频率调节值
    {
        return false;
    }
    }
    return false;
}

//调节频率
bool LightSerial::AdjustFrequency(int nFren)
{
    if(fd_com!=-1)
    {
    unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
    int nWrite = 0;

    szCmd[5] = 0x0A;
    szCmd[6] = nFren&0xff;

    printf("AdjustFrequency:%x\n", szCmd[6]);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));

    usleep(1000*50);
    return (nWrite == sizeof(szCmd));
    }
    return false;
}

//调节脉宽
bool LightSerial::AdjustPuls(int nWidth)
{
    if(fd_com!=-1)
    {
    printf("====nWidth:%d\n", nWidth);
    unsigned char szCmd[8]={0x2A, 0x57, 0xDD, 0x02, 0x08, 0x00, 0x00, 0x0D};
    int nWrite = 0;

    nWidth = 2*(m_nCount+1) - nWidth - 1;
    if(nWidth < 0)
    nWidth = 0;
    printf("===22=m_nCount=%d,nWidth:%d\n", m_nCount,nWidth);

    int nHigh8 = (nWidth >> 8) & 0xff;
    int nLow8 = nWidth & 0xff;

    szCmd[5] = 0x0B;
    szCmd[6] = nLow8&0xff;

    printf("SetPulsWidth1:szCmd[6]=%x\n", szCmd[6]);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));
    usleep(1000*100);

    if (nWrite != sizeof(szCmd))
    {
        cout << "设置脉宽错误" << endl;
        return false;
    }

    unsigned char protoclCmd[8]={0};
    int nread = read(fd_com,protoclCmd,8);
    printf("read PulsWidth1:protoclCmd[6]=%x\n", protoclCmd[0]);

    szCmd[5] = 0x0C;
    szCmd[6] = nHigh8&0xff;

    printf("SetPulsWidth1:szCmd[6]=%x\n", szCmd[6]);
    nWrite = write(fd_com, szCmd, sizeof(szCmd));

    usleep(1000*100);
    return (nWrite == sizeof(szCmd));
    }
    return false;
}


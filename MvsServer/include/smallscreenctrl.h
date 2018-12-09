#ifndef   SMALL_SCREEN_CTRL_H
#define   SMALL_SCREEN_CTRL_H


#include "AbstractSerial.h"
#include "Common.h"

class CSmallScreenCtrl:public AbstractSerial
{
public:
	CSmallScreenCtrl();
	~CSmallScreenCtrl();

public:
	//打开设备
	bool OpenDev();
	//void Init();
	void BeginThread();
	void RecData();
	void screen_ctrl(char a,char b);
	void position_line(char a,char b,char c);
	void line_disp(char* buf);
	void cursor(int key5);

private:
};

extern CSmallScreenCtrl g_SmallScreenCtrl;

#endif 

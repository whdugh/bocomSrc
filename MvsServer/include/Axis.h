#ifndef _AXIS_H_
#define _AXIS_H_

#include "AbstractCamera.h"


class Axis:public AbstractCamera
{
	public:
		Axis(int nCameraType,int nPixelFormat);
		~Axis();

		virtual bool Open();
		virtual bool Close();
		virtual bool ReOpen();

		
		void CaptureFrame();

	private:
		void Init();

		bool videoRequest();

		bool setPulseTime(int a_nTime);
		bool setDelayTime(int a_nTime);
		bool setOutputMode(int a_level);
		bool setManualShutter(int a_nValue);
		bool setManualGain(int a_nValue);
		

		int RecvData();
		int connect_tcp();
		int command_send(char *);

		int axis_mode(int a_nMode);
		int axis_shutterControl(int a_nMode);
		int axis_gainControl(int a_nMode);
		int axis_outputActiveControl(int a_nMode);
		


		int tcp_fd;

		std::string strBase64;
};

#endif  _AXIS_H_
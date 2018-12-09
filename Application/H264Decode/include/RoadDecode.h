#ifndef ROAD_DECODE_H
#define ROAD_DECODE_H
#include <stdlib.h>
extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include "libswscale/swscale.h"
}

typedef void (*fCallBack)(int index, uint8_t* pBuffer, int nSize);
class RoadDecode
{
    private:
        bool m_bRealTimeMode;
        int  m_videoStreamIndex;
        int m_nFrameRate;
        uint8_t *m_pBuffer;
        PixelFormat  m_OutputPixFormat;
         PixelFormat  m_InputPixFormat;
        AVFormatContext *pFormatCtx;
        AVCodecContext *pCodecCtx;
        AVCodec  *pCodec;
        AVFrame *pInputFrame;
        AVFrame *pOutputFrame;
        SwsContext *img_convert_ctx;
        //for real strem
        CodecID  m_VideoCodeID;
        int m_OutputVideoWidth;
        int m_OutputVideoHeight;
        bool initDecode(CodecID codeID);
    public:
        RoadDecode();
        virtual ~RoadDecode();
        void SetVideoSize(int width, int height);
        bool GetVideoSize(int &width, int &height,uint8_t* pBuffer = 0, int nInSize = 0);
        //0 for H264 , 1 for MJPEG
        void SetVideoCodeID(int type);
        bool GetVideoCodeID(int &type);

		//
		void GetFileHeaderInfo(uint8_t** pFileHeaderInfo,int& nFileHeaderSize);
		void SetFileHeaderInfo(uint8_t* pFileHeaderInfo,int nFileHeaderSize);

         PixelFormat GetPixelFormat();
	void SetPixelFormat(PixelFormat nPixelFormat);

        //
        bool GetVideoFrameRate(int &frameRate);
        int InitDecode(char * pFileName);
        void Decode(fCallBack pfnCallBack);
        bool GetNextFrame(uint8_t* pBuffer, int &nSize);
        void UinitDecode();
        bool DecodeFrame(uint8_t* pBuffer, int nInSize, uint8_t* pOutBuffer, int &nOutSize);
    protected:
    private:
		int m_uFileHeaderSize;
		uint8_t* m_pFileHeaderInfo;
		bool m_bDeCodeFile;//是否直接读文件

};

#endif // ROAD_DECODE_H

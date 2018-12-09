#ifndef _CONVNET_H_
#define _CONVNET_H_

#include <vector>
using namespace std;
#include "cv.h"

enum __LAYER_TYPE__{
	__LAYER_TYPE_UNKNOWN__,
	__LAYER_TYPE_CONV__,
	__LAYER_TYPE_MAXPOOL__,
	__LAYER_TYPE_AVGPOOL__,
	__LAYER_TYPE_LOCAL__,
	__LAYER_TYPE_LOCAL_SHARED__,
	__LAYER_TYPE_FULL__,
	__LAYER_TYPE_CONCATENATE__,
	__LAYER_TYPE_DATA__,
	__LAYER_TYPE_BN__,
	__LAYER_TYPE_PRELU__,
	__LAYER_TYPE_LRN__,
	__LAYER_TYPE_SIGM__,
	__LAYER_TYPE_TANH__,
	__LAYER_TYPE_TANH_ABS__,
	__LAYER_TYPE_ABS__,
	__LAYER_TYPE_ReLU__,
};

enum __ACTIVATION_FUNC__{
	__ACTIVATION_UNCHANGED__,
	__ACTIVATION_FUNC_SIGM__,
	__ACTIVATION_FUNC_TANH__,
	__ACTIVATION_FUNC_TANH_ABS__,
	__ACTIVATION_FUNC_ABS__,
	__ACTIVATION_FUNC_RELU__,
};

struct LayerParams
{	
	__LAYER_TYPE__ m_type;
	__ACTIVATION_FUNC__ m_func;
	int m_nInputs;
	int m_nOutputs;
	int m_nInputSize, m_nInputSizeH;
	int m_nOutputSize, m_nOutputSizeH;
	int m_nTileSize, m_nTileSizeH;
	int m_nKernelSize,m_nKernelSizeH;
	int m_nStride,m_nStrideH;
	int m_nSizeX,m_nSizeXH;
	float m_fDropout;
	int m_nMultiScale;
	int m_nPad,m_nPadH;	
	int m_nSoftMax;
	int m_nInputTileSize, m_nInputTileSizeH;
	int m_nTileNum, m_nTileNumH;
	float m_fScale;
	int m_nReserved[10];
	float m_fReserved[10];
	int m_group;
	int m_weight_offset;
	int m_bias_offset;
	int m_local_size;
	float alpha;
	float beta;	
	float k;
	char name[256];
	char bottom[4][256];
	char top[256];
};

class  DLConvNetOptimize;
class ConvNet
{
public:
	ConvNet();
	ConvNet( vector<LayerParams>& pLayerparams );

	~ConvNet();

	int Reshape( vector<LayerParams>& pLayerparams );

	int Create( vector<LayerParams>& pLayerparams );

	int LoadModel( char *params );

	int LoadModel( ifstream *params );
	
	//支持float输入
	int fprop( float *pSource, int nWidth, int nHeight, int nChannels,float *fConfidence = NULL, float fSmoothConfidence = 1.0f  );

	//支持IplImage输入
	int fprop( IplImage *pSourceImage, float *fConfidence = NULL, float fSmoothConfidence = 1.0f  );	

	int UnInitial();

private:
	int nFlagInital;
	DLConvNetOptimize *m_ConvNet;

public:
	int SetNetName( char* chNetName );
};

#endif/* _CONVNET_H_ */


// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef SURF_H
#define SURF_H

#include <cv.h>
#include "fasthessian.h"

#include <vector>



class Surf {
  
  public:
    
    //! Standard Constructor (img is an integral image)
    Surf(IplImage *img, std::vector<Ipoint> &ipts);

    //! Describe all features in the supplied vector
    void getDescriptors(bool bUpright = false);
  
  private:
    
    //---------------- Private Functions -----------------//

    //! Assign the current Ipoint an orientation
    void getOrientation();
    
    //! Get the descriptor. See Agrawal ECCV 08
    void getDescriptor(bool bUpright = false);

    //! Calculate the value of the 2d gaussian at x,y
    inline float gaussian(int x, int y, float sig);
    inline float gaussian(float x, float y, float sig);

    //! Calculate Haar wavelet responses in x and y directions
    inline float haarX(int row, int column, int size);
    inline float haarY(int row, int column, int size);

    //! Get the angle from the +ve x-axis of the vector given by [X Y]
    float getAngle(float X, float Y);


    //---------------- Private Variables -----------------//

    //! Integral image where Ipoints have been detected
    IplImage *img;

    //! Ipoints vector
    IpVec &ipts;

    //! Index of current Ipoint in the vector
    int index;
};


#endif
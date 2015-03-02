/// \file
#include <map>
#include <iostream>
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "api.h"
extern "C"
{
#include "../sizes.h"
#include "../typedefs.h"
}
using namespace cv;
Mat ProcessMatrix(const double* data,const double min,const double max,const unsigned int size)
{
	double* dispmat = (double*)malloc(sizeof(*dispmat)*size*size);
	for (unsigned int i=0;i<size;i++)
	{
		for (unsigned int j=0;j<size;j++)
		{
			dispmat[i*size+j]=(data[i*size+j] - min)/(max-min);
		}
	}
	Mat m(size,size,CV_64F,dispmat);
    Mat n;
    m.convertTo(n, CV_8UC1, 255.0 , 0);
	Mat outmat;
	applyColorMap(n,outmat,COLORMAP_JET);
    free(dispmat);
    return outmat;
}

void getcolors(const double* data, const double min, const double max, const unsigned int size, uchar* red,uchar* blue,uchar* green)
{
    Mat outmat = ProcessMatrix(data,min,max,size);
    for(unsigned int i=0;i<size;i++)
    {
        for(unsigned int j=0;j<size;j++)
        {
            Vec3b intensity = outmat.at<Vec3b>(i, j);
            blue[i*size+j]  = intensity.val[0];
            green[i*size+j] = intensity.val[1];
            red[i*size+j]   = intensity.val[2];
        }
    }
}

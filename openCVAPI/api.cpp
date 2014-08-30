#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "api.h"
using namespace cv;
Mat ProcessMatrix(const double* data,const double min,const double max,const unsigned int size)
{
	double* dispmat = (double*)malloc(sizeof(*dispmat)*size*size);
	for (int i=0;i<size;i++)
	{
		for (int j=0;j<size;j++)
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
void PlotColored(const char* winname,const double* data,const double min,const double max,const unsigned int size)
{
    Mat outmat = ProcessMatrix(data,min,max,size);
	imshow(winname,outmat);
}

void SaveImage(const char* filename,const double* data,const double min,const double max,const unsigned int size)
{
    Mat outmat = ProcessMatrix(data,min,max,size);
    imwrite(filename,outmat);
}


#ifndef __EFFECTMOSAIC_H__
#define __EFFECTMOSAIC_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

class EffectMosaic 
{
	private:
		void SetColor(IplImage *img, int col, int row, int r, int g, int b);
		void AverageColor(IplImage *img, int col, int row, int *r, int *g, int *b);
	public:
		EffectMosaic();
		~EffectMosaic();
		IplImage *Mosaic(IplImage *imgIn);
};

#endif

#ifndef __EFFECTCOMIC_H__
#define __EFFECTCOMIC_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

class EffectComic 
{
	private:
		int colorfulPrev;
		int cluster_count;
		void SetMatrix(CvMat *mat, int row, int col, int channel, int val);
		void Dilatation(IplImage *out, int diff, int method);
	public:
		EffectComic();
		~EffectComic();
		IplImage *Comic(IplImage *imgIn);
};

#endif

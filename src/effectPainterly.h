#ifndef __EFFECTPAINTERLY_H__
#define __EFFECTPAINTERLY_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

class EffectPainterly 
{
	private:
		IplImage *prevFrame;
		IplImage *prevBlur; 
		bool firstFrame;
	public:
		EffectPainterly();
		~EffectPainterly();
		void RestrictedSmooth(IplImage *image, IplImage *prevImage);
		IplImage *Painterly(IplImage *imgIn);
};

#endif

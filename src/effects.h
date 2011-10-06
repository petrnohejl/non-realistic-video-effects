#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

class Effects 
{
	private:
		bool showWin;
		char *outputFile;
	public:
		Effects();
		~Effects();
		enum EffectType {COMIC, PAINTERLY, MOSAIC};
		void run(char *inputVideo, EffectType effect);
		void setOutput(char *file);
		void showWindow(bool val);	
};

#endif

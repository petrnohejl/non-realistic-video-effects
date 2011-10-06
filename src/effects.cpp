#include "effects.h"
#include "effectComic.h"
#include "effectPainterly.h"
#include "effectMosaic.h"


// Konstruktor
Effects::Effects() {
	outputFile = NULL;
	showWin = false;
}

// Destruktor
Effects::~Effects() {
}

void Effects::setOutput(char *file) {
	outputFile = file;
}

void Effects::showWindow(bool val) {
	showWin = val;
}

void Effects::run(char *inputVideo, EffectType effect) {

	CvCapture* capture = NULL;
	IplImage *result = NULL;
	IplImage *frame = NULL;
	double fps = 25.0;

	// Ukladac videa
	CvVideoWriter *AVIWriter = NULL;

	fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);

	// Nebudeme video ukladat, ale zobrazovat
	if (outputFile == NULL) {
		showWin = true;
	}

	// Otevreni videa ...
	if (inputVideo == NULL) {	// ... z kamery
		capture = cvCaptureFromCAM(-1);
	}
	else {						// ... ze souboru
		capture = cvCaptureFromAVI(inputVideo);
	}

	// Vytvoreni pojmenovaneho okna
	if (showWin) {
		cvNamedWindow("Original", 1);
		cvNamedWindow("Result", 1);
	}

	EffectComic *effectComic;
	effectComic = new EffectComic();

	EffectPainterly *effectPainterly;
	effectPainterly = new EffectPainterly();

	EffectMosaic *effectMosaic;
	effectMosaic = new EffectMosaic();

	// Hlavni smycka
	while (capture != NULL)
	{
		if (!cvGrabFrame(capture)) {
			// Rychle zachyceni obrazu z kamery
			break;    
		}

		// Ziskani aktualniho obrazu
		frame = cvRetrieveFrame(capture);	
		
		if (result != NULL) {
			cvReleaseImage(&result);
			result = NULL;
		}

		// EFEKTY ////////////////////////////////////////////////////////////////////////////////
		switch(effect)
		{
			case COMIC:
				result = effectComic->Comic(frame);
				break;
			case PAINTERLY:
				result = effectPainterly->Painterly(frame);	
				break;
			case MOSAIC:
				result = effectMosaic->Mosaic(frame);
				break;
			default:
				result = NULL;
		}


		// Zobrazeni aktualniho obrazu
		if (showWin) {	
			cvShowImage("Original", frame);
			cvShowImage("Result", result);
		}
		
		// Vytvoreni ukladace videa
		if (outputFile != NULL) {
			if(AVIWriter == NULL) {
				AVIWriter = cvCreateVideoWriter(outputFile, CV_FOURCC('M','J','P','G'),
				fps, cvSize(result->width , result->height), 1);
			}
		}

		// Zapsani jednoho framu do video souboru
		if (outputFile != NULL && AVIWriter != NULL) {
			cvWriteFrame(AVIWriter, result);
		}

		// Pocka 10 ms na libovolnou klavesu
		if (cvWaitKey(10) >= 0) {
			break;
		}
	}

	if (capture != NULL) {
		cvReleaseCapture(&capture);
	}

	if (AVIWriter != NULL) {
		cvReleaseVideoWriter(&AVIWriter);
	}

	if (showWin) {
		cvDestroyWindow("Original");
		cvDestroyWindow("Result");
	}

}

#include "effectPainterly.h"

#define INIT_STEP 16
#define INIT_RADIUS 15
#define INIT_LINE_THICK 18

#define STEP_DECR 4
#define RADIUS_DECR 4
#define LINE_DECR 3

#define PASS_CNT 10
#define DIFFER 10
#define DIFFER_BLURED 10

float gauss[3][3] =
{
	{ 1.0/16.0, 2.0/16.0, 1.0/16.0 },
	{ 2.0/16.0, 4.0/16.0, 2.0/16.0 },
	{ 1.0/16.0, 2.0/16.0, 1.0/16.0 }
};


EffectPainterly::EffectPainterly()
{
	firstFrame = true;
	prevFrame = NULL;
	prevBlur = NULL;
}


EffectPainterly::~EffectPainterly()
{
	// Uvolneni pameti
	if (prevFrame != NULL)
	{
		cvReleaseImage(&prevFrame);
		prevFrame = NULL;
	}

	if (prevBlur != NULL)
	{
		cvReleaseImage(&prevBlur);
		prevBlur = NULL;
	}
}


void EffectPainterly::RestrictedSmooth(IplImage *image, IplImage *prevImage)
{
	float sumR, sumB, sumG;
	IplImage *tmpImage = cvCloneImage(image);

	// Pruchod celym obrazkem
	for (int row = 0; row < image->height; row++)
	{
		for (int col = 0; col < image->width; col++)
		{
			sumR = 0.0f;
			sumG = 0.0f;
			sumB = 0.0f;

			CvScalar currColor;
			currColor.val[0] = ((uchar *)(image->imageData + row*image->widthStep))[col*image->nChannels + 0]; // B
			currColor.val[1] = ((uchar *)(image->imageData + row*image->widthStep))[col*image->nChannels + 1]; // G
			currColor.val[2] = ((uchar *)(image->imageData + row*image->widthStep))[col*image->nChannels + 2]; // R

			CvScalar prevColor;
			prevColor.val[0] = ((uchar *)(prevImage->imageData + row*prevImage->widthStep))[col*prevImage->nChannels + 0]; // B
			prevColor.val[1] = ((uchar *)(prevImage->imageData + row*prevImage->widthStep))[col*prevImage->nChannels + 1]; // G
			prevColor.val[2] = ((uchar *)(prevImage->imageData + row*prevImage->widthStep))[col*prevImage->nChannels + 2]; // R

			CvScalar colorDiff;
			colorDiff.val[0] = abs(currColor.val[0] - prevColor.val[0]);
			colorDiff.val[1] = abs(currColor.val[1] - prevColor.val[1]);
			colorDiff.val[2] = abs(currColor.val[2] - prevColor.val[2]);

			if (colorDiff.val[0] > 0 || colorDiff.val[1] > 0 || colorDiff.val[2] > 0)
			{

				// Rozmazani jednoho pixelu
				for (int r = 0; r < 3; r++)
				{
					for (int c = 0; c < 3; c++)
					{
						int y = row+r-1;
						int x = col+c-1;

						if (x < 0) x = 0;
						if (x > image->width-1) x = image->width-1;
						if (y < 0) y = 0;
						if (y > image->height-1) y = image->height-1;

						CvScalar color;
						color.val[0] = ((uchar *)(image->imageData + y*image->widthStep))[x*image->nChannels + 0]; // B
						color.val[1] = ((uchar *)(image->imageData + y*image->widthStep))[x*image->nChannels + 1]; // G
						color.val[2] = ((uchar *)(image->imageData + y*image->widthStep))[x*image->nChannels + 2]; // R

						sumR += gauss[r][c] * (float)color.val[2];
						sumG += gauss[r][c] * (float)color.val[1];
						sumB += gauss[r][c] * (float)color.val[0];
					}
				}
				
				// Vysledek zapisujeme do pomocneho souboru
				((uchar *)(tmpImage->imageData + row*tmpImage->widthStep))[col*tmpImage->nChannels + 0] = (int)sumB;
				((uchar *)(tmpImage->imageData + row*tmpImage->widthStep))[col*tmpImage->nChannels + 1] = (int)sumG;
				((uchar *)(tmpImage->imageData + row*tmpImage->widthStep))[col*tmpImage->nChannels + 2] = (int)sumR;

			}
		}
	}

	cvCopy(tmpImage, image, NULL);

	// Uvolneni pameti
	cvReleaseImage(&tmpImage);
}

IplImage *EffectPainterly::Painterly(IplImage *imgIn)
{
	IplImage *grayScale = cvCreateImage(cvGetSize(imgIn), imgIn->depth, 1);
	IplImage *imgOut = cvCreateImage(cvGetSize(imgIn), imgIn->depth, imgIn->nChannels);
	IplImage *imgBlured = cvCreateImage(cvGetSize(imgIn), imgIn->depth, imgIn->nChannels);
	IplImage *imgSobelX = cvCreateImage(cvGetSize(grayScale), IPL_DEPTH_16S, 1);
	IplImage *imgSobelY = cvCreateImage(cvGetSize(grayScale), IPL_DEPTH_16S, 1);

	// Prevendeni obrazu do stupne sedi
	cvCvtColor(imgIn, grayScale, CV_BGR2GRAY);

	// Pouziti sobelova filtru ve smeru x a y
	cvSobel(grayScale, imgSobelX, 1, 0);
	cvSobel(grayScale, imgSobelY, 0, 1);

	// Rozmazani obrazu
	cvSmooth(imgIn, imgBlured, CV_GAUSSIAN, 13, 13);

	// V pripade ze se nejedna o prvni snimek, budeme kreslit do predchoziho
	if (firstFrame)
		imgOut = cvCloneImage(imgBlured);
	else
		imgOut = cvCloneImage(prevFrame);

	int step = INIT_STEP;
	int radius = INIT_RADIUS;
	int lineThick = INIT_LINE_THICK;

	// Kreslime do obrazu v nekolika pruchodech (kazdem novem pruchodu je pouzit tensi stetec)
	for (int pass = 0; pass < PASS_CNT; pass++)
	{
		if (pass > 0)
		{
			// Zmensujeme stetec a krok se kterym prochazime obraz
			step -= STEP_DECR;
			if (step < 1) break;
			lineThick -= LINE_DECR;
			if (lineThick < 1) lineThick = 1;
			radius -= RADIUS_DECR;
			if (radius < 1) radius = 1;
		}

		// Pruchod celym obrazem
		for(int row = 0; row < imgIn->height; row += step)
		{
			for(int col = 0; col < imgIn->width; col += step)
			{
				bool draw = true;

				// Ziskani hodnoty pixelu v danem miste puvodniho rozmazaneho obrazu
				CvScalar colorCenter;
				colorCenter.val[0] = ((uchar *)(imgBlured->imageData + row*imgBlured->widthStep))[col*imgBlured->nChannels + 0]; // B
				colorCenter.val[1] = ((uchar *)(imgBlured->imageData + row*imgBlured->widthStep))[col*imgBlured->nChannels + 1]; // G
				colorCenter.val[2] = ((uchar *)(imgBlured->imageData + row*imgBlured->widthStep))[col*imgBlured->nChannels + 2]; // R

				if (pass > 0)	// Nejedna se o prvni pruchod
				{
					// Zjiteni barvy z predchoziho pruchodu
					CvScalar colorPrev;
					colorPrev.val[0] = ((uchar *)(imgOut->imageData + row*imgOut->widthStep))[col*imgOut->nChannels + 0]; // B
					colorPrev.val[1] = ((uchar *)(imgOut->imageData + row*imgOut->widthStep))[col*imgOut->nChannels + 1]; // G
					colorPrev.val[2] = ((uchar *)(imgOut->imageData + row*imgOut->widthStep))[col*imgOut->nChannels + 2]; // R

					// Vypocet rozdilu barev
					CvScalar colorDiff;
					colorDiff.val[0] = abs(colorPrev.val[0] - colorCenter.val[0]);
					colorDiff.val[1] = abs(colorPrev.val[1] - colorCenter.val[1]);
					colorDiff.val[2] = abs(colorPrev.val[2] - colorCenter.val[2]);

					// Pokud se obrazek vyrazne lisi od puvodniho rozmazaneho obrazu, budeme prekreslovat
					if (colorDiff.val[0] > DIFFER || colorDiff.val[1] > DIFFER || colorDiff.val[2] > DIFFER)
						draw = true;
					else
						draw = false;
				}
				else {	// Pri prvni pruchodu budeme kreslit vzdy
					draw = true;
				}

				//if (!firstFrame)
				//{
				//	// Barva predchoziho rozmazaneho snimku
				//	CvScalar colorBlured;
				//	colorBlured.val[0] = ((uchar *)(prevBlur->imageData + row*prevBlur->widthStep))[col*prevBlur->nChannels + 0]; // B
				//	colorBlured.val[1] = ((uchar *)(prevBlur->imageData + row*prevBlur->widthStep))[col*prevBlur->nChannels + 1]; // G
				//	colorBlured.val[2] = ((uchar *)(prevBlur->imageData + row*prevBlur->widthStep))[col*prevBlur->nChannels + 2]; // R

				//	// Vypocet rozdilu barev mezi soucasnym a predchozim rozmazanym obrazem
				//	CvScalar colorDiff;
				//	colorDiff.val[0] = abs(colorBlured.val[0] - colorCenter.val[0]);
				//	colorDiff.val[1] = abs(colorBlured.val[1] - colorCenter.val[1]);
				//	colorDiff.val[2] = abs(colorBlured.val[2] - colorCenter.val[2]);

				//	// Pokud se predchozi rozmazany obrazek a soucasny rozmazany obrazek vyrazne nelisi, nebudeme kreslit
				//	if (colorDiff.val[0] > DIFFER_BLURED || colorDiff.val[1] > DIFFER_BLURED || colorDiff.val[2] > DIFFER_BLURED)
				//		/*draw = true*/;
				//	else
				//		draw = false;
				//}

				if (draw)
				{
					CvPoint dstPoint = cvPoint(col,row);

					// Ziskani gradientu ve smeru x a y
					int sobelX = ((int *)(imgSobelX->imageData + row*imgSobelX->widthStep))[col];
					int sobelY = ((int *)(imgSobelY->imageData + row*imgSobelY->widthStep))[col];

					// Prevedeni hodnot gradientu na kolmici ke gradientu
					int gradientNormalX = sobelY;
					int gradientNormalY = -sobelX;

					// Smernice kolmice gradientu
					if (gradientNormalX == 0) gradientNormalX = 1;
					float k = (float)gradientNormalY / (float)gradientNormalX;

					// Vypocitani pozice druheho bodu cary
					int pointX;
					int pointY;
					if (abs(gradientNormalX) > abs(gradientNormalY))
					{
						pointX = gradientNormalX > 0 ? radius : -radius;
						pointY = (int)(k * (float)pointX);
					}
					else {
						pointY = gradientNormalY > 0 ? radius : -radius;
						if (k == 0) k = 0.01f;
						pointX = (int)((float)pointY / k);
					}
					dstPoint = cvPoint(pointX+col,pointY+row);

					// Barva bodu z druheho bodu
					CvScalar colorPoint;
					colorPoint.val[0] = ((uchar *)(imgBlured->imageData + dstPoint.y*imgBlured->widthStep))[dstPoint.x*imgBlured->nChannels + 0]; // B
					colorPoint.val[1] = ((uchar *)(imgBlured->imageData + dstPoint.y*imgBlured->widthStep))[dstPoint.x*imgBlured->nChannels + 1]; // G
					colorPoint.val[2] = ((uchar *)(imgBlured->imageData + dstPoint.y*imgBlured->widthStep))[dstPoint.x*imgBlured->nChannels + 2]; // R

					// Prumerna barva
					CvScalar colorAverage;
					colorAverage.val[0] = (colorCenter.val[0] + colorPoint.val[0])/2;
					colorAverage.val[1] = (colorCenter.val[1] + colorPoint.val[1])/2;
					colorAverage.val[2] = (colorCenter.val[2] + colorPoint.val[2])/2;

					// Vykresleni cary od daneho pixelu ve smeru kolmice ke gradientu
					cvLine(imgOut, cvPoint(col,row), dstPoint, colorAverage, lineThick);
				}
			}
		}

		// Rozmazani vysledku
		if (firstFrame) {
			cvSmooth(imgOut, imgOut, CV_GAUSSIAN, 3, 3);
		}
		else {
			RestrictedSmooth(imgOut, prevFrame);
		}
	}

	// Ulozeni soucasneho vysledneho snimku
	if (prevFrame != NULL)
	{
		cvReleaseImage(&prevFrame);
		prevFrame = NULL;
	}
	prevFrame = cvCloneImage(imgOut);

	// Ulozeni soucasneho rozmazaneho snimku
	if (prevBlur != NULL)
	{
		cvReleaseImage(&prevBlur);
		prevBlur = NULL;
	}
	prevBlur = cvCloneImage(imgBlured);

	if (firstFrame) firstFrame = false;

	// Uvolneni pameti
	cvReleaseImage(&grayScale);
	cvReleaseImage(&imgBlured);
	cvReleaseImage(&imgSobelX);
	cvReleaseImage(&imgSobelY);

	return imgOut;
}

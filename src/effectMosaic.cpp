#include "effectMosaic.h"


#define MOSAIC_SIZE 6


// Konstruktor
EffectMosaic::EffectMosaic()
{
}

// Destruktor
EffectMosaic::~EffectMosaic() 
{
}

IplImage *EffectMosaic::Mosaic(IplImage *imgIn)
{	
	// vystupni obraz
	IplImage *imgOut = NULL;
	imgOut = cvCreateImage(cvGetSize(imgIn), imgIn->depth, imgIn->nChannels);

	
	// pruchod obrazem
	for(int i=0; (i < imgIn->height); i+=MOSAIC_SIZE)
	{
		for(int j=0; (j < imgIn->width); j+=MOSAIC_SIZE)
		{			
			int r;
			int g;
			int b;

			AverageColor(imgIn, j, i, &r, &g, &b);
			SetColor(imgOut, j, i, r, g, b);
		}
	}


	//cvCopy( imgIn, imgOut, NULL );
	return imgOut;
};



/*
 Vypocet prumerne barvy v danem vyseku obrazu
 img - obrazek
 col - cislo sloupce
 row - cislo radku
 r,g,b - vysledna barva
*/
void EffectMosaic::AverageColor(IplImage *img, int col, int row, int *r, int *g, int *b)
{
	// vynulovani barev
	*r = 0;
	*g = 0;
	*b = 0;

	int counter = 0;

	// pruchod polem a pricitani hodnot barev
	for(int i=row; (i < img->height)&&(i < row+MOSAIC_SIZE); i++)
	{
		for(int j=col; (j < img->width)&&(j < col+MOSAIC_SIZE); j++)
		{
			*b += ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 0]; // B
			*g += ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 1]; // G
			*r += ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 2]; // R
			counter++;
		}
	}

	// vypocet prumeru
	*r /= counter;
	*g /= counter;
	*b /= counter;

	return;
}



/*
 Obarveni vsech pixelu v danem vyseku stejnou barvou
 img - obrazek
 col - cislo sloupce
 row - cislo radku
 r,g,b - vysledna barva
*/
void EffectMosaic::SetColor(IplImage *img, int col, int row, int r, int g, int b)
{
	// pruchod polem a pricitani hodnot barev
	for(int i=row; (i < img->height)&&(i < row+MOSAIC_SIZE); i++)
	{
		for(int j=col; (j < img->width)&&(j < col+MOSAIC_SIZE); j++)
		{
			((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 0] = b; // B
			((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 1] = g; // G
			((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 2] = r; // R
		}
	}

	return;
}

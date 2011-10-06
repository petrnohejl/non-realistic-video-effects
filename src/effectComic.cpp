#include "effectComic.h"


#define EDGES_DIFF 40
#define DILATATION_DIFF 1
#define CANNY_LOW 10
#define CANNY_HIGH 120
#define HIST_SIZE 256


// Konstruktor
EffectComic::EffectComic()
{
	colorfulPrev = 0;
	cluster_count = 20;
}


// Destruktor
EffectComic::~EffectComic() {
}


//Set element in Matrix   
void EffectComic::SetMatrix(CvMat *mat, int row, int col, int channel, int val)   
{   
    ((float*)(mat->data.ptr+mat->step*row))[col*mat->step+channel] = (float)val;   
}


// Dilatace - frame, diference, metoda (0 je ctverec else kriz)
void EffectComic::Dilatation(IplImage *out, int diff, int method)
{
  IplImage *in = cvCloneImage(out);

  // pruchod framebufferem
  for (int y = diff; y < (in->height)-diff; y++)
  {
    for (int x = diff; x < (in->width)-diff; x++)
    {
      // hodnota aktualniho pixelu
      uchar value = ((uchar *)(in->imageData + y*in->widthStep))[x];

      // pixel je zkoumanym objektem
      if(value==255)
      {
        // ctverec
        if(method==0)
        {
          for(int j=-diff;j<=diff;j++)
          for(int i=-diff;i<=diff;i++)
          ((uchar *)(out->imageData + (y+j)*out->widthStep))[x+i] = 255;
        }
        // kriz
        else
        {
          for(int i=-diff;i<=diff;i++)
          {
            ((uchar *)(out->imageData + (y)*out->widthStep))[x+i] = 255;
            ((uchar *)(out->imageData + (y+i)*out->widthStep))[x] = 255;
          }
        }
      }
    }
  }

  cvReleaseImage(&in);
}


IplImage *EffectComic::Comic(IplImage *imgIn)
{	
	// CANNY //////////////////////////////////////////////////////////////////////////////////

	// Vytvoreni vystupniho obrazu
	IplImage *imgCanny = NULL;
	imgCanny = cvCreateImage(cvGetSize(imgIn), imgIn->depth, 1);

	// Prevendeni obrazu do stupne sedi
	cvCvtColor(imgIn, imgCanny, CV_BGR2GRAY);
	
	// Cannyho hranovy detektor
	cvCanny(imgCanny, imgCanny, CANNY_LOW, CANNY_HIGH, 3);

	// dilatace
	Dilatation(imgCanny, DILATATION_DIFF, 1);



	// NASTAVENI POCTU SHLUKU /////////////////////////////////////////////////////////////////

	// Prevendeni obrazu do stupne sedi
	IplImage *grayScale = NULL;
	grayScale = cvCreateImage(cvGetSize(imgIn), imgIn->depth, 1);
	cvCvtColor(imgIn, grayScale, CV_BGR2GRAY);

	IplImage* images[] = { grayScale };

	// Velikost histogramu
	int bins = HIST_SIZE;
	int hsize[] = {bins};

	// Rozsahy
	float xranges[] = { 0, HIST_SIZE };
	float* ranges[] = { xranges };

	CvHistogram* hist = cvCreateHist(1, hsize, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist(images, hist, 0, NULL);

	int colorful = 0;
	// Pruchod histogramem
	for (int i = 0; i < bins; i++)
	{
		float bin_val = cvQueryHistValue_1D(hist, i);
		if (bin_val > 128) colorful++;
	}

	
	// Rozdil mezi barevnosti soucasneho a predchoziho snimku
	int colorfulDiff = abs(colorful - colorfulPrev);
	colorfulPrev = colorful;

	// Pocet klastru zmnenime jenom pri vyrazne zmene v obrazu nebo kdyz neni nastaven
	if (colorfulDiff > 5) cluster_count = colorful/5;

	cvReleaseImage(&grayScale);
	cvReleaseHist(&hist);


	// K-MEANS ////////////////////////////////////////////////////////////////////////////////
	
	// vystupni obraz
	IplImage *imgOut = NULL;
	imgOut = cvCreateImage(cvGetSize(imgIn), imgIn->depth, imgIn->nChannels);

	// velikost obrazu
	int size = imgIn->width * imgIn->height;

	// matice bodu ktera vstupuje do k-mean
	CvMat* meansPoints = cvCreateMat(size, 1, CV_32FC(5));
	CvMat* meansClusters = cvCreateMat(size, 1, CV_32SC1);
	CvScalar *palette = new CvScalar[cluster_count];

	// pruchod framebufferem, i je radek, j je sloupec
	int counter = 0;
	for(int i=0; i < imgIn->height; i++)
	{
		for(int j=0; j < imgIn->width; j++)
		{
			byte col_b = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 0]; // B
			byte col_g = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 1]; // G
			byte col_r = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 2]; // R

            SetMatrix(meansPoints,counter,0,0,col_b);   
            SetMatrix(meansPoints,counter,0,1,col_g);   
            SetMatrix(meansPoints,counter,0,2,col_r);
			SetMatrix(meansPoints,counter,0,3,i);   
            SetMatrix(meansPoints,counter,0,4,j);  

			counter++;
		}
	}
	
	// shlukovani
	cvKMeans2(meansPoints, cluster_count, meansClusters, cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0));

	// inicializace
	for(int i=0; i < cluster_count; i++)
	{
		palette[i].val[0] = 0;
		palette[i].val[1] = 0;
		palette[i].val[2] = 0;
		palette[i].val[3] = 0;
	}

	// vypocet palety barev
	counter = 0;
	for(int i=0; i < imgOut->height; i++)
	{
		for(int j=0; j < imgOut->width; j++)
		{
			int cluster_idx = meansClusters->data.i[counter];

			int b = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 0];   
            int g = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 1];   
            int r = ((uchar *)(imgIn->imageData + i*imgIn->widthStep))[j*imgIn->nChannels + 2];   
			
			palette[cluster_idx].val[0] += r;
			palette[cluster_idx].val[1] += g;
			palette[cluster_idx].val[2] += b;
			palette[cluster_idx].val[3] += 1; // kolik pixelu je v danym clusteru

			counter++;
		}
	}

	for(int i=0; i < cluster_count; i++)
	{
		palette[i].val[0] = palette[i].val[0] / palette[i].val[3];
		palette[i].val[1] = palette[i].val[1] / palette[i].val[3];
		palette[i].val[2] = palette[i].val[2] / palette[i].val[3];
	}

	counter = 0;
	for(int i=0; i < imgOut->height; i++)
	{
		for(int j=0; j < imgOut->width; j++)
		{
			// cislo clusteru kam pixel patri
			int cluster_idx = meansClusters->data.i[counter];

			// zvyrazneni hran
			uchar cannyVal = ((uchar *)(imgCanny->imageData + i*imgCanny->widthStep))[j*imgCanny->nChannels];

			uchar diff;
			if(cannyVal == 255) diff = EDGES_DIFF;
			else diff = 0;

			// vysledna barva
			uchar r;
			uchar g;
			uchar b;

			if(palette[cluster_idx].val[0] - diff < 0) r = 0;
			else r = (uchar)palette[cluster_idx].val[0] - diff;

			if(palette[cluster_idx].val[1] - diff < 0) g = 0;
			else g = (uchar)palette[cluster_idx].val[1] - diff;

			if(palette[cluster_idx].val[2] - diff < 0) b = 0;
			else b = (uchar)palette[cluster_idx].val[2] - diff;

			((uchar *)(imgOut->imageData + i*imgOut->widthStep))[j*imgOut->nChannels + 0] = b; // B
			((uchar *)(imgOut->imageData + i*imgOut->widthStep))[j*imgOut->nChannels + 1] = g; // G
			((uchar *)(imgOut->imageData + i*imgOut->widthStep))[j*imgOut->nChannels + 2] = r; // R

			counter++;
		}
	}

	// uvolneni pameti
	cvReleaseImage(&imgCanny);
	cvReleaseMat(&meansPoints);
	cvReleaseMat(&meansClusters);
	delete palette;

	return imgOut;
};

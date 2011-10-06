#include "effects.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 
int main(int argc, char** argv)
{
	// zpracovani argumentu


	// napoveda
	if (argc == 2 && 
		(strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0)
	)
	{
		printf(
			"Nonrealistic Video Effects\n\n(c)2010 Petr Nohejl, Petr Panacek, Brno University of Technology, FIT\n\nApplication demonstrate 3 nonrealistic video effects: comic, painterly, mosaic.\nProgram is implemented in C/C++, using OpenCV library. Developed in Visual Studio 2008.\n\nUsage: VideoEffects.exe -h               - show help\n       VideoEffects.exe -c inputVideo    - demonstrate comic effect\n       VideoEffects.exe -p inputVideo    - demonstrate painterly effect\n       VideoEffects.exe -m inputVideo    - demonstrate mosaic effect\n\n       inputVideo is name of video file in AVI format\n"
		);
	}


	// efekty
	else if (argc == 3 &&
		((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"-p") == 0) || (strcmp(argv[1],"-m") == 0))
	)
	{
		//char *inputFile = NULL;
		//inputFile = "input.avi";

		Effects::EffectType type;
		if(strcmp(argv[1],"-c") == 0) type = Effects::COMIC;
		else if(strcmp(argv[1],"-p") == 0) type = Effects::PAINTERLY;
		else if(strcmp(argv[1],"-m") == 0) type = Effects::MOSAIC;

		Effects *effects = NULL;
		effects = new Effects();
		effects->run(argv[2], type);
	}


	// chybne argumenty
	else
	{
		fprintf(stderr, "Error: Incorrect arguments!\nTo show help, run program with parameter -h.\n");
	}


	return 0;
}

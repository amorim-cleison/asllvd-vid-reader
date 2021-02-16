#include "VidHeader.h"
#include <cstring>

// Arguments:
// - Path to the input video (.vid) file
// - Path to the output image (.ppm)
// - Frame
int main(int argc, char *argv[])
{
	VidHeader *vid = new VidHeader(argv[1], NULL);
	char *output_path = argv[2];
	int frame = atoi(argv[3]);

	// Validate if video was read successfully:
	if (strlen(vid->errMessage) > 0)
	{
		printf("ERROR: '%s'", vid->errMessage);
		return 901;
	}

	vid->readFrameFromVideoFile(frame);
	vid->writePPMimage(output_path, vid->pixmap);

	delete vid;
	return 0;
}

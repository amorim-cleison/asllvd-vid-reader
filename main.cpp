#include "VidHeader.h"
#include <cstdlib>
#include <cstring>

// Arguments:
// - Path to the input video (.vid) file
// - Output directory (where the images will be saved)
// - Prefix to be added to file names
// - Start frame
// - End frame
// - FPS in (actual frame rate for the video)
// - FPS out (number of frames/images to extract per second for the video)
int main(int argc, char *argv[])
{
	VidHeader *vid = new VidHeader(argv[1], NULL);
	char *output_dir = argv[2];
	char *prefix = argv[3];
	int frame_start = atoi(argv[4]);
	int frame_end = atoi(argv[5]);
	int fps_in = atoi(argv[6]);
	int fps_out = atoi(argv[7]);

	const char *output_path = "%s/%s_%05d.ppm";
	char outfile[1000];
	int frame_number = (frame_start - 1);
	int step = fps_in / fps_out;

	if (frame_end > vid->num_frames)
	{
		frame_end = vid->num_frames;
	}

	while (frame_number < frame_end)
	{
		vid->readFrameFromVideoFile(frame_number);
		sprintf(outfile, output_path, output_dir, prefix, (frame_number + 1));
		vid->writePPMimage(outfile, vid->pixmap);
		frame_number += step;
	}
	delete vid;
	return 0;
}

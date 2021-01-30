#ifndef VID_HEADER_H
#define VID_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include "showErrMsgBox.h"
#include <sys/types.h>
#include <sys/stat.h>

#if defined MSWIN
#define off_t __int64
#endif

#define ERR_BUFFER_LEN 10000
#define MAX_PATH 5000

class VidHeader{
public:

	char videoFileName[MAX_PATH], errMessage[ERR_BUFFER_LEN];
	FILE * videoFile;
	int file_open_read_or_write, vid_is_bayer_interpolated, vid_uses_two_bytes_per_pixel;
	int width, height, num_frames, current_frame, pixmap_size, start_frame_number, end_frame_number;
	double set_brightness;
	struct stat st;
	off_t * offsets;
	unsigned long compressed_buffer_size, uncompressed_buffer_size;
	unsigned char *compressed_buffer, *uncompressed_buffer, *pixmap;
	FILE * error_outfile;

	VidHeader();
	VidHeader( char * videoFileName, FILE * error_outfile );
	bool readNextFrameFromVideoFile();
	bool readFrameFromVideoFile(int frame_number);
	
	VidHeader( char * outVideoFileName, FILE * error_outfile, int out_width, int out_height, int out_num_frames );
	bool writeNextFrameToVideoFile();
	bool writeFrameOffsetsToFile();
	
	~VidHeader();

	void writePPMimage(char * fileName, unsigned char * write_pixmap);
	void writePGMimage(char * imgFileName, unsigned char * write_pixmap);
};

#endif

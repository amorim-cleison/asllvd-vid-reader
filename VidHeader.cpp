
//#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read-frame.h"
#include "VidHeader.h"
#include "bayer.h"

#if defined MSWIN
#define snprintf sprintf_s
#define fseeko _fseeki64
#define ftello _ftelli64
#define ZLIB_WINAPI
#undef	MAX_PATH
#endif

#include "zlib.h"
 
typedef unsigned char uchar; 

VidHeader :: VidHeader(){
	width = 0; height = 0; num_frames = 0;
	pixmap = NULL; compressed_buffer = NULL; uncompressed_buffer = NULL;
	videoFile = NULL;
	vid_uses_two_bytes_per_pixel = 1;
	vid_is_bayer_interpolated = 1;
}

VidHeader :: VidHeader( char * inVideoFileName, FILE * in_error_outfile  ){

	width = 0; height = 0; num_frames = 0;
	pixmap = NULL; compressed_buffer = NULL; uncompressed_buffer = NULL;
	videoFile = NULL;
	vid_is_bayer_interpolated = 1;
	set_brightness = 0;

	this -> error_outfile = in_error_outfile;
	strncpy( videoFileName, inVideoFileName, sizeof(videoFileName) );

	file_open_read_or_write = 0;
	videoFile = fopen( videoFileName, "rb" );
	if( videoFile == NULL )
	{
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Unable to open following video file in  'rb' mode \r\n %s \r\n", __FILE__, __LINE__, videoFileName);
		displayMessageBox(errMessage, error_outfile); 
		return;
	}

	uchar header[ 8 ];

	if( fread( header, 1, 8, videoFile ) < 8 ) {
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Unable to read from video file \n %s \n", __FILE__, __LINE__, videoFileName);
		displayMessageBox(errMessage, error_outfile); 
		return;
	}

	width = header[ 1 ];
	width = ( width << 8 ) | header[ 0 ];

	height = header[ 3 ];
	height = ( height << 8 ) | header[ 2 ];

	num_frames = header[ 7 ];
	num_frames = ( num_frames << 8 ) | header[ 6 ];
	num_frames = ( num_frames << 8 ) | header[ 5 ];
	num_frames = ( num_frames << 8 ) | header[ 4 ];

	if( width > 0x8FFF ){
		vid_uses_two_bytes_per_pixel = 0;
		width = width - 0x8FFF;
	} else
		vid_uses_two_bytes_per_pixel = 1;

	pixmap_size = 3 * width * height;
	pixmap = new uchar[pixmap_size];
	for(int i = 0; i < pixmap_size; i++) pixmap[i] = 0;

	if( num_frames < 0 ) {
		num_frames = -num_frames;
		vid_is_bayer_interpolated = 0;
		uncompressed_buffer_size = pixmap_size;
		uncompressed_buffer = pixmap;
	} else {
		vid_is_bayer_interpolated = 1;
		uncompressed_buffer_size = (width * height) << vid_uses_two_bytes_per_pixel;
		uncompressed_buffer = new uchar[uncompressed_buffer_size];
	}

	current_frame = 0;
	start_frame_number = 0;
	end_frame_number = num_frames;

	offsets = new off_t[num_frames];
	off_t offset;
	for( int i = 0; i < num_frames; i++ )
	{
		fseeko( videoFile, ( i + 1 ) << 3, SEEK_SET );

		if( fread( header, 1, 8, videoFile ) < 8 ) {
			snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Error reading offsets to video file \n %s \n", __FILE__, __LINE__, videoFileName);
			displayMessageBox(errMessage, error_outfile); 
			return;
		}

		int j = 7;
		offset = 0;
		while( j >= 0 )
			offset = (offset << 8) | header[ j-- ];	

		offsets[i] = offset;
	}

	compressed_buffer_size = 2 * uncompressed_buffer_size; //+ ( uncompressed_buffer_size >> 10 ) + 12;
	compressed_buffer = new uchar[compressed_buffer_size];
}

VidHeader :: VidHeader( char * outVideoFileName, FILE * in_error_outfile,
						int out_width, int out_height, int out_num_frames ){

	width = out_width; height = out_height; num_frames = out_num_frames;
	pixmap = NULL; compressed_buffer = NULL; uncompressed_buffer = NULL;
	videoFile = NULL;

	this -> error_outfile = in_error_outfile;
	strncpy( videoFileName, outVideoFileName, sizeof(videoFileName) );

	file_open_read_or_write = 1;
	videoFile = fopen( videoFileName, "wb" );
	if( videoFile == NULL ) {
		snprintf( errMessage, ERR_BUFFER_LEN, "%s:%d Unable to open following video file in  'wb' mode \r\n %s \r\n",
				  __FILE__, __LINE__, videoFileName);
		displayMessageBox(errMessage, error_outfile); 
		return;
	}

	current_frame = 0;
	start_frame_number = 0;
	end_frame_number = num_frames;

	vid_is_bayer_interpolated = 0;
	pixmap_size = 3 * width * height;
	uncompressed_buffer_size = pixmap_size;
	compressed_buffer_size = 2*uncompressed_buffer_size; //+ ( uncompressed_buffer_size >> 10 ) + 12;

	pixmap = new uchar[pixmap_size];
	compressed_buffer = new uchar[compressed_buffer_size];
	uncompressed_buffer = pixmap;

	offsets = new off_t[num_frames];
	offsets[0] = (num_frames + 1) << 3;
	for( int i = 1; i < num_frames; i++ ) offsets[i] = offsets[0];
}

VidHeader::~VidHeader() {
	if( file_open_read_or_write ) writeFrameOffsetsToFile();

	fclose(videoFile);
	delete [] offsets;
	delete [] pixmap;
	delete [] compressed_buffer;
	if( vid_is_bayer_interpolated )
		delete [] uncompressed_buffer;
}

bool VidHeader:: writeFrameOffsetsToFile(){
	off_t offset;
	uchar header[8];

	int tnum_frames = -current_frame;
    header[ 0 ] = width & 0xFF;
    header[ 1 ] = width >> 8;
    header[ 2 ] = height & 0xFF;
    header[ 3 ] = height >> 8;
    header[ 4 ] = tnum_frames & 0xFF;
    header[ 5 ] = ( tnum_frames >> 8 ) & 0xFF;
    header[ 6 ] = ( tnum_frames >> 16 ) & 0xFF;
    header[ 7 ] = ( tnum_frames >> 24 ) & 0xFF;

	fseeko( videoFile, 0, SEEK_SET );
	if( fwrite( header, 1, 8, videoFile ) < 8 ) {
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Unable to write header to video file \n %s \n", __FILE__, __LINE__, videoFileName);
		displayMessageBox(errMessage, error_outfile); 
		return false;
	}

	for( int i = 0; i < num_frames; i++ ) {
		offset = offsets[i];
		for( int j = 0; j < 8; j++ )
			header[j] = (uchar)((offset >> 8*j) & 0xFF);

		fseeko( videoFile, (i + 1) << 3, SEEK_SET );

		if( fwrite( header, 1, 8, videoFile ) < 8 ) {
			snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Error writing offsets to video file \n %s \n", __FILE__, __LINE__, videoFileName);
			displayMessageBox(errMessage, error_outfile); 
			return false;
		}
	}
	return true;
}

bool VidHeader:: readNextFrameFromVideoFile()
{
	bool ret_val = readFrameFromVideoFile(current_frame);
	if ( ret_val )
		current_frame++;
		
	return ret_val;
}

bool VidHeader:: readFrameFromVideoFile(int frame_number)
{
	if ( frame_number < 0 || frame_number >= num_frames ) {
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d asked frameNumber %d > num_frames %d\n", __FILE__, __LINE__, frame_number, num_frames);
		displayMessageBox(errMessage, error_outfile);
		return false;
	}

	if( fseeko( videoFile, offsets[frame_number], SEEK_SET ) < 0 ) {
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d cannot seek to offset for frame_number %d in video_file \n %s\n",
				__FILE__, __LINE__, frame_number, videoFileName);
		displayMessageBox(errMessage, error_outfile); return false;
	}

	int compressed_size;
	if( frame_number <  num_frames - 1 )
		compressed_size = (int) (offsets[frame_number+1] - offsets[frame_number]);
	else {
		fseeko( videoFile, 0, SEEK_END );
		off_t end_offset = ftello( videoFile );
		compressed_size = (int) (end_offset - offsets[frame_number]);
		fseeko( videoFile, offsets[frame_number], SEEK_SET );
	}

	memset( pixmap, 0, pixmap_size );
	if ( compressed_size == 0 ) 
		return true;

	if( fread( compressed_buffer, compressed_size, 1, videoFile ) !=  1 ) {
		snprintf( errMessage, ERR_BUFFER_LEN, "%s:%d cannot read frame_number %d in video_file \n %s\n", __FILE__, __LINE__,
					frame_number, videoFileName );
		displayMessageBox(errMessage, error_outfile); return false;
	}

	unsigned long uncompressed_size = uncompressed_buffer_size;
	//uncompress((Bytef *)uncompressed_buffer, (uLongf *)&uncompressed_size, (Bytef *)compressed_buffer, (uLong)compressed_size);
	uncompress(uncompressed_buffer, &uncompressed_size, compressed_buffer, compressed_size);

	if ( uncompressed_size !=  uncompressed_buffer_size ) {
		snprintf( errMessage, ERR_BUFFER_LEN, "%s:%d error uncompressing frame_number %d in video: \n",
					__FILE__, __LINE__, frame_number);
		displayMessageBox(errMessage, error_outfile); return false;
	}

	if( vid_is_bayer_interpolated ){
		if( !vid_uses_two_bytes_per_pixel )
			gp_bayer_decode( uncompressed_buffer, width, height, pixmap, BAYER_TILE_GBRG );
		else
			bayer_interpolate_frame( uncompressed_buffer, pixmap, width, height );
		/*
		if( vid_uses_two_bytes_per_pixel ){
			for(int pix=0, opix1=0, opix2=1; pix < width * height; pix++, opix1+=2, opix2+=2 )
				uncompressed_buffer[pix] = ( uncompressed_buffer[opix1] | ( uncompressed_buffer[opix2] << 8 ) ) >> 2;
		}
		writePGMimage("test_before_bayer.pgm", uncompressed_buffer);
		*/
	}
	//writePPMimage("test_after_bayer.ppm", pixmap);

	return true;
}

bool VidHeader:: writeNextFrameToVideoFile() {

	if( current_frame >= num_frames ) {
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d asked frameNumber %d > num_frames %d\n", __FILE__, __LINE__, current_frame, num_frames);
		displayMessageBox(errMessage, error_outfile);
		return false;
	}

	memset( compressed_buffer, 0, compressed_buffer_size );
	unsigned long compressed_size = compressed_buffer_size;
	compress(compressed_buffer, &compressed_size, uncompressed_buffer, pixmap_size);

	if( fseeko( videoFile, offsets[current_frame], SEEK_SET ) < 0 ) {
		snprintf( errMessage, ERR_BUFFER_LEN, "%s:%d cannot seek to offset for current_frame %d in video_file \n %s\n", __FILE__, __LINE__,
					current_frame, videoFileName );
		displayMessageBox(errMessage, error_outfile); 
		return false;
	}

	if( fwrite( compressed_buffer, compressed_size, 1, videoFile ) !=  1 ) {
		snprintf( errMessage, ERR_BUFFER_LEN, "%s:%d cannot write current_frame %d in video_file \n %s\n", __FILE__, __LINE__,
				  current_frame, videoFileName );
		displayMessageBox(errMessage, error_outfile);
		return false;
	}

	if( current_frame <  num_frames - 1 )
		offsets[current_frame+1] = compressed_size + offsets[current_frame];

	current_frame++;
	return true;
}

void VidHeader:: writePGMimage(char * imgFileName, unsigned char * write_pixmap)
{
	FILE * imgFile;
	imgFile = fopen( imgFileName, "wb" );
	if( imgFile == NULL )
	{
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Unable to open file to write image \r\n %s", __FILE__, __LINE__, imgFileName);
		displayMessageBox(errMessage, error_outfile); return;
	}
	fprintf(imgFile, "P5\n%d %d\n255\n", width, height);
	fwrite(write_pixmap, width * height, 1, imgFile);
	fclose(imgFile);
}

void VidHeader:: writePPMimage(char * imgFileName, unsigned char * write_pixmap)
{
	FILE * imgFile;
	imgFile = fopen( imgFileName, "wb" );
	if( imgFile == NULL )
	{
		snprintf(errMessage, ERR_BUFFER_LEN, "%s:%d Unable to open file to write image \r\n %s", __FILE__, __LINE__, imgFileName);
		displayMessageBox(errMessage, error_outfile); return;
	}
	fprintf(imgFile, "P6\n%d %d\n255\n", width, height);
	fwrite(write_pixmap, width * height * 3, 1, imgFile);
	fclose(imgFile);
}

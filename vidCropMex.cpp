#include <mex.h>
#include <string.h>
#include "VidHeader.h"

typedef unsigned char uchar; 

static void print_usage(const char * error_str){
	mexPrintf("%s\n", error_str);
	mexPrintf("Usage: vidCropMex, \n"
	"Inputs: \n"
	"   < input video file name : string >\n"
	"   < output video file name : string >\n"
	"	< [2x1] crop start and end frames : double >\n"
	"	[ [4x1] bbox crop [l t w h] : double ] \n"
	" OR (alternate version) \n"
	"   < {K x 1} cell array of < input video file names : string >\n"
	"   < output video file name : string >\n"
	"	< [2 x K] crop start and end frames for each video: double >\n"
	"	[ [4 x K] bbox crop for all video [l t w h] : double ] \n"
	"Outputs: \n"
	"   None.\n"
	);
	mexPrintf("\n"); mexErrMsgTxt(error_str);
}

#define my_max(a, b) (((a) > (b))? (a) : (b))
#define my_min(a, b) (((a) < (b))? (a) : (b))

void set_bounds(double  * ptr, int & l, int & t, int & w, int & h){
	l = (int)ptr[0];
	t = (int)ptr[1];
	w = (int)ptr[2];
	h = (int)ptr[3];
	l = my_max(l, 0);
	t = my_max(t, 0);
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] ){
	
	if( nrhs < 3 || mxGetClassID(prhs[1]) != mxCHAR_CLASS || mxGetClassID(prhs[2]) != mxDOUBLE_CLASS )
		print_usage("Incorrect number inputs / input datatype");
	
	int num_input_videos = 0;
	int buflen = 2000;
	char * in_fname = new char[buflen];
	VidHeader ** in_vid_header = NULL;
	double * input_video_crop_frames = new double[mxGetNumberOfElements(prhs[2])];
	memcpy( input_video_crop_frames, mxGetPr(prhs[2]), mxGetNumberOfElements(prhs[2]) * sizeof(double) );
	
	if ( mxGetClassID(prhs[0]) == mxCELL_CLASS ){
		
		num_input_videos = mxGetNumberOfElements(prhs[0]);
		if( num_input_videos != mxGetN(prhs[2]) || mxGetM(prhs[2]) != 2 )
			print_usage("Crop frames input array should be size [Number input videos should x 2].");
		
		in_vid_header = new VidHeader*[num_input_videos];
		for( int vidi = 0; vidi < num_input_videos; vidi++ ){
			if( mxGetClassID(mxGetCell(prhs[0],vidi)) != mxCHAR_CLASS )
				print_usage("One of the cell entries for filenames array is not a string.");
			
			mxGetString(mxGetCell(prhs[0],vidi), in_fname, buflen);
			in_vid_header[vidi] = new VidHeader(in_fname, stderr);
		}
		
	} else {
		if( mxGetClassID(prhs[0]) != mxCHAR_CLASS || mxGetNumberOfElements(prhs[2]) != 2 )
			print_usage("Input filename is not a string or crop frames array is not length 2.");
		
		num_input_videos = 1;
		in_vid_header = new VidHeader*[num_input_videos];
		mxGetString(prhs[0], in_fname, buflen);
		in_vid_header[0] = new VidHeader(in_fname, stderr);
	}
	
	int max_num_input_frames = 0;
	for( int vidi = 0; vidi < num_input_videos; vidi++ ){
		int start = (int)input_video_crop_frames[2*vidi];
		int end = (int)input_video_crop_frames[2*vidi+1];
		start = ( start < 0 )? 0 : start;
		end = ( end < in_vid_header[vidi] -> num_frames )? end : in_vid_header[vidi] -> num_frames;
		input_video_crop_frames[2*vidi] = start;
		input_video_crop_frames[2*vidi + 1] = end;
		max_num_input_frames = my_max(max_num_input_frames, end - start);
	}
	
	int out_w, out_h;
	double * crop_bboxes = NULL;
	if( nrhs >= 4 ){
		if( mxGetClassID(prhs[3]) != mxDOUBLE_CLASS || mxGetM(prhs[3]) != 4 || mxGetN(prhs[3]) != num_input_videos )
			print_usage("Incorrect crop bbox array size or datatype");

		crop_bboxes = mxGetPr(prhs[3]);
		out_w = 0; out_h = 0;
		for( int vidi = 0; vidi < num_input_videos; vidi++ ){
			int l, t, w, h; 
			set_bounds(crop_bboxes + 4*vidi, l, t, w, h);
			out_w = my_max(w, out_w);
			out_h = my_max(h, out_h);
		}
	} else {
		out_w = in_vid_header[0] -> width;
		out_h = in_vid_header[0] -> height;
	}
	int gap_between_videos = 12;
	
	char * out_fname = new char[buflen];
	mxGetString(prhs[1], out_fname, buflen);
	VidHeader * out_vid_header = NULL;
	int out_ww = (out_w + gap_between_videos)*num_input_videos - gap_between_videos;
	out_ww = (out_ww / 8 + 1)*8;
	int out_hh = (out_h / 8 + 1)*8;
	out_vid_header = new VidHeader(out_fname, stderr, out_ww, out_hh, max_num_input_frames);
	memset(out_vid_header->pixmap, 128, out_vid_header->pixmap_size);
	
	for( int framei = 0; framei < max_num_input_frames; framei++ ){
		for( int vidi = 0; vidi < num_input_videos; vidi++ ){
			int start = (int)input_video_crop_frames[2*vidi];
			int end = (int)input_video_crop_frames[2*vidi+1];
			if( framei >= end - start ) continue;
			
			in_vid_header[vidi] -> readFrameFromVideoFile(framei + start);
			
			int l, t, w, h; 
			if( crop_bboxes != NULL )
				set_bounds(crop_bboxes + 4*vidi, l, t, w, h);
			else{
				l = 0, t = 0, w = in_vid_header[vidi] -> width, h = in_vid_header[vidi] -> height;
			}
			
			for( int y = t; y < my_min(t + h, in_vid_header[vidi] -> height); y++ ){
				int in = 3 * ( y*in_vid_header[vidi] -> width + l );
				int out = 3 * ( (y-t) * out_vid_header -> width + (out_w + gap_between_videos)*vidi );
				for( int x = 3*l; x < 3*my_min(l + w, in_vid_header[vidi] -> width); x++ )
					out_vid_header -> pixmap[out++] = in_vid_header[vidi] -> pixmap[in++];	
			}
		}
		out_vid_header -> writeNextFrameToVideoFile();
	}
	
	for( int vidi = 0; vidi < num_input_videos; vidi++ )
		delete in_vid_header[vidi];
	delete [] in_vid_header;
	delete out_vid_header;
	
	delete [] in_fname;
	delete [] out_fname;
	delete [] input_video_crop_frames;
}

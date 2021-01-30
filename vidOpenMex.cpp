#include <mex.h>
#include <string.h>
#include "VidHeader.h"

static void print_usage(const char * error_str){
	mexPrintf("%s\n", error_str);
	mexPrintf("Usage: vidOpenMex, \n"
	"Inputs (Open for reading): \n"
	"   < input video file name : string >\n"
	"Inputs (Open for writing): \n"
	"   < output video file name : string >\n"
	"   < [3x1] -> [output video frame width, height, num video frames] : int32 >\n"
	"Outputs: \n"
	"   < handle to video read/write datastructure >\n"
	"(Additional outputs when open for reading)	\n"
	"   < [3x1] -> [input video frame width, height, num video frames] : double >\n\n"
	);
	mexPrintf("\n"); mexErrMsgTxt(error_str);
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] ){
	VidHeader * vid_header;
	char * fname, *buffer;
	int buflen;

	if( nrhs > 1 ){
		if( nrhs != 2 )
			print_usage("Incorrect number of inputs specified for opening video file in 'write' mode.");
		else if( nlhs != 1 )
			print_usage("Incorrect number of outputs specified for opening video file in 'write' mode.");
	} else {
		if( nrhs > 1 ) 
			print_usage("Incorrect number of inputs specified for opening video file in 'read' mode.");
		else if( nlhs != 2 )
			print_usage("Incorrect number of outputs specified for opening video file in 'read' mode.");
	}
	if( mxGetClassID(prhs[0]) != mxCHAR_CLASS )
		print_usage("First input should be a string.");

	buflen = mxGetN(prhs[0])*sizeof(mxChar)+1;
	fname = new char[buflen+10];
	mxGetString(prhs[0], fname, buflen);

	int video_read_write_mode = 0;
	if( nrhs > 1 ){
		video_read_write_mode = 1;
		if( mxGetClassID(prhs[1]) != mxINT32_CLASS )
			print_usage("Input argument should be uint32 class.");
		int * output_video_params = (int*) mxGetData(prhs[1]);
		vid_header = new VidHeader(fname, stderr, output_video_params[0], output_video_params[1], output_video_params[2]);
	} else
		vid_header = new VidHeader(fname, stderr);

	delete fname;
	if( vid_header -> compressed_buffer == NULL )
		mexErrMsgTxt("Error opening video file.\n");

	buflen = sizeof(VidHeader);
	plhs[0] = mxCreateDoubleMatrix(buflen/sizeof(double)+1, 1, mxREAL);
	buffer = (char*)mxGetPr(plhs[0]);
	memcpy(buffer, vid_header, buflen);

	if( video_read_write_mode == 0 ){
		plhs[1] = mxCreateDoubleMatrix(3, 1, mxREAL);
		double * out = mxGetPr(plhs[1]);
		out[0] = vid_header -> width;
		out[1] = vid_header -> height;
		out[2] = vid_header -> num_frames;
	}
}

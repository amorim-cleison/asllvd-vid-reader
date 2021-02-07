#include <mex.h>
#include "VidHeader.h"
#include <string.h>

static void print_usage(const char * error_str){
	mexPrintf("%s\n", error_str);
	mexPrintf("Usage: vidReadMex, \n"
	"Inputs: \n"
	"   < handle to video read datastructure >\n"
	"   < video frame number range [0, num_frames-1] : double >\n"
	"Outputs: \n"
	"   < [MxNx3] image frame from video : double >\n"
	);
	mexPrintf("\n"); mexErrMsgTxt(error_str);
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]){

	if( nrhs != 2 || nlhs != 1 )
		print_usage("Incorrect number of inputs or outputs specified.");

	if( mxGetClassID(prhs[0]) != mxDOUBLE_CLASS || mxGetNumberOfElements(prhs[0]) == 0 ) {
		mexErrMsgTxt("Invalid Vid handle\n");
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
		return;
	}
	VidHeader * vid_header = (VidHeader*)mxGetPr(prhs[0]);

	int frame_no = (int)(mxGetScalar(prhs[1]) + 0.5);
	if( frame_no > vid_header -> num_frames || frame_no < 0){
		mexPrintf("vidReadMex: input frame number should be in [0, #frames] \n", frame_no, vid_header -> num_frames );
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
		return;
	}
	
	mwSize out_frame_dims[3];
	out_frame_dims[0] = vid_header -> height;
	out_frame_dims[1] = vid_header -> width;
	out_frame_dims[2] = 3;
	plhs[0] = mxCreateNumericArray(3, out_frame_dims, mxDOUBLE_CLASS, mxREAL);
	double * buffer = mxGetPr(plhs[0]);

	vid_header -> readFrameFromVideoFile(frame_no);

	int i,j,k,l,x,y;
	int width = vid_header -> width;
	int height = vid_header -> height;
	l = 0;
	for(j=0; j<3; j++){
		for(x=0; x < width; x++){
			k = 3*x + j;
			for(y=0; y < height; y++){
				buffer[l++] = vid_header -> pixmap[k];
				k += 3*width;
			}
		}
	}
}

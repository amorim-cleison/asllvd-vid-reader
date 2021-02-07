#include <mex.h>
#include "VidHeader.h"
#include <string.h>

typedef unsigned char uchar; 

static void print_usage(const char * error_str){
	mexPrintf("%s\n", error_str);
	mexPrintf("Usage: vidWriteMex, \n"
	"Inputs: \n"
	"   < handle to video write datastructure >\n"
	"   < [MxNx3] image frame to add to video : uint8 >\n"
	"Outputs: \n"
	"   None.\n"
	);
	mexPrintf("\n"); mexErrMsgTxt(error_str);
}

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]){

	if( nrhs != 2 || nlhs != 0 )
		print_usage("Incorrect number of inputs or outputs specified.");

	if( mxGetClassID(prhs[0]) != mxDOUBLE_CLASS || mxGetNumberOfElements(prhs[0]) == 0 ) {
		mexErrMsgTxt("Invalid Vid handle\n");
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
		return;
	}
	VidHeader * vid_header = (VidHeader*)mxGetPr(prhs[0]);

	if( mxGetClassID(prhs[1]) != mxUINT8_CLASS )
		print_usage("Input image frame should be uint8 class.");
		
	const mwSize * input_frame_dims = mxGetDimensions(prhs[1]);
	if( mxGetNumberOfDimensions(prhs[1]) != 3 )
		//|| input_frame_dims[0] != vid_header -> height || input_frame_dims[1] != vid_header -> width )
		print_usage("Incorrect dimensions for input image frame.\n");
		
	uchar * buffer = (uchar *)mxGetData(prhs[1]);

	#define my_min(a,b) ((a < b)?(a):(b))

	int i,j,k,l,x,y;
	int out_width = vid_header -> width;
	int out_height = vid_header -> height;
	int in_width = input_frame_dims[1];
	int in_height = input_frame_dims[0];
	for(j=0; j<3; j++){
		for(x=0; x < my_min(in_width, out_width); x++){
			k = 3*x + j;
			l = in_width*in_height*j + x*in_height;
			for(y=0; y < my_min(in_height, out_height); y++){
				vid_header -> pixmap[k] = buffer[l++];
				k += 3*out_width;
			}
		}
	}

	vid_header -> writeNextFrameToVideoFile();
}

#include <mex.h>
#include "VidHeader.h"
#include <string.h>

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]){
	VidHeader * vid_header;
	if( mxGetClassID(prhs[0]) != mxDOUBLE_CLASS || mxGetNumberOfElements(prhs[0]) == 0 ) {
		mexErrMsgTxt("Invalid dxVid handle\n");
		return;
	}
	vid_header = (VidHeader*)new char[sizeof(VidHeader)];
	memcpy(vid_header, mxGetPr(prhs[0]), sizeof(VidHeader));
	delete vid_header;
}

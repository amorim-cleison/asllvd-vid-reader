#include "showErrMsgBox.h"
//#include "stdafx.h"
//#include "strsafe.h"
#include <stdio.h>
#ifdef MEX_COMPILE
	#include <mex.h>
#endif

void displayMessageBox(char * string, FILE * outfile)
{
#ifndef MEX_COMPILE 
	if(outfile != NULL)
		fprintf(outfile,"\n\n Vid_Reader error\n%s\n", string);
	else
		displayMessageBox(string);
#else
	mexErrMsgTxt(string);
#endif
}

void displayMessageBox(char * string)
{
	const int msgSize = 10000;

	fprintf(stderr, "%s\n", string);

	/*
	TCHAR szMsg[msgSize];

	(void)StringCchPrintf(szMsg, msgSize, TEXT("\r\n\r\n\t[%s]\n\n"), (LPCTSTR)string);

	MessageBox(NULL, szMsg, TEXT("PushSourceVidFrames filter error"), MB_ICONERROR | MB_OK);
	*/
}

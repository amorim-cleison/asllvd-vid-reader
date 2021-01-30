%% 
%  BaseClasses is from Direct X 9.0 SDK 2002
%  Mex code here based on GrabBitmaps.cpp in DirectShow samples
%

include_dirs = '-Izlib/zlib-1.2.3';
other_cpps = ' VidHeader.cpp read-frame.cpp showErrMsgBox.cpp bayer.cpp';
if ispc;
	defines = ' -DMSWIN -DMEX_COMPILE ';
	libraries = [' ./zlib/zlib-1.2.3/contrib/vstudio/vc8/x86/ZlibStatRelease/zlibstat.lib', ...
				 ' ./zlib/zlib-1.2.3/contrib/vstudio/vc8/x64/ZlibStatRelease/zlibstat.lib'];
else
	defines = ' -DMEX_COMPILE ';
	libraries = ' libz64.a ';
end

mex_cpps = {'vidCropMex.cpp', 'vidOpenMex.cpp', 'vidReadMex.cpp', 'vidCloseMex.cpp', 'vidWriteMex.cpp'};

for i = 1:length(mex_cpps);
	mex_str = ['mex ', mex_cpps{i}, ' ', defines, include_dirs, other_cpps, libraries];
	eval(mex_str);
end

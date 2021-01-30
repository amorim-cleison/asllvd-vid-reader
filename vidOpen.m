function [hdl, inf] = vidOpen(fname);
	hdl = [];
	inf = [];
	if ~exist(fname,'file')
		fprintf(2,'vidOpen error, cannot find file %s\n', fname);
		return;
	end
	[hdl, t] = vidOpenMex(fname);
	if length(hdl) == 0;
		error(['Error opening video file ', fname]);
		return;
	end
	inf.Width = t(1);
	inf.Height = t(2);
	inf.NumFrames = t(3);

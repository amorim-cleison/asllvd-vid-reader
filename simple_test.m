out_vid_file = '\\nessie\asl-data2/ASL_2008_02_29/scene10-camera1.vid';

while 1;
	[out_vid_handle, out_vid_info] = vidOpenMex(out_vid_file);
	
	view_frames = [0:100:out_vid_info(3)-1];
	
	for framei = view_frames;
		vid_frame = vidReadMex(out_vid_handle, framei);
	
		imshow(uint8(vid_frame));
		
		title(sprintf('%5d of %5d', framei, out_vid_info(3)));
		pause(0.1);
	end
	vidCloseMex(out_vid_handle);
end
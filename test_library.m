clear( 'start_frame', 'end_frame');
out_vid_file = '/research/asl-data3/ashwin/asl/handshape_cooccurence_substitution_videos/vid/cooccurrence/10___10.vid';
out_vid_file = '/research/asl-data3/ASL_liz_verify_annotations/scene1-camera1.vid';
out_vid_file = '/research/asl-data3/ashwin/temp/adopt.vid';

out_vid_file = '/research/asl-data3/ashwin/temp/adopt_appoint_identity.vid';
out_vid_file = '/research/asl-data3/ASL_2010_08_10_Liz_additional_lexicon_signs/scene1-camera3.vid';
%out_vid_file = '/research/asl-data/ASL_2006_01_20/scene18-camera4.vid';
out_vid_file = '/research/asl-data2/ASL_2008_02_29/scene1-camera1.vid';
out_vid_file = '/research/asl-data3/quicktime/annotate/verify_start_end_handshape_annotations/signs_vid/5.vid';
out_vid_file = '/research/asl-data4/PortableCapture_project/compressed_video/08-18-2010-test/scene1-camera3.vid';
out_vid_file = '\\nessie\asl-data2/ASL_2008_02_29/scene10-camera1.vid';
%start_frame = 0; end_frame = 4;

out_vid_file
if 0;
	dbmex on;
	vid_file1 = '/research/asl-data2/ASL_2008_08_13_session2/scene11-camera1.vid';
	vid_file2 = '/research/asl-data2/ASL_2008_08_13_session2/scene12-camera1.vid';
	
	[vid_handle1, vid_info1] = vidOpenMex(vid_file1);
	[vid_handle2, vid_info2] = vidOpenMex(vid_file2);
	
	h = vid_info1(2); w = vid_info1(1); out_num_frames = 10;
	out_vid_params = [w, 2*h, out_num_frames];
	out_vid_handle = vidOpenMex(out_vid_file, int32(out_vid_params));
	
	merged_frame = zeros(out_vid_params(2),out_vid_params(1),3);
	vid_frame = 1000;
	for out_framei = 1:out_num_frames;
		vid_frame1 = vidReadMex(vid_handle1, vid_frame);
		vid_frame2 = vidReadMex(vid_handle2, vid_frame);
		
		for planes = 1:3;
			merged_frame(1:h,:,planes) = vid_frame1(:,:,planes);
			merged_frame(h+1:end,:,planes) = vid_frame2(:,:,planes);
		end
		vidWriteMex(out_vid_handle, uint8(merged_frame));
		%imshow(uint8(merged_frame)); pause;
		vid_frame = vid_frame + 100;
	end
	vidCloseMex(out_vid_handle);
else
	%dbmex on;
	while 1;
		[out_vid_handle, out_vid_info] = vidOpenMex(out_vid_file);
		if exist('start_frame', 'var');
			view_frames = [start_frame:end_frame];
		else
			view_frames = [0:100:out_vid_info(3)-1];
		end
		for framei = view_frames;
			vid_frame = vidReadMex(out_vid_handle, framei);
			if size(vid_frame, 1) < 5000;
				imshow(uint8(vid_frame));
			else
				imshow(uint8(imresize(vid_frame, 0.5, 'bilinear')));
			end
			title(sprintf('%5d of %5d', framei, out_vid_info(3)));
			pause(0.1);
		end
		vidCloseMex(out_vid_handle);
	end
end

if 0;
	
	vids = {'/research/asl-data2/ASL_2008_01_11/scene7-camera1.vid', '/research/asl-data2/ASL_2008_01_11/scene19-camera1.vid', '/research/asl-data2/ASL_2008_05_29b/scene4-camera1.vid'};
	crop_frames = [785, 825; 860 890; 1486 1510]';
	crop_bboxes = [151 45 362 300; 151 45 362 300; 182 47 316 341]';
	%vidCropMex(vids, '/research/asl-data3/ashwin/temp/adopt_appoint_identity.vid', crop_frames, crop_bboxes);
	vids = {'/research/asl-data2/ASL_2008_01_11/scene7-camera1.vid', '/research/asl-data2/ASL_2008_01_11/scene8-camera1.vid'};
	crop_frames = [2110 2145; 601 656]';
	crop_bboxes = [151 45 362 300; 151 45 362 300]';
	vidCropMex(vids, '/research/asl-data3/ashwin/temp/advise_advisor.vid', crop_frames, crop_bboxes);

	%vidCropMex('/research/asl-data2/ASL_2008_01_11/scene7-camera1.vid', '/research/asl-data3/ashwin/temp/adopt.vid', [785, 825], [151 45 362 300]');
	%vidCropMex('/research/asl-data2/ASL_2008_01_11/scene19-camera1.vid', '/research/asl-data3/ashwin/temp/appoint.vid', [860 890], [151 45 362 300]');
	%vidCropMex('/research/asl-data2/ASL_2008_05_29b/scene4-camera1.vid', '/research/asl-data3/ashwin/temp/identity.vid', [1486 1510], [182 47 316 341]');

end

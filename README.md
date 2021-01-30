# `.vid` Reader

This code is based on the 'vid_reader' created by the Boston University for use with the ASLLVD Dataset. Please refer to this link to find more information:
http://www.bu.edu/asllrp/av/dai-asllvd.html.

## What is this?
With this program, you can read a `.vid` file and extract its frames as `.ppm` images.

## How can I use it?
Once compiled, the following arguments are required by the program, in this order:

1. Path to the input video (.vid) file
1. Output directory (where the images will be saved)
1. Prefix to be added to file names
1. Start frame
1. End frame
1. FPS in (actual frame rate for the video)
1. FPS out (number of frames/images to extract per second for the video)

Example call, using these arguments:
```
./vid_reader /path/to/my/video.vid ./path/for/output prefix-s002 5 1000 60 3
```

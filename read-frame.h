/*
 * Utility function to determine the format of a compressed video file.
 *
 * The input parameter "filename" must be set.
 *
 * On success, 0 is returned, and the values pointed to by "num_framesp",
 * "widthp" and "heightp" are filled in as appropriate.
 *
 * On an error, a message will be displayed to standard error and -1
 * will be returned.
 */
extern int file_statistics( char *filename, int *num_framesp,
			    int *widthp, int *heightp );

/*
 * Utility function to read (decompress, white balance, and Bayer interpolate)
 * a compressed video file.
 *
 * The input parameters "filename" and "frame" specify which file and
 * frame (where "0" indicates the first frame in the file) to decode,
 * and on successful exit, the return value will be a buffer of 8-bit
 * colour values (in R, G, B order, scanning horizontally from left to
 * right and then vertically from top to bottom).  This buffer must be
 * deallocated with "free".  If widthp and heightp are non-NULL, then
 * they will be updated to reflect the dimensions of the image.
 *
 * On an error, a message will be displayed to standard error and NULL
 * will be returned.
 */

extern void bayer_interpolate_frame( unsigned char * decompress_buf, unsigned char * interp_buf, int width, int height );

extern unsigned char *read_frame( char *filename, int frame,
				  int *widthp, int *heightp );

/*
 * White balancing: after calling this function, subsequent calls to
 * read_frame will use the weights specified here.
 *
 * 1.0 = no change, >1.0 = gain, <1.0 = attenuation.
 */
extern void white_balance( float r, float g, float b );

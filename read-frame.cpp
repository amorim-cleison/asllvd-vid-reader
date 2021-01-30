#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static float rbal = (float)1.00, gbal = (float)1.33, bbal = (float)3.05;

extern int file_statistics( char *filename, int *num_framesp,
			    int *widthp, int *heightp ) {
    FILE *in;
    unsigned char header[ 8 ];
    
    if( !( in = fopen( filename, "rb" ) ) ) {
	perror( filename );

	return -1;
    }

    if( fread( header, 8, 1, in ) != 1 ) {
	if( ferror( in ) )
	    perror( filename );
	else
	    fprintf( stderr, "%s: unexpected end of file\n", filename );

	fclose( in );
	return -1;
    }

    if( widthp )
	*widthp = header[ 0 ] | ( header[ 1 ] << 8 );
    
    if( heightp )
	*heightp = header[ 2 ] | ( header[ 3 ] << 8 );
    
    if( num_framesp )
	*num_framesp = header[ 4 ] | ( header[ 5 ] << 8 ) |
	    ( header[ 6 ] << 16 ) | ( header[ 7 ] << 24 );

    fclose( in );
    return 0;
}

static unsigned char clamp( float x ) {

    if( x < 0 )
	return 0;
    else if( x > 0xFF )
	return 0xFF;
    else
	return (unsigned char)floor(x);
}

static unsigned interp2( unsigned m, unsigned n ) {

    return ( m + n ) >> 1;
}

static unsigned interp4( unsigned k, unsigned l, unsigned m, unsigned n ) {

    return ( k + l + m + n ) >> 2;
}

#define rgb(y,x,c) \
	interp_buf[ ( (y) * width + (x) ) * 3 + (c) ]
#define bayer(y,x) \
	( decompress_buf[ ( (y) * width + (x) ) << 1 ] | \
	  ( decompress_buf[ ( ( (y) * width + (x) ) << 1 ) | 1 ] << 8 ) )

enum correlation { COR_HORIZ, COR_VERT, COR_NONE };

static enum correlation correlation( int h0, int h1, int v0, int v1 ) {

    int h, v;

    h = abs( h0 - h1 );
    v = abs( v0 - v1 );

    if( h > v * 1.1 )
	return COR_VERT;
    else if( v > h * 1.1 )
	return COR_HORIZ;
    else
	return COR_NONE;
}


extern void white_balance( float r, float g, float b ) {

    rbal = r;
    gbal = g;
    bbal = b;
}

extern void bayer_interpolate_frame( unsigned char * decompress_buf, unsigned char * interp_buf, int width, int height ) {
    
	char *bayer_pattern;
	int y,x;

	if( width == 1600 )
		bayer_pattern = "grbG";
	else
		bayer_pattern = "bGgr";

	for( y = 0; y < height; y++ )
		for( x = 0; x < width; x++ ) {
			int l, ll, r, rr, u, uu, d, dd, red=0, grn=0, blu=0;

			if( x > 0 )
				l = x - 1;
			else
				l = x + 1;

			if( x > 1 )
				ll = x - 2;
			else
				ll = x;

			if( x < width - 1 )
				r = x + 1;
			else
				r = x - 1;

			if( x < width - 2 )
				rr = x + 2;
			else
				rr = x;

			if( y > 0 )
				u = y - 1;
			else
				u = y + 1;

			if( y > 1 )
				uu = y - 2;
			else
				uu = y;

			if( y < height - 1 )
				d = y + 1;
			else
				d = y - 1;

			if( y < height - 2 )
				dd = y + 2;
			else
				dd = y;	    		

			switch( bayer_pattern[ ( ( y & 1 ) << 1 ) | ( x & 1 ) ] ) {
		case 'r':
			red = bayer( y, x );
			switch( correlation( bayer( y, ll ),
				bayer( y, rr ),
				bayer( uu, x ),
				bayer( dd, x ) ) ) {
		case COR_HORIZ:
			grn = interp2( bayer( y, l ), bayer( y, r ) );
			break;

		case COR_VERT:
			grn = interp2( bayer( u, x ), bayer( d, x ) );
			break;

		case COR_NONE:
			grn = interp4( bayer( y, l ), bayer( y, r ),
				bayer( u, x ), bayer( d, x ) );
			break;		    
			}
			blu = interp4( bayer( u, l ), bayer( u, r ),
				bayer( d, l ), bayer( d, r ) );
			break;

		case 'G':
			red = interp2( bayer( u, x ), bayer( d, x ) );
			grn = bayer( y, x );
			blu = interp2( bayer( y, l ), bayer( y, r ) );
			break;

		case 'g':
			red = interp2( bayer( y, l ), bayer( y, r ) );
			grn = bayer( y, x );
			blu = interp2( bayer( u, x ), bayer( d, x ) );
			break;

		case 'b':
			red = interp4( bayer( u, l ), bayer( u, r ),
				bayer( d, l ), bayer( d, r ) );
			switch( correlation( bayer( y, ll ),
				bayer( y, rr ),
				bayer( uu, x ),
				bayer( dd, x ) ) ) {
		case COR_HORIZ:
			grn = interp2( bayer( y, l ), bayer( y, r ) );
			break;

		case COR_VERT:
			grn = interp2( bayer( u, x ), bayer( d, x ) );
			break;

		case COR_NONE:
			grn = interp4( bayer( y, l ), bayer( y, r ),
				bayer( u, x ), bayer( d, x ) );
			break;		    
			}
			blu = bayer( y, x );
			}

			rgb( y, x, 0 ) = clamp( red * rbal / (float)4.0 );
			rgb( y, x, 1 ) = clamp( grn * gbal / (float)4.0 );
			rgb( y, x, 2 ) = clamp( blu * bbal / (float)4.0 );
		}
}

extern unsigned char *read_frame( char *filename, int frame,
				  int *widthp, int *heightp ) {
	  return 0;
}


/*
extern unsigned char *read_frame( char *filename, int frame,
				  int *widthp, int *heightp ) {
    FILE *in;
    unsigned char header[ 8 ];
    int num_frames, width, height, init = 0, x, y;
    unsigned char compress_buf[ 0x10000 ], *decompress_buf, *interp_buf;
    off_t offset;
    z_stream z;
    char *bayer_pattern;
    
    if( file_statistics( filename, &num_frames, &width, &height ) < 0 )
	return NULL;

    if( width == 1600 )
	bayer_pattern = "grbG";
    else
	bayer_pattern = "bGgr";
    
    if( frame < 0 || frame >= num_frames ) {
	fputs( "read_frame: frame index is out of range", stderr );

	return NULL;
    }
    
    if( widthp )
	*widthp = width;

    if( heightp )
	*heightp = height;
			 
    if( !( in = fopen( filename, "rb" ) ) ||
	fseeko( in, ( frame + 1 ) << 3, SEEK_SET ) < 0 ||
	fread( header, 8, 1, in ) != 1 ) {
	perror( filename );

	return NULL;
    }

    offset = (off_t) header[ 0 ] |
	( (off_t) header[ 1 ] << 8 ) |
	( (off_t) header[ 2 ] << 16 ) |
	( (off_t) header[ 3 ] << 24 ) |
	( (off_t) header[ 4 ] << 32 ) |
	( (off_t) header[ 5 ] << 40 ) |
	( (off_t) header[ 6 ] << 48 ) |
	( (off_t) header[ 7 ] << 56 );

    if( fseeko( in, offset, SEEK_SET ) < 0 ) {
	perror( filename );

	fclose( in );
	return NULL;
    }
		
    if( !( decompress_buf = malloc( width * height << 1 ) ) ) {
	perror( "read_frame" );

	fclose( in );
	return NULL;
    }

    z.next_out = decompress_buf;
    z.avail_out = width * height << 1;
    while( z.avail_out ) {
	int s;

	if( ( s = fread( compress_buf, 1, sizeof compress_buf, in ) ) < 1 ) {
	    if( ferror( in ) )
		perror( filename );
	    else
		fprintf( stderr, "%s: unexpected end of file\n", filename );
	    
	    free( decompress_buf );
	    fclose( in );
	    return NULL;
	}
	
	z.next_in = compress_buf;
	z.avail_in = s;
	
	if( !init ) {
	    z.zalloc = NULL;
	    z.zfree = NULL;
	    z.opaque = NULL;
	    inflateInit( &z );
	    init = 1;
	}

	while( z.avail_in && z.avail_out ) {
	    int ret = inflate( &z, 0 );

	    if( ret != Z_OK && z.avail_out ) {
		fputs( "frame_read: warning: possibly corrupted "
		       "compressed stream\n", stderr );
		z.avail_out = 0;
	    }
	}
    }

    inflateEnd( &z );
    fclose( in );
    
    if( !( interp_buf = malloc( width * height * 3 ) ) ) {
	perror( "read_frame" );
	free( decompress_buf );

	return NULL;
    }
    
    for( y = 0; y < height; y++ )
	for( x = 0; x < width; x++ ) {
	    int l, ll, r, rr, u, uu, d, dd, red, grn, blu;

	    if( x > 0 )
		l = x - 1;
	    else
		l = x + 1;

	    if( x > 1 )
		ll = x - 2;
	    else
		ll = x;

	    if( x < width - 1 )
		r = x + 1;
	    else
		r = x - 1;

	    if( x < width - 2 )
		rr = x + 2;
	    else
		rr = x;

	    if( y > 0 )
		u = y - 1;
	    else
		u = y + 1;

	    if( y > 1 )
		uu = y - 2;
	    else
		uu = y;

	    if( y < height - 1 )
		d = y + 1;
	    else
		d = y - 1;

	    if( y < height - 2 )
		dd = y + 2;
	    else
		dd = y;	    		
	    
	    switch( bayer_pattern[ ( ( y & 1 ) << 1 ) | ( x & 1 ) ] ) {
	    case 'r':
		red = bayer( y, x );
		switch( correlation( bayer( y, ll ),
				     bayer( y, rr ),
				     bayer( uu, x ),
				     bayer( dd, x ) ) ) {
		case COR_HORIZ:
		    grn = interp2( bayer( y, l ), bayer( y, r ) );
		    break;

		case COR_VERT:
		    grn = interp2( bayer( u, x ), bayer( d, x ) );
		    break;

		case COR_NONE:
		    grn = interp4( bayer( y, l ), bayer( y, r ),
				 bayer( u, x ), bayer( d, x ) );
		    break;		    
		}
		blu = interp4( bayer( u, l ), bayer( u, r ),
			     bayer( d, l ), bayer( d, r ) );
		break;
		
	    case 'G':
		red = interp2( bayer( u, x ), bayer( d, x ) );
		grn = bayer( y, x );
		blu = interp2( bayer( y, l ), bayer( y, r ) );
		break;
		
	    case 'g':
		red = interp2( bayer( y, l ), bayer( y, r ) );
		grn = bayer( y, x );
		blu = interp2( bayer( u, x ), bayer( d, x ) );
		break;

	    case 'b':
		red = interp4( bayer( u, l ), bayer( u, r ),
			     bayer( d, l ), bayer( d, r ) );
		switch( correlation( bayer( y, ll ),
				     bayer( y, rr ),
				     bayer( uu, x ),
				     bayer( dd, x ) ) ) {
		case COR_HORIZ:
		    grn = interp2( bayer( y, l ), bayer( y, r ) );
		    break;

		case COR_VERT:
		    grn = interp2( bayer( u, x ), bayer( d, x ) );
		    break;

		case COR_NONE:
		    grn = interp4( bayer( y, l ), bayer( y, r ),
				 bayer( u, x ), bayer( d, x ) );
		    break;		    
		}
		blu = bayer( y, x );
	    }

	    rgb( y, x, 0 ) = clamp( red * rbal / 4.0 );
	    rgb( y, x, 1 ) = clamp( grn * gbal / 4.0 );
	    rgb( y, x, 2 ) = clamp( blu * bbal / 4.0 );
	}

    free( decompress_buf );

    return interp_buf;
}
*/


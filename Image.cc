#include <png.h>
#include "Image.h"
#include "Background.h"
//-----------------------------------------------------------------------------
#define ID_IMAGEMASK \
	SubstructureRedirectMask|SubstructureNotifyMask| \
	ButtonPressMask|ButtonReleaseMask|PointerMotionMask|\
	EnterWindowMask|LeaveWindowMask
//-----------------------------------------------------------------------------
Image::Image( char *f, Window p ): WinControl( p, ID_IMAGEMASK, 0 )
{
	bg = NULL;
	source = NULL;
	Glowing = false;
	FreeAlpha = false;
	StateChanged = false;
	broken = false;

	SetSource( f );
	CreateHandler();
}
//-----------------------------------------------------------------------------
Image::~Image()
{
	FreeBuffer();
}
//-----------------------------------------------------------------------------
void Image::FreeBuffer()
{
	if( FreeAlpha ) {
		delete [] rgb;
		delete [] alpha[0];
		delete [] alpha[1];
	}
	if( bg ) {
		Imlib_kill_image( ScreenData, bg );
		bg = NULL;
	}
	if( source ) {
		Imlib_kill_image( ScreenData, source );
		source = NULL;
	}
}
//-----------------------------------------------------------------------------
void Image::SetSource( char *f )
{
	FreeBuffer();
	if( Load( f ) == -1 ) {
		source = Imlib_load_image( ScreenData, f );
		if( !source ) throw "Couldn't find image, bailing.\n";

		rgb = source->rgb_data;
		width = source->rgb_width;
		height = source->rgb_height;
		if( !source->alpha_data ) {
			FreeAlpha = true;
			uchar trans = 255 - (uchar)Transparency;
			alpha[0] = new uchar [width * height * sizeof(uchar)];
			alpha[1] = new uchar [width * height * sizeof(uchar)];
			memset(alpha[0], trans, width * height * sizeof(uchar));
			memset(alpha[1], 255, width * height * sizeof(uchar));
		}
		else {
			alpha[0] = source->alpha_data;
			alpha[1] = source->alpha_data;
		}
	}
}
//-----------------------------------------------------------------------------
void Image::SetGlowing( bool g )
{
	if( Glowing != g ) {
		StateChanged = true;
		Glowing = g;
		Draw();
		StateChanged = false;
	}
}
//-----------------------------------------------------------------------------
void Image::SetBroken( bool b )
{
	if( b != broken ) {
		broken = b;
		Draw();
	}
}
//-----------------------------------------------------------------------------
void Image::Draw()
{
	int i, j, a;
	uchar *bgrgb, *alpha_levels = alpha[Glowing ? 1 : 0];
	ImlibImage *bgcopy;

	if( !StateChanged ) {
		if( bg != NULL ) Imlib_kill_image( ScreenData, bg );
		bg = VdeskBg->Crop( x, y, width, height );
	}
	bgcopy = Imlib_clone_image( ScreenData, bg );
	bgrgb = bgcopy->rgb_data;
	for( i = 0, a = 0; i < (3 * width * height); i += 3, a++ ) {
		int alevel = ( (alpha_levels != 0) ? alpha_levels[a] : 255 );
		if( alevel == 0 ) {
			bgrgb[i] = bgrgb[i + 2] = 255;
			bgrgb[i + 1] = 0;
			continue;
		}
		int rlevel = 255 - alevel;
		bgrgb[i+0] = (bgrgb[i+0] * rlevel)/255 + (rgb[i+0] * alevel)/255;
		bgrgb[i+1] = (bgrgb[i+1] * rlevel)/255 + (rgb[i+1] * alevel)/255;
		bgrgb[i+2] = (bgrgb[i+2] * rlevel)/255 + (rgb[i+2] * alevel)/255;
	}

	if( broken ) {
		int sx = (width - BrokenIcon->rgb_width) / 2,
			sy = height - 1,
			ss[] = {3,2,1,1,1,1,1,1,1,1,1,1,2,3};

		for( i = BrokenIcon->rgb_height - 2; i > 0; i--, sy-- ) {
			uchar *d = &bgrgb[3 * (sy * width + sx + ss[i-1])];
			uchar *s = &BrokenIcon->rgb_data[3 * (i * BrokenIcon->rgb_width + ss[i-1])];
			for( j = 3 * (BrokenIcon->rgb_width - 2 * ss[i-1]); j > 0; j-- )
				*d++ = *s++;
		}
	}

	ImlibColor IClr = {255, 0, 255, 0};
	Imlib_set_image_shape( ScreenData, bgcopy, &IClr );

	Imlib_apply_image( ScreenData, bgcopy, this->handler );
	Imlib_kill_image( ScreenData, bgcopy );
}
//-----------------------------------------------------------------------------
int Image::Load( char *f )
{
	uchar *ptr, *aptr, *aptr2, **lines, *line, r, g, b, a;
	png_structp   pngdata;
	png_infop     pnginfo;
	png_uint_32   w, h;
	FILE          *fp;
	int           color, depth, interlace;
	string        fn = f;

	if( f[0] == '~' && f[1] == '/' )
		fn.replace( 0, 2, HomeFolder );
	if( !(fp = fopen( fn.c_str(), "r" )) ) {
		for( int i=0; i<IconFolders.size(); i++ ) {
			fn = IconFolders[i] + f;
			if( (fp = fopen( fn.c_str(), "r" )) ) break;
		}
		if( !fp ) {
			fp = fopen( DefaultIcon.c_str(), "r" );
			if( !fp ) return -1;
		}
	}

	pngdata = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	if( !pngdata ) return -1;
	pnginfo = png_create_info_struct( pngdata );
	if( !pnginfo ) {
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}

	if( setjmp(pngdata->jmpbuf) ) {
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}

	if( color == PNG_COLOR_TYPE_RGB_ALPHA ) {
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}

	png_init_io( pngdata, fp );
	png_read_info( pngdata, pnginfo );
	png_get_IHDR( pngdata, pnginfo, &w, &h, &depth, &color, &interlace, NULL, NULL );

	width = w;
	height = h;

	if( color == PNG_COLOR_TYPE_PALETTE ) png_set_expand( pngdata );
	png_set_strip_16( pngdata );
	png_set_packing( pngdata );

	if( png_get_valid(pngdata,pnginfo,PNG_INFO_tRNS) ) png_set_expand( pngdata );
	png_set_filler( pngdata, 0xff, PNG_FILLER_AFTER );


	rgb = new uchar [width * height * 3];
	if( !rgb ) {
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}
	alpha[0] = new uchar [width * height];
	alpha[1] = new uchar [width * height];
	if( !alpha[0] || !alpha[1] ) {
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}
	FreeAlpha = true;

	lines = new uchar * [height * sizeof(uchar *)];
	if( !lines ) {
		FreeBuffer();
		png_destroy_read_struct( &pngdata, NULL, NULL );
		return -1;
	}
	for( int i=0; i<height; i++ ) {
		lines[i] = new uchar [width * (sizeof(uchar*)) * 4];
		if(! lines[i] ) {
			FreeBuffer();
			png_destroy_read_struct( &pngdata, NULL, NULL );
			return -1;
		}
	}

	png_read_image( pngdata, lines );
	png_destroy_read_struct( &pngdata, &pnginfo, NULL );
	ptr = rgb;
	aptr = alpha[0];
	aptr2 = alpha[1];

	for(int y = 0; y < height; y++) {
		line = lines[y];
		for (int x = 0; x < width; x++) {
			if( color == PNG_COLOR_TYPE_GRAY_ALPHA ) {
				r = g = b = *line++;
				a = *line++;
			} else if( color == PNG_COLOR_TYPE_GRAY ) {
				r = g = b = *line++;
				a = 255;
			} else {
				r = *line++;
				g = *line++;
				b = *line++;
				a = *line++;
			}
			*ptr++ = r;
			*ptr++ = g;
			*ptr++ = b;

			// Transparency matrix.
			if( ( a - Transparency ) < 0 ) {
				*aptr++ = 0;
				*aptr2++ = a;
			}
			else {
				*aptr++ = a - Transparency;
				*aptr2++ = a;
			}
		}
	} 

	fclose( fp );
	for( int i=0; i<height; i++ )
		delete [] lines[i];
	delete [] lines;
	return 0;
}
//-----------------------------------------------------------------------------

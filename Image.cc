#include <png.h>
#include "Image.h"
#include "Background.h"
//-----------------------------------------------------------------------------
#define ID_IMAGEMASK \
	SubstructureRedirectMask|SubstructureNotifyMask| \
	ButtonPressMask|ButtonReleaseMask|PointerMotionMask|\
	EnterWindowMask|LeaveWindowMask
//-----------------------------------------------------------------------------
Image::Image( char *f ): WinControl( ID_IMAGEMASK )
{
	source = NULL;
	Glowing = false;
	FreeAlpha = false;

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
	if( Glowing!=g ) {
		Glowing = g;
		Draw();
	}
}
//-----------------------------------------------------------------------------
void Image::Draw()
{
	ImlibImage *bg;
	uchar *bgrgb;
	uchar *alpha_levels;

	alpha_levels = alpha[Glowing ? 1 : 0];
	bg = background->Crop( x, y, width, height );
	bgrgb = bg->rgb_data;

	for( int i=0, a=0; i<(3*width*height); i+=3, a++) {
		int alevel = ( (alpha_levels != 0) ? alpha_levels[a] : 255 );
		if( alevel == 0 ) { bgrgb[i] = bgrgb[i+2] = 255; bgrgb[i+1] = 0; continue; }
		bgrgb[i+0] = (bgrgb[i+0] * (255-alevel))/255 + (rgb[i+0] * alevel)/255;
		bgrgb[i+1] = (bgrgb[i+1] * (255-alevel))/255 + (rgb[i+1] * alevel)/255;
		bgrgb[i+2] = (bgrgb[i+2] * (255-alevel))/255 + (rgb[i+2] * alevel)/255;
	}

	ImlibColor IClr={255,0,255,0};
	Imlib_set_image_shape( ScreenData, bg, &IClr );
	Imlib_apply_image( ScreenData, bg, this->handler );
	Imlib_kill_image( ScreenData, bg );
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

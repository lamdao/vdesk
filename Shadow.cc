#include "Shadow.h"
//-----------------------------------------------------------------------------
#define SHADOW_SIZE	8
//-----------------------------------------------------------------------------
Shadow::Shadow(): Resource()
{
	Right = new WinControl( 0 );
	Bottom = new WinControl( 0 );
	Right->CreateHandler( SHADOW_SIZE, SHADOW_SIZE );
	Bottom->CreateHandler( SHADOW_SIZE, SHADOW_SIZE );
}
//-----------------------------------------------------------------------------
Shadow::~Shadow()
{
	delete Right;
	delete Bottom;
}
//-----------------------------------------------------------------------------
void Shadow::Show( WinControl *context )
{
	int n, i, j;
	register uchar *rgb;
	ImlibImage *bg;

	Right->MoveTo( context->X() + context->Width(), context->Y() );
	n = ScreenWidth - Right->X() - 1;
	if( n > 0 ) {
		if( n > SHADOW_SIZE ) n = SHADOW_SIZE;
		Right->Resize( n, context->Height() );

		bg = Imlib_create_image_from_drawable( ScreenData, Root, 0,
				Right->X(), Right->Y(), Right->Width(), Right->Height() );

		n--;
		for( i = 0; i < Right->Height(); i++ ) {
			rgb = &bg->rgb_data[i * Right->Width() * 3];
			for( j = 0; j < Right->Width() - n; j++ ) {
				*rgb++ = *rgb / 3 * 2;
				*rgb++ = *rgb / 3 * 2;
				*rgb++ = *rgb / 3 * 2;
			}
			if( n > 0 ) n--;
		} 

		Imlib_apply_image( ScreenData, bg, Right->Handler() );
		Imlib_kill_image( ScreenData, bg );
		Right->Show();
		Right->Raise();
	}

	Bottom->MoveTo( context->X(), context->Y() + context->Height() );
	i = ScreenWidth - Bottom->X();
	if( i > context->Width() + SHADOW_SIZE )
		i = context->Width() + SHADOW_SIZE;
	n = ScreenHeight - Bottom->Y();
	if( n > 0 ) {
		if( n > SHADOW_SIZE ) n = SHADOW_SIZE;
		Bottom->Resize( i, n );
		bg = Imlib_create_image_from_drawable( ScreenData, Root, 0,
				Bottom->X(), Bottom->Y(), Bottom->Width(), Bottom->Height() );

		for( i = 0; i < Bottom->Height(); i++ ) {
			rgb = &bg->rgb_data[(i * Bottom->Width() + SHADOW_SIZE - n) * 3];
			for( j = SHADOW_SIZE - n; j < Bottom->Width(); j++ ) {
				*rgb++ = *rgb / 3 * 2;
				*rgb++ = *rgb / 3 * 2;
				*rgb++ = *rgb / 3 * 2;
			}
			if( n > 1 ) n--;
		} 

		Imlib_apply_image( ScreenData, bg, Bottom->Handler() );
		Imlib_kill_image( ScreenData, bg );

		Bottom->Show();
		Bottom->Raise();
	}
}
//-----------------------------------------------------------------------------
void Shadow::Hide()
{
	Right->Hide();
	Bottom->Hide();
}
//-----------------------------------------------------------------------------

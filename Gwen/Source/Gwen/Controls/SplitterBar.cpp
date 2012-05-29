
#include "Gwen/Gwen.h"
#include "Gwen/Controls/SplitterBar.h"

using namespace Gwen;
using namespace Controls;

GWEN_CONTROL_CONSTRUCTOR( SplitterBar )
{
	SetTarget( this );
	RestrictToParent( true );
}

void SplitterBar::Render( Skin::Base* skin )
{
	if ( ShouldDrawBackground() ) {
//		skin->DrawButton( this, true, false, IsDisabled() );
        skin->GetRender()->SetDrawColor( Gwen::Color( 100, 100, 100, 255 ) );
        skin->GetRender()->DrawFilledRect( GetRenderBounds() );
    }
}

void SplitterBar::Layout( Skin::Base* /*skin*/ )
{
	MoveTo( X(), Y() );
}
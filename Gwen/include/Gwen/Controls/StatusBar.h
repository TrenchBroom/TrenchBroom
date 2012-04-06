#pragma once
#ifndef GWEN_CONTROLS_STATUSBAR_H
#define GWEN_CONTROLS_STATUSBAR_H

#include "Gwen/Gwen.h"
#include "Gwen/Controls/Label.h"

namespace Gwen 
{
	namespace Controls
	{
		class StatusBar : public Controls::Label
		{
			public:

				GWEN_CONTROL_INLINE( StatusBar, Controls::Label )
				{
					SetHeight( 22 );
					Dock( Pos::Bottom );
					SetPadding( Padding( 2, 2, 2, 2 ) );
					SetText( "Status Bar." );
					SetAlignment( Pos::Left | Pos::CenterV );
				}

				virtual void AddControl( Controls::Base* pCtrl, bool bRight )
				{
					pCtrl->SetParent( this );
					pCtrl->Dock( bRight ? Pos::Right : Pos::Left );
				}

				virtual void Render( Skin::Base* skin )
				{
					skin->DrawStatusBar( this );
				}
		};
	}
}
#endif

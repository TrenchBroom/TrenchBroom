/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Gwen.h"
#include <math.h>

namespace Gwen
{
	namespace Skin
	{
		/*

			Here we're drawing a few symbols such as the directional arrows and the checkbox check

			Texture'd skins don't generally use these - but the Simple skin does. We did originally
			use the marlett font to draw these.. but since that's a Windows font it wasn't a very
			good cross platform solution.

		*/

		void Base::DrawArrowDown( Gwen::Rect rect )
		{
			float x = (rect.w / 5.0f);
			float y = (rect.h / 5.0f);

			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*0.0f, rect.y + y*1.0f, x, y*1.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*1.0f, x, y*2.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*2.0f, rect.y + y*1.0f, x, y*3.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*3.0f, rect.y + y*1.0f, x, y*2.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*4.0f, rect.y + y*1.0f, x, y*1.0f ) );
		}

		void Base::DrawArrowUp( Gwen::Rect rect )
		{
			float x = (rect.w / 5.0f);
			float y = (rect.h / 5.0f);

			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*0.0f, rect.y + y*3.0f, x, y*1.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*2.0f, x, y*2.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*2.0f, rect.y + y*1.0f, x, y*3.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*3.0f, rect.y + y*2.0f, x, y*2.0f ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*4.0f, rect.y + y*3.0f, x, y*1.0f ) );
		}

		void Base::DrawArrowLeft( Gwen::Rect rect )
		{
			float x = (rect.w / 5.0f);
			float y = (rect.h / 5.0f);

			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*3.0f, rect.y + y*0.0f, x*1.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*2.0f, rect.y + y*1.0f, x*2.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*2.0f, x*3.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*2.0f, rect.y + y*3.0f, x*2.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*3.0f, rect.y + y*4.0f, x*1.0f, y ) );
		}

		void Base::DrawArrowRight( Gwen::Rect rect )
		{
			float x = (rect.w / 5.0f);
			float y = (rect.h / 5.0f);

			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*0.0f, x*1.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*1.0f, x*2.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*2.0f, x*3.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*3.0f, x*2.0f, y ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*4.0f, x*1.0f, y ) );
		}

		void Base::DrawCheck( Gwen::Rect rect )
		{
			float x = (rect.w / 5.0f);
			float y = (rect.h / 5.0f);

			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*0.0f, rect.y + y*3.0f, x*2, y*2 ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*1.0f, rect.y + y*4.0f, x*2, y*2 ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*2.0f, rect.y + y*3.0f, x*2, y*2 ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*3.0f, rect.y + y*1.0f, x*2, y*2 ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + x*4.0f, rect.y + y*0.0f, x*2, y*2 ) );
		}

		void Base::DrawTreeNode( Controls::Base* ctrl, bool bOpen, bool bSelected, int iLabelHeight, int iLabelWidth, int iHalfWay, int iLastBranch, bool bIsRoot )
		{
			GetRender()->SetDrawColor( Colors.Tree.Lines );

			if ( !bIsRoot )
				GetRender()->DrawFilledRect( Gwen::Rect( 8, iHalfWay, 16-9, 1 ) );

			if ( !bOpen ) return;

			GetRender()->DrawFilledRect( Gwen::Rect( 14 + 7, iLabelHeight+1, 1, iLastBranch + iHalfWay - iLabelHeight ) );
		}

		void Base::DrawPropertyTreeNode( Controls::Base* control, int BorderLeft, int BorderTop )
		{
			Gwen::Rect rect = control->GetRenderBounds();

			m_Render->SetDrawColor( Colors.Properties.Border );

			m_Render->DrawFilledRect( Gwen::Rect( rect.x, rect.y, BorderLeft, rect.h ) );
			m_Render->DrawFilledRect( Gwen::Rect( rect.x + BorderLeft, rect.y, rect.w - BorderLeft, BorderTop ) );
		}

		void Base::DrawPropertyRow( Controls::Base* control, int iWidth, bool bBeingEdited, bool bHovered )
		{
			Gwen::Rect rect = control->GetRenderBounds();

			if ( bBeingEdited )					m_Render->SetDrawColor( Colors.Properties.Column_Selected );
			else if ( bHovered )				m_Render->SetDrawColor( Colors.Properties.Column_Hover );
			else								m_Render->SetDrawColor( Colors.Properties.Column_Normal );
				
			m_Render->DrawFilledRect( Gwen::Rect( 0, rect.y, iWidth, rect.h ) );
	
			if ( bBeingEdited )					m_Render->SetDrawColor( Colors.Properties.Line_Selected );
			else if ( bHovered )				m_Render->SetDrawColor( Colors.Properties.Line_Hover );
			else								m_Render->SetDrawColor( Colors.Properties.Line_Normal );

			m_Render->DrawFilledRect( Gwen::Rect( iWidth, rect.y, 1, rect.h ) );

			rect.y += rect.h-1;
			rect.h = 1;

			m_Render->DrawFilledRect( rect );
		}
	}
}

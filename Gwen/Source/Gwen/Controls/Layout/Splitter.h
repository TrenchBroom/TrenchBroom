/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#pragma once
#ifndef GWEN_CONTROLS_LAYOUT_SPLITTER_H
#define GWEN_CONTROLS_LAYOUT_SPLITTER_H

#include "Gwen/Controls/Base.h"

namespace Gwen 
{
	namespace Controls
	{
		namespace Layout
		{

			class GWEN_EXPORT Splitter : public Base
			{
				public:

					typedef Base BaseClass;

					Splitter( Base* pParent ) : BaseClass( pParent )
					{
						for ( int i=0; i<2; i++ )
						{
							m_pPanel[i] = NULL;
							m_bScale[i] = true;
						}
					}

					void SetPanel( int i, Base* pPanel, bool bNoScale = false )
					{
						if ( i < 0 || i > 1 ) return;

						m_bScale[i] = !bNoScale;
						m_pPanel[i] = pPanel;

						if ( m_pPanel[i]  )
						{
							m_pPanel[i] ->SetParent( this );
						}
					}

					Base* GetPanel( int i ) const
					{
						if ( i < 0 || i > 1 ) return NULL;
						return m_pPanel[i];
					}

					void Layout( Skin::Base* skin )
					{
						LayoutVertical( skin );
					}

				private:

					void LayoutVertical( Skin::Base* /*skin*/ )
					{
						int w = Width();
						int h = Height();

						if ( m_pPanel[0] )
						{
							const Margin& m = m_pPanel[0]->GetMargin();
							if ( m_bScale[0] )
								m_pPanel[0]->SetBounds( m.left, m.top, w-m.left-m.right, (h * 0.5) - m.top - m.bottom );
							else
							{
								m_pPanel[0]->Position( Pos::Center, 0,  h * -0.25f );
							}
						}

						if ( m_pPanel[1] )
						{
							const Margin& m = m_pPanel[1]->GetMargin();

							if ( m_bScale[1] )
								m_pPanel[1]->SetBounds( m.left, m.top + (h * 0.5f), w-m.left-m.right, (h * 0.5f) - m.top - m.bottom );
							else
								m_pPanel[1]->Position( Pos::Center, 0,  h * 0.25f );
						}
					}

					void LayoutHorizontal( Skin::Base* /*skin*/ )
					{
						// Todo.
					}

					Base*	m_pPanel[2];
					bool	m_bScale[2];

			};
		}
	}
}
#endif

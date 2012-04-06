/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_CONTROLS_COLLAPSIBLELIST_H
#define GWEN_CONTROLS_COLLAPSIBLELIST_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/CollapsibleCategory.h"


namespace Gwen 
{
	namespace Controls
	{
		class GWEN_EXPORT CollapsibleList : public Gwen::Controls::ScrollControl
		{
			public:

				Gwen::Event::Caller	onSelection;

			public:

				GWEN_CONTROL_INLINE( CollapsibleList, Gwen::Controls::ScrollControl )
				{
					SetScroll( false, true );
					SetAutoHideBars( true );
				}

				virtual void Add( Gwen::Controls::CollapsibleCategory* pCategory )
				{
					pCategory->SetParent( this );
					pCategory->Dock( Pos::Top );
					pCategory->SetMargin( Margin( 1, 0, 1, 1 ) );
					pCategory->SetList( this );
					pCategory->onSelection.Add( this, &ThisClass::OnSelectionEvent );
				}

				virtual Gwen::Controls::CollapsibleCategory* Add( const TextObject& name )
				{
					Gwen::Controls::CollapsibleCategory* pCategory = new CollapsibleCategory( this );
					pCategory->SetText( name );
					Add( pCategory );

					return pCategory;
				}

				virtual void Render( Skin::Base* skin )
				{
					skin->DrawCategoryHolder( this );
				}

				virtual void UnselectAll()
				{
					Base::List& children = GetChildren();
					for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
					{
						Gwen::Controls::CollapsibleCategory* pChild = gwen_cast<Gwen::Controls::CollapsibleCategory>(*iter);
						if ( !pChild ) continue;

						pChild->UnselectAll();
					}
				}

				virtual Gwen::Controls::Button* GetSelected()
				{
					Base::List& children = GetChildren();
					for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
					{
						Gwen::Controls::CollapsibleCategory* pChild = gwen_cast<Gwen::Controls::CollapsibleCategory>(*iter);
						if ( !pChild ) continue;

						Gwen::Controls::Button* pSelected = pChild->GetSelected();
						if ( pSelected ) return pSelected;
					}

					return NULL;
				}

			protected:

				virtual void OnSelection( Gwen::Controls::CollapsibleCategory* pControl, Gwen::Controls::Button* pSelected )
				{
					onSelection.Call( this );
				}

				void OnSelectionEvent( Controls::Base* pControl )
				{
					Gwen::Controls::CollapsibleCategory* pChild = gwen_cast<Gwen::Controls::CollapsibleCategory>(pControl);
					if ( !pChild ) return;

					OnSelection( pChild, pChild->GetSelected() );
				}
		};
		
	}
}
#endif

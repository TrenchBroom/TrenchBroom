/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_CONTROLS_MENU_H
#define GWEN_CONTROLS_MENU_H

#include "Gwen/BaseRender.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/MenuItem.h"
#include "Gwen/Controls/ScrollControl.h"

namespace Gwen 
{
	namespace Controls
	{
		class MenuItem;

		class GWEN_EXPORT Menu : public ScrollControl
		{
			public:

				GWEN_CONTROL( Menu, ScrollControl );

				virtual void Render( Skin::Base* skin );
				virtual void RenderUnder( Skin::Base* skin );

				virtual void Layout( Skin::Base* skin );

				virtual MenuItem* AddItem( const TextObject& strName, const TextObject& strIconName = L"", const TextObject& strAccelerator = L"" );

				virtual void AddDivider();

				void OnHoverItem( Gwen::Controls::Base* pControl );
				void CloseAll();
				bool IsMenuOpen();
				void ClearItems();

				virtual void Open( unsigned int iPos );
				virtual void Close();

				virtual bool IsMenuComponent(){ return true; }
				virtual void CloseMenus();

				bool IconMarginDisabled() { return m_bDisableIconMargin; }
				void SetDisableIconMargin( bool bDisable ) { m_bDisableIconMargin = bDisable; }

				bool DeleteOnClose() { return m_bDeleteOnClose; }
				void SetDeleteOnClose( bool b ) { m_bDeleteOnClose = b; }


			protected:

				virtual bool ShouldHoverOpenMenu(){ return true; }
				virtual void OnAddItem( MenuItem* item );
			
				bool m_bDisableIconMargin;
				bool m_bDeleteOnClose;
		};

		class GWEN_EXPORT MenuDivider : public Base
		{
			public:

				GWEN_CONTROL_INLINE( MenuDivider, Base )
				{
					SetHeight( 1 );
				}

				void Render( Gwen::Skin::Base* skin );
		};
	}

}
#endif

/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/ComboBox.h"
#include "Gwen/Controls/Menu.h"


using namespace Gwen;
using namespace Gwen::Controls;
using namespace Gwen::ControlsInternal;

class GWEN_EXPORT DownArrow : public Controls::Base
{
	public:

		GWEN_CONTROL_INLINE( DownArrow, Controls::Base )
		{
			SetMouseInputEnabled( false );
			SetSize( 15, 15 );
		}

		void Render( Skin::Base* skin )
		{
			skin->DrawComboDownArrow( this, m_ComboBox->IsHovered(), m_ComboBox->IsDepressed(), m_ComboBox->IsMenuOpen(), m_ComboBox->IsDisabled() );
		}

		void SetComboBox( ComboBox* p ){ m_ComboBox = p; }

	protected:

		ComboBox*	m_ComboBox;
};

GWEN_CONTROL_CONSTRUCTOR( ComboBox )
{
	SetSize( 100, 20 );

	m_Menu = new Menu( this );
	m_Menu->SetHidden( true );
	m_Menu->SetDisableIconMargin( true );
	m_Menu->SetTabable( false );

	DownArrow* pArrow = new DownArrow( this );
	pArrow->SetComboBox( this );
	m_Button = pArrow;

	m_SelectedItem = NULL;

	SetAlignment( Gwen::Pos::Left | Gwen::Pos::CenterV );
	SetText( L"" );
	SetMargin( Margin( 3, 0, 0, 0 ) );

	SetTabable( true );
	SetKeyboardInputEnabled( true );

}

MenuItem* ComboBox::AddItem( const UnicodeString& strLabel, const String& strName )
{
	MenuItem* pItem = m_Menu->AddItem( strLabel, L"" );
	pItem->SetName( strName );

	pItem->onMenuItemSelected.Add( this, &ComboBox::OnItemSelected );

	//Default
	if ( m_SelectedItem == NULL )
		OnItemSelected( pItem );

	return pItem;
}

void ComboBox::Render( Skin::Base* skin )
{
	skin->DrawComboBox( this, IsDepressed(), IsMenuOpen());
}

void ComboBox::Layout( Skin::Base* skin )
{
	m_Button->Position( Pos::Right | Pos::CenterV, 4, 0 );

	BaseClass::Layout( skin );
}

void ComboBox::OnPress()
{
	if ( IsMenuOpen() )
	{
		return GetCanvas()->CloseMenus();
	}

	bool bWasMenuHidden = m_Menu->Hidden();

	GetCanvas()->CloseMenus();

	if ( bWasMenuHidden )
	{
		OpenList();
	}
}

void ComboBox::ClearItems()
{
	if ( m_Menu )
	{
		m_Menu->ClearItems();
	}
}
void ComboBox::OnItemSelected( Controls::Base* pControl )
{
	//Convert selected to a menu item
	MenuItem* pItem = gwen_cast<MenuItem>( pControl );
	if ( !pItem ) return;

	m_SelectedItem = pItem;
	SetText( m_SelectedItem->GetText() );
	m_Menu->SetHidden( true );

	onSelection.Call( this );

	Focus();
	Invalidate();
}

void ComboBox::OnLostKeyboardFocus()
{
	SetTextColor( Color( 0, 0, 0, 255 ) );
}


void ComboBox::OnKeyboardFocus()
{
	//Until we add the blue highlighting again
	SetTextColor( Color( 0, 0, 0, 255 ) );
	//m_SelectedText->SetTextColor( Color( 255, 255, 255, 255 ) );
}

Gwen::Controls::Label* ComboBox::GetSelectedItem()
{	
	return m_SelectedItem;
}

bool ComboBox::IsMenuOpen()
{
	return m_Menu && !m_Menu->Hidden();
}

void ComboBox::OpenList()
{
	if ( !m_Menu ) return;

	m_Menu->SetParent( GetCanvas() );
	m_Menu->SetHidden( false );
	m_Menu->BringToFront();

	Gwen::Point p = LocalPosToCanvas( Gwen::Point( 0, 0 ) );

	m_Menu->SetBounds( Gwen::Rect ( p.x, p.y + Height(), Width(), m_Menu->Height()) );
}

void ComboBox::CloseList()
{
	if ( !m_Menu ) return;

	m_Menu->Hide();
}


bool ComboBox::OnKeyUp( bool bDown )
{
	if ( bDown )
	{
		Base::List& children = m_Menu->GetChildren();

		Base::List::reverse_iterator it = std::find( children.rbegin(), children.rend(), m_SelectedItem );
		if ( it != children.rend() && ( ++it != children.rend() ) )
		{
			Base* pUpElement = *it;
			OnItemSelected( pUpElement );
		}
	}
	return true;
}
bool ComboBox::OnKeyDown( bool bDown )
{
	if ( bDown )
	{
		Base::List& children = m_Menu->GetChildren();

		Base::List::iterator it = std::find( children.begin(), children.end(), m_SelectedItem );
		if ( it != children.end() && ( ++it != children.end() ) )
		{
			Base* pDownElement = *it;
			OnItemSelected( pDownElement );
		}
	}
	return true;
}

void ComboBox::RenderFocus( Gwen::Skin::Base* /*skin*/ )
{
}
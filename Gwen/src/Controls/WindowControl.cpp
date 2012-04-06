/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/ImagePanel.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/Modal.h"

using namespace Gwen;
using namespace Gwen::Controls;
using namespace Gwen::ControlsInternal;

class GWEN_EXPORT Gwen::Controls::CloseButton : public Button
{
	GWEN_CONTROL_INLINE( CloseButton, Button ){}

	virtual void Render( Skin::Base* skin )
	{
		skin->DrawWindowCloseButton( this, IsDepressed() && IsHovered(), IsHovered() && ShouldDrawHover(), !m_pWindow->IsOnTop() );
	}

	virtual void SetWindow( WindowControl* p ) { m_pWindow = p; }

	WindowControl* m_pWindow;
};


GWEN_CONTROL_CONSTRUCTOR( WindowControl )
{
	m_Modal = NULL;
	m_bDeleteOnClose = false;

	m_TitleBar = new Dragger( this );
	m_TitleBar->SetHeight( 24 );
	m_TitleBar->SetPadding( Padding( 0, 0, 0, 0 ) );
	m_TitleBar->SetMargin( Margin( 0, 0, 0, 4 ) );
	m_TitleBar->SetTarget( this );
	m_TitleBar->Dock( Pos::Top );

	m_Title = new Label( m_TitleBar );
	m_Title->SetAlignment( Pos::Left | Pos::CenterV );
	m_Title->SetText( "Window" );
	m_Title->Dock( Pos::Fill );
	m_Title->SetPadding( Padding( 8, 0, 0, 0 ) );
	m_Title->SetTextColor( GetSkin()->Colors.Window.TitleInactive );

	m_CloseButton = new CloseButton( m_TitleBar );
	m_CloseButton->SetText( "" );
	m_CloseButton->SetSize( 24, 24 );
	m_CloseButton->Dock( Pos::Right );
	m_CloseButton->onPress.Add( this, &WindowControl::CloseButtonPressed );
	m_CloseButton->SetTabable( false );
	m_CloseButton->SetName( "closeButton" );
	m_CloseButton->SetWindow( this );

	//Create a blank content control, dock it to the top - Should this be a ScrollControl?
	m_InnerPanel = new Base( this );
	m_InnerPanel->Dock( Pos::Fill );

	GetResizer( 8 )->Hide();

	BringToFront();

	SetTabable( false );
	Focus();

	SetMinimumSize( Gwen::Point( 100, 40 ) );
	SetClampMovement( true );
	SetKeyboardInputEnabled( false );
}


WindowControl::~WindowControl()
{
	DestroyModal();
}

void WindowControl::MakeModal( bool bDrawBackground )
{
	if ( m_Modal ) return;

	m_Modal = new ControlsInternal::Modal( GetCanvas() );
	SetParent( m_Modal );

	m_Modal->SetShouldDrawBackground( bDrawBackground );
}

void WindowControl::DestroyModal()
{
	if ( !m_Modal ) return;

	// Really should be restoring our parent here.. but we don't know who it is.
	// Assume it's the canvas.
	SetParent( GetCanvas() );
	
	m_Modal->DelayedDelete();
	m_Modal = NULL;
}

bool WindowControl::IsOnTop()
{
	for (Base::List::reverse_iterator iter = GetParent()->Children.rbegin(); iter != GetParent()->Children.rend(); ++iter)
	{
		WindowControl* pWindow = gwen_cast<WindowControl>(*iter);

		if ( !pWindow )
			continue;

		if ( pWindow == this )
			return true;

		return false;
	}

	return false;

}

void WindowControl::Render( Skin::Base* skin )
{
	bool bHasFocus = IsOnTop();

	if ( bHasFocus )
		m_Title->SetTextColor( GetSkin()->Colors.Window.TitleActive );
	else
		m_Title->SetTextColor( GetSkin()->Colors.Window.TitleInactive );

	skin->DrawWindow( this, m_TitleBar->Bottom(), bHasFocus );
}

void WindowControl::RenderUnder( Skin::Base* skin )
{
	BaseClass::RenderUnder( skin );
	skin->DrawShadow( this );
}

void WindowControl::SetTitle(Gwen::UnicodeString title)
{
	m_Title->SetText( title );
}

void WindowControl::SetClosable( bool closeable )
{
	m_CloseButton->SetHidden( !closeable );
}

void WindowControl::SetHidden( bool hidden )
{
	if ( !hidden )
		BringToFront();

	BaseClass::SetHidden(hidden);
}

void WindowControl::Touch()
{
	BaseClass::Touch();
	BringToFront();
}

void WindowControl::CloseButtonPressed( Gwen::Controls::Base* /*pFromPanel*/ )
{
	DestroyModal();

	SetHidden( true );

	if ( m_bDeleteOnClose )
		DelayedDelete();
}


void WindowControl::RenderFocus( Gwen::Skin::Base* /*skin*/ )
{

}
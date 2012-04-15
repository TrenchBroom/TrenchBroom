/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/ListBox.h"
#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/InputHandler.h"

using namespace Gwen;
using namespace Gwen::Controls;

class ListBoxRow : public Layout::TableRow
{
	GWEN_CONTROL_INLINE( ListBoxRow, Layout::TableRow )
	{
		SetMouseInputEnabled( true );
		SetSelected( false );
	}

	void Render( Skin::Base* skin )
	{
		skin->DrawListBoxLine( this, IsSelected(), GetEven() );
	}

	bool IsSelected() const
	{
		return m_bSelected;
	}

	void OnMouseClickLeft( int /*x*/, int /*y*/, bool bDown )
	{
		if ( bDown )
		{
			SetSelected( true );
			onRowSelected.Call( this );
		}
	}

	void SetSelected( bool b )
	{
		m_bSelected = b;

		// TODO: Get these values from the skin.
		if ( b )
			SetTextColor( Gwen::Colors::White );
		else
			SetTextColor( Gwen::Colors::Black );
	}

	private:

	bool			m_bSelected;
	
};

GWEN_CONTROL_CONSTRUCTOR( ListBox )
{
	SetScroll( false, true );
	SetAutoHideBars( true );
	SetMargin( Margin( 1, 1, 1, 1 ) );

	m_Table = new Controls::Layout::Table( this );
	m_Table->Dock( Pos::Top );
	m_Table->SetColumnCount( 1 );

	m_bMultiSelect = false;
}

Layout::TableRow* ListBox::AddItem( const String& strLabel, const String& strName )
{
	return AddItem( Utility::StringToUnicode( strLabel ), strName );
}

Layout::TableRow* ListBox::AddItem( const UnicodeString& strLabel, const String& strName )
{
	ListBoxRow* pRow = new ListBoxRow( this );
	m_Table->AddRow( pRow );

	pRow->SetCellText( 0, strLabel );
	pRow->SetName( strName );

	pRow->onRowSelected.Add( this, &ListBox::OnRowSelected );

	m_Table->SizeToContents();

	return pRow;
}

void ListBox::RemoveItem( Layout::TableRow * row )
{
	m_SelectedRows.erase( std::find( m_SelectedRows.begin(), m_SelectedRows.end(), row ) );
	m_Table->Remove( row );
}

void ListBox::Render( Skin::Base* skin )
{
	skin->DrawListBox( this );
}

void ListBox::UnselectAll()
{
	std::list<Layout::TableRow*>::iterator it = m_SelectedRows.begin();
	while ( it != m_SelectedRows.end() )
	{
		ListBoxRow* pRow = static_cast<ListBoxRow*>(*it);
		it = m_SelectedRows.erase( it );

		pRow->SetSelected( false );
	}
}

void ListBox::OnRowSelected( Base* pControl )
{
	bool bClear = !Gwen::Input::IsShiftDown();
	if ( !AllowMultiSelect() ) bClear = true;

	SetSelectedRow( pControl, bClear );
}

Layout::TableRow* ListBox::GetSelectedRow()
{ 
	if ( m_SelectedRows.empty() ) return NULL;

	return *m_SelectedRows.begin();
}

void ListBox::Clear()
{
	UnselectAll();
	m_Table->Clear();
}

void ListBox::SetSelectedRow( Gwen::Controls::Base* pControl, bool bClearOthers )
{
	if ( bClearOthers )
		UnselectAll();

	ListBoxRow* pRow = gwen_cast<ListBoxRow>( pControl );
	if ( !pRow ) return;

	// TODO: make sure this is one of our rows!

	pRow->SetSelected( true );
	m_SelectedRows.push_back( pRow );
	onRowSelected.Call( this );
}



void ListBox::SelectByString( const TextObject& strName, bool bClearOthers )
{
	if ( bClearOthers )
		UnselectAll();

	Base::List& children = m_Table->GetChildren();

	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		ListBoxRow* pChild = gwen_cast<ListBoxRow>(*iter);
		if ( !pChild ) continue;

		if ( Utility::Strings::Wildcard( strName, pChild->GetText( 0 ) ) )
			SetSelectedRow( pChild, false );
	}
}
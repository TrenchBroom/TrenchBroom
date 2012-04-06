/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/Properties.h"
#include "Gwen/Utility.h"

using namespace Gwen;
using namespace Gwen::Controls;

GWEN_CONTROL_CONSTRUCTOR( Properties )
{
	m_SplitterBar = new SplitterBar( this );
	m_SplitterBar->SetPos( 80, 0 );
	m_SplitterBar->SetCursor( Gwen::CursorType::SizeWE );
	m_SplitterBar->onDragged.Add( this, &Properties::OnSplitterMoved );
	m_SplitterBar->SetShouldDrawBackground( false );
}

void Properties::PostLayout( Gwen::Skin::Base* /*skin*/ )
{
	m_SplitterBar->SetHeight( 0 );

	if ( SizeToChildren( false, true ) )
	{
		InvalidateParent();
	}

	m_SplitterBar->SetSize( 3, Height() );
}

void Properties::OnSplitterMoved( Controls::Base * /*control*/ )
{
	InvalidateChildren();
}

int Properties::GetSplitWidth()
{
	return m_SplitterBar->X();
}

PropertyRow* Properties::Add( const TextObject& text, const TextObject& value )
{
	return Add( text, new Property::Text( this ), value );
}

PropertyRow* Properties::Add( const TextObject& text, Property::Base* pProp, const TextObject& value )
{
	PropertyRow* row = new PropertyRow( this );
	row->Dock( Pos::Top );
	row->GetLabel()->SetText( text );
	row->SetProperty( pProp );

	pProp->SetPropertyValue( value, true );

	m_SplitterBar->BringToFront();
	return row;
}

void Properties::Clear()
{
	Base::List ChildListCopy = Children;
	for ( Base::List::iterator it = ChildListCopy.begin(); it != ChildListCopy.end(); ++it )
	{
		PropertyRow* row = gwen_cast<PropertyRow>(*it);
		if ( !row ) continue;

		row->DelayedDelete();
	}
}

class PropertyRowLabel : public Label 
{
	GWEN_CONTROL_INLINE ( PropertyRowLabel, Label )
	{
		SetAlignment( Pos::Left | Pos::CenterV );
		m_pPropertyRow = NULL;
	}

	void UpdateColours()
	{
		if ( IsDisabled() )										return SetTextColor( GetSkin()->Colors.Button.Disabled );
		if ( m_pPropertyRow && m_pPropertyRow->IsEditing() )	return SetTextColor( GetSkin()->Colors.Properties.Label_Selected );
		if ( m_pPropertyRow && m_pPropertyRow->IsHovered() )	return SetTextColor( GetSkin()->Colors.Properties.Label_Hover );

		SetTextColor( GetSkin()->Colors.Properties.Label_Normal );
	}

	void SetPropertyRow( PropertyRow * p ){ m_pPropertyRow = p; }

protected:

	PropertyRow*	m_pPropertyRow;
};


GWEN_CONTROL_CONSTRUCTOR( PropertyRow )
{
	m_Property = NULL;

	PropertyRowLabel* pLabel = new PropertyRowLabel( this );
	pLabel->SetPropertyRow( this );
	pLabel->Dock( Pos::Left );
	pLabel->SetAlignment( Pos::Left | Pos::Top );
	pLabel->SetMargin( Margin( 2, 2, 0, 0 ) );
	m_Label = pLabel;
}	

void PropertyRow::Render( Gwen::Skin::Base* skin )
{
	/* SORRY */
	if ( IsEditing() != m_bLastEditing )
	{
		OnEditingChanged();
		m_bLastEditing = IsEditing();
	}

	if ( IsHovered() != m_bLastHover )
	{
		OnHoverChanged();
		m_bLastHover = IsHovered();
	}
	/* SORRY */

	skin->DrawPropertyRow( this, m_Label->Right(), IsEditing(), IsHovered() | m_Property->IsHovered() );
}

void PropertyRow::Layout( Gwen::Skin::Base* /*skin*/ )
{
	Properties* pParent = gwen_cast<Properties>( GetParent() );
	if ( !pParent ) return;

	m_Label->SetWidth( pParent->GetSplitWidth() );

	if ( m_Property )
	{
		SetHeight( m_Property->Height() );
	}
}

void PropertyRow::SetProperty( Property::Base* prop )
{
	m_Property = prop;
	m_Property->SetParent( this );
	m_Property->Dock( Pos::Fill );
	m_Property->onChange.Add( this, &ThisClass::OnPropertyValueChanged );
}

void PropertyRow::OnPropertyValueChanged( Gwen::Controls::Base* /*control*/ )
{
	onChange.Call( this );
}

void PropertyRow::OnEditingChanged()
{
	m_Label->Redraw();
}

void PropertyRow::OnHoverChanged()
{
	m_Label->Redraw();
}
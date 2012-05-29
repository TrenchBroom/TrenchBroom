/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/Properties.h"
#include "Gwen/Utility.h"
#include <algorithm>
#include <vector>

using namespace Gwen;
using namespace Gwen::Controls;

GWEN_CONTROL_CONSTRUCTOR( Properties )
{
	m_SplitterBar = new SplitterBar( this );
	m_SplitterBar->SetPos( 80, 0 );
	m_SplitterBar->SetCursor( Gwen::CursorType::SizeWE );
	m_SplitterBar->onDragged.Add( this, &Properties::OnSplitterMoved );
	m_SplitterBar->SetShouldDrawBackground( false );
    m_sorted = false;
}

bool Properties::CompareControls::operator() (Gwen::Controls::Base* first, Gwen::Controls::Base* second) {
    PropertyRow* firstRow = gwen_cast<PropertyRow>(first);
    PropertyRow* secondRow = gwen_cast<PropertyRow>(second);
    
    if (firstRow == NULL && secondRow == NULL)
        return false;
    
    if (firstRow == NULL)
        return false;
    
    if (secondRow == NULL)
        return true;
    
    return firstRow->GetLabel()->GetText().compare(secondRow->GetLabel()->GetText()) <= 0;
}

Base::List Properties::GetChildrenForLayout()
{
    if (!m_sorted)
        return BaseClass::GetChildrenForLayout();
    
    std::vector<Gwen::Controls::Base*> SortedChildren;
    CompareControls compare;
    
    SortedChildren.insert(SortedChildren.begin(), Children.begin(), Children.end());
    std::sort(SortedChildren.begin(), SortedChildren.end(), compare);
    
    Base::List Result;
    Result.insert(Result.begin(), SortedChildren.begin(), SortedChildren.end());
    return Result;
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

void Properties::SetSorted(bool sorted)
{
    if (m_sorted == sorted)
        return;
    
    m_sorted = sorted;
    Invalidate();
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
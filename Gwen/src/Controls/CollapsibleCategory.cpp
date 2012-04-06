/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#include "Gwen/Controls/CollapsibleCategory.h"
#include "Gwen/Controls/CollapsibleList.h"

using namespace Gwen;
using namespace Gwen::Controls;

class CategoryButton : public Button 
{
	GWEN_CONTROL_INLINE ( CategoryButton, Button )
	{
		SetAlignment( Pos::Left | Pos::CenterV );
		m_bAlt = false;
	}

	virtual void Render( Skin::Base* skin )
	{
		if ( m_bAlt )
		{
			if ( IsDepressed() || GetToggleState() ) skin->GetRender()->SetDrawColor( skin->Colors.Category.LineAlt.Button_Selected );
			else if ( IsHovered() ) skin->GetRender()->SetDrawColor( skin->Colors.Category.LineAlt.Button_Hover );
			else skin->GetRender()->SetDrawColor( skin->Colors.Category.LineAlt.Button );
		}
		else
		{
			if ( IsDepressed() || GetToggleState() ) skin->GetRender()->SetDrawColor( skin->Colors.Category.Line.Button_Selected );
			else if ( IsHovered() ) skin->GetRender()->SetDrawColor( skin->Colors.Category.Line.Button_Hover );
			else skin->GetRender()->SetDrawColor( skin->Colors.Category.Line.Button );
		}
	
		skin->GetRender()->DrawFilledRect( this->GetRenderBounds() );
	}

	void UpdateColours()
	{
		if ( m_bAlt )
		{
			if ( IsDepressed() || GetToggleState() )	return SetTextColor( GetSkin()->Colors.Category.LineAlt.Text_Selected );
			if ( IsHovered() )							return SetTextColor( GetSkin()->Colors.Category.LineAlt.Text_Hover );

			return SetTextColor( GetSkin()->Colors.Category.LineAlt.Text );
		}

		if ( IsDepressed() || GetToggleState() )	return SetTextColor( GetSkin()->Colors.Category.Line.Text_Selected );
		if ( IsHovered() )							return SetTextColor( GetSkin()->Colors.Category.Line.Text_Hover );

		return SetTextColor( GetSkin()->Colors.Category.Line.Text );
	}

	bool m_bAlt;
};

class CategoryHeaderButton : public Button 
{
	GWEN_CONTROL_INLINE ( CategoryHeaderButton, Button )
	{
		SetShouldDrawBackground( false );
		SetIsToggle( true );
		SetAlignment( Pos::Center );
	}

	void UpdateColours()
	{
		if ( IsDepressed() || GetToggleState() )	return SetTextColor( GetSkin()->Colors.Category.Header_Closed );
		return SetTextColor( GetSkin()->Colors.Category.Header );
	}

};


GWEN_CONTROL_CONSTRUCTOR( CollapsibleCategory )
{
	m_pList = NULL;

	m_pButton = new CategoryHeaderButton( this );
	m_pButton->SetText( "Category Title" );
	m_pButton->Dock( Pos::Top );
	m_pButton->SetHeight( 20 );

	SetPadding( Padding( 1, 0, 1, 5 ) );
	SetSize( 512, 512 );
}

Button* CollapsibleCategory::Add( const TextObject& name )
{
	CategoryButton* pButton = new CategoryButton( this );
	pButton->SetText( name );
	pButton->Dock( Pos::Top );
	pButton->SizeToContents();
	pButton->SetSize( pButton->Width() + 4, pButton->Height() + 4 );
	pButton->SetPadding( Padding( 5, 2, 2, 2 ) );
	pButton->onPress.Add( this, &ThisClass::OnSelection );

	return pButton;
}

void CollapsibleCategory::OnSelection( Controls::Base* control )
{
	CategoryButton* pChild = gwen_cast<CategoryButton>(control);
	if ( !pChild ) return;

	if ( m_pList )
	{
		m_pList->UnselectAll();
	}
	else
	{
		UnselectAll();
	}
	
	pChild->SetToggleState( true );

	onSelection.Call( this );

}

void CollapsibleCategory::Render( Skin::Base* skin )
{
	skin->DrawCategoryInner( this, m_pButton->GetToggleState() );
}

void CollapsibleCategory::SetText( const TextObject& text )
{
	m_pButton->SetText( text );
}

void CollapsibleCategory::UnselectAll()
{
	Base::List& children = GetChildren();
	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		CategoryButton* pChild = gwen_cast<CategoryButton>(*iter);
		if ( !pChild ) continue;

		pChild->SetToggleState( false );
	}
}

void CollapsibleCategory::PostLayout( Skin::Base* /*skin*/ )
{
	if ( m_pButton->GetToggleState() )
	{
		SetHeight( m_pButton->Height() );
	}
	else
	{
		SizeToChildren( false, true );
	}

	Base::List& children = GetChildren();
	bool b = true;
	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		CategoryButton* pChild = gwen_cast<CategoryButton>(*iter);
		if ( !pChild ) continue;

		pChild->m_bAlt = b;
		pChild->UpdateColours();
		b = !b;
	}
};

Button* CollapsibleCategory::GetSelected()
{
	Base::List& children = GetChildren();
	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		CategoryButton* pChild = gwen_cast<CategoryButton>(*iter);
		if ( !pChild ) continue;

		if ( pChild->GetToggleState() )
			return pChild;
	}

	return NULL;
}
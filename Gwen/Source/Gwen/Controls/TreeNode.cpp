/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/TreeNode.h"
#include "Gwen/Controls/TreeControl.h"
#include "Gwen/Utility.h"

using namespace Gwen;
using namespace Gwen::Controls;

class OpenToggleButton : public Button 
{
	GWEN_CONTROL_INLINE ( OpenToggleButton, Button )
	{
		SetIsToggle( true );
		SetTabable( false );

	}

	virtual void RenderFocus( Skin::Base* /*skin*/ ) {}

	virtual void Render( Skin::Base* skin )
	{
		skin->DrawTreeButton( this, GetToggleState() );
	}
};

class TreeNodeText : public Button 
{
	GWEN_CONTROL_INLINE ( TreeNodeText, Button )
	{
		SetAlignment( Pos::Left | Pos::CenterV );
		SetShouldDrawBackground( false );
		SetHeight( 16 );
	}

	void UpdateColours()
	{
		if ( IsDisabled() )							return SetTextColor( GetSkin()->Colors.Button.Disabled );
		if ( IsDepressed() || GetToggleState() )	return SetTextColor( GetSkin()->Colors.Tree.Selected );
		if ( IsHovered() )							return SetTextColor( GetSkin()->Colors.Tree.Hover );

		SetTextColor( GetSkin()->Colors.Tree.Normal );
	}
};

const int TreeIndentation = 14;

GWEN_CONTROL_CONSTRUCTOR( TreeNode )
{
	m_TreeControl = NULL;

	m_ToggleButton = new OpenToggleButton( this );
	m_ToggleButton->SetBounds( 0, 0, 15, 15 );
	m_ToggleButton->onToggle.Add( this, &TreeNode::OnToggleButtonPress );

	m_Title = new TreeNodeText( this );
	m_Title->Dock( Pos::Top );
	m_Title->SetMargin( Margin( 16, 0, 0, 0 ) );
	m_Title->onDoubleClick.Add( this, &TreeNode::OnDoubleClickName );
	m_Title->onDown.Add( this, &TreeNode::OnClickName );
	m_Title->onRightPress.Add( this, &TreeNode::OnRightPress );

	m_InnerPanel = new Base( this );
	m_InnerPanel->Dock( Pos::Top );
	m_InnerPanel->SetHeight( 100 );
	m_InnerPanel->SetMargin( Margin( TreeIndentation, 1, 0, 0 ) );
	m_InnerPanel->Hide();

	m_bRoot = false;
	m_bSelected = false;
	m_bSelectable = true;
}

void TreeNode::Render( Skin::Base* skin )
{
	int iBottom = 0;
	if ( m_InnerPanel->Children.size() > 0 )
	{
		iBottom = m_InnerPanel->Children.back()->Y() + m_InnerPanel->Y();
	}
	
	skin->DrawTreeNode( this, m_InnerPanel->Visible(), IsSelected(), m_Title->Height(), m_Title->TextRight(), m_ToggleButton->Y() + m_ToggleButton->Height() * 0.5, iBottom, GetParent() == m_TreeControl );
}

TreeNode* TreeNode::AddNode( const UnicodeString& strLabel )
{
	TreeNode* node = new TreeNode( this );
	node->SetText( strLabel );
	node->Dock( Pos::Top );
	node->SetRoot( gwen_cast<TreeControl>( this ) != NULL );
	node->SetTreeControl( m_TreeControl );

	if ( m_TreeControl )
	{
		m_TreeControl->OnNodeAdded( node );
	}

	return node;
}

TreeNode* TreeNode::AddNode( const String& strLabel )
{
	return AddNode( Utility::StringToUnicode( strLabel ) );
}


void TreeNode::Layout( Skin::Base* skin )
{
	if ( m_ToggleButton )
	{
		if ( m_Title )
		{
			m_ToggleButton->SetPos( 0, (m_Title->Height() - m_ToggleButton->Height()) * 0.5 );
		}

		if ( m_InnerPanel->NumChildren() == 0 )
		{
			m_ToggleButton->Hide();
			m_ToggleButton->SetToggleState( false );
			m_InnerPanel->Hide();
		}
		else
		{
			m_ToggleButton->Show();
			m_InnerPanel->SizeToChildren( false, true );
		}
	}

	BaseClass::Layout( skin );
}

void TreeNode::PostLayout( Skin::Base* /*skin*/ )
{
	if ( SizeToChildren( false, true ) )
	{
		InvalidateParent();
	}
}

void TreeNode::SetText( const UnicodeString& text ){ m_Title->SetText( text ); };
void TreeNode::SetText( const String& text ){ m_Title->SetText( text ); };

void TreeNode::Open()
{
	m_InnerPanel->Show();
	if ( m_ToggleButton ) m_ToggleButton->SetToggleState( true );
	Invalidate();
}

void TreeNode::Close()
{
	m_InnerPanel->Hide();
	if ( m_ToggleButton ) m_ToggleButton->SetToggleState( false );
	Invalidate();
}

void TreeNode::ExpandAll()
{
	Open();

	Base::List& children = m_InnerPanel->GetChildren();
	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		TreeNode* pChild = gwen_cast<TreeNode>(*iter);
		if ( !pChild ) continue;

		pChild->ExpandAll();
	}
}

Button* TreeNode::GetButton(){ return m_Title; }


void TreeNode::OnToggleButtonPress( Base* /*control*/ )
{
	if ( m_ToggleButton->GetToggleState() )
	{
		Open();
	}
	else
	{
		Close();
	}
}

void TreeNode::OnDoubleClickName( Base* /*control*/ )
{
	if ( !m_ToggleButton->Visible() ) return;

	m_ToggleButton->Toggle();
}

void TreeNode::OnClickName( Base* /*control*/ )
{
	onNamePress.Call( this );

	SetSelected( !IsSelected() );
}

void TreeNode::OnRightPress( Base* control )
{
	onRightPress.Call( this );
}

void TreeNode::SetSelected( bool b )
{ 
	if ( !m_bSelectable ) return;
	if ( m_bSelected == b ) return;

	m_bSelected = b; 

	if ( m_Title )
		m_Title->SetToggleState( m_bSelected );

	onSelectChange.Call( this );

	if ( m_bSelected )
	{
		onSelect.Call( this );
	}
	else
	{
		onUnselect.Call( this );
	}

	Redraw();
}

void TreeNode::DeselectAll()
{
	m_bSelected = false;
	
	if ( m_Title )
		m_Title->SetToggleState( m_bSelected );

	Base::List& children = m_InnerPanel->GetChildren();
	for ( Base::List::iterator iter = children.begin(); iter != children.end(); ++iter )
	{
		TreeNode* pChild = gwen_cast<TreeNode>(*iter);
		if ( !pChild ) continue;

		pChild->DeselectAll( );
	}
}


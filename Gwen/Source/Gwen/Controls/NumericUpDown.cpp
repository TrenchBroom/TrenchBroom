/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Gwen.h"
#include "Gwen/Utility.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/NumericUpDown.h"
#include "Gwen/Controls/Layout/Splitter.h"

using namespace Gwen;
using namespace Gwen::Controls;



GWEN_CONTROL_CONSTRUCTOR( NumericUpDown )
{
	SetSize( 100, 20 );

	Layout::Splitter* pSplitter = new Layout::Splitter( this );
		pSplitter->Dock( Pos::Right );
		pSplitter->SetSize( 13, 13 );

	NumericUpDownButton_Up* pButtonUp = new NumericUpDownButton_Up( pSplitter );
		pButtonUp->onPress.Add( this, &NumericUpDown::OnButtonUp );
		pButtonUp->SetTabable( false );

		pSplitter->SetPanel( 0, pButtonUp, false );
		

	NumericUpDownButton_Down* pButtonDown = new NumericUpDownButton_Down( pSplitter );
		pButtonDown->onPress.Add( this, &NumericUpDown::OnButtonDown );
		pButtonDown->SetTabable( false );
		pButtonUp->SetPadding( Padding( 0, 1, 1, 0 ) );

		pSplitter->SetPanel( 1, pButtonDown, false );

	m_fMax = std::numeric_limits<float>::max();
	m_fMin = -std::numeric_limits<float>::max();
	m_fNumber = 0;
    m_fIncrement = 1;
    m_bHasValue = false;
	SetText( "");
}

void NumericUpDown::OnButtonUp( Base* /*control*/ )
{
    if (!m_bHasValue) return;
	SyncNumberFromText();
	SetValue( m_fNumber + m_fIncrement );
}

void NumericUpDown::OnButtonDown( Base* /*control*/ )
{
	SyncNumberFromText();
	SetValue( m_fNumber - m_fIncrement );
}


void NumericUpDown::SyncTextFromNumber()
{
    if (!m_bHasValue) {
        SetText( "" );
    } else {
        SetText( Utility::ToString( m_fNumber ) );
    }
}

void NumericUpDown::SyncNumberFromText()
{
	SetValue( GetFloatFromText() );
}

void NumericUpDown::SetMin( float f )
{
	m_fMin = f;
}

void NumericUpDown::SetMax( float f )
{
	m_fMax = f;
}

void NumericUpDown::SetValue( float f )
{
	if ( f > m_fMax ) f = m_fMax;
	if ( f < m_fMin ) f = m_fMin;

	if ( m_fNumber == f )
	{		
		return;
	}

	m_fNumber = f;

	// Don't update the text if we're typing in it..
	// Undone - any reason why not?
	//if ( !HasFocus() )
	{
		SyncTextFromNumber();
	}

	OnChange();
}

float NumericUpDown::GetValue() {
    if (!m_bHasValue)
        return std::numeric_limits<float>::quiet_NaN();
    return m_fNumber;
}

void NumericUpDown::SetIncrement( float f )
{
    m_fIncrement = f;
}

void NumericUpDown::SetHasValue( bool b ) {
    if (m_bHasValue == b) return;
    m_bHasValue = b;
    SyncTextFromNumber();
}

void NumericUpDown::OnChange()
{
    if (m_bHasValue)
        onChanged.Call( this );
}

void NumericUpDown::OnTextChanged()
{
	BaseClass::OnTextChanged();

	SyncNumberFromText();
}

void NumericUpDown::OnEnter()
{
	SyncNumberFromText();
	SyncTextFromNumber();
}


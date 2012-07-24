/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_CONTROLS_PROPERTY_CHECKBOX_H
#define GWEN_CONTROLS_PROPERTY_CHECKBOX_H

#include "Gwen/Controls/Property/BaseProperty.h"
#include "Gwen/Controls/CheckBox.h"

namespace Gwen 
{
	namespace Controls
	{
		namespace Property
		{
			class Checkbox : public Property::Base
			{
				public:

					GWEN_CONTROL_INLINE( Checkbox, Property::Base )
					{
						m_Checkbox = new Gwen::Controls::CheckBox( this );
						m_Checkbox->SetShouldDrawBackground( false );
						m_Checkbox->onCheckChanged.Add( this, &BaseClass::OnContentChanged );
						m_Checkbox->SetTabable( true );
						m_Checkbox->SetKeyboardInputEnabled( true );
						m_Checkbox->SetPos( 2, 1 );
                        m_Checkbox->onHoverEnter.Add(this, &BaseClass::OnChildHoverEnter);
                        m_Checkbox->onHoverLeave.Add(this, &BaseClass::OnChildHoverLeave);

						SetHeight( 18 );
					}

					virtual UnicodeString GetContent()
					{
						return m_Checkbox->IsChecked() ? L"1" : L"0";
					}

					virtual void SetContent( const TextObject& v, bool bFireChangeEvents )
					{
						if ( v == L"1" || v == L"true" || v == L"TRUE" || v == L"yes" || v == L"YES" )
							return m_Checkbox->SetChecked( true );

						return m_Checkbox->SetChecked( false );
					}

					virtual bool IsEditing()
					{
						return m_Checkbox->HasFocus();
					}

					virtual bool IsHovered()
					{
						return BaseClass::IsHovered() || m_Checkbox->IsHovered();
					}

					Gwen::Controls::CheckBox* m_Checkbox;
			};
		}
	}
}
#endif

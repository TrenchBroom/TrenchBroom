/*
 GWEN
 Copyright (c) 2010 Facepunch Studios
 See license in Gwen.h
 */

#pragma once
#ifndef GWEN_CONTROLS_PROPERTY_BASEPROPERTY_H
#define GWEN_CONTROLS_PROPERTY_BASEPROPERTY_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Utility.h"


namespace Gwen 
{
	namespace Controls
	{
		namespace Property
		{
			class Base : public Gwen::Controls::Base
			{
            protected:
                UnicodeString m_oldPropertyValue;
            public:
                
                GWEN_CONTROL_INLINE( Base, Gwen::Controls::Base )
                {
                    SetHeight( 17 );
                }
                
                virtual String GetPropertyValueAnsi()
                {
                    return Gwen::Utility::UnicodeToString( GetPropertyValue() );
                }
                
                virtual UnicodeString GetPropertyValue() = 0;
                
                virtual void SetPropertyValue( const TextObject& v, bool bFireChangeEvents = false ) = 0;
                
                virtual bool IsEditing() = 0;
                
                virtual void DoChanged()
                {
                    if (GetPropertyValue() != m_oldPropertyValue) {
                        onChange.Call( this );
                        m_oldPropertyValue = GetPropertyValue();
                    }
                }
                
                void OnBeginEditingPropertyValue (Gwen::Controls::Base* /*control*/ )
                {
                    m_oldPropertyValue = GetPropertyValue();
                }
                
                void OnPropertyValueChanged( Gwen::Controls::Base* /*control*/ )
                {
                    DoChanged();
                }
                
                virtual void SetPlaceholderString( const TextObject& str) {};
                
                Event::Caller	onChange;
			};
		}
	}
}
#endif

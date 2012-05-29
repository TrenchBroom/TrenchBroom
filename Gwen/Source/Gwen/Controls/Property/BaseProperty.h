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
						onChange.Call( this );
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

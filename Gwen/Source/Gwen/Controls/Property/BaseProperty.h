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
                UnicodeString m_oldContent;
            public:
                
                GWEN_CONTROL_INLINE( Base, Gwen::Controls::Base )
                {
                    SetHeight( 17 );
                }
                
                virtual String GetContentAnsi()
                {
                    return Gwen::Utility::UnicodeToString( GetContent() );
                }
                
                virtual UnicodeString GetContent() = 0;
                
                virtual void SetContent( const TextObject& v, bool bFireChangeEvents = false ) = 0;
                
                virtual bool IsEditing() = 0;
                
                virtual void DoChanged()
                {
                    if (GetContent() != m_oldContent) {
                        onChange.Call( this );
                        m_oldContent = GetContent();
                    }
                }
                
                void OnBeginEditingContent (Gwen::Controls::Base* /*control*/ )
                {
                    m_oldContent = GetContent();
                }
                
                void OnContentChanged( Gwen::Controls::Base* /*control*/ )
                {
                    DoChanged();
                }
                
                virtual void OnChildHoverEnter(Gwen::Controls::Base* control)
                {
                    onHoverEnter.Call(this);
                }
                
                virtual void OnChildHoverLeave(Gwen::Controls::Base* control)
                {
                    onHoverLeave.Call(this);
                }

                virtual void SetPlaceholderString( const TextObject& str) {};
                
                
                Event::Caller	onChange;
			};
		}
	}
}
#endif

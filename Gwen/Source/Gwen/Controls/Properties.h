/*
 GWEN
 Copyright (c) 2010 Facepunch Studios
 See license in Gwen.h
 */

#pragma once
#ifndef GWEN_CONTROLS_PROPERTIES_H
#define GWEN_CONTROLS_PROPERTIES_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/Property/BaseProperty.h"
#include "Gwen/Controls/Property/Text.h"
#include "Gwen/Controls/SplitterBar.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"


namespace Gwen 
{
	namespace Controls
	{
        
		class PropertyRow;
        
		class Properties : public Base
		{
        public:
            
            GWEN_CONTROL( Properties, Base );

            virtual Base::List GetChildrenForLayout();
            
            virtual void PostLayout( Gwen::Skin::Base* skin );
            
            PropertyRow* Add( const TextObject& key, const TextObject& value = L"" );
            PropertyRow* Add( const TextObject& key, Property::Base* pProp, const TextObject& value = L"" );
            
            virtual int GetSplitWidth();
            
            virtual void Clear();
            
            virtual void SetSorted(bool sorted);
            
            virtual void SetShowEmptyRow( bool showEmptyRow );
            
            virtual void Think();
            
            Event::Caller onRowAdd;
        protected:
            
            struct CompareControls {
                bool operator() (Gwen::Controls::Base* first, Gwen::Controls::Base* second);
            };
            
            virtual void OnSplitterMoved( Controls::Base * control );
            
            Controls::SplitterBar*	m_SplitterBar;
            
            bool m_sorted;
            
            Controls::PropertyRow* m_emptyRow;
            Controls::PropertyRow* m_formerEmptyRow;
            void EmptyPropertyChanged( Gwen::Controls::Base* control );
		};
        
		class PropertyRow : public Base
		{
        public:
            
            GWEN_CONTROL( PropertyRow, Base );
            
            virtual void SetKey( const TextObject& text );
            virtual Property::Text* GetKey(){ return m_Key; }
            virtual TextObject& GetOldKey() { return m_OldKey; }
            virtual void SetValue( Property::Base* prop );
            virtual Property::Base* GetValue(){ return m_Value; }
            virtual void SetDeletable(bool deletable);
            virtual bool GetDeletable() { return m_deletable; }
            
            virtual void Layout( Gwen::Skin::Base* skin );
            virtual void Render( Gwen::Skin::Base* skin );
            
            virtual bool IsKeyEditing(){ return m_Key && m_Key->IsEditing(); }
            virtual bool IsKeyHovered(){ return BaseClass::IsHovered() || (m_Key && m_Key->IsHovered()); }
            virtual void OnKeyEditingChanged();
            
            virtual bool IsValueEditing(){ return m_Value && m_Value->IsEditing(); }
            virtual bool IsValueHovered(){ return BaseClass::IsHovered() || (m_Value && m_Value->IsHovered()); }
            virtual void OnValueEditingChanged();
            
            virtual void OnChildHoverEnter(Base* control);
            virtual void OnChildHoverLeave(Base* control);
            
            virtual void Think();
            
            Event::Caller	onKeyChange;
            Event::Caller   onValueChange;
            Event::Caller   onDelete;
            
        protected:
            Button* createDeleteButton();
            
            void OnPropertyKeyChanged( Gwen::Controls::Base* control );
            void OnPropertyValueChanged( Gwen::Controls::Base* control );
            void OnDeleteButtonPressed( Gwen::Controls::Base* control );
            
            bool IsDeleteButtonHovered() { return BaseClass::IsHovered() || (m_deleteButton && m_deleteButton->IsHovered()); }
            
            TextObject m_OldKey;
            Property::Text*	m_Key;
            Property::Base*	m_Value;
            Button* m_deleteButton;
            bool m_removeDeleteButton;
            bool m_deletable;
            
            bool			m_bLastKeyEditing;
            bool            m_bLastValueEditing;
            
		};
	}
}
#endif

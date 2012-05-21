/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#include "ButtonStrip.h"
#include "Gwen/Utility.h"
#include "Gwen/Skin.h"
#include "Gwen/BaseRender.h"

namespace Gwen {
    namespace Controls {
        GWEN_CONTROL_CONSTRUCTOR(StripButton) {
        }

        void StripButton::SetStripPosition(StripButton::Pos position) {
            if (m_position == position)
                return;
            m_position = position;
            Redraw();
        }
        
        StripButton::Pos StripButton::GetStripPosition() {
            return m_position;
        }
        
        void StripButton::Render(Skin::Base* skin) {
            if (!ShouldDrawBackground()) return;
            
            bool bDrawDepressed = IsDepressed() && IsHovered();
            if (IsToggle()) bDrawDepressed = bDrawDepressed || GetToggleState();
            
            bool bDrawHovered = IsHovered() && ShouldDrawHover();
            
            if (m_position == Left)
                skin->DrawLeftStripButton(this, bDrawDepressed, bDrawHovered, IsDisabled());
            else if (m_position == Mid)
                skin->DrawMidStripButton(this, bDrawDepressed, bDrawHovered, IsDisabled());
            else if (m_position == Right)
                skin->DrawRightStripButton(this, bDrawDepressed, bDrawHovered, IsDisabled());
            else
                skin->DrawButton(this, bDrawDepressed, bDrawHovered, IsDisabled());
        }

        void ButtonStrip::OnButtonToggle(Gwen::Controls::Base* control) {
            if (m_ignoreToggle) return;
            Gwen::Controls::Button* button = static_cast<Gwen::Controls::Button*>(control);
            SetSelectedButton(button);
        }

        GWEN_CONTROL_CONSTRUCTOR(ButtonStrip) {
            m_selected = NULL;
            m_ignoreToggle = false;
        }

        Button* ButtonStrip::AddButton(const Gwen::String& text) {
            return AddButton(Gwen::Utility::StringToUnicode(text));
        }
        
        Button* ButtonStrip::AddButton(const Gwen::UnicodeString& text) {
            StripButton* button = new StripButton(this);
            button->SetIsToggle(true);
            button->SetText(text);
            button->onToggle.Add(this, &ButtonStrip::OnButtonToggle);

            Gwen::Point size = GetSkin()->GetRender()->MeasureText(button->GetFont(), text);
            button->SetWidth(size.x + 10);

            Base::List& children = GetChildren();
            if (children.size() == 1) {
                button->SetStripPosition(StripButton::Only);
                SetSelectedButton(button);
                button->SetPos(0, 0);
            } else {
                StripButton* previous = static_cast<StripButton*>(*(--(--children.end())));
                if (children.size() == 2)
                    previous->SetStripPosition(StripButton::Left);
                else
                    previous->SetStripPosition(StripButton::Mid);
                button->SetStripPosition(StripButton::Right);
                button->SetPos(previous->X() + previous->Width(), previous->Y());
            }
            
            SizeToChildren();
            Redraw();

            return button;
        }
        
        Button* ButtonStrip::GetSelectedButton() {
            return m_selected;
        }
        
        int ButtonStrip::GetSelectedButtonIndex() {
            int index = 0;
            Base::List& children = GetChildren();
            Base::List::iterator childIt;
            for (childIt = children.begin(); childIt != children.end(); ++childIt) {
                if (m_selected == *childIt)
                    return index;
                index++;
            }
            return -1;
        }

        void ButtonStrip::SetSelectedButton(Button* selectedButton) {
            m_ignoreToggle = true;
            
            Base::List& children = GetChildren();
            Base::List::iterator childIt;
            for (childIt = children.begin(); childIt != children.end(); ++childIt) {
                StripButton* button = static_cast<StripButton*>(*childIt);
                if (button->GetToggleState())
                    button->SetToggleState(false);
            }
            selectedButton->SetToggleState(true);
            m_selected = selectedButton;
            OnChange();
            
            m_ignoreToggle = false;
        }
        
        void ButtonStrip::OnChange() {
            onSelectionChange.Call(this);
        }
    }
}

/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#ifndef TrenchBroom_ButtonStrip_h
#define TrenchBroom_ButtonStrip_h

#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Button.h"
#include <vector>

namespace Gwen {
    namespace Controls {
        class StripButton : public Button {
        public:
            typedef enum {
                Left = 0,
                Mid = 1,
                Right = 2,
                Only = 3
            } Pos;
        protected:
            Pos m_position;
        public:
            GWEN_CONTROL(StripButton, Button);
            void SetStripPosition(StripButton::Pos position);
            StripButton::Pos GetStripPosition();
            virtual void Render(Skin::Base* skin);
        };
        
        class ButtonStrip : public Base {
        protected:
            Button* m_selected;
            bool m_ignoreToggle;
            virtual void OnButtonToggle(Gwen::Controls::Base* control);
            virtual void OnChange();
        public:
            GWEN_CONTROL(ButtonStrip, Base);
            
            virtual Button* AddButton(const Gwen::String& text);
            virtual Button* AddButton(const Gwen::UnicodeString& text);
            virtual Button* GetSelectedButton();
            virtual int GetSelectedButtonIndex();
            virtual void SetSelectedButton(Button* selectedButton);

            Event::Caller onSelectionChange;
        };
    }
}

#endif

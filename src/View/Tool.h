/*
 Copyright (C) 2010-2013 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Tool_h
#define TrenchBroom_Tool_h

#include <iostream>

namespace TrenchBroom {
    namespace View {
        class InputState;
        
        class EmptyPolicy {
        public:
            bool doMouseDown(const InputState& inputState);
            bool doMouseUp(const InputState& inputState);
            bool doMouseDoubleClick(const InputState& inputState);
            bool doScroll(const InputState& inputState);
            void doMouseMove(const InputState& inputState);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
        };
        
        class MousePolicy {
        public:
            virtual ~MousePolicy();

            virtual bool doMouseDown(const InputState& inputState);
            virtual bool doMouseUp(const InputState& inputState);
            virtual bool doMouseDoubleClick(const InputState& inputState);
            virtual void doScroll(const InputState& inputState);
            virtual void doMouseMove(const InputState& inputState);
        };
        
        class MouseDragPolicy {
        public:
            virtual ~MouseDragPolicy();
            
            virtual bool doStartMouseDrag(const InputState& inputState) = 0;
            virtual bool doMouseDrag(const InputState& inputState) = 0;
            virtual void doEndMouseDrag(const InputState& inputState) = 0;
            virtual void doCancelMouseDrag(const InputState& inputState) = 0;
        };
        
        class BaseTool {
        public:
            virtual ~BaseTool();

            virtual bool mouseDown(const InputState& inputState) = 0;
            virtual bool mouseUp(const InputState& inputState) = 0;
            virtual bool mouseDoubleClick(const InputState& inputState) = 0;
            virtual void scroll(const InputState& inputState) = 0;
            virtual void mouseMove(const InputState& inputState) = 0;
            
            virtual BaseTool* startMouseDrag(const InputState& inputState) = 0;
            virtual bool mouseDrag(const InputState& inputState) = 0;
            virtual void endMouseDrag(const InputState& inputState) = 0;
            virtual void cancelMouseDrag(const InputState& inputState) = 0;
        };
        
        template <class MousePolicyType, class MouseDragPolicyType>
        class Tool : public BaseTool, private MousePolicyType, private MouseDragPolicyType {
        private:
            BaseTool* m_next;
        public:
            Tool(BaseTool* next) :
            m_next(next) {}
            
            inline bool mouseDown(const InputState& inputState) {
                if (static_cast<MousePolicyType&>(*this).doMouseDown(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseDown(inputState);
                return false;
            }
            
            inline bool mouseUp(const InputState& inputState) {
                if (static_cast<MousePolicyType&>(*this).doMouseUp(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseUp(inputState);
                return false;
            }
            
            inline bool mouseDoubleClick(const InputState& inputState) {
                if (static_cast<MousePolicyType&>(*this).doMouseDoubleClick(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseDoubleClick(inputState);
                return false;
            }
            
            inline void scroll(const InputState& inputState) {
                static_cast<MousePolicyType&>(*this).doScroll(inputState);
                if (m_next != NULL)
                    m_next->scroll(inputState);
            }
            
            inline void mouseMove(const InputState& inputState) {
                static_cast<MousePolicyType&>(*this).doMouseMove(inputState);
                if (m_next != NULL)
                    m_next->mouseMove(inputState);
            }

            inline BaseTool* startMouseDrag(const InputState& inputState) {
                if (static_cast<MouseDragPolicyType&>(*this).doStartMouseDrag(inputState))
                    return this;
                if (m_next != NULL)
                    return m_next->startMouseDrag(inputState);
                return NULL;
            }
            
            inline bool mouseDrag(const InputState& inputState) {
                if (static_cast<MouseDragPolicyType&>(*this).doMouseDrag(inputState))
                    return true;
                if (m_next != NULL)
                    return m_next->mouseDrag(inputState);
                return false;
            }
            
            inline void endMouseDrag(const InputState& inputState) {
                static_cast<MouseDragPolicyType&>(*this).doEndMouseDrag(inputState);
                if (m_next != NULL)
                    m_next->endMouseDrag(inputState);
            }
            
            inline void cancelMouseDrag(const InputState& inputState) {
                static_cast<MouseDragPolicyType&>(*this).doCancelMouseDrag(inputState);
                if (m_next != NULL)
                    m_next->cancelMouseDrag(inputState);
            }
        };
    }
}

#endif

/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "Controller/Tool.h"
#include "Controller/ClipHandle.h"

namespace TrenchBroom {
    namespace Renderer {
        class ClipHandleFigure;
    }
    
    namespace Controller {
        class ClipHandle;
        
        class ClipTool : public Tool {
        protected:
            ClipHandle m_handle;
            Renderer::ClipHandleFigure* m_handleFigure;
            
            virtual bool handleActivated(InputEvent& event);
            virtual bool handleDeactivated(InputEvent& event);
            virtual bool handleMouseMoved(InputEvent& event);
            virtual bool handleMouseUp(InputEvent& event);

            virtual bool handleBeginDrag(InputEvent& event);
            virtual bool handleDrag(InputEvent& event);
            virtual void handleEndDrag(InputEvent& event);
        public:
            ClipTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);

            bool updateHits(InputEvent& event);
            bool suppressOtherFeedback(InputEvent& event);
            bool updateFeedback(InputEvent& event);
            
            void toggleClipSide();
            bool canPerformClip();
            void performClip();
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */

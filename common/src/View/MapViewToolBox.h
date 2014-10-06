/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapViewToolBox__
#define __TrenchBroom__MapViewToolBox__

#include "View/ToolBox.h"
#include "View/ViewTypes.h"

class wxBookCtrlBase;

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class CameraTool;
        class MovementRestriction;
        class MoveObjectsTool;
        class RotateObjectsTool;
        class SelectionTool;

        class MapViewToolBox : public ToolBox {
        private:
            MovementRestriction* m_movementRestriction;
            CameraTool* m_cameraTool;
            MoveObjectsTool* m_moveObjectsTool;
            RotateObjectsTool* m_rotateObjectsTool;
            SelectionTool* m_selectionTool;
        public:
            MapViewToolBox(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl);
            ~MapViewToolBox();
            
            const MovementRestriction& movementRestriction() const;
            MovementRestriction& movementRestriction();

            void setCamera(Renderer::Camera* camera);
        public: // tools
            void toggleRotateObjectsTool();
            bool rotateObjectsToolActive() const;
            double rotateToolAngle() const;
            const Vec3 rotateToolCenter() const;
        private: // Tool related methods
            void createTools(MapDocumentWPtr document, wxBookCtrlBase* bookCtrl);
            void destroyTools();
        private: // notification
            void bindObservers();
            void unbindObservers();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
        };
    }
}

#endif /* defined(__TrenchBroom__MapViewToolBox__) */

/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_MoveObjectsTool
#define TrenchBroom_MoveObjectsTool

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/Tool.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Hit;
    }
    
    namespace View {
        class Grid;
        class InputState;
        
        class MoveObjectsTool : public Tool {
        public:
            typedef enum {
                MR_Continue,
                MR_Deny,
                MR_Cancel
            } MoveResult;
        private:
            MapDocumentWPtr m_document;
            bool m_duplicateObjects;
        public:
            MoveObjectsTool(MapDocumentWPtr document);
        public:
            const Grid& grid() const;
            
            bool startMove(const InputState& inputState);
            MoveResult move(const InputState& inputState, const Vec3& delta);
            void endMove(const InputState& inputState);
            void cancelMove();
        private:
            bool duplicateObjects(const InputState& inputState) const;
            
            wxWindow* doCreatePage(wxWindow* parent);
        };
    }
}

#endif /* defined(TrenchBroom_MoveObjectsTool) */

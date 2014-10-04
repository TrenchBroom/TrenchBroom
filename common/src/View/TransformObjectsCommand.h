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

#ifndef __TrenchBroom__TransformObjectsCommand__
#define __TrenchBroom__TransformObjectsCommand__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Snapshot.h"
#include "View/UndoableCommand.h"

namespace TrenchBroom {
    namespace View {
        class TransformObjectsCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            typedef enum {
                Action_Translate,
                Action_Rotate,
                Action_Flip
            } Action;
            
            Action m_action;
            Mat4x4 m_transformation;
            bool m_lockTextures;
            
            Model::Snapshot m_snapshot;
        };
    }
}

#endif /* defined(__TrenchBroom__TransformObjectsCommand__) */

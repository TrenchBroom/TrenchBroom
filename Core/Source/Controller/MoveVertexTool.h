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

#ifndef TrenchBroom_MoveVertexTool_h
#define TrenchBroom_MoveVertexTool_h

#include "Controller/VertexTool.h"

namespace TrenchBroom {
    namespace Controller {
        class MoveVertexTool : public VertexTool {
        protected:
            virtual int hitType();
            virtual size_t index(Model::Hit& hit);
            virtual std::string undoName();
            virtual Vec3f movePosition(const Model::Brush& brush, size_t index);
            virtual Model::MoveResult performMove(Model::Brush& brush, size_t index, const Vec3f& delta);
        public:
            MoveVertexTool(Controller::Editor& editor) : VertexTool(editor) {}
            virtual ~MoveVertexTool() {}

            virtual const Vec4f& handleColor();
            virtual const Vec4f& hiddenHandleColor();
            virtual const Vec4f& selectedHandleColor();
            virtual const Vec4f& hiddenSelectedHandleColor();

            virtual const Vec3fList handlePositions();
            virtual const Vec3fList selectedHandlePositions();
            virtual const Vec3f draggedHandlePosition();
        };
    }
}

#endif

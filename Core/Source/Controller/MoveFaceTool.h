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

#ifndef TrenchBroom_MoveFaceTool_h
#define TrenchBroom_MoveFaceTool_h

#include "Controller/VertexTool.h"

namespace TrenchBroom {
    namespace Controller {
        class MoveFaceTool : public VertexTool {
        protected:
            virtual Model::EHitType hitType();
            virtual std::string undoName();
            virtual Vec3f movePosition(const Model::Brush& brush, int index);
            virtual const Vec4f& handleColor();
            virtual const Vec4f& hiddenHandleColor();
            virtual const Vec4f& selectedHandleColor();
            virtual const Vec4f& hiddenSelectedHandleColor();
            virtual Model::MoveResult performMove(Model::Brush& brush, int index, const Vec3f& delta);
            
            virtual void updateHandleFigure();
            virtual void updateSelectedHandleFigure(const Model::Brush& brush, int index);
        public:
            MoveFaceTool(Controller::Editor& editor) : VertexTool(editor) {}
            virtual ~MoveFaceTool() {}
        };
    }
}

#endif

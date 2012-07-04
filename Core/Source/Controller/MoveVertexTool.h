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
            virtual int index(Model::Hit& hit);
            virtual std::string undoName();
            virtual Vec3f movePosition(const Model::Brush& brush, int index);
            virtual const Vec4f& handleColor();
            virtual const Vec4f& hiddenHandleColor();
            virtual const Vec4f& selectedHandleColor();
            virtual const Vec4f& hiddenSelectedHandleColor();
            virtual Model::MoveResult performMove(Model::Brush& brush, int index, const Vec3f& delta);

            virtual void updateHandleFigure(Renderer::HandleFigure& handleFigure);
            virtual void updateSelectedHandleFigures(Renderer::HandleFigure& handleFigure, Renderer::PointGuideFigure& guideFigure, const Model::Brush& brush, int index);
        public:
            MoveVertexTool(Controller::Editor& editor) : VertexTool(editor) {}
            virtual ~MoveVertexTool() {}
        };
    }
}

#endif

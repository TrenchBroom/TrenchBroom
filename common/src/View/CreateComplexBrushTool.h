/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_CreateComplexBrushTool
#define TrenchBroom_CreateComplexBrushTool

#include "TrenchBroom.h"
#include "Polyhedron.h"
#include "Model/ModelTypes.h"
#include "View/CreateBrushToolBase.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class BrushRenderer;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class CreateComplexBrushTool : public CreateBrushToolBase {
        private:
            Polyhedron3 m_polyhedron;
        public:
            CreateComplexBrushTool(MapDocumentWPtr document);

            const Polyhedron3& polyhedron() const;
            void update(const Polyhedron3& polyhedron);
        private:
            bool doActivate() override;
            bool doDeactivate() override;
            void doBrushWasCreated() override;
        };
    }
}

#endif /* defined(TrenchBroom_CreateComplexBrushTool) */

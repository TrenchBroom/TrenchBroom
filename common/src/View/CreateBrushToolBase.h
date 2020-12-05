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

#pragma once

#include "Macros.h"
#include "View/Tool.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
    }

    namespace Renderer {
        class BrushRenderer;
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class Grid;
        class MapDocument;

        class CreateBrushToolBase : public Tool {
        protected:
            std::weak_ptr<MapDocument> m_document;
        private:
            Model::BrushNode* m_brush;
            Renderer::BrushRenderer* m_brushRenderer;
        public:
            CreateBrushToolBase(bool initiallyActive, std::weak_ptr<MapDocument> document);
            ~CreateBrushToolBase() override;
        public:
            const Grid& grid() const;

            void createBrush();
            void cancel();

            void render(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        private:
            void renderBrush(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        protected:
            void updateBrush(Model::BrushNode* brush);
        private:
            virtual void doBrushWasCreated();

            deleteCopyAndMove(CreateBrushToolBase)
        };
    }
}

#endif /* defined(TrenchBroom_CreateBrushToolBase) */

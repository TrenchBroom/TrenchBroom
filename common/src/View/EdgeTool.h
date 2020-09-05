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

#ifndef EdgeTool_h
#define EdgeTool_h

#include "FloatType.h"
#include "View/VertexToolBase.h"

#include <vecmath/segment.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class PickResult;
    }

    namespace Renderer {
        class Camera;
    }

    namespace View {
        class EdgeTool : public VertexToolBase<vm::segment3> {
        private:
            std::unique_ptr<EdgeHandleManager> m_edgeHandles;
        public:
            EdgeTool(std::weak_ptr<MapDocument> document);
        public:
            std::vector<Model::BrushNode*> findIncidentBrushes(const vm::segment3& handle) const;
        private:
            using VertexToolBase::findIncidentBrushes;
        public:
            void pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const override;
        public:
            EdgeHandleManager& handleManager() override;
            const EdgeHandleManager& handleManager() const override;
        public:
            MoveResult move(const vm::vec3& delta) override;

            std::string actionName() const override;

            void removeSelection();
        };
    }
}

#endif /* EdgeTool_h */

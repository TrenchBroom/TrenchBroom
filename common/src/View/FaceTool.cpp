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

#include "FaceTool.h"

#include "FloatType.h"

#include <kdl/memory_utils.h>
#include <kdl/string_format.h>

namespace TrenchBroom {
    namespace View {
        FaceTool::FaceTool(std::weak_ptr<MapDocument> document) :
        VertexToolBase(document),
        m_faceHandles(std::make_unique<FaceHandleManager>()) {}

        std::vector<Model::BrushNode*> FaceTool::findIncidentBrushes(const vm::polygon3& handle) const {
            return findIncidentBrushes(*m_faceHandles, handle);
        }

        void FaceTool::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            m_faceHandles->pickCenterHandle(pickRay, camera, pickResult);
        }

        FaceHandleManager& FaceTool::handleManager() {
            return *m_faceHandles;
        }

        const FaceHandleManager& FaceTool::handleManager() const {
            return *m_faceHandles;
        }

        FaceTool::MoveResult FaceTool::move(const vm::vec3& delta) {
            auto document = kdl::mem_lock(m_document);

            auto handles = m_faceHandles->selectedHandles();
            if (document->moveFaces(std::move(handles), delta)) {
                m_dragHandlePosition = m_dragHandlePosition.translate(delta);
                return MR_Continue;
            }
            return MR_Deny;
        }

        std::string FaceTool::actionName() const {
            return kdl::str_plural(m_faceHandles->selectedHandleCount(), "Move Face", "Move Faces");
        }

        void FaceTool::removeSelection() {
            const auto handles = m_faceHandles->selectedHandles();
            const auto brushMap = buildBrushMap(*m_faceHandles, std::begin(handles), std::end(handles));

            Transaction transaction(m_document, kdl::str_plural(handleManager().selectedHandleCount(), "Remove Face", "Remove Faces"));
            kdl::mem_lock(m_document)->removeFaces(brushMap);
        }
    }
}

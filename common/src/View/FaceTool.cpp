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

#include "TrenchBroom.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace View {
        FaceTool::FaceTool(MapDocumentWPtr document) :
        VertexToolBase(document) {}
        
        Model::BrushSet FaceTool::findIncidentBrushes(const vm::polygon3& handle) const {
            return findIncidentBrushes(m_faceHandles, handle);
        }
        
        void FaceTool::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            m_faceHandles.pickCenterHandle(pickRay, camera, pickResult);
        }
        
        FaceHandleManager& FaceTool::handleManager() {
            return m_faceHandles;
        }
        
        const FaceHandleManager& FaceTool::handleManager() const {
            return m_faceHandles;
        }
        
        FaceTool::MoveResult FaceTool::move(const vm::vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            
            const auto handles = m_faceHandles.selectedHandles();
            const auto brushMap = buildBrushMap(m_faceHandles, std::begin(handles), std::end(handles));
            if (document->moveFaces(brushMap, delta)) {
                m_dragHandlePosition = m_dragHandlePosition.translate(delta);
                return MR_Continue;
            }
            return MR_Deny;
        }

        bool FaceTool::canDoCsgConvexMerge() {
            return handleManager().selectedHandleCount() > 1;
        }

        void FaceTool::csgConvexMerge() {
            std::vector<vm::vec3> vertices;
            const auto faces = handleManager().selectedHandles();
            vm::polygon3::getVertices(std::begin(faces), std::end(faces), std::back_inserter(vertices));
            const Polyhedron3 polyhedron(vertices);
            if (!polyhedron.polyhedron() || !polyhedron.closed()) {
                return;
            }

            MapDocumentSPtr document = lock(m_document);
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            auto* brush = builder.createBrush(polyhedron, document->currentTextureName());
            brush->cloneFaceAttributesFrom(document->selectedNodes().brushes());

            const Transaction transaction(document, "CSG Convex Merge");
            deselectAll();
            document->addNode(brush, document->currentParent());
        }
        
        String FaceTool::actionName() const {
            return StringUtils::safePlural(m_faceHandles.selectedHandleCount(), "Move Face", "Move Faces");
        }
        
        void FaceTool::removeSelection() {
            const auto handles = m_faceHandles.selectedHandles();
            const auto brushMap = buildBrushMap(m_faceHandles, std::begin(handles), std::end(handles));

            Transaction transaction(m_document, StringUtils::safePlural(handleManager().selectedHandleCount(), "Remove Face", "Remove Faces"));
            lock(m_document)->removeFaces(brushMap);
        }
    }
}

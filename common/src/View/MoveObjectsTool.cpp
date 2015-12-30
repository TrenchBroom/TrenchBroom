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

#include "MoveObjectsTool.h"

#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderContext.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/MoveObjectsToolPage.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MoveObjectsTool::MoveObjectsTool(MapDocumentWPtr document) :
        Tool(true),
        MoveToolDelegate(),
        m_document(document),
        m_duplicateObjects(false) {}

        bool MoveObjectsTool::doHandleMove(const InputState& inputState) const {
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKAlt) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd) &&
                !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt))
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedNodes())
                return false;

            const Model::Hit& hit = findHit(inputState);
            if (!hit.isMatch())
                return false;
            const Model::Node* node = Model::hitToNode(hit);
            return node->selected();
        }
        
        Vec3 MoveObjectsTool::doGetMoveOrigin(const InputState& inputState) const {
            const Model::Hit& hit = findHit(inputState);
            assert(hit.isMatch());
            return hit.hitPoint();
        }

        const Model::Hit& MoveObjectsTool::findHit(const InputState& inputState) const {
            const Model::PickResult& pickResult = inputState.pickResult();
            return pickResult.query().pickable().type(Model::Group::GroupHit | Model::Entity::EntityHit | Model::Brush::BrushHit).selected().first();
        }

        String MoveObjectsTool::doGetActionName(const InputState& inputState) const {
            return duplicateObjects(inputState) ? "Duplicate" : "Move";
        }
        
        bool MoveObjectsTool::doStartMove(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction(duplicateObjects(inputState) ? "Duplicate Objects" : "Move Objects");
            m_duplicateObjects = duplicateObjects(inputState);
            return true;
        }
        
        Vec3 MoveObjectsTool::doSnapDelta(const InputState& inputState, const Vec3& delta) const {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            return grid.snap(delta);
        }
        
        MoveResult MoveObjectsTool::doMove(const InputState& inputState, const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            const BBox3 bounds = document->selectionBounds();
            if (!worldBounds.contains(bounds.translated(delta)))
                return MoveResult_Deny;
            
            if (m_duplicateObjects) {
                m_duplicateObjects = false;
                if (!document->duplicateObjects())
                    return MoveResult_Conclude;
            }
            
            if (!document->translateObjects(delta))
                return MoveResult_Deny;
            return MoveResult_Continue;
        }
        
        void MoveObjectsTool::doEndMove(const InputState& inputState) {
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
        }
        
        void MoveObjectsTool::doCancelMove() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
        }

        bool MoveObjectsTool::duplicateObjects(const InputState& inputState) const {
            return inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
        }

        wxWindow* MoveObjectsTool::doCreatePage(wxWindow* parent) {
            return new MoveObjectsToolPage(parent, m_document);
        }
    }
}

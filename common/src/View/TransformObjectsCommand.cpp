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

#include "TransformObjectsCommand.h"

#include "Macros.h"
#include "Model/Snapshot.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType TransformObjectsCommand::Type = Command::freeType();

        TransformObjectsCommand::Ptr TransformObjectsCommand::translate(const Vec3& delta, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(delta);
            return Ptr(new TransformObjectsCommand(Action_Translate, transform, lockTextures));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::rotate(const Vec3& center, const Vec3& axis, const FloatType angle, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(center) * rotationMatrix(axis, angle) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Translate, transform, lockTextures));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::flip(const Vec3& center, const Math::Axis::Type axis, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(center) * mirrorMatrix<FloatType>(axis) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Translate, transform, lockTextures));
        }

        TransformObjectsCommand::~TransformObjectsCommand() {
            deleteSnapshot();
        }
        
        TransformObjectsCommand::TransformObjectsCommand(const Action action, const Mat4x4& transform, const bool lockTextures) :
        DocumentCommand(Type, makeName(action)),
        m_action(action),
        m_transform(transform),
        m_lockTextures(lockTextures),
        m_snapshot(NULL) {}
        
        String TransformObjectsCommand::makeName(const Action action) {
            switch (action) {
                case Action_Translate:
                    return "Move objects";
                case Action_Rotate:
                    return "Rotate objects";
                case Action_Flip:
                    return "Flip objects";
                switchDefault()
            }
        }
        
        bool TransformObjectsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            takeSnapshot(document->selectedNodes().nodes());
            document->performTransform(m_transform, m_lockTextures);
            return true;
        }
        
        bool TransformObjectsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            assert(m_snapshot != NULL);
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            return true;
        }
        
        void TransformObjectsCommand::takeSnapshot(const Model::NodeList& nodes) {
            assert(m_snapshot == NULL);
            m_snapshot = new Model::Snapshot(nodes.begin(), nodes.end());
        }
        
        void TransformObjectsCommand::deleteSnapshot() {
            delete m_snapshot;
            m_snapshot = NULL;
        }

        bool TransformObjectsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }
        
        UndoableCommand::Ptr TransformObjectsCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new TransformObjectsCommand(m_action, m_transform, m_lockTextures));
        }
        
        bool TransformObjectsCommand::doCollateWith(UndoableCommand::Ptr command) {
            TransformObjectsCommand* other = static_cast<TransformObjectsCommand*>(command.get());
            if (other->m_lockTextures != m_lockTextures)
                return false;
            if (other->m_action != m_action)
                return false;
            m_transform = m_transform * other->m_transform;
            return true;
        }
    }
}

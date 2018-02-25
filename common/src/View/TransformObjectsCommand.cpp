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
            return Ptr(new TransformObjectsCommand(Action_Translate, "Move Objects", transform, lockTextures));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::rotate(const Vec3& center, const Vec3& axis, const FloatType angle, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(center) * rotationMatrix(axis, angle) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Rotate, "Rotate Objects", transform, lockTextures));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::scale(const Vec3& center, const Vec3& scaleFactors, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(center) * scalingMatrix(scaleFactors) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Rotate, "Scale Objects", transform, lockTextures));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::flip(const Vec3& center, const Math::Axis::Type axis, const bool lockTextures) {
            const Mat4x4 transform = translationMatrix(center) * mirrorMatrix<FloatType>(axis) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(Action_Flip, "Flip Objects", transform, lockTextures));
        }

        TransformObjectsCommand::~TransformObjectsCommand() {
            deleteSnapshot();
        }
        
        TransformObjectsCommand::TransformObjectsCommand(const Action action, const String& name, const Mat4x4& transform, const bool lockTextures) :
        DocumentCommand(Type, name),
        m_action(action),
        m_transform(transform),
        m_lockTextures(lockTextures),
        m_snapshot(nullptr) {}
        
        bool TransformObjectsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            takeSnapshot(document->selectedNodes().nodes());
            document->performTransform(m_transform, m_lockTextures);
            return true;
        }
        
        bool TransformObjectsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            ensure(m_snapshot != nullptr, "snapshot is null");
            document->restoreSnapshot(m_snapshot);
            deleteSnapshot();
            return true;
        }
        
        void TransformObjectsCommand::takeSnapshot(const Model::NodeList& nodes) {
            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(nodes), std::end(nodes));
        }
        
        void TransformObjectsCommand::deleteSnapshot() {
            delete m_snapshot;
            m_snapshot = nullptr;
        }

        bool TransformObjectsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }
        
        UndoableCommand::Ptr TransformObjectsCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new TransformObjectsCommand(m_action, m_name, m_transform, m_lockTextures));
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

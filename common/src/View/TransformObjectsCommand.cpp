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

#include "FloatType.h"
#include "View/MapDocumentCommandFacade.h"

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType TransformObjectsCommand::Type = Command::freeType();

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::translate(const vm::vec3& delta, const bool lockTextures) {
            const auto transform = vm::translation_matrix(delta);
            return std::make_unique<TransformObjectsCommand>(Action::Translate, "Move Objects", transform, lockTextures);
        }

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::rotate(const vm::vec3& center, const vm::vec3& axis, const FloatType angle, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::rotation_matrix(axis, angle) * vm::translation_matrix(-center);
            return std::make_unique<TransformObjectsCommand>(Action::Rotate, "Rotate Objects", transform, lockTextures);
        }

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::scale(const vm::bbox3& oldBBox, const vm::bbox3& newBBox, const bool lockTextures) {
            const auto transform = vm::scale_bbox_matrix(oldBBox, newBBox);
            return std::make_unique<TransformObjectsCommand>(Action::Scale, "Scale Objects", transform, lockTextures);
        }

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::scale(const vm::vec3& center, const vm::vec3& scaleFactors, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::scaling_matrix(scaleFactors) * vm::translation_matrix(-center);
            return std::make_unique<TransformObjectsCommand>(Action::Scale, "Scale Objects", transform, lockTextures);
        }

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::shearBBox(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta, const bool lockTextures) {
            const auto transform = vm::shear_bbox_matrix(box, sideToShear, delta);
            return std::make_unique<TransformObjectsCommand>(Action::Shear, "Shear Objects", transform, lockTextures);
        }

        std::unique_ptr<TransformObjectsCommand> TransformObjectsCommand::flip(const vm::vec3& center, const vm::axis::type axis, const bool lockTextures) {
            const auto transform = vm::translation_matrix(center) * vm::mirror_matrix<FloatType>(axis) * vm::translation_matrix(-center);
            return std::make_unique<TransformObjectsCommand>(Action::Flip, "Flip Objects", transform, lockTextures);
        }

        TransformObjectsCommand::TransformObjectsCommand(const Action action, const std::string& name, const vm::mat4x4& transform, const bool lockTextures) :
        SnapshotCommand(Type, name),
        m_action(action),
        m_transform(transform),
        m_lockTextures(lockTextures) {}

        std::unique_ptr<CommandResult> TransformObjectsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const bool success = document->performTransform(m_transform, m_lockTextures);
            if (!success) {
                restoreSnapshot(document);
            }
            return std::make_unique<CommandResult>(success);
        }

        bool TransformObjectsCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedNodes();
        }

        std::unique_ptr<UndoableCommand> TransformObjectsCommand::doRepeat(MapDocumentCommandFacade*) const {
            return std::make_unique<TransformObjectsCommand>(m_action, m_name, m_transform, m_lockTextures);
        }

        bool TransformObjectsCommand::doCollateWith(UndoableCommand* command) {
            auto* other = static_cast<TransformObjectsCommand*>(command);
            if (other->m_lockTextures != m_lockTextures) {
                return false;
            } else if (other->m_action != m_action) {
                return false;
            } else {
                m_transform = m_transform * other->m_transform;
                return true;
            }
        }
    }
}

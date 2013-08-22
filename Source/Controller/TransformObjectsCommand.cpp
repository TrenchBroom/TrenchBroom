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

#include "TransformObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"
#include "Utility/Console.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool TransformObjectsCommand::performDo() {
            if (!m_entities.empty()) {
                makeSnapshots(m_entities);
                document().entitiesWillChange(m_entities);

                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    entity.transform(m_pointTransform, m_vectorTransform, m_lockTextures, m_invertOrientation);
                }
                document().entitiesDidChange(m_entities);
            }
            
            if (!m_brushes.empty()) {
                makeSnapshots(m_brushes);
                document().brushesWillChange(m_brushes);
                
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    brush.transform(m_pointTransform, m_vectorTransform, m_lockTextures, m_invertOrientation);
                }
                document().brushesDidChange(m_brushes);
            }
            
            return true;
        }

        bool TransformObjectsCommand::performUndo() {
            if (!m_entities.empty()) {
                document().entitiesWillChange(m_entities);
                restoreSnapshots(m_entities);
                document().entitiesDidChange(m_entities);
            }
            
            if (!m_brushes.empty()) {
                document().brushesWillChange(m_brushes);
                restoreSnapshots(m_brushes);
                document().brushesDidChange(m_brushes);
            }

            clear();
            return true;
        }

        TransformObjectsCommand::TransformObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Mat4f& pointTransform, const Mat4f& vectorTransform, bool invertOrientation) :
        SnapshotCommand(TransformObjects, document, name),
        m_entities(entities),
        m_brushes(brushes),
        m_pointTransform(pointTransform),
        m_vectorTransform(vectorTransform),
        m_lockTextures(document.textureLock()),
        m_invertOrientation(invertOrientation) {}

        TransformObjectsCommand* TransformObjectsCommand::translateObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& delta) {
            const wxString commandName = Command::makeObjectActionName(wxT("Move"), entities, brushes);
            const Mat4f& vectorTransform = Mat4f::Identity;
            const Mat4f pointTransform = translationMatrix(delta);
            return new TransformObjectsCommand(document, entities, brushes, commandName, pointTransform, vectorTransform, false);
        }

        TransformObjectsCommand* TransformObjectsCommand::translateEntity(Model::MapDocument& document, Model::Entity& entity, const Vec3f& delta) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return translateObjects(document, entities, Model::EmptyBrushList, delta);
        }

        TransformObjectsCommand* TransformObjectsCommand::rotateObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center) {
            const wxString commandName = Command::makeObjectActionName(wxT("Rotate"), entities, brushes);
            const Mat4f vectorTransform = clockwise ? rotationMatrix(-angle, axis) : rotationMatrix(angle, axis);
            const Mat4f pointTransform = translationMatrix(center) * vectorTransform * translationMatrix(-center);
            return new TransformObjectsCommand(document, entities, brushes, commandName, pointTransform, vectorTransform, false);
        }

        TransformObjectsCommand* TransformObjectsCommand::flipObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Axis::Type& axis, const Vec3f& center) {
            const wxString commandName = Command::makeObjectActionName(wxT("Flip"), entities, brushes);
            Mat4f vectorTransform;
            switch (axis) {
                case Axis::AX:
                    vectorTransform = Mat4f::MirX;
                    break;
                case Axis::AY:
                    vectorTransform = Mat4f::MirY;
                    break;
                case Axis::AZ:
                    vectorTransform = Mat4f::MirZ;
                    break;
            }
            const Mat4f pointTransform = translationMatrix(center) * vectorTransform * translationMatrix(-center);
            return new TransformObjectsCommand(document, entities, brushes, commandName, pointTransform, vectorTransform, true);
        }
    }
}

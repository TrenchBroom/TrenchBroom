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

#include "RotateObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Utility/Console.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RotateObjectsCommand::performDo() {
            float angle = m_angle;
            if (angle > 0.0f) {
                while (angle - 2.0f * Math::Pi >= 0.0f)
                    angle -= 2.0f * Math::Pi;
            } else if (angle < 0.0f) {
                while (angle + 2.0f * Math::Pi <= 0.0f)
                    angle += 2.0f * Math::Pi;
            }
            
            if (angle == 0.0f)
                return false;
            
            document().console().info("Rotation center: %f %f %f", m_center.x, m_center.y, m_center.z);
            
            makeSnapshots(m_entities);
            makeSnapshots(m_brushes);
            
            if (angle < 0.0f)
                angle += 2.0f * Math::Pi;
            
            assert(angle > 0.0f);
            
            document().entitiesWillChange(m_entities);
            document().brushesWillChange(m_brushes);
            
            // if we are rotating about one of the coordinate system axes, we can get a more precise result by rotating
            // by 90 degrees as often as possible
            if (m_axis.equals(m_axis.firstAxis())) {
                unsigned int quarters = static_cast<unsigned int>(2.0f * angle / Math::Pi);
                quarters %= 4;
                
                if (quarters > 0) {
                    angle = angle - quarters * Math::Pi / 2.0f;
                    Axis::Type component = m_axis.firstComponent();
                    
                    for (unsigned int i = 0; i < quarters; i++) {
                        Model::EntityList::const_iterator entityIt, entityEnd;
                        for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                            Model::Entity& entity = **entityIt;
                            entity.rotate90(component, m_center, angle < 0.0f, m_lockTextures);
                        }
                        
                        Model::BrushList::const_iterator brushIt, brushEnd;
                        for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                            Model::Brush& brush = **brushIt;
                            brush.rotate90(component, m_center, angle < 0.0f, m_lockTextures);
                        }
                    }
                }
            }
            
            if (angle > 0.0f) {
                Quat rotation(angle, m_axis);
                Model::EntityList::const_iterator entityIt, entityEnd;
                for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                    Model::Entity& entity = **entityIt;
                    entity.rotate(rotation, m_center, m_lockTextures);
                }
                
                Model::BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    brush.rotate(rotation, m_center, m_lockTextures);
                }
            }
            
            document().entitiesDidChange(m_entities);
            document().brushesDidChange(m_brushes);

            return true;
        }
        
        bool RotateObjectsCommand::performUndo() {
            document().entitiesWillChange(m_entities);
            document().brushesWillChange(m_brushes);

            restoreSnapshots(m_brushes);
            restoreSnapshots(m_entities);
            clear();

            document().entitiesDidChange(m_entities);
            document().brushesDidChange(m_brushes);

            return true;
        }
        
        RotateObjectsCommand::RotateObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures) :
        SnapshotCommand(RotateObjects, document, name),
        m_entities(entities),
        m_brushes(brushes),
        m_axis(axis),
        m_angle(clockwise ? angle : -angle),
        m_center(center),
        m_lockTextures(lockTextures) {}

        RotateObjectsCommand* RotateObjectsCommand::rotate(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures) {
            wxString commandName = Command::makeObjectActionName(wxT("Rotate"), entities, brushes);
            return new RotateObjectsCommand(document, entities, brushes, commandName, axis, angle, clockwise, center, lockTextures);
        }
    }
}

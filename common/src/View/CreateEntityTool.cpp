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

#include "CreateEntityTool.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <vecmath/bbox.h>

#include <string>

namespace TrenchBroom {
    namespace View {
        CreateEntityTool::CreateEntityTool(std::weak_ptr<MapDocument> document) :
        Tool(true),
        m_document(document),
        m_entity(nullptr) {}

        bool CreateEntityTool::createEntity(const std::string& classname) {
            auto document = kdl::mem_lock(m_document);
            const Assets::EntityDefinitionManager& definitionManager = document->entityDefinitionManager();
            Assets::EntityDefinition* definition = definitionManager.definition(classname);
            if (definition == nullptr)
                return false;

            if (definition->type() != Assets::EntityDefinitionType::PointEntity)
                return false;

            const Model::WorldNode* world = document->world();
            m_entity = world->createEntity(Model::Entity({
                {Model::AttributeNames::Classname, definition->name()}
            }));

            m_referenceBounds = document->referenceBounds();

            document->startTransaction("Create '" + definition->name() + "'");
            document->deselectAll();
            document->addNode(m_entity, document->parentForNodes());
            document->select(m_entity);

            return true;
        }

        void CreateEntityTool::removeEntity() {
            ensure(m_entity != nullptr, "entity is null");
            auto document = kdl::mem_lock(m_document);
            document->cancelTransaction();
            m_entity = nullptr;
        }

        void CreateEntityTool::commitEntity() {
            ensure(m_entity != nullptr, "entity is null");
            auto document = kdl::mem_lock(m_document);
            document->commitTransaction();
            m_entity = nullptr;
        }

        void CreateEntityTool::updateEntityPosition2D(const vm::ray3& pickRay) {
            ensure(m_entity != nullptr, "entity is null");

            auto document = kdl::mem_lock(m_document);

            const auto toMin = m_referenceBounds.min - pickRay.origin;
            const auto toMax = m_referenceBounds.max - pickRay.origin;
            const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction) ? m_referenceBounds.min : m_referenceBounds.max;
            const auto dragPlane = vm::plane3(anchor, -pickRay.direction);

            const auto distance = vm::intersect_ray_plane(pickRay, dragPlane);
            if (vm::is_nan(distance)) {
                return;
            }

            const auto& grid = document->grid();
            const auto delta = grid.moveDeltaForBounds(dragPlane, m_entity->logicalBounds(), document->worldBounds(), pickRay);

            if (!vm::is_zero(delta, vm::C::almost_zero())) {
                document->translateObjects(delta);
            }
        }

        void CreateEntityTool::updateEntityPosition3D(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            ensure(m_entity != nullptr, "entity is null");

            auto document = kdl::mem_lock(m_document);

            vm::vec3 delta;
            const auto& grid = document->grid();
            const auto& hit = pickResult.query().pickable().type(Model::BrushNode::BrushHitType).occluded().first();
            if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
                const auto& face = faceHandle->face();
                delta = grid.moveDeltaForBounds(face.boundary(), m_entity->logicalBounds(), document->worldBounds(), pickRay);
            } else {
                const auto newPosition = vm::point_at_distance(pickRay, static_cast<FloatType>(Renderer::Camera::DefaultPointDistance));
                const auto boundsCenter = m_entity->logicalBounds().center();
                delta = grid.moveDeltaForPoint(boundsCenter, newPosition - boundsCenter);
            }

            if (!vm::is_zero(delta, vm::C::almost_zero())) {
                document->translateObjects(delta);
            }
        }
    }
}

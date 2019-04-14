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

#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/ModelDefinition.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/Hit.h"
#include "Model/HitQuery.h"
#include "Model/Layer.h"
#include "Model/PickResult.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <vecmath/bbox.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateEntityTool::CreateEntityTool(MapDocumentWPtr document) :
        Tool(true),
        m_document(document),
        m_entity(nullptr) {}

        bool CreateEntityTool::createEntity(const String& classname) {
            MapDocumentSPtr document = lock(m_document);
            const Assets::EntityDefinitionManager& definitionManager = document->entityDefinitionManager();
            Assets::EntityDefinition* definition = definitionManager.definition(classname);
            if (definition == nullptr)
                return false;

            if (definition->type() != Assets::EntityDefinition::Type_PointEntity)
                return false;

            const Model::World* world = document->world();
            m_entity = world->createEntity();
            m_entity->addOrUpdateAttribute(Model::AttributeNames::Classname, definition->name());

            m_referenceBounds = document->referenceBounds();

            document->beginTransaction("Create '" + definition->name() + "'");
            document->deselectAll();
            document->addNode(m_entity, document->currentParent());
            document->select(m_entity);

            return true;
        }

        void CreateEntityTool::removeEntity() {
            ensure(m_entity != nullptr, "entity is null");
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_entity = nullptr;
        }

        void CreateEntityTool::commitEntity() {
            ensure(m_entity != nullptr, "entity is null");
            MapDocumentSPtr document = lock(m_document);
            document->commitTransaction();
            m_entity = nullptr;
        }

        void CreateEntityTool::updateEntityPosition2D(const vm::ray3& pickRay) {
            ensure(m_entity != nullptr, "entity is null");

            auto document = lock(m_document);

            const auto toMin = m_referenceBounds.min - pickRay.origin;
            const auto toMax = m_referenceBounds.max - pickRay.origin;
            const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction) ? m_referenceBounds.min : m_referenceBounds.max;
            const auto dragPlane = vm::plane3(anchor, -pickRay.direction);

            const auto distance = vm::intersectRayAndPlane(pickRay, dragPlane);
            if (vm::isnan(distance)) {
                return;
            }

            const auto hitPoint = pickRay.pointAtDistance(distance);

            const auto& grid = document->grid();
            const auto delta = grid.moveDeltaForBounds(dragPlane, m_entity->definitionBounds(), document->worldBounds(), pickRay, hitPoint);

            if (!isZero(delta, vm::C::almostZero())) {
                document->translateObjects(delta);
            }
        }

        void CreateEntityTool::updateEntityPosition3D(const vm::ray3& pickRay, const Model::PickResult& pickResult) {
            ensure(m_entity != nullptr, "entity is null");

            auto document = lock(m_document);

            vm::vec3 delta;
            const auto& grid = document->grid();
            const auto& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const auto* face = Model::hitToFace(hit);
                const auto dragPlane = alignedOrthogonalPlane(hit.hitPoint(), face->boundary().normal);
                delta = grid.moveDeltaForBounds(dragPlane, m_entity->definitionBounds(), document->worldBounds(), pickRay, hit.hitPoint());
            } else {
                const auto newPosition = pickRay.pointAtDistance(Renderer::Camera::DefaultPointDistance);
                const auto boundsCenter = m_entity->definitionBounds().center();
                delta = grid.moveDeltaForPoint(boundsCenter, document->worldBounds(), newPosition - boundsCenter);
            }

            if (!isZero(delta, vm::C::almostZero())) {
                document->translateObjects(delta);
            }
        }
    }
}

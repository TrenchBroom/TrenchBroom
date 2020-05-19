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

#include "VertexHandleManager.h"

#include "FloatType.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Polyhedron.h"
#include "View/Grid.h"

#include <vecmath/distance.h>
#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        VertexHandleManagerBase::~VertexHandleManagerBase() {}

        const Model::HitType::Type VertexHandleManager::HandleHitType = Model::HitType::freeType();

        void VertexHandleManager::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const auto& entry : m_handles) {
                const auto& position = entry.first;
                const auto distance = camera.pickPointHandle(pickRay, position, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                if (!vm::is_nan(distance)) {
                    const auto hitPoint = vm::point_at_distance(pickRay, distance);
                    const auto error = vm::squared_distance(pickRay, position).distance;
                    pickResult.addHit(Model::Hit::hit(HandleHitType, distance, hitPoint, position, error));
                }
            }
        }

        void VertexHandleManager::addHandles(const Model::BrushNode* brush) {
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                add(vertex->position());
            }
        }

        void VertexHandleManager::removeHandles(const Model::BrushNode* brush) {
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                assertResult(remove(vertex->position()))
            }
        }

        Model::HitType::Type VertexHandleManager::hitType() const {
            return HandleHitType;
        }

        bool VertexHandleManager::isIncident(const Handle& handle, const Model::BrushNode* brush) const {
            return brush->hasVertex(handle);
        }

        const Model::HitType::Type EdgeHandleManager::HandleHitType = Model::HitType::freeType();

        void EdgeHandleManager::pickGridHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const vm::segment3& position = entry.first;
                const FloatType edgeDist = camera.pickLineSegmentHandle(pickRay, position, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                if (!vm::is_nan(edgeDist)) {
                    const vm::vec3 pointHandle = grid.snap(vm::point_at_distance(pickRay, edgeDist), position);
                    const FloatType pointDist = camera.pickPointHandle(pickRay, pointHandle, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                    if (!vm::is_nan(pointDist)) {
                        const vm::vec3 hitPoint = vm::point_at_distance(pickRay, pointDist);
                        pickResult.addHit(Model::Hit::hit(HandleHitType, pointDist, hitPoint, HitType(position, pointHandle)));
                    }
                }
            }
        }

        void EdgeHandleManager::pickCenterHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const vm::segment3& position = entry.first;
                const vm::vec3 pointHandle = position.center();

                const FloatType pointDist = camera.pickPointHandle(pickRay, pointHandle, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                if (!vm::is_nan(pointDist)) {
                    const vm::vec3 hitPoint = vm::point_at_distance(pickRay, pointDist);
                    pickResult.addHit(Model::Hit::hit(HandleHitType, pointDist, hitPoint, position));
                }
            }
        }

        void EdgeHandleManager::addHandles(const Model::BrushNode* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                add(vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
            }
        }

        void EdgeHandleManager::removeHandles(const Model::BrushNode* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                assertResult(remove(vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position())))
            }
        }

        Model::HitType::Type EdgeHandleManager::hitType() const {
            return HandleHitType;
        }

        bool EdgeHandleManager::isIncident(const Handle& handle, const Model::BrushNode* brush) const {
            return brush->hasEdge(handle);
        }

        const Model::HitType::Type FaceHandleManager::HandleHitType = Model::HitType::freeType();

        void FaceHandleManager::pickGridHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const auto& entry : m_handles) {
                const auto& position = entry.first;

                const auto [valid, plane] = vm::from_points(std::begin(position), std::end(position));
                if (!valid) {
                    continue;
                }

                const auto distance = vm::intersect_ray_polygon(pickRay, plane, std::begin(position), std::end(position));
                if (!vm::is_nan(distance)) {
                    const auto pointHandle = grid.snap(vm::point_at_distance(pickRay, distance), plane);

                    const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                    if (!vm::is_nan(pointDist)) {
                        const auto hitPoint = vm::point_at_distance(pickRay, pointDist);
                        pickResult.addHit(Model::Hit::hit(HandleHitType, pointDist, hitPoint, HitType(position, pointHandle)));
                    }
                }
            }
        }

        void FaceHandleManager::pickCenterHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const auto& position = entry.first;
                const auto pointHandle = position.center();

                const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, static_cast<FloatType>(pref(Preferences::HandleRadius)));
                if (!vm::is_nan(pointDist)) {
                    const auto hitPoint = vm::point_at_distance(pickRay, pointDist);
                    pickResult.addHit(Model::Hit::hit(HandleHitType, pointDist, hitPoint, position));
                }
            }
        }

        void FaceHandleManager::addHandles(const Model::BrushNode* brushNode) {
            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushFace* face : brush.faces()) {
                add(face->polygon());
            }
        }

        void FaceHandleManager::removeHandles(const Model::BrushNode* brushNode) {
            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushFace* face : brush.faces()) {
                assertResult(remove(face->polygon()))
            }
        }

        Model::HitType::Type FaceHandleManager::hitType() const {
            return HandleHitType;
        }

        bool FaceHandleManager::isIncident(const Handle& handle, const Model::BrushNode* brush) const {
            return brush->hasFace(handle);
        }
    }
}

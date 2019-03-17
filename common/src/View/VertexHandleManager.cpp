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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/Grid.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/intersection.h>

namespace TrenchBroom {
    namespace View {
        VertexHandleManagerBase::~VertexHandleManagerBase() {}

        const Model::Hit::HitType VertexHandleManager::HandleHit = Model::Hit::freeHitType();

        void VertexHandleManager::pick(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const auto& entry : m_handles) {
                const auto& position = entry.first;
                const auto distance = camera.pickPointHandle(pickRay, position, pref(Preferences::HandleRadius));
                if (!vm::isnan(distance)) {
                    const auto hitPoint = pickRay.pointAtDistance(distance);
                    const auto error = vm::squaredDistance(pickRay, position).distance;
                    pickResult.addHit(Model::Hit::hit(HandleHit, distance, hitPoint, position, error));
                }
            }
        }

        void VertexHandleManager::addHandles(const Model::Brush* brush) {
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                add(vertex->position());
            }
        }

        void VertexHandleManager::removeHandles(const Model::Brush* brush) {
            for (const Model::BrushVertex* vertex : brush->vertices()) {
                assertResult(remove(vertex->position()));
            }
        }

        Model::Hit::HitType VertexHandleManager::hitType() const {
            return HandleHit;
        }

        bool VertexHandleManager::isIncident(const Handle& handle, const Model::Brush* brush) const {
            return brush->hasVertex(handle);
        }

        const Model::Hit::HitType EdgeHandleManager::HandleHit = Model::Hit::freeHitType();

        void EdgeHandleManager::pickGridHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const vm::segment3& position = entry.first;
                const FloatType edgeDist = camera.pickLineSegmentHandle(pickRay, position, pref(Preferences::HandleRadius));
                if (!vm::isnan(edgeDist)) {
                    const vm::vec3 pointHandle = grid.snap(pickRay.pointAtDistance(edgeDist), position);
                    const FloatType pointDist = camera.pickPointHandle(pickRay, pointHandle, pref(Preferences::HandleRadius));
                    if (!vm::isnan(pointDist)) {
                        const vm::vec3 hitPoint = pickRay.pointAtDistance(pointDist);
                        pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, hitPoint, HitType(position, pointHandle)));
                    }
                }
            }
        }

        void EdgeHandleManager::pickCenterHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const vm::segment3& position = entry.first;
                const vm::vec3 pointHandle = position.center();

                const FloatType pointDist = camera.pickPointHandle(pickRay, pointHandle, pref(Preferences::HandleRadius));
                if (!vm::isnan(pointDist)) {
                    const vm::vec3 hitPoint = pickRay.pointAtDistance(pointDist);
                    pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, hitPoint, position));
                }
            }
        }

        void EdgeHandleManager::addHandles(const Model::Brush* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                add(vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position()));
            }
        }

        void EdgeHandleManager::removeHandles(const Model::Brush* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                assertResult(remove(vm::segment3(edge->firstVertex()->position(), edge->secondVertex()->position())));
            }
        }

        Model::Hit::HitType EdgeHandleManager::hitType() const {
            return HandleHit;
        }

        bool EdgeHandleManager::isIncident(const Handle& handle, const Model::Brush* brush) const {
            return brush->hasEdge(handle);
        }

        const Model::Hit::HitType FaceHandleManager::HandleHit = Model::Hit::freeHitType();

        void FaceHandleManager::pickGridHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const auto& entry : m_handles) {
                const auto& position = entry.first;

                const auto [valid, plane] = vm::fromPoints(std::begin(position), std::end(position));
                if (!valid) {
                    continue;
                }

                const auto distance = vm::intersect(pickRay, plane, std::begin(position), std::end(position));
                if (!vm::isnan(distance)) {
                    const auto pointHandle = grid.snap(pickRay.pointAtDistance(distance), plane);

                    const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, pref(Preferences::HandleRadius));
                    if (!vm::isnan(pointDist)) {
                        const auto hitPoint = pickRay.pointAtDistance(pointDist);
                        pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, hitPoint, HitType(position, pointHandle)));
                    }
                }
            }
        }

        void FaceHandleManager::pickCenterHandle(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const auto& position = entry.first;
                const auto pointHandle = position.center();

                const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, pref(Preferences::HandleRadius));
                if (!vm::isnan(pointDist)) {
                    const auto hitPoint = pickRay.pointAtDistance(pointDist);
                    pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, hitPoint, position));
                }
            }
        }

        void FaceHandleManager::addHandles(const Model::Brush* brush) {
            for (const Model::BrushFace* face : brush->faces()) {
                add(face->polygon());
            }
        }

        void FaceHandleManager::removeHandles(const Model::Brush* brush) {
            for (const Model::BrushFace* face : brush->faces()) {
                assertResult(remove(face->polygon()));
            }
        }

        Model::Hit::HitType FaceHandleManager::hitType() const {
            return HandleHit;
        }

        bool FaceHandleManager::isIncident(const Handle& handle, const Model::Brush* brush) const {
            return brush->hasFace(handle);
        }
    }
}

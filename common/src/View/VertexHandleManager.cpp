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

namespace TrenchBroom {
    namespace View {
        VertexHandleManagerBase::~VertexHandleManagerBase() {}

        const Model::Hit::HitType VertexHandleManager::HandleHit = Model::Hit::freeHitType();

        void VertexHandleManager::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const Vec3& position = entry.first;
                const FloatType distance = camera.pickPointHandle(pickRay, position, pref(Preferences::HandleRadius));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    pickResult.addHit(Model::Hit::hit(HandleHit, distance, hitPoint, position));
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
                remove(vertex->position());
            }
        }
        
        const Model::Hit::HitType EdgeHandleManager::HandleHit = Model::Hit::freeHitType();
        
        void EdgeHandleManager::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const Edge3& position = entry.first;
                const FloatType distance = camera.pickLineSegmentHandle(pickRay, position, pref(Preferences::HandleRadius));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    pickResult.addHit(Model::Hit::hit(HandleHit, distance, hitPoint, position));
                }
            }
        }
        
        void EdgeHandleManager::pick(const Ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const Edge3& position = entry.first;
                const FloatType edgeDist = camera.pickLineSegmentHandle(pickRay, position, pref(Preferences::HandleRadius));
                if (!Math::isnan(edgeDist)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(edgeDist);
                    const Vec3 snapped = grid.snap(hitPoint, position);
                    const FloatType pointDist = camera.pickPointHandle(pickRay, snapped, pref(Preferences::HandleRadius));
                    if (!Math::isnan(pointDist))
                        pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, pickRay.pointAtDistance(pointDist), snapped));
                }
            }
        }

        void EdgeHandleManager::addHandles(const Model::Brush* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                add(Edge3(edge->firstVertex()->position(), edge->secondVertex()->position()));
            }
        }
        
        void EdgeHandleManager::removeHandles(const Model::Brush* brush) {
            for (const Model::BrushEdge* edge : brush->edges()) {
                remove(Edge3(edge->firstVertex()->position(), edge->secondVertex()->position()));
            }
        }
        
        const Model::Hit::HitType FaceHandleManager::HandleHit = Model::Hit::freeHitType();
        
        void FaceHandleManager::pick(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const Polygon3& position = entry.first;
                const FloatType distance = intersectPolygonWithRay(pickRay, std::begin(position), std::end(position));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    pickResult.addHit(Model::Hit::hit(HandleHit, distance, hitPoint, position));
                }
            }
        }
        
        void FaceHandleManager::pick(const Ray3& pickRay, const Renderer::Camera& camera, const Grid& grid, Model::PickResult& pickResult) const {
            for (const HandleEntry& entry : m_handles) {
                const Polygon3& position = entry.first;
                
                Plane3 plane;
                if (!getPlane(std::begin(position), std::end(position), plane))
                    continue;
                
                const FloatType distance = intersectPolygonWithRay(pickRay, plane, std::begin(position), std::end(position));
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                    const Vec3 snapped = grid.snap(hitPoint, position, plane.normal);
                    
                    const FloatType pointDist = camera.pickPointHandle(pickRay, snapped, pref(Preferences::HandleRadius));
                    if (!Math::isnan(pointDist))
                        pickResult.addHit(Model::Hit::hit(HandleHit, pointDist, pickRay.pointAtDistance(pointDist), snapped));
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
                remove(face->polygon());
            }
        }
    }
}

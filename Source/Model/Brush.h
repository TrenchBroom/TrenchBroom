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

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "IO/ByteBuffer.h"
#include "Model/BrushGeometry.h"
#include "Model/EditState.h"
#include "Model/FaceTypes.h"
#include "Model/MapObject.h"
#include "Utility/Allocator.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <iterator>
#include <vector>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Face;
        class Texture;

        class Brush : public MapObject, public Utility::Allocator<Brush> {
        protected:
            class Entity* m_entity;
            FaceList m_faces;
            BrushGeometry* m_geometry;

            unsigned int m_selectedFaceCount;

            const BBoxf& m_worldBounds;
            bool m_forceIntegerFacePoints;
            
            // set if touched during vertex manip, cleared in MoveVerticesTool::handleDeactivate
            bool m_needsRebuild;
            
            void init();
        public:
            Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const FaceList& faces);
            Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Brush& brushTemplate);
            Brush(const BBoxf& worldBounds, bool forceIntegerFacePoints, const BBoxf& brushBounds, Texture* texture);
            ~Brush();

            void restore(const Brush& brushTemplate, bool checkId = false);
            void restore(const FaceList& faces);

            inline MapObject::Type objectType() const {
                return MapObject::BrushObject;
            }

            inline class Entity* entity() const {
                return m_entity;
            }

            void setEntity(class Entity* entity);

            inline const FaceList& faces() const {
                return m_faces;
            }

            inline bool partiallySelected() const {
                return m_selectedFaceCount > 0;
            }

            inline void incSelectedFaceCount() {
                m_selectedFaceCount++;
            }

            inline void decSelectedFaceCount() {
                m_selectedFaceCount--;
            }

            virtual EditState::Type setEditState(EditState::Type editState);

            inline const BBoxf& worldBounds() const {
                return m_worldBounds;
            }

            inline bool forceIntegerFacePoints() const {
                return m_forceIntegerFacePoints;
            }
            
            void setForceIntegerFacePoints(bool forceIntegerFacePoints);
            
            inline bool needsRebuild() const {
                return m_needsRebuild;
            }
            
            void setNeedsRebuild(bool needsRebuild) {
                m_needsRebuild = needsRebuild;
            }
            
            inline const Vec3f& center() const {
                return m_geometry->center;
            }

            inline const BBoxf& bounds() const {
                return m_geometry->bounds;
            }

            inline const VertexList& vertices() const {
                return m_geometry->vertices;
            }

            inline const FaceList incidentFaces(const Vertex& vertex) const {
                const SideList sides = m_geometry->incidentSides(&vertex);
                FaceList result;
                result.reserve(sides.size());

                SideList::const_iterator it, end;
                for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                    const Side& side = **it;
                    result.push_back(side.face);
                }

                return result;
            }

            inline const EdgeList& edges() const {
                return m_geometry->edges;
            }

            inline bool closed() const {
                return m_geometry->closed();
            }

            void rebuildGeometry();

            void transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTextures, const bool invertOrientation);

            bool clip(Face& face);
            
            void correct(float epsilon);
            void snap(unsigned int snapTo);

            bool canMoveBoundary(const Face& face, const Vec3f& delta) const;
            void moveBoundary(Face& face, const Vec3f& delta, bool lockTexture);

            bool canMoveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta) const;
            Vec3f::List moveVertices(const Vec3f::List& vertexPositions, const Vec3f& delta);
            bool canMoveEdges(const EdgeInfoList& edgeInfos, const Vec3f& delta) const;
            EdgeInfoList moveEdges(const EdgeInfoList& edgeInfos, const Vec3f& delta);
            bool canMoveFaces(const FaceInfoList& faceInfos, const Vec3f& delta) const;
            FaceInfoList moveFaces(const FaceInfoList& faceInfos, const Vec3f& delta);

            bool canSplitEdge(const EdgeInfo& edgeInfo, const Vec3f& delta) const;
            Vec3f splitEdge(const EdgeInfo& edgeInfo, const Vec3f& delta);
            bool canSplitFace(const FaceInfo& faceInfo, const Vec3f& delta) const;
            Vec3f splitFace(const FaceInfo& faceInfo, const Vec3f& delta);

            void pick(const Rayf& ray, PickResult& pickResults);
            bool containsPoint(const Vec3f point) const;
            bool intersectsBrush(const Brush& brush) const;
            bool containsBrush(const Brush& brush) const;
            bool intersectsEntity(const Entity& entity) const;
            bool containsEntity(const Entity& entity) const;
        };
        
        inline static EntityBrushesMap entityBrushes(const BrushList& brushes) {
            EntityBrushesMap entityBrushesMap;
            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                Model::Entity* entity = brush->entity();
                entityBrushesMap[entity].push_back(brush);
            }
            return entityBrushesMap;
        }
        
        inline static EntityBrushesMap mergeEntityBrushes(const EntityBrushesMap& left, const EntityBrushesMap& right) {
            EntityBrushesMap result = left;
            EntityBrushesMap::const_iterator it, end;
            for (it = right.begin(), end = right.end(); it != end; ++it) {
                Entity* entity = it->first;
                const BrushList& brushes = it->second;
                BrushList& resultBrushes = result[entity];
                resultBrushes.insert(resultBrushes.end(), brushes.begin(), brushes.end());
            }
            return result;
        }
    }
}

#endif /* defined(__TrenchBroom__Brush__) */

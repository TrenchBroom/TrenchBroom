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

#ifndef __TrenchBroom__Face__
#define __TrenchBroom__Face__

#include "Model/BrushGeometry.h"
#include "Model/FaceTypes.h"
#include "Renderer/FaceVertex.h"
#include "Utility/Allocator.h"
#include "Utility/FindPlanePoints.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Texture;

        class Face;
        class FindFacePoints {
        protected:
            virtual size_t selectInitialPoints(const Face& face, FacePoints& points) const = 0;
            virtual void findPoints(const Planef& plane, FacePoints& points, size_t numPoints) const = 0;
        public:
            virtual ~FindFacePoints() {}

            static const FindFacePoints& instance(bool forceIntegerCoordinates);
            inline void operator()(const Face& face, FacePoints& points) const;
        };

        class FindIntegerFacePoints : public FindFacePoints {
        private:
            FindIntegerPlanePoints m_findPoints;
        protected:
            inline size_t selectInitialPoints(const Face& face, FacePoints& points) const;
            inline void findPoints(const Planef& plane, FacePoints& points, size_t numPoints) const;
        public:
            static const FindIntegerFacePoints Instance;
        };

        class FindFloatFacePoints : public FindFacePoints {
        private:
            FindFloatPlanePoints m_findPoints;
        protected:
            inline size_t selectInitialPoints(const Face& face, FacePoints& points) const;
            inline void findPoints(const Planef& plane, FacePoints& points, size_t numPoints) const;
        public:
            static const FindFloatFacePoints Instance;
        };

        class Face : public Utility::Allocator<Face> {
        public:
            enum ContentType {
                CTLiquid,
                CTClip,
                CTSkip,
                CTHint,
                CTTrigger,
                CTDefault
            };
            
            class WeightOrder {
            private:
                const Planef::WeightOrder& m_planeOrder;
            public:
                WeightOrder(const Planef::WeightOrder& planeOrder) :
                m_planeOrder(planeOrder) {}

                inline bool operator()(const Face* lhs, const Face* rhs) const {
                    return m_planeOrder(lhs->boundary(), rhs->boundary());
                }
            };
        protected:
            static const Vec3f BaseAxes[18];

            Brush* m_brush;
            Side* m_side;

            unsigned int m_faceId;

            /*
             * The order of points, when looking from outside the face:
             *
             * 0-----------1
             * |
             * |
             * |
             * |
             * 2
             *
             * It must hold that
             * (m_points[2] - m_points[0]).cross(m_points[1] - m_points[0]).equals(boundary().normal)
             */
            FacePoints m_points;
            Planef m_boundary;
            BBoxf m_worldBounds;
            bool m_forceIntegerFacePoints;

            String m_textureName;
            Texture* m_texture;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;

            mutable bool m_texAxesValid;
            mutable unsigned int m_texPlanefNormIndex;
            mutable unsigned int m_texFaceNormIndex;
            mutable Vec3f m_texAxisX;
            mutable Vec3f m_texAxisY;
            mutable Vec3f m_scaledTexAxisX;
            mutable Vec3f m_scaledTexAxisY;

            mutable bool m_vertexCacheValid;
            mutable Renderer::FaceVertex::List m_vertexCache;

            size_t m_filePosition;
            bool m_selected;
            
            ContentType m_contentType;

            inline void rotateTexAxes(Vec3f& xAxis, Vec3f& yAxis, const float angle, const unsigned int planeNormIndex) const {
                // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
                const Quatf rot(planeNormIndex == 12 ? -angle : angle, BaseAxes[planeNormIndex]);
                xAxis = rot * xAxis;
                yAxis = rot * yAxis;
            }

            void init();
            void texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, unsigned int& planeNormIndex, unsigned int& faceNormIndex) const;
            void validateTexAxes(const Vec3f& faceNormal) const;
            void validateVertexCache() const;

            void projectOntoTexturePlane(Vec3f& xAxis, Vec3f& yAxis);
            void compensateTransformation(const Mat4f& transformation);
            void updateContentType();
        public:
            Face(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName);
            Face(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Face& faceTemplate);
            Face(const Face& face);
			~Face();

            void restore(const Face& faceTemplate);

            inline Brush* brush() const {
                return m_brush;
            }

            void setBrush(Brush* brush);

            inline Side* side() const {
                return m_side;
            }

            inline void setSide(Side* side) {
                m_side = side;
            }

            inline FaceInfo faceInfo() const {
                assert(m_side != NULL);
                return m_side->info();
            }

            inline unsigned int faceId() const {
                return m_faceId;
            }

            void updatePointsFromVertices();
            void updatePointsFromBoundary();

            inline void getPoints(Vec3f& point1, Vec3f& point2, Vec3f& point3) const {
                point1 = m_points[0];
                point2 = m_points[1];
                point3 = m_points[2];
            }

            inline const Vec3f& point(size_t index) const {
                assert(index < 3);
                return m_points[index];
            }

            inline const Planef& boundary() const {
                return m_boundary;
            }

            inline const BBoxf& worldBounds() const {
                return m_worldBounds;
            }

            void correctFacePoints();
            
            inline bool forceIntegerFacePoints() const {
                return m_forceIntegerFacePoints;
            }

            void setForceIntegerFacePoints(bool forceIntegerFacePoints);
            
            inline const VertexList& vertices() const {
                return m_side->vertices;
            }

            inline const EdgeList& edges() const {
                return m_side->edges;
            }

            inline Vec3f center() const {
                return centerOfVertices(m_side->vertices);
            }

            inline ContentType contentType() const {
                return m_contentType;
            }
            
            inline const String& textureName() const {
                return m_textureName;
            }

            inline void setTextureName(const String& textureName) {
                m_textureName = textureName;
                updateContentType();
            }

            inline Texture* texture() const {
                return m_texture;
            }

            void setTexture(Texture* texture);

            inline float xOffset() const {
                return m_xOffset;
            }

            inline void setXOffset(float xOffset) {
                if (xOffset == m_xOffset)
                    return;
                m_xOffset = xOffset;
                m_vertexCacheValid = false;
            }

            inline float yOffset() const {
                return m_yOffset;
            }

            inline void setYOffset(float yOffset) {
                if (yOffset == m_yOffset)
                    return;
                m_yOffset = yOffset;
                m_vertexCacheValid = false;
            }

            inline float rotation() const {
                return m_rotation;
            }

            inline void setRotation(float rotation) {
                if (rotation == m_rotation)
                    return;
                m_rotation = rotation;
                m_texAxesValid = false;
                m_vertexCacheValid = false;
            }

            inline float xScale() const {
                return m_xScale;
            }

            inline void setXScale(float xScale) {
                if (xScale == m_xScale)
                    return;
                m_xScale = xScale;
                m_texAxesValid = false;
                m_vertexCacheValid = false;
            }

            inline float yScale() const {
                return m_yScale;
            }

            inline void setYScale(float yScale) {
                if (yScale == m_yScale)
                    return;
                m_yScale = yScale;
                m_texAxesValid = false;
                m_vertexCacheValid = false;
            }

            inline void setAttributes(const Face& face) {
                setXScale(face.xScale());
                setYScale(face.yScale());
                setXOffset(face.xOffset());
                setYOffset(face.yOffset());
                setRotation(face.rotation());
                setTextureName(face.textureName());
                setTexture(face.texture());
            }

            inline void invalidateTexAxes() {
                m_texAxesValid = false;
            }

            void moveTexture(const Vec3f& up, const Vec3f& right, Direction direction, float distance);
            void rotateTexture(float angle);

            inline void invalidateVertexCache() {
                m_vertexCacheValid = false;
            }

            inline const Renderer::FaceVertex::List& cachedVertices() const {
                if (!m_vertexCacheValid)
                    validateVertexCache();
                return m_vertexCache;
            }

            inline bool selected() const {
                return m_selected;
            }

            void setSelected(bool selected);

            inline size_t filePosition() const {
                return m_filePosition;
            }

            inline void setFilePosition(size_t filePosition) {
                m_filePosition = filePosition;
            }

            void transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTexture, const bool invertOrientation);
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */

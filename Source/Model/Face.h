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
#include "Utility/String.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Texture;
        
        /**
         * \brief This class represents a brush face.
         *
         * Each face is described by a boundary plane which is given by three points. Additionally, faces are associated
         * with a texture name, the texture offset, rotation and scale. The offset, rotation and scale parameters 
         * control the generation of texture coordinates.
         *
         * Texture coordinates and texture axes are transient (computed on demand) and are therefore marked as mutable.
         * Geometric data such as edges and vertices are stored in an instance of class Side.
         */
        class Face : public Utility::Allocator<Face> {
        protected:
            static const Vec3f* BaseAxes[18];
            
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
            Vec3f m_points[3];
            Plane m_boundary;
            BBox m_worldBounds;
            
            String m_textureName;
            Texture* m_texture;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;

            mutable bool m_texAxesValid;
            mutable unsigned int m_texPlaneNormIndex;
            mutable unsigned int m_texFaceNormIndex;
            mutable Vec3f m_texAxisX;
            mutable Vec3f m_texAxisY;
            mutable Vec3f m_scaledTexAxisX;
            mutable Vec3f m_scaledTexAxisY;

            mutable bool m_vertexCacheValid;
            mutable Renderer::FaceVertex::List m_vertexCache;
            
            size_t m_filePosition;
            bool m_selected;

            inline void rotateTexAxes(Vec3f& xAxis, Vec3f& yAxis, const float angle, const unsigned int planeNormIndex) const {
                // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
                Quat rot(planeNormIndex == 12 ? -angle : angle, *BaseAxes[planeNormIndex]);
                xAxis = rot * xAxis;
                yAxis = rot * yAxis;
            }
            
            void init();
            void texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, unsigned int& planeNormIndex, unsigned int& faceNormIndex) const;
            void validateTexAxes(const Vec3f& faceNormal) const;
            void validateVertexCache() const;
            
            void projectOntoTexturePlane(Vec3f& xAxis, Vec3f& yAxis);
            
            void compensateTransformation(const Mat4f& transformation);
        public:
            Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName);
            Face(const BBox& worldBounds, const Face& faceTemplate);
            Face(const Face& face);
			~Face();
            
            /**
             * Restores the boundary, texture name, offset, rotation and scale parameters as well as the selection state
             * from the given face. Invalidates transient state of this face.
             */
            void restore(const Face& faceTemplate);
            
            /**
             * Returns the brush which owns this face.
             */
            inline Brush* brush() const {
                return m_brush;
            }
            
            /**
             * Sets the brush that owns this face. Also increments and decrements the number of selected faces of the 
             * current owner and the given brush if they are not null.
             *
             * @param brush the new owner of this face. May be null.
             */
            void setBrush(Brush* brush);
            
            /**
             * Returns the Side instance that stores the geometric data of this face.
             */
            inline Side* side() const {
                return m_side;
            }
            
            /**
             * Sets the Side instance that stores the geometric data of this face.
             *
             * @param side the side instance. Maybe be null.
             */
            inline void setSide(Side* side) {
                m_side = side;
            }
            
            /**
             * Returns a unique id for this face. This id is not persistent.
             */
            inline unsigned int faceId() const {
                return m_faceId;
            }
            
            /**
             * Updates the boundary points from the vertices of this face. Afterwards, all vertices of this face lie
             * on the boundary plane. Be aware that the Side that belongs to this face must not be null.
             */
            void updatePoints();
            
            /**
             * Sets the given points to the points of the boundary plane.
             */
            inline void getPoints(Vec3f& point1, Vec3f& point2, Vec3f& point3) const {
                point1 = m_points[0];
                point2 = m_points[1];
                point3 = m_points[2];
            }
            
            /**
             * Returns the boundary point with the given index (zero based).
             */
            inline const Vec3f& point(unsigned int index) const {
                assert(index < 3);
                return m_points[index];
            }
            
            /**
             * Returns the boundary plane.
             */
            inline const Plane& boundary() const {
                return m_boundary;
            }
            
            /**
             * Returns the maximum bounds of the world.
             */
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }

            /**
             * Returns the vertices of this face in clockwise order.
             */
            inline const VertexList& vertices() const {
                return m_side->vertices;
            }
            
            /**
             * Returns the edges of this face in clockwise order. The start vertex of the first edge is the first 
             * vertex in the list returned by vertices().
             */
            inline const EdgeList& edges() const {
                return m_side->edges;
            }
            
            /**
             * Returns the center of this face.
             */
            inline Vec3f center() const {
                return centerOfVertices(m_side->vertices);
            }
            
            /**
             * Returns the name of the texture for this face.
             */
            inline const String& textureName() const {
                return m_textureName;
            }

            /**
             * Sets the name of the texture for this face.
             */
            inline void setTextureName(const String& textureName) {
                m_textureName = textureName;
            }
            
            /**
             * Returns the texture for this face. May return null if the texture was not set, due to e.g. no texture
             * being found in the texture manager during map load.
             */
            inline Texture* texture() const {
                return m_texture;
            }
            
            /**
             * Sets the texture for this face. Increments and decrements the usage count for the current texture and the
             * given texture. Also sets the texture name for this face accordingly.
             *
             * The new texture for this face. May be null.
             */
            void setTexture(Texture* texture);
            
            /**
             * Returns the texture X offset of this face.
             */
            inline float xOffset() const {
                return m_xOffset;
            }
            
            /**
             * Sets the texture X offset of this face and invalidates the transient texture data.
             */
            inline void setXOffset(float xOffset) {
                if (xOffset == m_xOffset)
                    return;
                m_xOffset = xOffset;
                m_vertexCacheValid = false;
            }
            
            /**
             * Returns the texture Y offset of this face.
             */
            inline float yOffset() const {
                return m_yOffset;
            }
            
            /**
             * Sets the texture Y offset of this face and invalidates the transient texture data.
             */
            inline void setYOffset(float yOffset) {
                if (yOffset == m_yOffset)
                    return;
                m_yOffset = yOffset;
                m_vertexCacheValid = false;
            }
            
            /**
             * Returns the texture rotation of this face.
             */
            inline float rotation() const {
                return m_rotation;
            }
            
            /**
             * Sets the texture rotation of this face and invalidates the transient texture data.
             */
            inline void setRotation(float rotation) {
                if (rotation == m_rotation)
                    return;
                m_rotation = rotation;
                m_texAxesValid = false;
                m_vertexCacheValid = false;
            }
            
            /**
             * Returns the texture X scale of this face.
             */
            inline float xScale() const {
                return m_xScale;
            }
            
            /**
             * Sets the texture X scale of this face and invalidates the transient texture data.
             */
            inline void setXScale(float xScale) {
                if (xScale == m_xScale)
                    return;
                m_xScale = xScale;
                m_texAxesValid = false;
                m_vertexCacheValid = false;
            }
            
            /**
             * Returns the texture Y scale of this face.
             */
            inline float yScale() const {
                return m_yScale;
            }
            
            /**
             * Sets the texture Y scale of this face and invalidates the transient texture data.
             */
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
            
            /**
             * Invalidates the cached texture axes.
             */
            inline void invalidateTexAxes() {
                m_texAxesValid = false;
            }
            
            /**
             * Modifies the offsets such that the texture is moved in the given direction by the given distance, relative
             * to the given view coordinate system.
             */
            void moveTexture(const Vec3f& up, const Vec3f& right, Direction direction, float distance);
            
            /**
             * Modifies the rotation such that the texture is rotated by the given angle (in degrees) in clockwise
             * direction.
             */
            void rotateTexture(float angle);
            
            /**
             * Invalidates the vertex cache.
             */
            inline void invalidateVertexCache() {
                m_vertexCacheValid = false;
            }
            
            inline const Renderer::FaceVertex::List& cachedVertices() const {
                if (!m_vertexCacheValid)
                    validateVertexCache();
                return m_vertexCache;
            }
            
            /**
             * Indicates whether this face is currently selected.
             */
            inline bool selected() const {
                return m_selected;
            }
            
            /**
             * Specifies whether this face is currently selected. This method should usually only be called by the
             * EditStateManager.
             */
            void setSelected(bool selected);
            
            /**
             * Returns the line of the map file from which this texture was read, if applicable. Returns -1 if this face
             * was not read from a map file.
             */
            inline size_t filePosition() const {
                return m_filePosition;
            }
            
            /**
             * Specifies the line of the map file from which this texture was read. This method should usually only be
             * called by the map parser.
             */
            inline void setFilePosition(size_t filePosition) {
                m_filePosition = filePosition;
            }
            
            /**
             * Translates this face by the given delta vector.
             */
            void translate(const Vec3f& delta, bool lockTexture);

            /**
             * Rotates this face about the given axis and center.
             */
            void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTexture);
            
            /**
             * Rotates this face by the given rotation and center.
             */
            void rotate(const Quat& rotation, const Vec3f& center, bool lockTexture);

            /**
             * Flips this face along the given axis.
             */
            void flip(Axis::Type axis, const Vec3f& center, bool lockTexture);
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */

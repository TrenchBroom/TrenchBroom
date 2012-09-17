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
        class Face {
        protected:
            Brush* m_brush;
            Side* m_side;
            
            int m_faceId;
            
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
            mutable int m_texPlaneNormIndex;
            mutable int m_texFaceNormIndex;
            mutable Vec3f m_texAxisX;
            mutable Vec3f m_texAxisY;
            mutable Vec3f m_scaledTexAxisX;
            mutable Vec3f m_scaledTexAxisY;
            
            mutable bool m_coordsValid;
            mutable Vec2f::List m_texCoords;
            
            size_t m_filePosition;
            bool m_selected;
            
            void init();
            void texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const;
            void validateTexAxes(const Vec3f& faceNormal) const;
            void validateCoords() const;
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
            inline int faceId() const {
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
             * Returns the name of the texture for this face.
             */
            inline const String& textureName() const {
                return m_textureName;
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
                m_coordsValid = false;
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
                m_coordsValid = false;
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
                m_coordsValid = false;
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
                m_coordsValid = false;
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
                m_coordsValid = false;
            }
            
            /**
             * Returns the texture coordinates for each vertex of this face. The texture coordinates are in the same
             * order as the vertices returned by the vertices() function.
             */
            inline const Vec2f::List& texCoords() const {
                if (!m_coordsValid)
                    validateCoords();
                return m_texCoords;
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
             * Moves this face along its normal by the given distance.
             *
             * @param dist the distance by which to move the face
             * @param lockTexture specifies whether texture lock is enabled
             */
            void move(float dist, bool lockTexture);

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
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */

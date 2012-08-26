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

#include "Model/FaceTypes.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Texture;
        class Side;
        
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

            bool m_texAxesValid;
            bool m_coordsValid;
            int m_texPlaneNormIndex;
            int m_texFaceNormIndex;
            Vec3f m_texAxisX;
            Vec3f m_texAxisY;
            Vec3f m_scaledTexAxisX;
            Vec3f m_scaledTexAxisY;
            
            Vec2f::List m_gridCoords;
            Vec2f::List m_texCoords;
            
            size_t m_filePosition;
            bool m_selected;
            
            void init();
        public:
            Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName);
            Face(const BBox& worldBounds, const Face& faceTemplate);
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
            
            inline int faceId() const {
                return m_faceId;
            }
            
            void updatePoints();
            
            inline void getPoints(Vec3f& point1, Vec3f& point2, Vec3f& point3) const {
                point1 = m_points[0];
                point2 = m_points[1];
                point3 = m_points[2];
            }
            
            inline const Plane& boundary() const {
                return m_boundary;
            }
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }

            inline const String& textureName() const {
                return m_textureName;
            }
            
            void setTextureName(const String& textureName);
            
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
                m_coordsValid = false;
            }
            
            inline float yOffset() const {
                return m_yOffset;
            }
            
            inline void setYOffset(float yOffset) {
                if (yOffset == m_yOffset)
                    return;
                m_yOffset = yOffset;
                m_coordsValid = false;
            }
            
            inline float rotation() const {
                return m_rotation;
            }
            
            inline void setRotation(float rotation) {
                if (rotation == m_rotation)
                    return;
                m_rotation = rotation;
                m_texAxesValid = false;
                m_coordsValid = false;
            }
            
            inline float xScale() const {
                return m_xScale;
            }
            
            inline void setXScale(float xScale) {
                if (xScale == m_xScale)
                    return;
                m_xScale = xScale;
                m_texAxesValid = false;
                m_coordsValid = false;
            }
            
            inline float yScale() const {
                return m_yScale;
            }
            
            inline void setYScale(float yScale) {
                if (yScale == m_yScale)
                    return;
                m_yScale = yScale;
                m_texAxesValid = false;
                m_coordsValid = false;
            }
            
            inline bool selected() const {
                return m_selected;
            }
            
            inline void setSelected(bool selected) {
                m_selected = selected;
            }
            
            inline size_t filePosition() const {
                return m_filePosition;
            }
            
            inline void setFilePosition(size_t filePosition) {
                m_filePosition = filePosition;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Face__) */

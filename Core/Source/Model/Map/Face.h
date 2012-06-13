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

#ifndef TrenchBroom_Face_h
#define TrenchBroom_Face_h

#include <vector>
#include "Utilities/VecMath.h"
#include "Model/Map/BrushGeometry.h"

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;
    }
    
    namespace Model {
        namespace Assets {
            class Texture;
        }
        
        class Brush;
        
        class Face {
        protected:
            int m_texPlaneNormIndex;
            int m_texFaceNormIndex;
            Vec3f m_texAxisX;
            Vec3f m_texAxisY;
            Vec3f m_scaledTexAxisX;
            Vec3f m_scaledTexAxisY;
            
            std::vector<Vec2f> m_gridCoords;
            std::vector<Vec2f> m_texCoords;

            void init();
            void texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const;
            void validateTexAxes(const Vec3f& faceNormal);
            void compensateTransformation(const Mat4f& transformation);
            void validateCoords();
        public:
            int faceId;
            Brush* brush;
            
            Vec3f points[3];
            Plane boundary;
            const BBox& worldBounds;
            
            std::string textureName;
            Assets::Texture* texture;
            float xOffset;
            float yOffset;
            float rotation;
            float xScale;
            float yScale;

            void setTexture(Assets::Texture* aTexture);
            void setXOffset(float aXOffset);
            void setYOffset(float aYOffset);
            void setRotation(float aRotation);
            void setXScale(float aXScale);
            void setYScale(float aYScale);

            Side* side;
            
            int filePosition;
            bool selected;
            bool coordsValid;
            bool texAxesValid;

            Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const std::string& textureName);
            Face(const BBox& worldBounds, const Face& faceTemplate);
            Face(const Face& face);
            
            void restore(const Face& faceTemplate);
            
            void getPoints(Vec3f& point1, Vec3f& point2, Vec3f& point3) const;
            void updatePoints();
            Vec3f center() const;
            const std::vector<Vec2f>& gridCoords();
            const std::vector<Vec2f>& texCoords();

            void translateOffsets(float delta, Vec3f dir);
            void rotateTexture(float angle);
            void translate(Vec3f delta, bool lockTexture);
            void rotate90(EAxis axis, Vec3f center, bool clockwise, bool lockTexture);
            void rotate(Quat rotation, Vec3f center, bool lockTexture);
            void flip(EAxis axis, Vec3f center, bool lockTexture);
            void move(float dist, bool lockTexture);
        };
    }
}

#endif

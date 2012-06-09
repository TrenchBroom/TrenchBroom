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
#include "Model/Map/FaceTypes.h"

namespace TrenchBroom {
    namespace Renderer {
        class VboBlock;
    }
    
    namespace Model {
        namespace Assets {
            class Texture;
        }
        
        class Brush;
        class Side;
        class Vertex;
        class Edge;
        
        class Face {
        protected:
            int m_faceId;
            Brush* m_brush;
            
            Vec3f m_points[3];
            Plane m_boundary;
            const BBox& m_worldBounds;
            
            std::string m_textureName;
            Assets::Texture* m_texture;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            
            Side* m_side;
            
            int m_texPlaneNormIndex;
            int m_texFaceNormIndex;
            Vec3f m_texAxisX;
            Vec3f m_texAxisY;
            Vec3f m_scaledTexAxisX;
            Vec3f m_scaledTexAxisY;
            bool m_texAxesValid;
            
            int m_filePosition;
            bool m_selected;
            Renderer::VboBlock* m_vboBlock;
            
            void init();
            void texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const;
            void validateTexAxes(const Vec3f& faceNormal);
            void compensateTransformation(const Mat4f& transformation);
        public:
            Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const std::string& textureName);
            Face(const BBox& worldBounds, const Face& faceTemplate);
            Face(const Face& face);
            ~Face();
            
            void restore(const Face& faceTemplate);
            
            int faceId() const;
            Brush* brush() const;
            void setBrush(Brush* brush);
            void setSide(Side* side);
            
            void points(Vec3f& point1, Vec3f& point2, Vec3f& point3) const;
            void updatePoints();
            Vec3f normal() const;
            Plane boundary() const;
            Vec3f center() const;
            const BBox& worldBounds() const;
            const std::vector<Vertex*>& vertices() const;
            const std::vector<Edge*>& edges() const;
            
            Assets::Texture* texture() const;
            const std::string& textureName() const;
            void setTexture(Assets::Texture* texture);
            int xOffset() const;
            void setXOffset(int xOffset);
            int yOffset() const;
            void setYOffset(int yOffset);
            float rotation() const;
            void setRotation(float rotation);
            float xScale() const;
            void setXScale(float xScale);
            float yScale() const;
            void setYScale(float yScale);
            
            void translateOffsets(float delta, Vec3f dir);
            void rotateTexture(float angle);
            void translate(Vec3f delta, bool lockTexture);
            void rotate90(EAxis axis, Vec3f center, bool clockwise, bool lockTexture);
            void rotate(Quat rotation, Vec3f center, bool lockTexture);
            void flip(EAxis axis, Vec3f center, bool lockTexture);
            void move(float dist, bool lockTexture);
            
            Vec2f textureCoords(const Vec3f& vertex);
            Vec2f gridCoords(const Vec3f& vertex);
            
            int filePosition() const;
            void setFilePosition(int filePosition);
            bool selected() const;
            void setSelected(bool selected);
            Renderer::VboBlock* vboBlock() const;
            void setVboBlock(Renderer::VboBlock* vboBlock);
        };
    }
}

#endif

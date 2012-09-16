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

#include "Face.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Texture.h"

namespace TrenchBroom {
    namespace Model {
        static const Vec3f* BaseAxes[18] = {
            &Vec3f::PosZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::NegZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::PosX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::NegX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::PosY, &Vec3f::PosX, &Vec3f::NegZ,
            &Vec3f::NegY, &Vec3f::PosX, &Vec3f::NegZ
        };
        
        void Face::init() {
            static int currentId = 1;
            m_faceId = currentId++;
            m_xOffset = 0.0f;
            m_yOffset = 0.0f;
            m_rotation = 0.0f;
            m_xScale = 1.0f;
            m_yScale = 1.0f;
            m_brush = NULL;
            m_texture = NULL;
            m_filePosition = 0;
            m_selected = false;
            m_texAxesValid = false;
            m_coordsValid = false;
        }
        
        void Face::texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const {
            int bestIndex = 0;
            float bestDot = -1;
            for (unsigned int i = 0; i < 6; i++) {
                float dot = faceNormal.dot(*BaseAxes[i * 3]);
                if (dot >= bestDot) {
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = *BaseAxes[bestIndex * 3 + 1];
            yAxis = *BaseAxes[bestIndex * 3 + 2];
            faceNormIndex = bestIndex * 3;
            planeNormIndex = (bestIndex / 2) * 6;
        }
        
        void Face::validateTexAxes(const Vec3f& faceNormal) const {
            texAxesAndIndices(faceNormal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            
            Quat rot(m_rotation * Math::Pi / 180, *BaseAxes[m_texPlaneNormIndex]);
            m_texAxisX = rot * m_texAxisX;
            m_texAxisY = rot * m_texAxisY;
            m_scaledTexAxisX = m_texAxisX / m_xScale;
            m_scaledTexAxisY = m_texAxisY / m_yScale;
            
            m_texAxesValid = true;
        }
        
        void Face::validateCoords() const {
            assert(m_side != NULL);
            
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            Axis::Type axis = m_boundary.normal.firstComponent();
            int width = m_texture != NULL ? m_texture->width() : 1;
            int height = m_texture != NULL ? m_texture->height() : 1;
            
            size_t vertexCount = m_side->vertices.size();
            m_gridCoords.resize(vertexCount);
            m_texCoords.resize(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                const Vec3f& vertex = m_side->vertices[i]->position;
                
                m_texCoords[i].x = (vertex.dot(m_scaledTexAxisX) + m_xOffset) / width,
                m_texCoords[i].y = (vertex.dot(m_scaledTexAxisY) + m_yOffset) / height;
                
                switch (axis) {
                    case Axis::AX:
                        m_gridCoords[i].x = (vertex.y + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.z + 0.5f) / 256.0f;
                        break;
                    case Axis::AY:
                        m_gridCoords[i].x = (vertex.x + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.z + 0.5f) / 256.0f;
                        break;
                    default:
                        m_gridCoords[i].x = (vertex.x + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.y + 0.5f) / 256.0f;
                        break;
                }
            }
            
            m_coordsValid = true;
        }
        
        Face::Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName) : m_worldBounds(worldBounds), m_textureName(textureName) {
            init();
            m_points[0] = point1;
            m_points[1] = point2;
            m_points[2] = point3;
            m_boundary.setPoints(m_points[0], m_points[1], m_points[2]);
        }
        
        Face::Face(const BBox& worldBounds, const Face& faceTemplate) : m_worldBounds(worldBounds) {
            init();
            restore(faceTemplate);
        }
        
        Face::Face(const Face& face) :
        m_faceId(face.faceId()),
        m_boundary(face.boundary()),
        m_worldBounds(face.worldBounds()),
        m_texture(face.texture()),
        m_textureName(face.textureName()),
        m_xOffset(face.xOffset()),
        m_yOffset(face.yOffset()),
        m_xScale(face.xScale()),
        m_yScale(face.yScale()),
        m_rotation(face.rotation()),
        m_side(NULL),
        m_texAxesValid(false),
        m_coordsValid(false),
        m_filePosition(face.filePosition()),
        m_selected(false) {
            face.getPoints(m_points[0], m_points[1], m_points[2]);
        }
        
		Face::~Face() {
			m_texPlaneNormIndex = -1;
			m_texFaceNormIndex = -1;
			m_texAxisX = Vec3f::NaN;
			m_texAxisY = Vec3f::NaN;
			m_scaledTexAxisX = Vec3f::NaN;
			m_scaledTexAxisY = Vec3f::NaN;
            
			m_gridCoords.clear();
			m_texCoords.clear();
            
			m_faceId *= -1;
			m_brush = NULL;
			
			for (unsigned int i = 0; i < 3; i++)
				m_points[i] = Vec3f::NaN;
			m_boundary.normal = Vec3f::NaN;
			m_boundary.distance = -1.0f;
            
			m_textureName = "___deleted";
			m_texture = NULL;
			m_xOffset = 0;
			m_yOffset = 0;
			m_rotation = 0.0f;
			m_xScale = 0.0f;
			m_yScale = 0.0f;
			m_side = NULL;
			m_filePosition = -1;
			m_selected = false;
			m_coordsValid = false;
			m_texAxesValid = false;
		}
        
        void Face::restore(const Face& faceTemplate) {
            faceTemplate.getPoints(m_points[0], m_points[1], m_points[2]);
            m_boundary = faceTemplate.boundary();
            m_xOffset = faceTemplate.xOffset();
            m_yOffset = faceTemplate.yOffset();
            m_rotation = faceTemplate.rotation();
            m_xScale = faceTemplate.xScale();
            m_yScale = faceTemplate.yScale();
            setTexture(faceTemplate.texture());
            m_texAxesValid = false;
            m_coordsValid = false;
			m_selected = faceTemplate.selected();
        }

        void Face::setBrush(Brush* brush) {
            if (brush == m_brush)
                return;
            
            if (m_brush != NULL && m_selected)
                m_brush->decSelectedFaceCount();
            m_brush = brush;
            if (m_brush != NULL && m_selected)
                m_brush->incSelectedFaceCount();
        }
        
        void Face::updatePoints() {
            Vec3f v1, v2;
            
            float bestDot = 1;
            int best = -1;
            size_t vertexCount = m_side->vertices.size();
            for (unsigned int i = 0; i < vertexCount && bestDot > 0; i++) {
                m_points[2] = m_side->vertices[pred(i, vertexCount)]->position;
                m_points[0] = m_side->vertices[i]->position;
                m_points[1] = m_side->vertices[succ(i, vertexCount)]->position;
                
                v1 = (m_points[2] - m_points[0]).normalized();
                v2 = (m_points[1] - m_points[0]).normalized();
                float dot = v1.dot(v2);
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            assert(best != -1);
            
            m_points[2] = m_side->vertices[pred(best, vertexCount)]->position;
            m_points[0] = m_side->vertices[best]->position;
            m_points[1] = m_side->vertices[succ(best, vertexCount)]->position;
            
            if (!m_boundary.setPoints(m_points[0], m_points[1], m_points[2])) {
                std::stringstream msg;
                msg << "Invalid face m_points "
                << m_points[0].x << " " << m_points[0].y << " " << m_points[0].z << "; "
                << m_points[1].x << " " << m_points[1].y << " " << m_points[1].z << "; "
                << m_points[2].x << " " << m_points[2].y << " " << m_points[2].z
                << " for face with ID " << m_faceId;
                throw GeometryException(msg);
            }
        }

        void Face::setTexture(Texture* texture) {
            if (texture == m_texture)
                return;
            
            if (m_texture != NULL)
                m_texture->decUsageCount();
            
            m_texture = texture;
            if (m_texture != NULL)
                m_textureName = texture->name();
            else
                m_textureName = Texture::Empty;
            
            if (m_texture != NULL)
                m_texture->incUsageCount();
            m_coordsValid = false;
        }

        void Face::setSelected(bool selected) {
            if (selected == m_selected)
                return;

            m_selected = selected;
            if (m_brush != NULL) {
                if (m_selected)
                    m_brush->incSelectedFaceCount();
                else
                    m_brush->decSelectedFaceCount();
            }
        }

        void Face::move(float dist, bool lockTexture) {
        }
    }
}

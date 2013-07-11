/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "BrushFace.h"

#include "Exceptions.h"
#include "VecMath.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        const Vec3 TextureCoordinateSystem::BaseAxes[18] = {
            Vec3::PosZ, Vec3::PosX, Vec3::NegY,
            Vec3::NegZ, Vec3::PosX, Vec3::NegY,
            Vec3::PosX, Vec3::PosY, Vec3::NegZ,
            Vec3::NegX, Vec3::PosY, Vec3::NegZ,
            Vec3::PosY, Vec3::PosX, Vec3::NegZ,
            Vec3::NegY, Vec3::PosX, Vec3::NegZ
        };
        
        TextureCoordinateSystem::TextureCoordinateSystem(BrushFace* face) :
        m_face(face),
        m_valid(false) {
            assert(m_face != NULL);
        }
        
        Vec2f TextureCoordinateSystem::textureCoordinates(const Vec3& vertex) const {
            if (!m_valid)
                validate();
            
            Texture::Ptr texture = m_face->texture();
            const size_t width = texture != NULL ? texture->width() : 1;
            const size_t height = texture != NULL ? texture->height() : 1;
            
            const float x = (vertex.dot(m_scaledTexAxisX) + m_face->xOffset()) / width;
            const float y = (vertex.dot(m_scaledTexAxisY) + m_face->yOffset()) / height;
            return Vec2f(x, y);
        }
        
        void TextureCoordinateSystem::invalidate() {
            m_valid = false;
        }
        
        void TextureCoordinateSystem::validate() const {
            const Vec3& normal = m_face->boundary().normal;
            
            axesAndIndices(normal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            rotateAxes(m_texAxisX, m_texAxisY, Math<FloatType>::radians(m_face->rotation()), m_texPlaneNormIndex);
            m_scaledTexAxisX = m_texAxisX / (m_face->xScale() == 0.0f ? 1.0f : m_face->xScale());
            m_scaledTexAxisY = m_texAxisY / (m_face->yScale() == 0.0f ? 1.0f : m_face->yScale());
            
            m_valid = true;
        }
        
        void TextureCoordinateSystem::axesAndIndices(const Vec3& normal, Vec3& xAxis, Vec3& yAxis, size_t& planeNormIndex, size_t& faceNormIndex) const {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; i++) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = BaseAxes[bestIndex * 3 + 1];
            yAxis = BaseAxes[bestIndex * 3 + 2];
            planeNormIndex = (bestIndex / 2) * 6;
            faceNormIndex = bestIndex * 3;
        }
        
        void TextureCoordinateSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex) const  {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(planeNormIndex == 12 ? -angle : angle, BaseAxes[planeNormIndex]);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }
        
        const String BrushFace::NoTextureName = "__TB_empty";
        const BrushFace::List BrushFace::EmptyList = BrushFace::List();
        
        BrushFace::BrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) :
        m_textureName(textureName),
        m_side(NULL),
        m_textureCoordinateSystem(this) {
            setPoints(point0, point1, point2);
        }
        
        BrushFace::Ptr BrushFace::newBrushFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) {
            return BrushFace::Ptr(new BrushFace(point0, point1, point2, textureName));
        }

        const BrushFace::Points& BrushFace::points() const {
            return m_points;
        }

        bool BrushFace::arePointsOnPlane(const Plane3& plane) const {
            for (size_t i = 0; i < 3; i++)
                if (plane.pointStatus(m_points[i]) != PointStatus::PSInside)
                    return false;
            return true;
        }

        const String& BrushFace::textureName() const {
            return m_textureName;
        }
        
        Texture::Ptr BrushFace::texture() const {
            return m_texture;
        }

        const Plane3& BrushFace::boundary() const {
            return m_boundary;
        }
        
        float BrushFace::xOffset() const {
            return m_xOffset;
        }
        
        float BrushFace::yOffset() const {
            return m_yOffset;
        }
        
        float BrushFace::rotation() const {
            return m_rotation;
        }
        
        float BrushFace::xScale() const {
            return m_xScale;
        }
        
        float BrushFace::yScale() const {
            return m_yScale;
        }
        
        void BrushFace::setTexture(Texture::Ptr texture) {
            if (m_texture != NULL)
                m_texture->decUsageCount();
            m_texture = texture;
            if (m_texture != NULL) {
                m_textureName = m_texture->name();
                m_texture->incUsageCount();
            }
            m_textureCoordinateSystem.invalidate();
        }

        void BrushFace::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setRotation(const float rotation) {
            m_rotation = rotation;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setXScale(const float xScale) {
            m_xScale = xScale;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setYScale(const float yScale) {
            m_yScale = yScale;
            m_textureCoordinateSystem.invalidate();
        }
        
        void BrushFace::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }
        
        void BrushFace::setSide(BrushFaceGeometry* side) {
            m_side = side;
        }
        
        void BrushFace::addToMesh(Mesh& mesh) {
            assert(m_side != NULL);
            
            mesh.beginTriangleSet(m_texture);
            
            const BrushVertex::List& vertices = m_side->vertices();
            for (size_t i = 1; i < vertices.size() - 1; i++) {
                const Vertex v1(vertices[0]->position(),
                                           m_boundary.normal,
                                           m_textureCoordinateSystem.textureCoordinates(vertices[0]->position()));
                const Vertex v2(vertices[i]->position(),
                                           m_boundary.normal,
                                           m_textureCoordinateSystem.textureCoordinates(vertices[i]->position()));
                const Vertex v3(vertices[i+1]->position(),
                                           m_boundary.normal,
                                           m_textureCoordinateSystem.textureCoordinates(vertices[i+1]->position()));
                mesh.addTriangleToSet(v1, v2, v3);
            }
            
            mesh.endTriangleSet();
        }

        void BrushFace::setPoints(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            m_points[0] = point0;
            m_points[1] = point1;
            m_points[2] = point2;
            
            if (!setPlanePoints(m_boundary, m_points)) {
                GeometryException e;
                e << "Colinear face points: (" <<
                m_points[0].asString() << ") (" <<
                m_points[1].asString() << ") (" <<
                m_points[2].asString() << ")";
                throw e;
            }
        }
    }
}

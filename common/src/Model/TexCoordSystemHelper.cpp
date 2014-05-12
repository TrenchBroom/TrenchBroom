/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TexCoordSystemHelper.h"

#include "CollectionUtils.h"
#include "Model/BrushFace.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        TexCoordSystemHelper::TexCoordSystemHelper(const BrushFace* face) :
        m_face(face),
        m_overrideOffset(false),
        m_overrideScale(false),
        m_offset(Vec2f::Null),
        m_scale(Vec2f::One),
        m_mode(Default) {
            assert(m_face != NULL);
        }
        
        TexCoordSystemHelper TexCoordSystemHelper::texCoordSystem(const BrushFace* face) {
            TexCoordSystemHelper helper(face);
            helper.setTranslate();
            helper.setScale();
            helper.setProject();
            return helper;
        }
        
        TexCoordSystemHelper TexCoordSystemHelper::faceCoordSystem(const BrushFace* face) {
            TexCoordSystemHelper helper(face);
            helper.setProject();
            return helper;
        }

        void TexCoordSystemHelper::setScale(const bool on) {
            toggleMode(Scale, on);
        }

        void TexCoordSystemHelper::setOverrideScale(const Vec2f& scale) {
            setScale(true);
            m_scale = scale;
            m_overrideScale = true;
        }
        
        void TexCoordSystemHelper::setTranslate(const bool on) {
            toggleMode(Translate, on);
        }

        void TexCoordSystemHelper::setOverrideTranslate(const Vec2f& offset) {
            setTranslate(true);
            m_offset = offset;
            m_overrideOffset = true;
        }

        void TexCoordSystemHelper::setProject(const bool project) {
            toggleMode(Project, project);
        }
        
        void TexCoordSystemHelper::toggleMode(const Mode mode, const bool on) {
            if (on)
                m_mode |= mode;
            else
                m_mode &= ~mode;
        }
        
        Vec2f TexCoordSystemHelper::worldToTex(const Vec2f& v) const {
            return Vec2f(worldToTex(Vec3(v)));
        }
        
        Vec2f::List TexCoordSystemHelper::worldToTex(const Vec2f::List& vs) const {
            return VectorUtils::cast<Vec2f>(worldToTex(VectorUtils::cast<Vec3>(vs)));
        }
        
        Vec2f TexCoordSystemHelper::texToWorld(const Vec2f& v) const {
            return Vec2f(texToWorld(Vec3(v)));
        }
        
        Vec2f::List TexCoordSystemHelper::texToWorld(const Vec2f::List& vs) const {
            return VectorUtils::cast<Vec2f>(texToWorld(VectorUtils::cast<Vec3>(vs)));
        }
        
        Vec2f TexCoordSystemHelper::texToTex(const Vec2f& v, const TexCoordSystemHelper& other) const {
            return Vec2f(texToTex(Vec3(v), other));
        }
        
        Vec2f::List TexCoordSystemHelper::texToTex(const Vec2f::List& vs, const TexCoordSystemHelper& other) const {
            return VectorUtils::cast<Vec2f>(texToTex(VectorUtils::cast<Vec3>(vs), other));
        }

        Vec3 TexCoordSystemHelper::worldToTex(const Vec3& v) const {
            return toTexMatrix(project()) * v;
        }
        
        Vec3::List TexCoordSystemHelper::worldToTex(const Vec3::List& vs) const {
            return toTexMatrix(project()) * vs;
        }
        
        Vec3 TexCoordSystemHelper::texToWorld(const Vec3& v) const {
            return toWorldMatrix(project()) * v;
        }
        
        Vec3::List TexCoordSystemHelper::texToWorld(const Vec3::List& vs) const {
            return toWorldMatrix(project()) * vs;
        }
        
        Vec3 TexCoordSystemHelper::texToTex(const Vec3& v, const TexCoordSystemHelper& other) const {
            return other.worldToTex(toWorldMatrix(false) * v);
        }
        
        Vec3::List TexCoordSystemHelper::texToTex(const Vec3::List& vs, const TexCoordSystemHelper& other) const {
            return other.worldToTex(toWorldMatrix(false) * vs);
        }
        
        Mat4x4 TexCoordSystemHelper::toTexMatrix() const {
            return toTexMatrix(project());
        }
        
        Mat4x4 TexCoordSystemHelper::toWorldMatrix() const {
            return toWorldMatrix(project());
        }

        Mat4x4 TexCoordSystemHelper::toTexMatrix(const bool project) const {
            if (!project)
                return m_face->toTexCoordSystemMatrix(offset(), scale());
            return Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(offset(), scale());
        }
        
        Mat4x4 TexCoordSystemHelper::toWorldMatrix(const bool project) const {
            if (!project)
                return m_face->fromTexCoordSystemMatrix(offset(), scale());;
            return m_face->projectToBoundaryMatrix() * m_face->fromTexCoordSystemMatrix(offset(), scale());
        }

        const Vec2f& TexCoordSystemHelper::offset() const {
            if ((m_mode & Translate) != 0) {
                if (m_overrideOffset)
                    return m_offset;
                return m_face->offset();
            } else {
                return Vec2f::Null;
            }
        }
        
        const Vec2f& TexCoordSystemHelper::scale() const {
            if ((m_mode & Scale) != 0) {
                if (m_overrideScale)
                    return m_scale;
                return m_face->scale();
            } else {
                return Vec2f::One;
            }
        }
        
        bool TexCoordSystemHelper::project() const {
            return (m_mode & Project) != 0;
        }
    }
}

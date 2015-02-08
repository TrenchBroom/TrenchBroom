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

#include "BrushBuilder.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ModelFactory.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        BrushBuilder::BrushBuilder(ModelFactory* factory, const BBox3& worldBounds) :
        m_factory(factory),
        m_worldBounds(worldBounds) {
            assert(m_factory != NULL);
        }
        
        Brush* BrushBuilder::createCube(const FloatType size, const String& textureName) const {
            return createCuboid(BBox3(size / 2.0), textureName);
        }
        
        Brush* BrushBuilder::createCuboid(const Vec3& size, const String& textureName) const {
            return createCuboid(BBox3(-size / 2.0, size / 2.0), textureName);
        }
        
        Brush* BrushBuilder::createCuboid(const BBox3& bounds, const String& textureName) const {
            BrushFaceList faces(6);
            // left face
            faces[0] = m_factory->createFace(bounds.min + Vec3::Null,
                                             bounds.min + Vec3::PosY,
                                             bounds.min + Vec3::PosZ,
                                             textureName);
            // right face
            faces[1] = m_factory->createFace(bounds.max + Vec3::Null,
                                             bounds.max + Vec3::PosZ,
                                             bounds.max + Vec3::PosY,
                                             textureName);
            // front face
            faces[2] = m_factory->createFace(bounds.min + Vec3::Null,
                                             bounds.min + Vec3::PosZ,
                                             bounds.min + Vec3::PosX,
                                             textureName);
            // back face
            faces[3] = m_factory->createFace(bounds.max + Vec3::Null,
                                             bounds.max + Vec3::PosX,
                                             bounds.max + Vec3::PosZ,
                                             textureName);
            // top face
            faces[4] = m_factory->createFace(bounds.max + Vec3::Null,
                                             bounds.max + Vec3::PosY,
                                             bounds.max + Vec3::PosX,
                                             textureName);
            // bottom face
            faces[5] = m_factory->createFace(bounds.min + Vec3::Null,
                                             bounds.min + Vec3::PosX,
                                             bounds.min + Vec3::PosY,
                                             textureName);
            
            return m_factory->createBrush(m_worldBounds, faces);
        }
    }
}

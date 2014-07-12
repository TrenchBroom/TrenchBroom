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

#include "ModelFactory.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/QuakeEntityRotationPolicy.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        ModelFactory::ModelFactory() :
        m_format(MapFormat::Unknown) {}
        
        ModelFactory::ModelFactory(const MapFormat::Type format) :
        m_format(format) {
            assert(m_format != MapFormat::Unknown);
        }

        Map* ModelFactory::createMap() const {
            assert(m_format != MapFormat::Unknown);
            return new Map(m_format);
        }

        Entity* ModelFactory::createEntity() const {
            assert(m_format != MapFormat::Unknown);
            return new ConfigurableEntity<QuakeEntityRotationPolicy>();
        }
        
        Brush* ModelFactory::createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            assert(m_format != MapFormat::Unknown);
            return new Brush(worldBounds, faces);
        }

        BrushFace* ModelFactory::createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const {
            assert(m_format != MapFormat::Unknown);
            switch (m_format) {
                case MapFormat::Valve:
                    return new BrushFace(point1, point2, point3, textureName, new ParallelTexCoordSystem(point1, point2, point3));
                default:
                    return new BrushFace(point1, point2, point3, textureName, new ParaxialTexCoordSystem(point1, point2, point3));
            }
        }

        BrushFace* ModelFactory::createFaceWithAxes(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const {
            assert(m_format != MapFormat::Unknown);
            switch (m_format) {
                case MapFormat::Valve:
                    return new BrushFace(point1, point2, point3, textureName, new ParallelTexCoordSystem(texAxisX, texAxisY));
                default:
                    return new BrushFace(point1, point2, point3, textureName, new ParaxialTexCoordSystem(point1, point2, point3));
            }
        }
    }
}

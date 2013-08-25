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

#include "ModelFactory.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/QuakeEntityRotator.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        ModelFactory::ModelFactory(const MapFormat format) :
        m_format(format) {
            assert(m_format != MFUnknown);
        }

        Entity* ModelFactory::createEntity() const {
            return new ConfigurableEntity<QuakeEntityRotationPolicy>();
        }
        
        Brush* ModelFactory::createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            return new Brush(worldBounds, faces);
        }
        
        BrushFace* ModelFactory::createFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) const {
            switch (m_format) {
                case MFValve:
                    return createValveFace(point0, point1, point2, textureName);
                default:
                    return createDefaultFace(point0, point1, point2, textureName);
            }
        }

        BrushFace* ModelFactory::createValveFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) const {
            return new ConfigurableBrushFace<ParallelTexCoordSystem>(point0, point1, point2, textureName);
        }
        
        BrushFace* ModelFactory::createDefaultFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) const {
            return new ConfigurableBrushFace<ParaxialTexCoordSystem>(point0, point1, point2, textureName);
        }
    }
}

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

#ifndef __TrenchBroom__ModelFactory__
#define __TrenchBroom__ModelFactory__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Model/BrushContentTypeBuilder.h"
#include "Model/BrushContentType.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class ModelFactory {
        private:
            MapFormat::Type m_format;
            BrushContentTypeBuilder::Ptr m_brushContentTypeBuilder;
        public:
            ModelFactory();
            ModelFactory(MapFormat::Type format, BrushContentTypeBuilder::Ptr brushContentTypeBuilder);

            MapFormat::Type format() const;
            
            Map* createMap() const;
            Entity* createEntity() const;
            Brush* createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
            BrushFace* createFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const;
            BrushFace* createFaceWithAxes(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const;
        };
    }
}

#endif /* defined(__TrenchBroom__ModelFactory__) */

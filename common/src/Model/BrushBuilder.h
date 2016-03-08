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

#ifndef TrenchBroom_BrushBuilder
#define TrenchBroom_BrushBuilder

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class ModelFactory;
        
        class BrushBuilder {
        private:
            ModelFactory* m_factory;
            const BBox3 m_worldBounds;
        public:
            BrushBuilder(ModelFactory* factory, const BBox3& worldBounds);
            
            Brush* createCube(FloatType size, const String& textureName) const;
            Brush* createCuboid(const Vec3& size, const String& textureName) const;
            Brush* createCuboid(const BBox3& bounds, const String& textureName) const;
            Brush* createBrush(const Vec3::List& points, const String& textureName) const;
            Brush* createBrush(const Polyhedron3& polyhedron, const String& textureName) const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushBuilder) */

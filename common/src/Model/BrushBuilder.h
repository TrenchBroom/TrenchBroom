/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include "StringUtils.h"

#include <vecmath/bbox.h>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class ModelFactory;

        class BrushBuilder {
        private:
            ModelFactory* m_factory;
            const vm::bbox3 m_worldBounds;
        public:
            BrushBuilder(ModelFactory* factory, const vm::bbox3& worldBounds);

            Brush* createCube(FloatType size, const String& textureName) const;
            Brush* createCube(FloatType size, const String& leftTexture, const String& rightTexture, const String& frontTexture, const String& backTexture, const String& topTexture, const String& bottomTexture) const;

            Brush* createCuboid(const vm::vec3& size, const String& textureName) const;
            Brush* createCuboid(const vm::vec3& size, const String& leftTexture, const String& rightTexture, const String& frontTexture, const String& backTexture, const String& topTexture, const String& bottomTexture) const;

            Brush* createCuboid(const vm::bbox3& bounds, const String& textureName) const;
            Brush* createCuboid(const vm::bbox3& bounds, const String& leftTexture, const String& rightTexture, const String& frontTexture, const String& backTexture, const String& topTexture, const String& bottomTexture) const;

            Brush* createBrush(const std::vector<vm::vec3>& points, const String& textureName) const;
            Brush* createBrush(const Polyhedron3& polyhedron, const String& textureName) const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushBuilder) */

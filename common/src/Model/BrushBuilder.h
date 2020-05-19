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

#include "FloatType.h"
#include "Model/Polyhedron3.h"
#include "BrushFaceAttributes.h"

#include <vecmath/bbox.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class ModelFactory;

        class BrushBuilder {
        private:
            ModelFactory* m_factory;
            const vm::bbox3 m_worldBounds;
            const BrushFaceAttributes m_defaultAttribs;
        public:
            BrushBuilder(ModelFactory* factory, const vm::bbox3& worldBounds);
            BrushBuilder(ModelFactory* factory, const vm::bbox3& worldBounds, const BrushFaceAttributes& defaultAttribs);

            BrushNode* createCube(FloatType size, const std::string& textureName) const;
            BrushNode* createCube(FloatType size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const;

            BrushNode* createCuboid(const vm::vec3& size, const std::string& textureName) const;
            BrushNode* createCuboid(const vm::vec3& size, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const;

            BrushNode* createCuboid(const vm::bbox3& bounds, const std::string& textureName) const;
            BrushNode* createCuboid(const vm::bbox3& bounds, const std::string& leftTexture, const std::string& rightTexture, const std::string& frontTexture, const std::string& backTexture, const std::string& topTexture, const std::string& bottomTexture) const;

            BrushNode* createBrush(const std::vector<vm::vec3>& points, const std::string& textureName) const;
            BrushNode* createBrush(const Polyhedron3& polyhedron, const std::string& textureName) const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushBuilder) */

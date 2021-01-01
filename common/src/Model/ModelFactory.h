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

#pragma once

#include "FloatType.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        enum class BrushError;
        class BrushFace;
        class BrushNode;
        class BrushFaceAttributes;
        class Entity;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        enum class MapFormat;
        class WorldNode;

        class ModelFactory {
        public:
            virtual ~ModelFactory();

            MapFormat format() const;
        private:
            virtual MapFormat doGetFormat() const = 0;
        };
    }
}


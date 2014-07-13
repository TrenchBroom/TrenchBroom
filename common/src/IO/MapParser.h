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

#ifndef TrenchBroom_MapParser_h
#define TrenchBroom_MapParser_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace IO {
        class MapParser {
        public:
            virtual ~MapParser();
            Model::Map* parseMap(const BBox3& worldBounds);
            Model::EntityList parseEntities(const BBox3& worldBounds, Model::MapFormat::Type format);
            Model::BrushList parseBrushes(const BBox3& worldBounds, Model::MapFormat::Type format);
            Model::BrushFaceList parseFaces(const BBox3& worldBounds, Model::MapFormat::Type format);
        private:
            virtual Model::Map* doParseMap(const BBox3& worldBounds) = 0;
            virtual Model::EntityList doParseEntities(const BBox3& worldBounds, Model::MapFormat::Type format) = 0;
            virtual Model::BrushList doParseBrushes(const BBox3& worldBounds, Model::MapFormat::Type format) = 0;
            virtual Model::BrushFaceList doParseFaces(const BBox3& worldBounds, Model::MapFormat::Type format) = 0;
        };
    }
}

#endif

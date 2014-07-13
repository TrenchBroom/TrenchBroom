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

#include "MapParser.h"

namespace TrenchBroom {
    namespace IO {
        MapParser::~MapParser() {}

        Model::Map* MapParser::parseMap(const BBox3& worldBounds) {
            return doParseMap(worldBounds);
        }

        Model::EntityList MapParser::parseEntities(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            return doParseEntities(worldBounds, format);
        }
        
        Model::BrushList MapParser::parseBrushes(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            return doParseBrushes(worldBounds, format);
        }
        
        Model::BrushFaceList MapParser::parseFaces(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            return doParseFaces(worldBounds, format);
        }
    }
}

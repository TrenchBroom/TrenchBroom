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

#ifndef TrenchBroom_AssetTypes_h
#define TrenchBroom_AssetTypes_h

#include "StringType.h"

#include <memory>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Palette;
        class PaletteLoader;

        class Texture;
        using TextureList = std::vector<Texture*>;

        class TextureCollection;
        using TextureCollectionList = std::vector<TextureCollection*>;

        class EntityDefinition;
        class PointEntityDefinition;
        class BrushEntityDefinition;
        using EntityDefinitionList = std::vector<EntityDefinition*>;

        class EntityDefinitionFileSpec;

        class AttributeDefinition;
        using AttributeDefinitionPtr = std::shared_ptr<AttributeDefinition>;
        using AttributeDefinitionList = std::vector<AttributeDefinitionPtr>;
        using AttributeDefinitionMap = std::map<String, AttributeDefinitionPtr>;

        class ModelDefinition;

        class EntityModelManager;
        class EntityModel;
        using EntityModelList = std::vector<EntityModel*>;
    }
}

#endif

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

#ifndef TrenchBroom_Asset_Forward_h
#define TrenchBroom_Asset_Forward_h

namespace TrenchBroom {
    namespace Assets {
        class Palette;
        class PaletteLoader;

        class Texture;
        class TextureCollection;
        class TextureManager;
        class Quake3Shader;
        class Quake3ShaderStage;

        class EntityDefinitionFileSpec;

        class EntityDefinition;
        enum class EntityDefinitionType;
        enum class EntityDefinitionSortOrder;

        class PointEntityDefinition;
        class BrushEntityDefinition;

        class EntityDefinitionGroup;

        class AttributeDefinition;
        class FlagsAttributeDefinition;
        class FlagsAttributeOption;

        class ModelDefinition;
        struct ModelSpecification;

        class EntityModel;
        class EntityModelFrame;

        class EntityDefinitionManager;
        class EntityModelManager;
    }
}

#endif

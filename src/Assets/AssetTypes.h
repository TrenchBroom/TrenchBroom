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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_AssetTypes_h
#define TrenchBroom_AssetTypes_h

#include "StringUtils.h"
#include "SharedPointer.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Palette;
        
        class Texture;
        typedef std::vector<Texture*> TextureList;
        
        class FaceTexture;
        typedef std::vector<FaceTexture*> FaceTextureList;
        
        class FaceTextureCollection;
        typedef std::vector<FaceTextureCollection*> TextureCollectionList;
        
        class EntityDefinition;
        typedef std::vector<EntityDefinition*> EntityDefinitionList;
        typedef std::map<String, EntityDefinitionList> EntityDefinitionGroups;
        
        class PropertyDefinition;
        typedef std::tr1::shared_ptr<PropertyDefinition> PropertyDefinitionPtr;
        typedef std::vector<PropertyDefinitionPtr> PropertyDefinitionList;
        typedef std::map<String, PropertyDefinitionPtr> PropertyDefinitionMap;
        
        class ModelDefinition;
        typedef std::tr1::shared_ptr<ModelDefinition> ModelDefinitionPtr;
        typedef std::vector<ModelDefinitionPtr> ModelDefinitionList;
        static const ModelDefinitionList EmptyModelDefinitionList;
        
        class EntityModel;
        typedef std::vector<EntityModel*> EntityModelList;
        
        class AutoTexture;
        typedef std::tr1::shared_ptr<AutoTexture> AutoTexturePtr;
        typedef std::vector<AutoTexture*> AutoTextureList;
    }
}

#endif

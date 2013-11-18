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

#ifndef __TrenchBroom__MockGame__
#define __TrenchBroom__MockGame__

#include <gmock/gmock.h>

#include "StringUtils.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "Model/ModelTypes.h"

#include <iostream>

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class MockGame;
        typedef std::tr1::shared_ptr<MockGame> MockGamePtr;
        
        class MockGame : public Game {
        public:
            static MockGamePtr newGame();

            MOCK_CONST_METHOD0(doGameName, const String&());
            MOCK_METHOD1(doSetGamePath, void(const IO::Path&));
            MOCK_METHOD1(doSetAdditionalSearchPaths, void(const IO::Path::List& searchPaths));
            
            MOCK_CONST_METHOD1(doNewMap, Map*(Model::MapFormat::Type));
            MOCK_CONST_METHOD2(doLoadMap, Map*(const BBox3&, const IO::Path&));
            MOCK_CONST_METHOD2(doParseEntities, Model::EntityList(const BBox3&, const String&));
            MOCK_CONST_METHOD2(doParseBrushes, Model::BrushList(const BBox3&, const String&));
            MOCK_CONST_METHOD2(doParseFaces, Model::BrushFaceList(const BBox3&, const String&));

            MOCK_CONST_METHOD2(doWriteMap, void(Map&, const IO::Path&));
            MOCK_CONST_METHOD3(doWriteObjectsToStream, void(Model::MapFormat::Type, const Model::ObjectList&, std::ostream&));
            MOCK_CONST_METHOD3(doWriteFacesToStream, void(Model::MapFormat::Type, const Model::BrushFaceList&, std::ostream&));
            
            MOCK_CONST_METHOD0(doFindBuiltinTextureCollections, IO::Path::List());
            MOCK_CONST_METHOD1(doExtractTexturePaths, IO::Path::List(const Map*));
            MOCK_CONST_METHOD1(doLoadTextureCollection, Assets::TextureCollection*(const IO::Path&));
            
            MOCK_CONST_METHOD1(doLoadEntityDefinitions, Assets::EntityDefinitionList(const IO::Path&));
            MOCK_CONST_METHOD0(doAllEntityDefinitionFiles, IO::Path::List());
            MOCK_CONST_METHOD1(doExtractEntityDefinitionFile, IO::Path(const Map* map));
            MOCK_CONST_METHOD1(doLoadModel, Assets::EntityModel*(const IO::Path&));

            MOCK_CONST_METHOD0(doAvailableMods, StringList());
            MOCK_CONST_METHOD1(doExtractEnabledMods, StringList(const Map*));
        };
        
    }
}

#endif /* defined(__TrenchBroom__MockGame__) */

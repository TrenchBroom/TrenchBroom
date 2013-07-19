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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MockGame__
#define __TrenchBroom__MockGame__

#include <gmock/gmock.h>

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class MockGame;
        typedef std::tr1::shared_ptr<MockGame> MockGamePtr;
        
        class MockGame : public Game {
        public:
            static MockGamePtr newGame();

            MOCK_CONST_METHOD2(doLoadMap, Map*(const BBox3&, const IO::Path&));
            MOCK_CONST_METHOD1(doExtractTexturePaths, IO::Path::List(Map*));
            MOCK_CONST_METHOD1(doLoadTextureCollection, TextureCollection*(const IO::Path&));
            MOCK_CONST_METHOD1(doUploadTextureCollection, void(TextureCollection*));
        };
        
    }
}

#endif /* defined(__TrenchBroom__MockGame__) */

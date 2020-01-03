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

#ifndef GameEngineConfigParser_h
#define GameEngineConfigParser_h

#include "Macros.h"
#include "IO/ConfigParserBase.h"
#include "IO/IO_Forward.h"
#include "Model/Model_Forward.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class GameEngineConfigParser : public ConfigParserBase {
        public:
            GameEngineConfigParser(const char* begin, const char* end, const Path& path);
            GameEngineConfigParser(const std::string& str, const Path& path);

            Model::GameEngineConfig parse();
        private:
            std::vector<Model::GameEngineProfile*> parseProfiles(const EL::Value& value) const;
            Model::GameEngineProfile* parseProfile(const EL::Value& value) const;

            deleteCopyAndMove(GameEngineConfigParser)
        };
    }
}

#endif /* GameEngineConfigParser_h */

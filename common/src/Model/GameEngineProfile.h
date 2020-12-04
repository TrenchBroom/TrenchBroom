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

#include "Macros.h"
#include "IO/Path.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile {
        private:
            std::string m_name;
            IO::Path m_path;
            std::string m_parameterSpec;
        public:
            GameEngineProfile(const std::string& name, const IO::Path& path, const std::string& parameterSpec);

            std::unique_ptr<GameEngineProfile> clone() const;
            bool operator==(const GameEngineProfile& other) const;
            bool operator!=(const GameEngineProfile& other) const;

            const std::string& name() const;
            const IO::Path& path() const;
            const std::string& parameterSpec() const;

            void setName(const std::string& name);
            void setPath(const IO::Path& path);
            void setParameterSpec(const std::string& parameterSpec);

            deleteCopyAndMove(GameEngineProfile)
        };
    }
}



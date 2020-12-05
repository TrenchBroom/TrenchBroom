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

#include "EL/VariableStore.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        namespace CompilationVariableNames {
            extern const std::string WORK_DIR_PATH;
            extern const std::string MAP_DIR_PATH;
            extern const std::string MAP_BASE_NAME;
            extern const std::string MAP_FULL_NAME;
            extern const std::string CPU_COUNT;
            extern const std::string GAME_DIR_PATH;
            extern const std::string MODS;
            extern const std::string APP_DIR_PATH;
        }

        class CommonVariables : public EL::VariableTable {
        protected:
            explicit CommonVariables(std::shared_ptr<MapDocument> document);
        };

        class CommonCompilationVariables : public CommonVariables {
        protected:
            explicit CommonCompilationVariables(std::shared_ptr<MapDocument> document);
        };

        class CompilationWorkDirVariables : public CommonCompilationVariables {
        public:
            explicit CompilationWorkDirVariables(std::shared_ptr<MapDocument> document);
        };

        class CompilationVariables : public CommonCompilationVariables {
        public:
            CompilationVariables(std::shared_ptr<MapDocument> document, const std::string& workDir);
        };

        class LaunchGameEngineVariables : public CommonVariables {
        public:
            explicit LaunchGameEngineVariables(std::shared_ptr<MapDocument> document);
        };
    }
}


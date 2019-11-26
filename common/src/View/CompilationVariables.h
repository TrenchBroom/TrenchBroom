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

#ifndef CompilationVariables_h
#define CompilationVariables_h

#include "StringType.h"
#include "EL/VariableStore.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        namespace CompilationVariableNames {
            extern const String WORK_DIR_PATH;
            extern const String MAP_DIR_PATH;
            extern const String MAP_BASE_NAME;
            extern const String MAP_FULL_NAME;
            extern const String CPU_COUNT;
            extern const String GAME_DIR_PATH;
            extern const String MODS;
            extern const String APP_DIR_PATH;
        }

        class CommonVariables : public EL::VariableTable {
        protected:
            explicit CommonVariables(MapDocumentSPtr document);
        };

        class CommonCompilationVariables : public CommonVariables {
        protected:
            explicit CommonCompilationVariables(MapDocumentSPtr document);
        };

        class CompilationWorkDirVariables : public CommonCompilationVariables {
        public:
            explicit CompilationWorkDirVariables(MapDocumentSPtr document);
        };

        class CompilationVariables : public CommonCompilationVariables {
        public:
            CompilationVariables(MapDocumentSPtr document, const String& workDir);
        };

        class LaunchGameEngineVariables : public CommonVariables {
        public:
            explicit LaunchGameEngineVariables(MapDocumentSPtr document);
        };
    }
}

#endif /* CompilationVariables_h */

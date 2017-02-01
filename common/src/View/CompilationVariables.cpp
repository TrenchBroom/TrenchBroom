/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CompilationVariables.h"

#include "IO/SystemPaths.h"
#include "Model/Game.h"
#include "View/MapDocument.h"

#include <wx/thread.h>

namespace TrenchBroom {
    namespace View {
        namespace CompilationVariableNames {
            const String WORK_DIR_PATH  = "WORK_DIR_PATH";
            const String MAP_DIR_PATH   = "MAP_DIR_PATH";
            const String MAP_BASE_NAME  = "MAP_BASE_NAME";
            const String MAP_FULL_NAME  = "MAP_FULL_NAME";
            const String CPU_COUNT      = "CPU_COUNT";
            const String GAME_DIR_PATH  = "GAME_DIR_PATH";
            const String MODS           = "MODS";
            const String APP_DIR_PATH   = "APP_DIR_PATH";
        }

        CommonVariables::CommonVariables(MapDocumentSPtr document) {
            const IO::Path filename = document->path().lastComponent();
            const IO::Path gamePath = document->game()->gamePath();
            
            StringArray mods;
            mods.push_back(document->defaultMod());
            VectorUtils::append(mods, document->mods());
 
            using namespace CompilationVariableNames;
            declare(MAP_BASE_NAME, EL::Value(filename.deleteExtension().asString()));
            declare(GAME_DIR_PATH, EL::Value(gamePath.asString()));
            declare(MODS, EL::Value(mods));
        }

        CommonCompilationVariables::CommonCompilationVariables(MapDocumentSPtr document) :
        CommonVariables(document) {
            const IO::Path filename = document->path().lastComponent();
            const IO::Path appPath = IO::SystemPaths::appDirectory();

            using namespace CompilationVariableNames;
            declare(MAP_FULL_NAME, EL::Value(filename.asString()));
            declare(APP_DIR_PATH, EL::Value(appPath.asString()));
        }
        
        CompilationWorkDirVariables::CompilationWorkDirVariables(MapDocumentSPtr document) :
        CommonCompilationVariables(document) {
            const IO::Path filePath = document->path().deleteLastComponent();
            
            using namespace CompilationVariableNames;
            declare(MAP_DIR_PATH, EL::Value(filePath.asString()));
        }

        CompilationVariables::CompilationVariables(MapDocumentSPtr document, const String& workDir) :
        CommonCompilationVariables(document) {
            using namespace CompilationVariableNames;
            declare(CPU_COUNT, EL::Value(wxThread::GetCPUCount()));
            declare(WORK_DIR_PATH, EL::Value(workDir));
        }

        LaunchGameEngineVariables::LaunchGameEngineVariables(MapDocumentSPtr document) :
        CommonVariables(document) {}
    }
}

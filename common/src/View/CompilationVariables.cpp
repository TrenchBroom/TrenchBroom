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

namespace TrenchBroom {
    namespace View {
        namespace CompilationVariableNames {
            const String WORK_DIR_PATH  = "WORK_DIR_PATH";
            const String MAP_DIR_PATH   = "MAP_DIR_PATH";
            const String MAP_BASE_NAME  = "MAP_BASE_NAME";
            const String MAP_FULL_NAME  = "MAP_FULL_NAME";
            const String CPU_COUNT      = "CPU_COUNT";
            const String GAME_DIR_PATH  = "GAME_DIR_PATH";
            const String MOD_DIR_PATH   = "MOD_DIR_PATH";
            const String MOD_NAME       = "MOD_NAME";
            const String APP_DIR_PATH   = "APP_DIR_PATH";
        }

        VariableTable createCompilationWorkDirVariableTable();
        
        const VariableTable& compilationWorkDirVariables() {
            static const VariableTable variables = createCompilationWorkDirVariableTable();
            return variables;
        }
        
        VariableTable createCompilationWorkDirVariableTable() {
            using namespace CompilationVariableNames;
            
            VariableTable result;
            result.declare(MAP_BASE_NAME);
            result.declare(MAP_FULL_NAME);
            result.declare(GAME_DIR_PATH);
            result.declare(MOD_DIR_PATH);
            result.declare(MOD_NAME);
            result.declare(APP_DIR_PATH);

            result.declare(MAP_DIR_PATH);
            return result;
        }

        VariableTable createCompilationVariableTable();
        const VariableTable& compilationVariables() {
            static const VariableTable variables = createCompilationVariableTable();
            return variables;
        }

        VariableTable createCompilationVariableTable() {
            using namespace CompilationVariableNames;
            
            VariableTable result;
            result.declare(MAP_BASE_NAME);
            result.declare(MAP_FULL_NAME);
            result.declare(GAME_DIR_PATH);
            result.declare(MOD_DIR_PATH);
            result.declare(MOD_NAME);
            result.declare(APP_DIR_PATH);
            
            result.declare(WORK_DIR_PATH);
            result.declare(CPU_COUNT);
            return result;
        }
    }
}

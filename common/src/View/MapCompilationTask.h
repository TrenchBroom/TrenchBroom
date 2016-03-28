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

#ifndef MapCompilationTask_h
#define MapCompilationTask_h

#include "StringUtils.h"
#include "IO/Path.h"

#include <list>

namespace TrenchBroom {
    namespace View {
        class MapCompilationContext;
        
        class MapCompilationTask {
        public:
            typedef std::list<MapCompilationTask*> List;
        protected:
            MapCompilationTask();
        public:
            virtual ~MapCompilationTask();
            
            bool execute(MapCompilationContext& context) const;
        private:
            virtual bool doExecute(MapCompilationContext& context) const = 0;
        private:
            MapCompilationTask(const MapCompilationTask& other);
            MapCompilationTask& operator=(const MapCompilationTask& other);
        };
        
        class MapCompilationCopyFiles : public MapCompilationTask {
        private:
            String m_sourceFileSpec;
            String m_targetFileSpec;
        public:
            MapCompilationCopyFiles(const String& sourceFileSpec, const String& targetFileSpec);
        private:
            bool doExecute(MapCompilationContext& context) const;
        
            IO::Path getSourceFilePath(const MapCompilationContext& context) const;
            IO::Path getTargetFilePath(const MapCompilationContext& context) const;
        private:
            MapCompilationCopyFiles(const MapCompilationCopyFiles& other);
            MapCompilationCopyFiles& operator=(const MapCompilationCopyFiles& other);
        };

        class MapCompilationRunTool : public MapCompilationTask {
        private:
            String m_toolPathSpec;
            String m_toolParameterSpec;
        private:
            MapCompilationRunTool(const MapCompilationRunTool& other);
            MapCompilationRunTool& operator=(const MapCompilationRunTool& other);
        };
        
        class MapCompilationRunGame : public MapCompilationTask {
        private:
            MapCompilationRunGame(const MapCompilationRunGame& other);
            MapCompilationRunGame& operator=(const MapCompilationRunGame& other);
        };
    }
}

#endif /* MapCompilationTask_h */

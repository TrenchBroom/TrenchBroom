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

#include <list>

namespace TrenchBroom {
    namespace View {
        class MapCompilationContext;
        
        class MapCompilationTask {
        public:
            typedef std::list<MapCompilationTask*> List;
        public:
            MapCompilationTask();
            MapCompilationTask(const MapCompilationTask& other);
            
            bool execute(MapCompilationContext& context) const;
        private:
            bool doExecute(MapCompilationContext& context) const;
        };
        
        class MapCompilationCopyFiles : public MapCompilationTask {
        };

        class MapCompilationMoveFiles : public MapCompilationTask {
        };
        
        class MapCompilationRunTool : public MapCompilationTask {
        };
        
        class MapCompilationRunGame : public MapCompilationTask {
        };
    }
}

#endif /* MapCompilationTask_h */

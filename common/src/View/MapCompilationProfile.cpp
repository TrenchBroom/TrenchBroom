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

#include "MapCompilationProfile.h"

#include "CollectionUtils.h"
#include "View/MapCompilationTask.h"

namespace TrenchBroom {
    namespace View {
        MapCompilationProfileRunner::MapCompilationProfileRunner(MapCompilationContext& context, const MapCompilationTask::List& tasks) :
        m_tasks(NULL) {
            if (!tasks.empty()) {
                MapCompilationTask::List::const_reverse_iterator it = tasks.rbegin();
                MapCompilationTask::TaskRunner* runner = (*it)->createTaskRunner(context);
                ++it;
                
                while (it != tasks.rend()) {
                    runner = (*it)->createTaskRunner(context, runner);
                    ++it;
                }
                
                m_tasks = runner;
            }
        }
        
        MapCompilationProfileRunner::~MapCompilationProfileRunner() {
            if (m_tasks != NULL) {
                m_tasks->terminate();
                delete m_tasks;
            }
        }

        void MapCompilationProfileRunner::execute() {
            if (m_tasks != NULL)
                m_tasks->execute();
        }
        
        void MapCompilationProfileRunner::terminate() {
            if (m_tasks != NULL)
                m_tasks->terminate();
        }

        MapCompilationProfile::MapCompilationProfile() {}
        
        MapCompilationProfile::~MapCompilationProfile() {
            ListUtils::clearAndDelete(m_tasks);
        }

        MapCompilationProfileRunner* MapCompilationProfile::createRunner(MapCompilationContext& context) const {
            return new MapCompilationProfileRunner(context, m_tasks);
        }
    }
}

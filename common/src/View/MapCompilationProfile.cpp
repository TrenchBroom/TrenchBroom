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
        MapCompilationProfile::MapCompilationProfile() {}
        
        MapCompilationProfile::~MapCompilationProfile() {
            ListUtils::clearAndDelete(m_tasks);
        }

        bool MapCompilationProfile::execute(MapCompilationContext& context) const {
            MapCompilationTask::List::const_iterator it, end;
            for (it = m_tasks.begin(), end = m_tasks.end(); it != end; ++it) {
                const MapCompilationTask* task = *it;
                if (!task->execute(context))
                    return false;
            }
            return true;
        }
    }
}

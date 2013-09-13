/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_AutoSaver_h
#define TrenchBroom_AutoSaver_h

#include "Utility/String.h"

#include <ctime>

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Controller {
        unsigned int backupNoOfFile(const String& path);
        bool compareByBackupNo(const String& file1, const String& file2);

        class Autosaver {
        protected:
            Model::MapDocument& m_document;
            
            time_t m_saveInterval;
            time_t m_idleInterval;
            unsigned int m_maxBackups;
            time_t m_lastSaveTime;
            time_t m_lastModificationTime;
            bool m_dirty;
            
            String backupName(const String& mapBasename, unsigned int backupNo);
            bool isBackupName(const String& basename, const String& mapBasename, unsigned int& backupNo);
            void autosave();
        public:
            Autosaver(Model::MapDocument& document, time_t saveInterval = 10 * 60, time_t idleInterval = 3, unsigned int maxBackups = 30);
            ~Autosaver();
            
            void triggerAutosave();
            void updateLastModificationTime();
            void clearDirtyFlag();
        };
    }
}

#endif

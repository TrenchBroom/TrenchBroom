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

#include <ctime>
#include <string>

namespace TrenchBroom {
    namespace Controller {
        class Editor;

        unsigned int backupNoOfFile(const std::string& path);
        bool compareByBackupNo(const std::string& file1, const std::string& file2);

        class Autosaver {
        protected:
            Editor& m_editor;
            time_t m_saveInterval;
            time_t m_idleInterval;
            unsigned int m_maxBackups;
            time_t m_lastSaveTime;
            time_t m_lastModificationTime;
            bool m_dirty;
            
            std::string backupName(const std::string& mapBasename, unsigned int backupNo);
            void autosave();
        public:
            Autosaver(Editor& editor, time_t saveInterval = 5 * 60, time_t idleInterval = 3, unsigned int maxBackups = 50) : m_editor(editor), m_saveInterval(saveInterval), m_idleInterval(idleInterval), m_maxBackups(maxBackups), m_lastSaveTime(time(NULL)), m_lastModificationTime(0), m_dirty(false) {}
            ~Autosaver() {
                autosave();
            }
            
            void triggerAutosave();
            void updateLastModificationTime();
            void clearDirtyFlag();
        };
    }
}

#endif

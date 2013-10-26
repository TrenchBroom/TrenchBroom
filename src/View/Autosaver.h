/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Autosaver__
#define __TrenchBroom__Autosaver__

#include "IO/Path.h"
#include "View/ViewTypes.h"

#include <ctime>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class WritableDiskFileSystem;
    }
    
    namespace View {
        class Autosaver {
        private:
            View::MapDocumentPtr m_document;
            Logger* m_logger;
            
            time_t m_saveInterval;
            time_t m_idleInterval;
            size_t m_maxBackups;
            
            time_t m_lastSaveTime;
            time_t m_lastModificationTime;
            bool m_dirty;
        public:
            Autosaver(View::MapDocumentPtr document, const time_t saveInterval = 10 * 60, const time_t idleInterval = 3, const size_t maxBackups = 30);
            ~Autosaver();
            
            void triggerAutosave(Logger* logger);
            void updateLastModificationTime();
        private:
            void autosave();
            IO::WritableDiskFileSystem createBackupFileSystem(const IO::Path& mapPath) const;
            IO::Path::List collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const;
            bool isBackup(const IO::Path& backupPath, const IO::Path& mapBasename) const;
            void thinBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups) const;
            void cleanBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups, const IO::Path& mapBasename) const;
            String makeBackupName(const IO::Path& mapBasename, const size_t index) const;
        };

        size_t extractBackupNo(const IO::Path& path);
    }
}

#endif /* defined(__TrenchBroom__Autosaver__) */

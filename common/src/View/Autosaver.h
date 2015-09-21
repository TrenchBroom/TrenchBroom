/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Autosaver
#define TrenchBroom_Autosaver

#include "IO/Path.h"
#include "View/ViewTypes.h"

#include <ctime>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class WritableDiskFileSystem;
    }
    
    namespace View {
        class Command;
        
        class Autosaver {
        private:
            View::MapDocumentWPtr m_document;
            Logger* m_logger;
            
            time_t m_saveInterval;
            time_t m_idleInterval;
            size_t m_maxBackups;
            
            time_t m_lastSaveTime;
            time_t m_lastModificationTime;
            size_t m_lastModificationCount;
        public:
            Autosaver(View::MapDocumentWPtr document, time_t saveInterval = 10 * 60, time_t idleInterval = 3, size_t maxBackups = 30);
            ~Autosaver();
            
            void triggerAutosave(Logger* logger);
        private:
            void autosave(View::MapDocumentSPtr document);
            IO::WritableDiskFileSystem createBackupFileSystem(const IO::Path& mapPath) const;
            IO::Path::List collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const;
            bool isBackup(const IO::Path& backupPath, const IO::Path& mapBasename) const;
            void thinBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups) const;
            void cleanBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups, const IO::Path& mapBasename) const;
            String makeBackupName(const IO::Path& mapBasename, const size_t index) const;
        private:
            void bindObservers();
            void unbindObservers();
            void documentModificationCountDidChangeNotifier();
        };

        size_t extractBackupNo(const IO::Path& path);
    }
}

#endif /* defined(TrenchBroom_Autosaver) */

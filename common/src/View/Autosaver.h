/*
 Copyright (C) 2010-2017 Kristian Duske

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
        public:
            class BackupFileMatcher {
            private:
                const IO::Path m_mapBasename;
            public:
                explicit BackupFileMatcher(const IO::Path& mapBasename);
                bool operator()(const IO::Path& path, bool directory) const;
            };
        private:
            View::MapDocumentWPtr m_document;

            /**
             * The time after which a new autosave is attempted, in seconds.
             */
            std::time_t m_saveInterval;
            /**
             * The idle time. The editor must have been idle for this interval before a new autosave is attempted.
             * In seconds.
             */
            std::time_t m_idleInterval;

            /**
             * The maximum number of backups to create. When this number is exceeded, old backups are deleted until
             * the number of backups is equal to the number of backups again.
             */
            size_t m_maxBackups;

            /**
             * The time at which the last autosave has succeeded. POSIX timestamp.
             */
            std::time_t m_lastSaveTime;

            /**
             * The time at which the map was modified last. POSIX timestamp.
             */
            std::time_t m_lastModificationTime;

            /**
             * The modification count that was last recorded.
             */
            size_t m_lastModificationCount;
        public:
            explicit Autosaver(View::MapDocumentWPtr document, std::time_t saveInterval = 10 * 60, std::time_t idleInterval = 3, size_t maxBackups = 50);
            ~Autosaver();

            void triggerAutosave(Logger& logger);
        private:
            void autosave(Logger& logger, View::MapDocumentSPtr document);
            IO::WritableDiskFileSystem createBackupFileSystem(Logger& logger, const IO::Path& mapPath) const;
            IO::Path::List collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const;
            void thinBackups(Logger& logger, IO::WritableDiskFileSystem& fs, IO::Path::List& backups) const;
            void cleanBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups, const IO::Path& mapBasename) const;
            IO::Path makeBackupName(const IO::Path& mapBasename, const size_t index) const;
        private:
            void bindObservers();
            void unbindObservers();
            void documentModificationCountDidChangeNotifier();
        };

        size_t extractBackupNo(const IO::Path& path);
    }
}

#endif /* defined(TrenchBroom_Autosaver) */

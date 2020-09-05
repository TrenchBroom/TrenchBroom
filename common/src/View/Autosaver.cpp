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

#include "Autosaver.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <algorithm> // for std::sort
#include <cassert>
#include <limits>
#include <memory>

namespace TrenchBroom {
    namespace View {
        Autosaver::BackupFileMatcher::BackupFileMatcher(const IO::Path& mapBasename) :
        m_mapBasename(mapBasename) {}

        bool Autosaver::BackupFileMatcher::operator()(const IO::Path& path, const bool directory) const {
            if (directory) {
                return false;
            }
            if (!kdl::ci::str_is_equal(path.extension(), "map")) {
                return false;
            }

            const auto backupName = path.lastComponent().deleteExtension();
            const auto backupBasename = backupName.deleteExtension();
            if (backupBasename != m_mapBasename) {
                return false;
            }

            const auto backupExtension = backupName.extension();
            if (!kdl::str_is_numeric(backupExtension)) {
                return false;
            }

            const auto backupNo = kdl::str_to_size(backupName.extension()).value_or(0u);
            return backupNo > 0u;
        }

        Autosaver::Autosaver(std::weak_ptr<MapDocument> document, const std::chrono::milliseconds saveInterval, const size_t maxBackups) :
        m_document(document),
        m_saveInterval(saveInterval),
        m_maxBackups(maxBackups),
        m_lastSaveTime(Clock::now()),
        m_lastModificationCount(kdl::mem_lock(m_document)->modificationCount()) {}

        void Autosaver::triggerAutosave(Logger& logger) {
            if (kdl::mem_expired(m_document)) {
                return;
            }

            const auto currentTime = Clock::now();

            auto document = kdl::mem_lock(m_document);
            if (!document->modified()) {
                return;
            }
            if (document->modificationCount() == m_lastModificationCount) {
                return;
            }
            if (currentTime - m_lastSaveTime < m_saveInterval) {
                return;
            }

            const auto documentPath = document->path();
            if (!documentPath.isAbsolute()) {
                return;
            }
            if (!IO::Disk::fileExists(IO::Disk::fixPath(document->path()))) {
                return;
            }

            autosave(logger, document);
        }

        void Autosaver::autosave(Logger& logger, std::shared_ptr<MapDocument> document) {
            const auto& mapPath = document->path();
            assert(IO::Disk::fileExists(IO::Disk::fixPath(mapPath)));

            const auto mapFilename = mapPath.lastComponent();
            const auto mapBasename = mapFilename.deleteExtension();

            try {
                auto fs = createBackupFileSystem(logger, mapPath);
                auto backups = collectBackups(fs, mapBasename);

                thinBackups(logger, fs, backups);
                cleanBackups(fs, backups, mapBasename);

                assert(backups.size() < m_maxBackups);
                const auto backupNo = backups.size() + 1;

                const auto backupFilePath = fs.makeAbsolute(makeBackupName(mapBasename, backupNo));

                m_lastSaveTime = Clock::now();
                m_lastModificationCount = document->modificationCount();
                document->saveDocumentTo(backupFilePath);

                logger.info() << "Created autosave backup at " << backupFilePath;
            } catch (const FileSystemException& e) {
                logger.error() << "Aborting autosave: " << e.what();
            }
        }

        IO::WritableDiskFileSystem Autosaver::createBackupFileSystem(Logger& logger, const IO::Path& mapPath) const {
            const auto basePath = mapPath.deleteLastComponent();
            const auto autosavePath = basePath + IO::Path("autosave");

            try {
                // ensures that the directory exists or is created if it doesn't
                return IO::WritableDiskFileSystem(autosavePath, true);
            } catch (const FileSystemException& e) {
                logger.error() << "Cannot create autosave directory at " << autosavePath;
                throw e;
            }
        }

        bool compareBackupsByNo(const IO::Path& lhs, const IO::Path& rhs);
        bool compareBackupsByNo(const IO::Path& lhs, const IO::Path& rhs) {
            return extractBackupNo(lhs) < extractBackupNo(rhs);
        }

        std::vector<IO::Path> Autosaver::collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const {
            auto backups = fs.findItems(IO::Path(), BackupFileMatcher(mapBasename));
            std::sort(std::begin(backups), std::end(backups), compareBackupsByNo);
            return backups;
        }

        void Autosaver::thinBackups(Logger& logger, IO::WritableDiskFileSystem& fs, std::vector<IO::Path>& backups) const {
            while (backups.size() > m_maxBackups - 1) {
                const auto filename = backups.front();
                try {
                    fs.deleteFile(filename);
                    logger.debug() << "Deleted autosave backup " << filename;
                    backups.erase(std::begin(backups));
                } catch (const FileSystemException& e) {
                    logger.error() << "Cannot delete autosave backup " << filename;
                    throw e;
                }
            }
        }

        void Autosaver::cleanBackups(IO::WritableDiskFileSystem& fs, std::vector<IO::Path>& backups, const IO::Path& mapBasename) const {
            for (size_t i = 0; i < backups.size(); ++i) {
                const auto& oldName = backups[i].lastComponent();
                const auto newName = makeBackupName(mapBasename, i + 1);

                if (oldName != newName) {
                    fs.moveFile(oldName, newName, false);
                }
            }
        }

        IO::Path Autosaver::makeBackupName(const IO::Path& mapBasename, const size_t index) const {
            return IO::Path(kdl::str_to_string(mapBasename,".", index, ".map"));
        }

        size_t extractBackupNo(const IO::Path& path) {
                // currently this function is only used when comparing file names which have already been verified as
                // valid backup file names, so this should not go wrong, but if it does, sort the invalid file names to
                // the end to avoid modifying them
                return kdl::str_to_size(path.deleteExtension().extension()).value_or(std::numeric_limits<size_t>::max());
        }
    }
}

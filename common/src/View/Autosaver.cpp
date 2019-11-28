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
#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "View/MapDocument.h"

#include <algorithm> // for std::sort
#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace View {
        Autosaver::BackupFileMatcher::BackupFileMatcher(const IO::Path& mapBasename) :
        m_mapBasename(mapBasename) {}

        bool Autosaver::BackupFileMatcher::operator()(const IO::Path& path, const bool directory) const {
            if (directory) {
                return false;
            }
            if (!StringUtils::caseInsensitiveEqual(path.extension(), "map")) {
                return false;
            }

            const auto backupName = path.lastComponent().deleteExtension();
            const auto backupBasename = backupName.deleteExtension();
            if (backupBasename != m_mapBasename) {
                return false;
            }

            const auto backupExtension = backupName.extension();
            if (!StringUtils::isNumber(backupExtension)) {
                return false;
            }

            const auto no = StringUtils::stringToSize(backupName.extension());
            return no > 0;

        }

        Autosaver::Autosaver(std::weak_ptr<MapDocument> document, const std::time_t saveInterval, const std::time_t idleInterval, const size_t maxBackups) :
        m_document(document),
        m_saveInterval(saveInterval),
        m_idleInterval(idleInterval),
        m_maxBackups(maxBackups),
        m_lastSaveTime(time(nullptr)),
        m_lastModificationTime(0),
        m_lastModificationCount(lock(m_document)->modificationCount()) {
            bindObservers();
        }

        Autosaver::~Autosaver() {
            unbindObservers();
            NullLogger logger;
            triggerAutosave(logger);
        }

        void Autosaver::triggerAutosave(Logger& logger) {
            const auto currentTime = std::time(nullptr);

            auto document = lock(m_document);
            if (!document->modified()) {
                return;
            }
            if (document->modificationCount() == m_lastModificationCount) {
                return;
            }
            if (currentTime - m_lastModificationTime < m_idleInterval) {
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

                m_lastSaveTime = std::time(nullptr);
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

        IO::Path::List Autosaver::collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const {
            auto backups = fs.findItems(IO::Path(), BackupFileMatcher(mapBasename));
            std::sort(std::begin(backups), std::end(backups), compareBackupsByNo);
            return backups;
        }

        void Autosaver::thinBackups(Logger& logger, IO::WritableDiskFileSystem& fs, IO::Path::List& backups) const {
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

        void Autosaver::cleanBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups, const IO::Path& mapBasename) const {
            for (size_t i = 0; i < backups.size(); ++i) {
                const auto& oldName = backups[i].lastComponent();
                const auto newName = makeBackupName(mapBasename, i + 1);

                if (oldName != newName) {
                    fs.moveFile(oldName, newName, false);
                }
            }
        }

        IO::Path Autosaver::makeBackupName(const IO::Path& mapBasename, const size_t index) const {
            StringStream str;
            str << mapBasename.asString() << "." << index << ".map";
            return IO::Path(str.str());
        }

        size_t extractBackupNo(const IO::Path& path) {
            const auto no = StringUtils::stringToSize(path.deleteExtension().extension());
            assert(no > 0);
            return no;
        }

        void Autosaver::bindObservers() {
            auto document = lock(m_document);
            document->documentModificationStateDidChangeNotifier.addObserver(this, &Autosaver::documentModificationCountDidChangeNotifier);
        }

        void Autosaver::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->documentModificationStateDidChangeNotifier.removeObserver(this, &Autosaver::documentModificationCountDidChangeNotifier);
            }
        }

        void Autosaver::documentModificationCountDidChangeNotifier() {
            m_lastModificationTime = std::time(nullptr);
        }
    }
}

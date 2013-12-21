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

#include "Autosaver.h"

#include "StringUtils.h"
#include "IO/DiskFileSystem.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        Autosaver::Autosaver(View::MapDocumentWPtr document, const time_t saveInterval, const time_t idleInterval, const size_t maxBackups) :
        m_document(document),
        m_logger(NULL),
        m_saveInterval(saveInterval),
        m_idleInterval(idleInterval),
        m_maxBackups(maxBackups),
        m_lastSaveTime(time(NULL)),
        m_lastModificationTime(0),
        m_dirty(false) {}
        
        Autosaver::~Autosaver() {
            triggerAutosave(NULL);
        }
        
        void Autosaver::triggerAutosave(Logger* logger) {
            const time_t currentTime = time(NULL);
            
            MapDocumentSPtr document = lock(m_document);
            const IO::Path documentPath = document->path();
            if (document->modified() &&
                m_dirty &&
                m_lastModificationTime > 0 &&
                currentTime - m_lastModificationTime >= m_idleInterval &&
                currentTime - m_lastSaveTime >= m_saveInterval &&
                documentPath.isAbsolute() &&
                IO::Disk::fileExists(IO::Disk::fixPath(document->path()))) {
                
                m_logger = logger;
                try {
                    autosave(document);
                    m_logger = NULL;
                } catch (...) {
                    m_logger = NULL;
                    throw;
                }
            }
        }
        
        void Autosaver::updateLastModificationTime() {
            m_lastModificationTime = time(NULL);
            m_dirty = true;
        }
        
        void Autosaver::autosave(MapDocumentSPtr document) {
            const IO::Path& mapPath = document->path();
            assert(IO::Disk::fileExists(IO::Disk::fixPath(mapPath)));
            
            const IO::Path mapFilename = mapPath.lastComponent();
            const IO::Path mapBasename = mapFilename.deleteExtension();
            
            try {
                IO::WritableDiskFileSystem fs = createBackupFileSystem(mapPath);
                IO::Path::List backups = collectBackups(fs, mapBasename);
                
                thinBackups(fs, backups);
                cleanBackups(fs, backups, mapBasename);

                assert(backups.size() < m_maxBackups);
                const size_t backupNo = backups.size() + 1;
                
                const IO::Path backupFilePath = fs.getPath() + makeBackupName(mapBasename, backupNo);
                document->saveBackup(backupFilePath);
                
                if (m_logger != NULL)
                    m_logger->info("Created autosave backup at %s", backupFilePath.asString().c_str());
                
                m_lastSaveTime = time(NULL);
                m_dirty = false;
            } catch (FileSystemException e) {
                if (m_logger != NULL)
                    m_logger->error("Aborting autosave");
            }
        }
        
        IO::WritableDiskFileSystem Autosaver::createBackupFileSystem(const IO::Path& mapPath) const {
            const IO::Path basePath = mapPath.deleteLastComponent();
            const IO::Path autosavePath = basePath + IO::Path("autosave");

            try {
                // ensures that the directory exists or is created if it doesn't
                return IO::WritableDiskFileSystem(autosavePath, true);
            } catch (FileSystemException e) {
                if (m_logger != NULL)
                    m_logger->error("Cannot create autosave directory at %s", autosavePath.asString().c_str());
                throw e;
            }
        }

        struct BackupFileMatcher {
            const IO::Path& mapBasename;
            
            BackupFileMatcher(const IO::Path& i_mapBasename) :
            mapBasename(i_mapBasename) {}
            
            bool operator()(const IO::Path& path, const bool directory) const {
                if (directory)
                    return false;
                if (!StringUtils::caseInsensitiveEqual(path.extension(), "map"))
                    return false;
                
                const IO::Path backupName = path.lastComponent().deleteExtension();
                const IO::Path backupBasename = backupName.deleteExtension();
                if (backupBasename != mapBasename)
                    return false;
                
                const size_t no = std::atoi(backupName.extension().c_str());
                return no > 0;

            }
        };
        
        bool compareBackupsByNo(const IO::Path& lhs, const IO::Path& rhs) {
            return extractBackupNo(lhs) < extractBackupNo(rhs);
        }
        
        IO::Path::List Autosaver::collectBackups(const IO::WritableDiskFileSystem& fs, const IO::Path& mapBasename) const {
            IO::Path::List backups = fs.findItems(IO::Path(""), BackupFileMatcher(mapBasename));
            std::sort(backups.begin(), backups.end(), compareBackupsByNo);
            return backups;
        }
        
        void Autosaver::thinBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups) const {
            while (backups.size() > m_maxBackups - 1) {
                const IO::Path filename = backups.front();
                try {
                    fs.deleteFile(filename);
                    if (m_logger != NULL)
                        m_logger->debug("Deleted autosave backup %s", filename.asString().c_str());
                    backups.erase(backups.begin());
                } catch (FileSystemException e) {
                    if (m_logger != NULL)
                        m_logger->error("Cannot delete autosave backup %s", filename.asString().c_str());
                    throw e;
                }
            }
        }
        
        void Autosaver::cleanBackups(IO::WritableDiskFileSystem& fs, IO::Path::List& backups, const IO::Path& mapBasename) const {
            for (size_t i = 0; i < backups.size(); ++i) {
                const IO::Path& oldName = backups[i].lastComponent();
                const IO::Path newName = makeBackupName(mapBasename, i + 1);
                
                if (oldName != newName)
                    fs.moveFile(oldName, newName, false);
            }
        }
        
        String Autosaver::makeBackupName(const IO::Path& mapBasename, const size_t index) const {
            StringStream str;
            str << mapBasename.asString() << "." << index << ".map";
            return str.str();
        }
        
        size_t extractBackupNo(const IO::Path& path) {
            const size_t no = std::atoi(path.deleteExtension().extension().c_str());
            assert(no > 0);
            return no;
        }
    }
}

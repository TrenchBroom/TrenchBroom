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
#include "IO/FileSystem.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        Autosaver::Autosaver(View::MapDocumentPtr document, const time_t saveInterval, const time_t idleInterval, const size_t maxBackups) :
        m_document(document),
        m_logger(NULL),
        m_saveInterval(saveInterval),
        m_idleInterval(idleInterval),
        m_maxBackups(maxBackups),
        m_lastSaveTime(time(NULL)),
        m_lastModificationTime(0),
        m_dirty(false) {}
        
        Autosaver::~Autosaver() {
            autosave();
        }
        
        void Autosaver::triggerAutosave(Logger* logger) {
            IO::FileSystem fs;
            const time_t currentTime = time(NULL);
            
            if (fs.exists(m_document->path()) &&
                m_document->modified() &&
                m_dirty &&
                m_lastModificationTime > 0 &&
                currentTime - m_lastModificationTime >= m_idleInterval &&
                currentTime - m_lastSaveTime >= m_saveInterval) {
                
                m_logger = logger;
                try {
                    autosave();
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
        
        void Autosaver::autosave() {
            IO::FileSystem fs;
            
            const IO::Path& mapPath = m_document->path();
            assert(fs.exists(mapPath));
            
            const IO::Path basePath = mapPath.deleteLastComponent();
            const IO::Path autosavePath = basePath + IO::Path("autosave");
            const IO::Path mapFilename = mapPath.lastComponent();
            const IO::Path mapBasename = mapFilename.deleteExtension();
            
            if (!createAutosaveDirectory(autosavePath)) {
                if (m_logger != NULL)
                    m_logger->error("Aborting autosave");
                return;
            }
            
            IO::Path::List backups = collectBackups(autosavePath, mapBasename);
            if (!thinBackups(backups, autosavePath)) {
                if (m_logger != NULL)
                    m_logger->error("Aborting autosave");
                return;
            }
            
            if (!cleanBackups(backups, autosavePath, mapBasename)) {
                if (m_logger != NULL)
                    m_logger->error("Aborting autosave");
                return;
            }
            
            assert(backups.size() < m_maxBackups);
            const size_t backupNo = backups.size() + 1;
            
            const IO::Path backupFilePath = autosavePath + makeBackupName(mapBasename, backupNo);
            m_document->saveBackup(backupFilePath);
            if (m_logger != NULL)
                m_logger->info("Created autosave backup at %s", backupFilePath.asString().c_str());
            
            m_lastSaveTime = time(NULL);
            m_dirty = false;
        }
        
        bool Autosaver::createAutosaveDirectory(const IO::Path& autosavePath) const {
            IO::FileSystem fs;
            
            if (!fs.exists(autosavePath)) {
                try {
                    fs.createDirectory(autosavePath);
                } catch (FileSystemException e) {
                    if (m_logger != NULL)
                        m_logger->error("Cannot create autosave directory at %s", autosavePath.asString().c_str());
                    return false;
                }
                if (m_logger != NULL)
                    m_logger->info("Autosave directory created at %s", autosavePath.asString().c_str());
            } else if (!fs.isDirectory(autosavePath)) {
                if (m_logger != NULL)
                    m_logger->error("Cannot create autosave directory at %s because a file exists at %s", autosavePath.asString().c_str());
                return false;
            }
            return true;
        }
        
        bool compareBackupsByNo(const IO::Path& lhs, const IO::Path& rhs) {
            return extractBackupNo(lhs) < extractBackupNo(rhs);
        }
        
        IO::Path::List Autosaver::collectBackups(const IO::Path& autosavePath, const IO::Path& mapBasename) const {
            IO::FileSystem fs;
            IO::Path::List backups = fs.directoryContents(autosavePath, IO::FileSystem::FSFiles, "map");
            
            IO::Path::List::iterator it = backups.begin();
            while (it != backups.end()) {
                const IO::Path& backup = *it;
                if (!isBackup(backup, mapBasename))
                    it = backups.erase(it);
                else
                    ++it;
            }
            
            std::sort(backups.begin(), backups.end(), compareBackupsByNo);
            return backups;
        }
        
        bool Autosaver::isBackup(const IO::Path& backupPath, const IO::Path& mapBasename) const {
            const IO::Path backupName = backupPath.lastComponent().deleteExtension();
            const IO::Path backupBasename = backupName.deleteExtension();
            if (backupBasename != mapBasename)
                return false;
            
            const size_t no = std::atoi(backupName.extension().c_str());
            return no > 0;
        }
        
        bool Autosaver::thinBackups(IO::Path::List& backups, const IO::Path& autosavePath) const {
            IO::FileSystem fs;
            while (backups.size() > m_maxBackups - 1) {
                const IO::Path filePath = autosavePath + backups.front();
                try {
                    fs.deleteFile(filePath);
                    if (m_logger != NULL)
                        m_logger->debug("Deleted autosave backup %s", filePath.asString().c_str());
                    backups.erase(backups.begin());
                } catch (FileSystemException e) {
                    if (m_logger != NULL)
                        m_logger->error("Cannot delete autosave backup %s", filePath.asString().c_str());
                    return false;
                }
            }
            return true;
        }
        
        bool Autosaver::cleanBackups(IO::Path::List& backups, const IO::Path& autosavePath, const IO::Path& mapBasename) const {
            for (size_t i = 0; i < backups.size(); ++i) {
                const IO::Path& oldName = backups[i].lastComponent();
                const IO::Path newName = makeBackupName(mapBasename, i + 1);
                
                if (oldName != newName) {
                    const IO::Path sourcePath = autosavePath + oldName;
                    const IO::Path destPath = autosavePath + newName;
                    if (!moveBackup(sourcePath, destPath))
                        return false;
                }
            }
            
            return true;
        }
        
        bool Autosaver::moveBackup(const IO::Path& sourcePath, const IO::Path& destPath) const {
            IO::FileSystem fs;
            if (fs.exists(destPath)) {
                if (m_logger != NULL)
                    m_logger->error("Cannot move autosave backup %s to %s",
                                    sourcePath.asString().c_str(),
                                    destPath.asString().c_str());
                return false;
            } else {
                try {
                    fs.moveFile(sourcePath, destPath, false);
                    if (m_logger != NULL)
                        m_logger->debug("Moved autosave backup %s to %s",
                                        sourcePath.asString().c_str(),
                                        destPath.asString().c_str());
                    return true;
                } catch (FileSystemException e) {
                    if (m_logger != NULL)
                        m_logger->error("Cannot move autosave backup %s to %s",
                                        sourcePath.asString().c_str(),
                                        destPath.asString().c_str());
                    return false;
                }
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

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

#include "Autosaver.h"

#include "IO/FileManager.h"
#include "IO/MapWriter.h"
#include "Model/MapDocument.h"
#include "Utility/Console.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

namespace TrenchBroom {
    namespace Controller {
        unsigned int backupNoOfFile(const String& path) {
            IO::FileManager fileManager;
            String basePath = fileManager.deleteExtension(path);
            size_t spaceIndex = basePath.find_last_of(' ');
            if (spaceIndex == String::npos)
                return 0;
            
            int backupNo = std::atoi(basePath.substr(spaceIndex + 1).c_str());
            backupNo = (std::max)(backupNo, 0);
            return static_cast<unsigned int>(backupNo);
        }
        
        bool compareByBackupNo(const String& file1, const String& file2) {
            unsigned int backupNo1 = backupNoOfFile(file1);
            unsigned int backupNo2 = backupNoOfFile(file2);
            return backupNo1 < backupNo2;
        }
        
        String Autosaver::backupName(const String& mapBasename, unsigned int backupNo) {
            std::stringstream sstream;
            sstream << mapBasename;
            sstream << " ";
            sstream << backupNo;
            sstream << ".map";
            return sstream.str();
        }
        
        bool Autosaver::isBackupName(const String& basename, const String& mapBasename, unsigned int& backupNo) {
            if (basename.length() < mapBasename.length() + 2)
                return false;
            if (basename.substr(0, mapBasename.length()) != mapBasename)
                return false;
            
            int no = std::atoi(basename.substr(mapBasename.length()).c_str());
            if (no <= 0)
                return false;
            backupNo = static_cast<unsigned int>(no);
            return true;
        }
        
        void Autosaver::autosave() {
            const String mapPath = m_document.GetFilename().ToStdString();
            if (mapPath.empty())
                return;
            
            IO::FileManager fileManager;
            String basePath = fileManager.deleteLastPathComponent(mapPath);
            String autosavePath = fileManager.appendPath(basePath, "autosave");
            String mapFilename = fileManager.pathComponents(mapPath).back();
            String mapBasename = fileManager.deleteExtension(mapFilename);
            
            if (!fileManager.exists(autosavePath)) {
                if (!fileManager.makeDirectory(autosavePath)) {
                    m_document.console().error("Cannot create autosave directory at %s", autosavePath.c_str());
                    return;
                }
                
                m_document.console().info("Autosave directory created at %s", autosavePath.c_str());
            } else if (!fileManager.isDirectory(autosavePath)) {
                m_document.console().error("Cannot create autosave directory at %s because a file exists at that path", autosavePath.c_str());
                return;
            }
            
            // collect the actual backup files and determine the highest backup no
            StringList contents = fileManager.directoryContents(autosavePath, "map");
            StringList backups;
            
            unsigned int highestBackupNo = 0;
            for (size_t i = 0; i < contents.size(); i++) {
                const String& filename = contents[i];
                String basename = fileManager.deleteExtension(filename);
                unsigned int backupNo;
                if (isBackupName(basename, mapBasename, backupNo)) {
                    highestBackupNo = (std::max)(highestBackupNo, backupNo);
                    backups.push_back(filename);
                }
            }
            
            if (!backups.empty()) {
                // sort the backups by their backup nos in ascending order
                std::sort(backups.begin(), backups.end(), compareByBackupNo);
                
                // remove the oldest backups until backups.size() == m_maxBackups - 1
                while (backups.size() > m_maxBackups - 1) {
                    const String filePath = fileManager.appendPath(autosavePath, backups.front());
                    if (!fileManager.deleteFile(filePath)) {
                        m_document.console().error("Cannot delete file %s", filePath.c_str());
                        return;
                    } else {
                        m_document.console().debug("Deleted file %s", filePath.c_str());
                    }
                    
                    backups.erase(backups.begin());
                }
                
                // reorganize the backups and close gaps in the numbering
                for (unsigned int i = 0; i < backups.size(); i++) {
                    const String& filename = backups[i];
                    const String backupFilename = backupName(mapBasename, i + 1);
                    
                    if (filename != backupFilename) {
                        const String filePath = fileManager.appendPath(autosavePath, filename);
                        const String backupFilePath = fileManager.appendPath(autosavePath, backupFilename);
                        if (fileManager.exists(backupFilePath)) {
                            m_document.console().error("Cannot move file %s to %s because a file exists at that path", filePath.c_str(), backupFilePath.c_str());
                            return;
                        }
                        
                        if (!fileManager.moveFile(filePath, backupFilePath, false)) {
                            m_document.console().error("Cannot move file %s to %s", filePath.c_str(), backupFilePath.c_str());
                            return;
                        } else {
                            m_document.console().debug("Moved file %s to %s", filePath.c_str(), backupFilePath.c_str());
                        }
                    }
                }

                highestBackupNo = static_cast<unsigned int>(backups.size());
            }
            
            assert(highestBackupNo == static_cast<unsigned int>(backups.size()));
            assert(highestBackupNo < m_maxBackups);
            
            // save the backup
            const String backupFilename = backupName(mapBasename, highestBackupNo + 1);
            const String backupFilePath = fileManager.appendPath(autosavePath, backupFilename);
            
            wxStopWatch watch;
            IO::MapWriter mapWriter;
            mapWriter.writeToFileAtPath(m_document.map(), backupFilePath, true);
            m_document.console().debug("Autosaved to %s in %f seconds", backupFilePath.c_str(), watch.Time() / 1000.0f);
        }
        
        Autosaver::Autosaver(Model::MapDocument& document, time_t saveInterval, time_t idleInterval, unsigned int maxBackups) :
        m_document(document),
        m_saveInterval(saveInterval),
        m_idleInterval(idleInterval),
        m_maxBackups(maxBackups),
        m_lastSaveTime(time(NULL)),
        m_lastModificationTime(0),
        m_dirty(false) {}

        Autosaver::~Autosaver() {
            autosave();
        }

        void Autosaver::triggerAutosave() {
            time_t currentTime = time(NULL);
            IO::FileManager fileManager;
            if (fileManager.exists(m_document.GetFilename().ToStdString()) &&
                // m_dirty &&
                m_lastModificationTime > 0 &&
                currentTime - m_lastModificationTime >= m_idleInterval &&
                currentTime - m_lastSaveTime >= m_saveInterval) {
                
                autosave();
                m_lastSaveTime = currentTime;
                m_dirty = false;
            }
        }
        
        void Autosaver::updateLastModificationTime() {
            m_lastModificationTime = time(NULL);
            m_dirty = true;
        }
        
        void Autosaver::clearDirtyFlag() {
            m_dirty = false;
        }
    }
}

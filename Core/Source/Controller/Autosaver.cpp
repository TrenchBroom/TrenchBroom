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

#include "Controller/Editor.h"
#include "IO/FileManager.h"
#include "Model/Map/Map.h"
#include "Utilities/Console.h"

#include <string>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <algorithm>

namespace TrenchBroom {
    namespace Controller {
        unsigned int backupNoOfFile(const std::string& path) {
            IO::FileManager& fileManager = *IO::FileManager::sharedFileManager;
            std::string basePath = fileManager.deleteExtension(path);
            size_t spaceIndex = basePath.find_last_of(' ');
            if (spaceIndex == std::string::npos)
                return 0;
            
            int backupNo = std::atoi(basePath.substr(spaceIndex + 1).c_str());
            backupNo = std::max(backupNo, 0);
            return static_cast<unsigned int>(backupNo);
        }

        bool compareByBackupNo(const std::string& file1, const std::string& file2) {
            unsigned int backupNo1 = backupNoOfFile(file1);
            unsigned int backupNo2 = backupNoOfFile(file2);
            return backupNo1 > backupNo2;
        }

        std::string Autosaver::backupName(const std::string& mapBasename, unsigned int backupNo) {
            std::stringstream sstream;
            sstream << mapBasename;
            sstream << " ";
            sstream << backupNo;
            sstream << ".map";
            return sstream.str();
        }

        void Autosaver::triggerAutosave() {
            time_t currentTime = time(NULL);
            if (m_dirty && m_lastModificationTime > 0 && currentTime - m_lastModificationTime >= m_idleInterval && currentTime - m_lastSaveTime >= m_saveInterval) {
                const std::string& mapPath = m_editor.mapPath();
                if (mapPath.empty())
                    return;
                
                IO::FileManager& fileManager = *IO::FileManager::sharedFileManager;
                std::string basePath = fileManager.deleteLastPathComponent(mapPath);
                std::string autosavePath = fileManager.appendPath(basePath, "autosave");
                std::string mapFilename = fileManager.pathComponents(mapPath).back();
                std::string mapBasename = fileManager.deleteExtension(mapFilename);
                
                if (!fileManager.exists(autosavePath)) {
                    if (!fileManager.makeDirectory(autosavePath)) {
                        log(TB_LL_ERR, "Cannot create autosave directory at %s", autosavePath.c_str());
                        return;
                    }
                    
                    log(TB_LL_INFO, "Autosave directory created at %s", autosavePath.c_str());
                } else if (!fileManager.isDirectory(autosavePath)) {
                    log(TB_LL_ERR, "Cannot create autosave directory at %s because a file exists at that path", autosavePath.c_str());
                    return;
                }
                
                // collect the actual backup files and determine the highest backup no
                std::vector<std::string> contents = fileManager.directoryContents(autosavePath);
                std::vector<std::string> backups;
                unsigned int highestBackupNo = 0;
                for (unsigned int i = 0; i < contents.size(); i++) {
                    const std::string& filename = contents[i];
                    if (filename.substr(0, mapBasename.length()) == mapBasename) {
                        unsigned int backupNo = backupNoOfFile(filename);
                        if (backupNo > 0) {
                            highestBackupNo = std::max(highestBackupNo, backupNo);
                            backups.push_back(filename);
                        }
                    }
                }
                
                if (!backups.empty()) {
                    // sort the backups by their backup nos in descending order
                    std::sort(backups.begin(), backups.end(), compareByBackupNo);
                    
                    // remove the oldest backups until backups.size() == m_maxBackups - 1
                    while (backups.size() > m_maxBackups - 1) {
                        const std::string filePath = fileManager.appendPath(autosavePath, backups.back());
                        if (!fileManager.deleteFile(filePath)) {
                            log(TB_LL_ERR, "Cannot delete file %s", filePath.c_str());
                            return;
                        }
                        
                        backups.pop_back();
                    }
                    
                    if (highestBackupNo > backups.size()) {
                        // reorganize the backups and close gaps in the numbering
                        for (int i = backups.size() - 1; i >= 0; i--) {
                            const std::string& filename = backups[i];
                            const std::string backupFilename = backupName(mapBasename, i + 1);
                            if (filename != backupFilename) {
                                const std::string filePath = fileManager.appendPath(autosavePath, filename);
                                const std::string backupFilePath = fileManager.appendPath(autosavePath, backupFilename);
                                if (fileManager.exists(backupFilePath)) {
                                    log(TB_LL_ERR, "Cannot move file %s to %s because a file exists at that path", filePath.c_str(), backupFilePath.c_str());
                                    return;
                                }
                                
                                if (!fileManager.moveFile(filePath, backupFilePath, false)) {
                                    log(TB_LL_ERR, "Cannot move file %s to %s", filePath.c_str(), backupFilePath.c_str());
                                    return;
                                }
                                
                                backups[i] = backupFilename;
                            }
                        }
                        
                        highestBackupNo = backups.size();
                    }
                }

                assert(highestBackupNo == backups.size());
                assert(highestBackupNo < m_maxBackups);
                
                // save the backup
                const std::string backupFilename = backupName(mapBasename, highestBackupNo + 1);
                const std::string backupFilePath = fileManager.appendPath(autosavePath, backupFilename);
                m_editor.map().save(backupFilePath);

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
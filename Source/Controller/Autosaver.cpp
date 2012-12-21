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
            return backupNo1 > backupNo2;
        }

        String Autosaver::backupName(const String& mapBasename, unsigned int backupNo) {
            std::stringstream sstream;
            sstream << mapBasename;
            sstream << " ";
            sstream << backupNo;
            sstream << ".map";
            return sstream.str();
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
            StringList::iterator fwIt, fwEnd;
            
            unsigned int highestBackupNo = 0;
            for (fwIt = contents.begin(), fwEnd = contents.end(); fwIt != fwEnd; ++fwIt) {
                const String& filename = *fwIt;
                if (filename.substr(0, mapBasename.length()) == mapBasename) {
                    unsigned int backupNo = backupNoOfFile(filename);
                    if (backupNo > 0) {
                        highestBackupNo = (std::max)(highestBackupNo, backupNo);
                        backups.push_back(filename);
                    }
                }
            }
            
            if (!backups.empty()) {
                // sort the backups by their backup nos in descending order
                std::sort(backups.begin(), backups.end(), compareByBackupNo);
                
                // remove the oldest backups until backups.size() == m_maxBackups - 1
                while (backups.size() > m_maxBackups - 1) {
                    const String filePath = fileManager.appendPath(autosavePath, backups.back());
                    if (!fileManager.deleteFile(filePath)) {
                        m_document.console().error("Cannot delete file %s", filePath.c_str());
                        return;
                    }
                    
                    backups.pop_back();
                }
                
                if (highestBackupNo > backups.size()) {
                    // reorganize the backups and close gaps in the numbering
                    StringList::reverse_iterator rvIt, rvEnd;
                    unsigned int backupNo = static_cast<unsigned int>(backups.size());
                    for (rvIt = backups.rbegin(), rvEnd = backups.rend(); rvIt != rvEnd; ++rvIt) {
                        String& filename = *rvIt;
                        const String backupFilename = backupName(mapBasename, backupNo);
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
                            }
                            
                            filename = backupFilename;
                        }
                    }
                    
                    highestBackupNo = static_cast<unsigned int>(backups.size());
                }
            }
            
            assert(highestBackupNo == static_cast<unsigned int>(backups.size()));
            assert(highestBackupNo < m_maxBackups);
            
            // save the backup
            const String backupFilename = backupName(mapBasename, highestBackupNo + 1);
            const String backupFilePath = fileManager.appendPath(autosavePath, backupFilename);
            
            IO::MapWriter mapWriter;
            mapWriter.writeToFileAtPath(m_document.map(), backupFilePath, true);
        }
        
        void Autosaver::triggerAutosave() {
            time_t currentTime = time(NULL);
            if (m_dirty && m_lastModificationTime > 0 && currentTime - m_lastModificationTime >= m_idleInterval && currentTime - m_lastSaveTime >= m_saveInterval) {
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

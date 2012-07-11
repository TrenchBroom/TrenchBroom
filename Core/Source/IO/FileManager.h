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

#ifndef TrenchBroom_FileManager_h
#define TrenchBroom_FileManager_h

#include <vector>
#include <string>

namespace TrenchBroom {
    namespace IO {
        class FileManager {
        public:
            FileManager() {}
            virtual ~FileManager() {}
            
            static FileManager* sharedFileManager;
            
            virtual bool isDirectory(const std::string& path) = 0;
            virtual bool exists(const std::string& path) = 0;
            virtual bool makeDirectory(const std::string& path) = 0;
            virtual bool deleteFile(const std::string& path) = 0;
            virtual bool moveFile(const std::string& sourcePath, const std::string& destPath, bool overwrite) = 0;
            
            virtual std::vector<std::string> directoryContents(const std::string& path, std::string extension = "") = 0;

            std::vector<std::string> pathComponents(const std::string& path);
            std::string deleteLastPathComponent(const std::string& path);
            std::string appendPathComponent(const std::string& path, const std::string& component);
            std::string appendPath(const std::string& prefix, const std::string& suffix);

            std::string pathExtension(const std::string& path);
            std::string appendExtension(const std::string& path, const std::string& ext);
            std::string deleteExtension(const std::string& path);
            
            virtual char pathSeparator() = 0;
        };
    }
}

#endif

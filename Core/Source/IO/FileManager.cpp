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

#include "FileManager.h"

namespace TrenchBroom {
    namespace IO {
		FileManager* FileManager::sharedFileManager = 0;

        std::vector<std::string> FileManager::pathComponents(const std::string& path) {
            std::vector<std::string> components;
            if (path.empty()) return components;
            
            size_t lastPos = 0;
            size_t pos = 0;
            while ((pos = path.find_first_of(pathSeparator(), pos)) != std::string::npos) {
                if (pos > lastPos + 1)
                    components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
                lastPos = pos;
                pos++;
            }
            if (pos > lastPos + 1)
                components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
            
            return components;
        }
        
        std::string FileManager::deleteLastPathComponent(const std::string& path) {
            if (path.empty()) return path;
            size_t sepPos = path.find_last_of(pathSeparator());
            if (sepPos == std::string::npos) return "";
            return path.substr(0, sepPos);
        }
        
        std::string FileManager::appendPathComponent(const std::string& path, const std::string& component) {
            return appendPath(path, component);
        }
        
        std::string FileManager::appendPath(const std::string& prefix, const std::string& suffix) {
            if (prefix.empty()) return suffix;
            if (suffix.empty()) return prefix;
            
            std::string path = prefix;
            if (prefix[prefix.length() - 1] != pathSeparator() && suffix[0] != pathSeparator())
                path += pathSeparator();
            return path + suffix;
        }
        
        std::string FileManager::pathExtension(const std::string& path) {
            size_t pos = path.find_last_of('.');
            if (pos == std::string::npos) return "";
            return path.substr(pos + 1);
        }
        
        std::string FileManager::appendExtension(const std::string& path, const std::string& ext) {
            if (path.empty()) return "";
            if (ext.empty()) return path;
            
            std::string pathWithExt = path;
            if (ext[0] != '.')
                pathWithExt += '.';
            return pathWithExt + ext;
        }
        
        std::string FileManager::deleteExtension(const std::string& path) {
            size_t pos = path.find_last_of('.');
            if (pos == std::string::npos) return path;
            return path.substr(0, pos);
        }
    }
}

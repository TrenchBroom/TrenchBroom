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

#include "AbstractFileManager.h"

#include <wx/wx.h>
#include <wx/filename.h>

namespace TrenchBroom {
    namespace IO {
        bool AbstractFileManager::isDirectory(const String& path) {
            return wxDirExists(path);
        }
        
        bool AbstractFileManager::exists(const String& path) {
            return wxFileExists(path);
        }
        
        bool AbstractFileManager::makeDirectory(const String& path) {
            return wxMkdir(path);
        }
        
        bool AbstractFileManager::deleteFile(const String& path) {
            return wxRemoveFile(path);
        }
        
        bool AbstractFileManager::moveFile(const String& sourcePath, const String& destPath, bool overwrite) {
            return wxRenameFile(sourcePath, destPath, overwrite);
        }
        
        char AbstractFileManager::pathSeparator() {
            char c;
			wxFileName::GetPathSeparator().GetAsChar(&c);
            return c;
        }
        
        StringList AbstractFileManager::directoryContents(const String& path, String extension) {
            StringList result;
            if (!wxSetWorkingDirectory(path))
                return result;
            
            wxString spec = extension.empty() ? "*.*" : "*." + extension;
            wxString filename = wxFindFirstFile(spec);
            while (!filename.empty()) {
                result.push_back(pathComponents(filename.ToStdString()).back());
                filename = wxFindNextFile();
            }
            
            return result;
        }
        
        StringList AbstractFileManager::pathComponents(const String& path) {
            StringList components;
            if (path.empty()) return components;
            
            size_t lastPos = 0;
            size_t pos = 0;
            while ((pos = path.find_first_of(pathSeparator(), pos)) != String::npos) {
                if (pos > lastPos + 1)
                    components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
                lastPos = pos;
                pos++;
            }
            
            if (lastPos == 0)
                components.push_back(path);
            else if (lastPos < path.length() - 1)
                components.push_back(path.substr(lastPos + 1));
            
            return components;
        }
        
        String AbstractFileManager::deleteLastPathComponent(const String& path) {
            if (path.empty()) return path;
            size_t sepPos = path.find_last_of(pathSeparator());
            if (sepPos == String::npos) return "";
            return path.substr(0, sepPos);
        }
        
        String AbstractFileManager::appendPathComponent(const String& path, const String& component) {
            return appendPath(path, component);
        }
        
        String AbstractFileManager::appendPath(const String& prefix, const String& suffix) {
            if (prefix.empty()) return suffix;
            if (suffix.empty()) return prefix;
            
            String path = prefix;
            if (prefix[prefix.length() - 1] != pathSeparator() && suffix[0] != pathSeparator())
                path += pathSeparator();
            return path + suffix;
        }
        
        String AbstractFileManager::pathExtension(const String& path) {
            size_t pos = path.find_last_of('.');
            if (pos == String::npos) return "";
            return path.substr(pos + 1);
        }
        
        String AbstractFileManager::appendExtension(const String& path, const String& ext) {
            if (path.empty()) return "";
            if (ext.empty()) return path;
            
            String pathWithExt = path;
            if (ext[0] != '.')
                pathWithExt += '.';
            return pathWithExt + ext;
        }
        
        String AbstractFileManager::deleteExtension(const String& path) {
            size_t pos = path.find_last_of('.');
            if (pos == String::npos) return path;
            return path.substr(0, pos);
        }
    }
}
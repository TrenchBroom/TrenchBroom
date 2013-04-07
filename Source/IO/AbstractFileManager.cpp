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

#include <map> 

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace TrenchBroom {
    namespace IO {
#ifndef _WIN32
        PosixMappedFile::PosixMappedFile(int filedesc, char* address, size_t size) :
        MappedFile(address, address + size),
        m_filedesc(filedesc) {}
        
        PosixMappedFile::~PosixMappedFile() {
            if (m_begin != NULL) {
                munmap(m_begin, m_size);
                m_begin = NULL;
                m_end = NULL;
            }
            
            if (m_filedesc >= 0) {
                close(m_filedesc);
                m_filedesc = -1;
            }
        }
#endif
        
        bool AbstractFileManager::isAbsolutePath(const String& path) {
            return wxIsAbsolutePath(path);
        }

        bool AbstractFileManager::isDirectory(const String& path) {
            return wxDirExists(path);
        }
        
        bool AbstractFileManager::exists(const String& path) {
            if (isDirectory(path))
                return true;
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
            static const char c = wxFileName::GetPathSeparator();
            return c;
        }
        
        StringList AbstractFileManager::directoryContents(const String& path, String extension, bool directories, bool files) {
            StringList result;
            if (!isDirectory(path) || !wxSetWorkingDirectory(path))
                return result;
            if (!directories && !files)
                return result;
            
            int flags;
            if (directories && files)
                flags = 0;
            else if (directories)
                flags = wxDIR;
            else
                flags = wxFILE;
            
            String lowerExtension = Utility::toLower(extension);
            wxString filename = wxFindFirstFile("*", flags);
            while (!filename.empty()) {
                String stdFilename = filename.ToStdString();

                bool matches = extension.empty() || Utility::toLower(pathExtension(stdFilename)) == lowerExtension;
                if (matches)
                    result.push_back(pathComponents(stdFilename).back());
                
                filename = wxFindNextFile();
            }
            
            return result;
        }
        
        bool AbstractFileManager::resolveRelativePath(const String& relativePath, const StringList& rootPaths, String& absolutePath) {
            StringList::const_iterator rootIt, rootEnd;
            for (rootIt = rootPaths.begin(), rootEnd = rootPaths.end(); rootIt != rootEnd; ++rootIt) {
                const String& rootPath = *rootIt;
                absolutePath = makeAbsolute(relativePath, rootPath);
                if (exists(absolutePath))
                    return true;
            }
            
            return false;
        }

        StringList AbstractFileManager::resolveSearchpaths(const String& rootPath, const StringList& searchPaths) {
            typedef std::map<String, String> NameMap;
            NameMap nameMap;
            
            StringList contents = directoryContents(rootPath, "", true, false);
            for (size_t i = 0; i < contents.size(); i++)
                nameMap[Utility::toLower(contents[i])] = contents[i];

            StringList result;
            for (size_t i = 0; i < searchPaths.size(); i++) {
                NameMap::iterator it = nameMap.find(Utility::toLower(searchPaths[i]));
                if (it != nameMap.end())
                    result.push_back(appendPath(rootPath, it->second));
            }
            
            return result;
        }

        StringList AbstractFileManager::pathComponents(const String& path) {
            StringList components;
            if (path.empty()) return components;
            
            size_t pos = path.find_first_of(pathSeparator(), 0);
            if (pos != String::npos) {
                if (pos > 0)
                    components.push_back(path.substr(0, pos));
                size_t lastPos = pos;
                while ((pos = path.find_first_of(pathSeparator(), lastPos + 1)) != String::npos) {
                    components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
                    lastPos = pos;
                }
                if (lastPos < path.length() - 1)
                    components.push_back(path.substr(lastPos + 1));
            } else {
                components.push_back(path);
            }

            return components;
        }
        
        String AbstractFileManager::joinComponents(const StringList& pathComponents) {
            if (pathComponents.empty())
                return "";
            StringStream result;
            for (unsigned int i = 0; i < pathComponents.size() - 1; i++)
                result << pathComponents[i] << pathSeparator();
            result << pathComponents.back();
            
            return result.str();
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
        
        String AbstractFileManager::resolvePath(const String& path) {
            StringList components = resolvePath(pathComponents(path));
            String cleanPath = joinComponents(components);
            if (path[0] == '/')
                cleanPath = "/" + cleanPath;
            return cleanPath;
        }

        StringList AbstractFileManager::resolvePath(const StringList& pathComponents) {
            StringList cleanComponents;
            for (unsigned int i = 0; i < pathComponents.size(); i++) {
                const String& component = pathComponents[i];
                if (component != ".") {
                    if (component == ".." && !cleanComponents.empty())
                        cleanComponents.pop_back();
                    else
                        cleanComponents.push_back(component);
                }
            }
            return cleanComponents;
        }

        String AbstractFileManager::makeRelative(const String& absolutePath, const String& referencePath) {
            if (!::wxIsAbsolutePath(absolutePath))
                return absolutePath;
            if (!::wxIsAbsolutePath(referencePath))
                return "";
            
            StringList absolutePathComponents = resolvePath(pathComponents(absolutePath));
            StringList referencePathComponents = resolvePath(pathComponents(referencePath));
            StringList relativePathComponents;

            if (!isDirectory(referencePath))
                referencePathComponents.pop_back();
            
            unsigned int i = 0;
            for (; i < (std::min)(absolutePathComponents.size(), referencePathComponents.size()); i++) {
                const String& absolutePathComponent = absolutePathComponents[i];
                const String& referencePathComponent = referencePathComponents[i];
                if (absolutePathComponent != referencePathComponent)
                    break;
            }
            
            for (unsigned int j = i; j < referencePathComponents.size(); j++)
                relativePathComponents.push_back("..");
            
            for (unsigned int j = i; j < absolutePathComponents.size(); j++)
                relativePathComponents.push_back(absolutePathComponents[j]);
            
            return joinComponents(relativePathComponents);
        }

        String AbstractFileManager::makeAbsolute(const String& relativePath, const String& referencePath) {
            if (::wxIsAbsolutePath(relativePath))
                return relativePath;
            if (!::wxIsAbsolutePath(referencePath))
                return "";

            String folderPath = isDirectory(referencePath) ? referencePath : deleteLastPathComponent(referencePath);
            return resolvePath(appendPath(folderPath, relativePath));
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

#ifndef _WIN32
        MappedFile::Ptr AbstractFileManager::mapFile(const String& path, std::ios_base::openmode mode) {
            int filedesc = -1;
            char* address = NULL;
            size_t size = 0;
            
            int flags = 0;
            int prot = 0;
            if ((mode & std::ios_base::in)) {
                if ((mode & std::ios_base::out))
                    flags = O_RDWR;
                else
                    flags = O_RDONLY;
                prot |= PROT_READ;
            }
            if ((mode & std::ios_base::out)) {
                if (!(mode & std::ios_base::in))
                    flags = O_WRONLY;
                prot |= PROT_WRITE;
            }
            
            filedesc = open(path.c_str(), flags);
            if (filedesc >= 0) {
                size = static_cast<size_t>(lseek(filedesc, 0, SEEK_END));
                lseek(filedesc, 0, SEEK_SET);
                address = static_cast<char*>(mmap(NULL, size, prot, MAP_FILE | MAP_PRIVATE, filedesc, 0));
                if (address == NULL) {
                    close(filedesc);
                    filedesc = -1;
                } else {
                    return MappedFile::Ptr(new PosixMappedFile(filedesc, address, size));
                }
            }
            
            return MappedFile::Ptr();
        }
#endif
    }
}

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

#include "MacFileManager.h"

#include "CoreFoundation/CoreFoundation.h"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sys/mman.h>

namespace TrenchBroom {
    namespace IO {
        MacMappedFile::MacMappedFile(int filedesc, char* address, size_t size) :
        MappedFile(address, address + size),
        m_filedesc(filedesc) {}
        
        MacMappedFile::~MacMappedFile() {
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

        String MacFileManager::logDirectory() {
            CFBundleRef mainBundle = CFBundleGetMainBundle ();
            CFURLRef mainBundlePathUrl = CFBundleCopyBundleURL(mainBundle);

            UInt8 buffer[512];
            CFURLGetFileSystemRepresentation(mainBundlePathUrl, true, buffer, 512);
            CFRelease(mainBundlePathUrl);

            StringStream result;
            for (unsigned int i = 0; i < 512; i++) {
                UInt8 c = buffer[i];
                if (c == 0)
                    break;
                result << c;
            }
            
            return result.str();
        }

        String MacFileManager::resourceDirectory() {
            CFBundleRef mainBundle = CFBundleGetMainBundle ();
            CFURLRef resourcePathUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);

            UInt8 buffer[512];
            CFURLGetFileSystemRepresentation(resourcePathUrl, true, buffer, 512);
            CFRelease(resourcePathUrl);
            
            StringStream result;
            for (unsigned int i = 0; i < 512; i++) {
                UInt8 c = buffer[i];
                if (c == 0)
                    break;
                result << c;
            }
            
            return result.str();
        }

        String MacFileManager::resolveFontPath(const String& fontName) {
            String fontDirectoryPaths[2] = {"/System/Library/Fonts/", "/Library/Fonts/"};
            String extensions[2] = {".ttf", ".ttc"};
            
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    String fontPath = fontDirectoryPaths[i] + fontName + extensions[j];
                    std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
                    if (fs.is_open())
                        return fontPath;
                }
            }
            
            return "/System/Library/Fonts/LucidaGrande.ttc";
        }

        MappedFile::Ptr MacFileManager::mapFile(const String& path, std::ios_base::openmode mode) {
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
                    return MappedFile::Ptr(new MacMappedFile(filedesc, address, size));
                }
            }
            
            return MappedFile::Ptr();
        }
    }
}

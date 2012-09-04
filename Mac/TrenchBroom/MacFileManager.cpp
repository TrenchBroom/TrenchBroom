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

#include "CoreFoundation/CoreFoundation.h"

#include <fstream>

namespace TrenchBroom {
    namespace IO {
        String MacFileManager::resourceDirectory() {
            CFBundleRef mainBundle = CFBundleGetMainBundle ();
            CFURLRef resourcePathUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);

            UInt8 buffer[256];
            CFURLGetFileSystemRepresentation(resourcePathUrl, true, buffer, 256);
            CFRelease(resourcePathUrl);
            
            StringStream result;
            for (unsigned int i = 0; i < 256; i++) {
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
    }
}
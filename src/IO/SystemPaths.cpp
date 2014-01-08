/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "SystemPaths.h"

#include "IO/DiskFileSystem.h"
#include "IO/Path.h"

#if defined __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#elif defined _WIN32
#include <Windows.h>
#elif defined __linux__
#endif

namespace TrenchBroom {
    namespace IO {
        namespace SystemPaths {
#if defined __APPLE__
            Path appDirectory() {
                CFBundleRef mainBundle = CFBundleGetMainBundle ();
                CFURLRef bundleUrl = CFBundleCopyBundleURL(mainBundle);
                
                UInt8 buffer[1024];
                CFURLGetFileSystemRepresentation(bundleUrl, true, buffer, 1024);
                CFRelease(bundleUrl);
                
                StringStream result;
                for (size_t i = 0; i < 1024; ++i) {
                    UInt8 c = buffer[i];
                    if (c == 0)
                        break;
                    result << c;
                }
                
                return Path(result.str()).deleteLastComponent();
            }
#elif defined _WIN32
            Path appDirectory() {
                TCHAR uAppPathC[MAX_PATH] = L"";
                DWORD numChars = GetModuleFileName(0, uAppPathC, MAX_PATH - 1);
                
                char appPathC[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, uAppPathC, numChars, appPathC, numChars, NULL, NULL);
                appPathC[numChars] = 0;
                
                const String appPathStr(appPathC);
                const Path appPath(appPathStr);
                return appPath.deleteLastComponent();
            }
#elif defined __linux__
            Path appDirectory() {
                char buf[1024];
                const ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf));
                
                const String appPathStr(buf, len);
                const Path appPath(appPathStr);
                return appPath.deleteLastComponent();
            }
#endif
            
#if defined __APPLE__
            Path resourceDirectory() {
                CFBundleRef mainBundle = CFBundleGetMainBundle ();
                CFURLRef resourcePathUrl = CFBundleCopyResourcesDirectoryURL(mainBundle);
                
                UInt8 buffer[1024];
                CFURLGetFileSystemRepresentation(resourcePathUrl, true, buffer, 1024);
                CFRelease(resourcePathUrl);
                
                StringStream result;
                for (size_t i = 0; i < 1024; ++i) {
                    UInt8 c = buffer[i];
                    if (c == 0)
                        break;
                    result << c;
                }
                
                return Path(result.str());
            }
#elif defined _WIN32
            Path resourceDirectory() {
                return appDirectory() + Path("Resources");
            }
#elif defined __linux__
            Path resourceDirectory() {
                return appDirectory() + Path("Resources");
            }
#endif
            
#if defined __APPLE__
            Path findFontFile(const String& fontName) {
                const Path fontDirectoryPaths[2] = {
                    Disk::fixPath(Path("/System/Library/Fonts/")),
                    Disk::fixPath(Path("/Library/Fonts/"))
                };
                const String extensions[2] = {".ttf", ".ttc"};
                
                for (size_t i = 0; i < 2; ++i) {
                    if (Disk::directoryExists(fontDirectoryPaths[i])) {
                        const DiskFileSystem fs(fontDirectoryPaths[i]);
                        for (size_t j = 0; j < 2; ++j) {
                            if (fs.fileExists(Path(fontName + extensions[j])))
                                return fontDirectoryPaths[i] + Path(fontName + extensions[j]);
                        }
                    }
                }
                
                return Path("/System/Library/Fonts/LucidaGrande.ttc");
            }
#elif defined _WIN32
            Path findFontFile(const String& fontName) {
                TCHAR uWindowsPathC[MAX_PATH] = L"";
                DWORD numChars = GetWindowsDirectory(uWindowsPathC, MAX_PATH - 1);
                
                char windowsPathC[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, uWindowsPathC, numChars, windowsPathC, numChars, NULL, NULL);
                windowsPathC[numChars] = 0;
                
                const Path windowsPath(String(windowsPathC, numChars));
                
                const String extensions[2] = {".ttf", ".ttc"};
                const Path fontDirectoryPath = windowsPath + Path("Fonts");
                
                if (Disk::directoryExists(fontDirectoryPath)) {
                    const DiskFileSystem fs(fontDirectoryPath);
                    for (size_t i = 0; i < 2; ++i) {
                        if (fs.fileExists(Path(fontName + extensions[i])))
                            return fontDirectoryPath + Path(fontName + extensions[i]);
                    }
                }
                
                return fontDirectoryPath + Path("Arial.ttf");
            }
#elif defined __linux__
            Path findFontFile(const String& fontName) {
                const Path fontDirectoryPath = Disk::fixPath(Path("/usr/share/fonts/truetype/"));
                const String fontNames[3] = {fontName, "DejaVuSans", "ttf-dejavu/DejaVuSans"};
                const String extensions[2] = {".ttf", ".ttc"};
                
                if (Disk::directoryExists(fontDirectoryPath)) {
                    DiskFileSystem fs(fontDirectoryPath);
                    for (size_t i = 0; i < 3; ++i) {
                        for (size_t j = 0; j < 2; ++j) {
                            if (fs.fileExists(Path(fontNames[i] + extensions[j])))
                                return fontDirectoryPath + Path(fontNames[i] + extensions[j]);
                        }
                    }
                }
                
                return Path("");
            }
#endif
        }
    }
}

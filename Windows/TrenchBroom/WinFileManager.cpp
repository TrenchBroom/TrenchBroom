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

#include "WinFileManager.h"

#include <Windows.h>
#include <fstream>

namespace TrenchBroom {
	namespace IO {
        String WinFileManager::resourceDirectory() {
			TCHAR uAppPathC[MAX_PATH] = L"";
			DWORD numChars = GetModuleFileName(0, uAppPathC, MAX_PATH - 1);

			char appPathC[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, uAppPathC, numChars, appPathC, numChars, NULL, NULL);
			appPathC[numChars] = 0;

			String appPath(appPathC);
			String appDirectory = deleteLastPathComponent(appPath);
			return appendPath(appDirectory, "Resources");
		}

		String WinFileManager::resolveFontPath(const String& fontName) {
			TCHAR uWindowsPathC[MAX_PATH] = L"";
			DWORD numChars = GetWindowsDirectory(uWindowsPathC, MAX_PATH - 1);

			char windowsPathC[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, uWindowsPathC, numChars, windowsPathC, numChars, NULL, NULL);
			windowsPathC[numChars] = 0;

			String windowsPath(windowsPathC);
			if (windowsPath.back() != '\\')
				windowsPath.push_back('\\');

			String extensions[2] = {".ttf", ".ttc"};
			String fontDirectoryPath = windowsPath + "Fonts\\";
			String fontBasePath = fontDirectoryPath + fontName;

			for (int i = 0; i < 2; i++) {
				String fontPath = fontBasePath + extensions[i];
				std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
				if (fs.is_open())
					return fontPath;
			}

			return fontDirectoryPath + "Arial.ttf";            
		}
	}
}
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


#include "WinFontManager.h"
#include "stdafx.h"
#include "Utilities/Utils.h"
#include <fstream>

namespace TrenchBroom {
    namespace Renderer {
        std::string WinFontManager::resolveFont(const std::string& name) {
			std::string extensions[2] = {".ttf", ".ttc"};

			TCHAR windowsPathC[MAX_PATH];
			GetWindowsDirectory(windowsPathC, MAX_PATH);
			std::string windowsPath(windowsPathC);
			if (windowsPath.back() != '\\')
				windowsPath.push_back('\\');
			std::string fontDirectoryPath = windowsPath + "Fonts\\";
			std::string fontBasePath = fontDirectoryPath + name;

			for (int i = 0; i < 2; i++) {
				std::string fontPath = fontBasePath + extensions[i];
				std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
				if (fs.is_open())
					return fontPath;
			}
			return fontDirectoryPath + "Arial.ttf";
        }
    }
}
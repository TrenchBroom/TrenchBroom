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


#import "MacFontManager.h"
#import "Utilities/Utils.h"
#import <fstream>

namespace TrenchBroom {
    namespace Renderer {
        std::string MacFontManager::resolveFont(const std::string& name) {
            std::string fontDirectoryPaths[2] = {"/System/Library/Fonts/", "/Library/Fonts/"};
            std::string extensions[2] = {".ttf", ".ttc"};
            
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    std::string fontPath = fontDirectoryPaths[i] + name + extensions[j];
                    std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
                    if (fs.is_open())
                        return fontPath;
                }
            }
            
            return "/System/Library/Fonts/LucidaGrande.ttc";
        }
    }
}
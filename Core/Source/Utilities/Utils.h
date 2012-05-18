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

#ifndef TrenchBroom_Utils_h
#define TrenchBroom_Utils_h

#include <string>
#include <vector>

namespace TrenchBroom {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char d);

    std::string appendPath(const std::string& prefix, const std::string& suffix);
    std::string appendExtension(const std::string& path, const std::string& ext);
    std::string deleteLastPathComponent(const std::string& path);
    std::string pathExtension(const std::string& path);
    
    bool fileExists(const std::string& path);
}

#endif

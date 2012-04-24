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

#include "Utils.h"
#include <fstream>

namespace TrenchBroom {
    string trim(const string& str) {
        if (str.length() == 0) return str;
        size_t first = str.find_first_not_of(" \n\t\r" + 0);
        size_t last = str.find_last_not_of(" \n\t\r" + 0);
        if (first >= last) return "";
        return str.substr(first, last - first + 1);
    }
    
    vector<string> split(const string& str, char d) {
        vector<string> result;
        int lastIndex = 0;
        for (int i = 0; i < str.length(); i++) {
            char c = str[i];
            if (c == d && lastIndex < i) {
                result.push_back(str.substr(lastIndex, i - lastIndex));
                lastIndex = i + 1;
            }
        }
        if (lastIndex < str.length() - 1)
            result.push_back(str.substr(lastIndex, str.length() - lastIndex));
        return result;
    }
    
    string appendPath(const string& prefix, const string& suffix) {
        if (prefix.empty()) return suffix;
        if (suffix.empty()) return prefix;
        
        string path = prefix;
        if (prefix[prefix.length() - 1] != '/' && suffix[0] != '/')
            path += '/';
        return path + suffix;
    }

    string deleteLastPathComponent(const string& path) {
        if (path.empty()) return path;
        size_t sepPos = path.find_last_of("/\\");
        if (sepPos == string::npos) return "";
        return path.substr(0, sepPos);
    }
    
    string pathExtension(const string& path) {
        size_t pos = path.find_last_of('.');
        if (pos == string::npos) return "";
        return path.substr(pos + 1);
    }

    bool fileExists(const string& path) {
        fstream testStream(path.c_str());
        return testStream.is_open();
    }
}
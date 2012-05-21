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
#include <algorithm>

namespace TrenchBroom {
    std::string trim(const std::string& str) {
        if (str.length() == 0) return str;
        size_t first = str.find_first_not_of(" \n\t\r" + 0);
        size_t last = str.find_last_not_of(" \n\t\r" + 0);
        if (first >= last) return "";
        return str.substr(first, last - first + 1);
    }
    
    std::vector<std::string> split(const std::string& str, char d) {
        std::vector<std::string> result;
        unsigned int lastIndex = 0;
        for (unsigned int i = 0; i < str.length(); i++) {
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
    
    std::string appendPath(const std::string& prefix, const std::string& suffix) {
        if (prefix.empty()) return suffix;
        if (suffix.empty()) return prefix;
        
        std::string path = prefix;
        if (prefix[prefix.length() - 1] != '/' && suffix[0] != '/')
            path += '/';
        return path + suffix;
    }

    std::string appendExtension(const std::string& path, const std::string& ext) {
        if (path.empty()) return "";
        if (ext.empty()) return path;
        
        std::string pathWithExt = path;
        if (ext[0] != '.')
            pathWithExt += '.';
        return pathWithExt + ext;
    }

    std::string deleteLastPathComponent(const std::string& path) {
        if (path.empty()) return path;
        size_t sepPos = path.find_last_of("/\\");
        if (sepPos == std::string::npos) return "";
        return path.substr(0, sepPos);
    }
    
    std::vector<std::string> pathComponents(const std::string& path) {
        std::vector<std::string> components;
        if (path.empty()) return components;
        
        size_t lastPos = 0;
        size_t pos = 0;
        while ((pos = path.find_first_of("/\\", pos)) != std::string::npos) {
            if (pos > lastPos + 1)
                components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
            lastPos = pos;
            pos++;
        }
        if (pos > lastPos + 1)
            components.push_back(path.substr(lastPos + 1, pos - lastPos - 1));
        
        return components;
    }

    std::string pathExtension(const std::string& path) {
        size_t pos = path.find_last_of('.');
        if (pos == std::string::npos) return "";
        return path.substr(pos + 1);
    }
    
    bool caseInsensitiveCharEqual(char c1, char c2) {
        return std::toupper(c1) == std::toupper(c2);
    }
    
    bool caseSensitiveCharEqual(char c1, char c2) {
        return c1 == c2;
    }
    
    bool containsString(const std::string& haystack, const std::string& needle, bool caseSensitive) {
        std::string::const_iterator it;
        if (caseSensitive)
            it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseSensitiveCharEqual);
        else
            it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), caseInsensitiveCharEqual);
        return it != haystack.end();
    }

    bool fileExists(const std::string& path) {
        std::fstream testStream(path.c_str());
        return testStream.is_open();
    }
}
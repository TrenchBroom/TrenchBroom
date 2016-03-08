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

#ifndef TrenchBroom_Path
#define TrenchBroom_Path

#include "StringUtils.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path {
        public:
            typedef std::vector<Path> List;
            static char separator();
            
            struct ToString {
                char m_separator;
                ToString(const char i_separator = separator()) :
                m_separator(i_separator) {}
                
                String operator()(const Path& path) const {
                    return path.asString(m_separator);
                }
            };
        private:
            static const String& separators();
            
            StringList m_components;
            bool m_absolute;
            
            Path(bool absolute, const StringList& components);
        public:
            Path(const String& path = "");
            
            Path operator+(const Path& rhs) const;
            int compare(const Path& rhs) const;
            bool operator==(const Path& rhs) const;
            bool operator!= (const Path& rhs) const;
            bool operator<(const Path& rhs) const;
            bool operator>(const Path& rhs) const;
            
            String asString(const char sep = separator()) const;
            String asString(const String& sep) const;
            static StringList asStrings(const Path::List& paths, const char sep = separator());
            
            size_t length() const;
            bool isEmpty() const;
            Path firstComponent() const;
            Path deleteFirstComponent() const;
            Path lastComponent() const;
            Path deleteLastComponent() const;
            Path prefix(const size_t count) const;
            Path suffix(const size_t count) const;
            Path subPath(const size_t index, const size_t count) const;
            const String extension() const;
            Path deleteExtension() const;
            Path addExtension(const String& extension) const;
            
            bool isAbsolute() const;
            bool canMakeRelative(const Path& absolutePath) const;
            Path makeAbsolute(const Path& relativePath) const;
            Path makeRelative(const Path& absolutePath) const;
            Path makeCanonical() const;
            Path makeLowerCase() const;
            
            static List makeAbsoluteAndCanonical(const List& paths, const String& relativePath);
        private:
            static bool hasDriveSpec(const StringList& components);
            static bool hasDriveSpec(const String& component);
            StringList resolvePath(const bool absolute, const StringList& components) const;
        };
    }
}

#endif /* defined(TrenchBroom_Path) */

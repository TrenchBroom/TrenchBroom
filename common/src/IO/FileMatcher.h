/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef FileMatcher_h
#define FileMatcher_h

#include "StringUtils.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class FileTypeMatcher {
        private:
            bool m_files;
            bool m_directories;
        public:
            FileTypeMatcher(bool files = true, bool directories = true);
            bool operator()(const Path& path, bool directory) const;
        };
        
        class FileExtensionMatcher {
        private:
            String m_extension;
        public:
            FileExtensionMatcher(const String& extension);
            bool operator()(const Path& path, bool directory) const;
        };
        
        class FileNameMatcher {
        private:
            String m_pattern;
        public:
            FileNameMatcher(const String& pattern);
            bool operator()(const Path& path, bool directory) const;
        };

        class ExecutableFileMatcher {
        public:
            bool operator()(const Path& path, bool directory) const;
        };
    }
}

#endif /* FileMatcher_h */

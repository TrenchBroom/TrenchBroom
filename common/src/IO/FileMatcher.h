/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path;

        class FileTypeMatcher {
        private:
            bool m_files;
            bool m_directories;
        public:
            explicit FileTypeMatcher(bool files = true, bool directories = true);
            bool operator()(const Path& path, bool directory) const;
        };

        class FileExtensionMatcher {
        private:
            std::vector<std::string> m_extensions;
        public:
            explicit FileExtensionMatcher(const std::string& extension);
            explicit FileExtensionMatcher(const std::vector<std::string>& extensions);
            bool operator()(const Path& path, bool directory) const;
        };

        class FileBasenameMatcher : public FileExtensionMatcher {
        private:
            std::string m_basename;
        public:
            FileBasenameMatcher(const std::string& basename, const std::string& extension);
            FileBasenameMatcher(const std::string& basename, const std::vector<std::string>& extensions);
            bool operator()(const Path& path, bool directory) const;
        };

        class FileNameMatcher {
        private:
            std::string m_pattern;
        public:
            explicit FileNameMatcher(const std::string& pattern);
            bool operator()(const Path& path, bool directory) const;
        };

        class ExecutableFileMatcher {
        public:
            bool operator()(const Path& path, bool directory) const;
        };
    }
}

#endif /* FileMatcher_h */

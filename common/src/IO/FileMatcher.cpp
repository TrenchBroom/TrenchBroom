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

#include "FileMatcher.h"

#include "IO/Path.h"
#include "IO/PathQt.h"

#include <QFileInfo>

namespace TrenchBroom {
    namespace IO {
        FileTypeMatcher::FileTypeMatcher(const bool files, const bool directories) :
        m_files(files),
        m_directories(directories) {}

        bool FileTypeMatcher::operator()(const Path& path, const bool directory) const {
            if (m_files && !directory)
                return true;
            if (m_directories && directory)
                return true;
            return false;
        }

        FileExtensionMatcher::FileExtensionMatcher(const String& extension) :
        m_extensions(1, extension) {}

        FileExtensionMatcher::FileExtensionMatcher(const StringList& extensions) :
        m_extensions(extensions) {}

        bool FileExtensionMatcher::operator()(const Path& path, const bool directory) const {
            return !directory && path.hasExtension(m_extensions, false);
        }

        FileBasenameMatcher::FileBasenameMatcher(const String& basename, const String& extension) :
        FileExtensionMatcher(extension),
        m_basename(basename) {}

        FileBasenameMatcher::FileBasenameMatcher(const String& basename, const StringList& extensions) :
        FileExtensionMatcher(extensions),
        m_basename(basename) {}

        bool FileBasenameMatcher::operator()(const Path& path, bool directory) const {
            return StringUtils::caseInsensitiveEqual(path.basename(), m_basename) &&
                   FileExtensionMatcher::operator()(path, directory);
        }

        FileNameMatcher::FileNameMatcher(const String& pattern) :
        m_pattern(pattern) {}

        bool FileNameMatcher::operator()(const Path& path, const bool directory) const {
            const String filename = path.lastComponent().asString();
            return StringUtils::caseInsensitiveMatchesPattern(filename, m_pattern);
        }

        bool ExecutableFileMatcher::operator()(const Path& path, const bool directory) const {
#ifdef __APPLE__
            if (directory && StringUtils::caseInsensitiveEqual(path.extension(), "app"))
                return true;
#endif
            return QFileInfo(pathAsQString(path)).isExecutable();
        }
    }
}

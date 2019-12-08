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

#ifndef TrenchBroom_Path
#define TrenchBroom_Path

#include "StringType.h"
#include "StringList.h"

#include <iosfwd>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class Path {
        public:
            using List = std::vector<Path>;
            static const List EmptyList;
            static const Path EmptyPath;
            static String separator();

            struct ToString {
                String m_separator;
                ToString(const String i_separator = separator()) :
                m_separator(i_separator) {}

                String operator()(const Path& path) const {
                    return path.asString(m_separator);
                }
            };

            template <typename StringLess>
            class Less {
            private:
                StringLess m_less;
            public:
                bool operator()(const Path& lhs, const Path& rhs) const {
                    return std::lexicographical_compare(std::begin(lhs.m_components), std::end(lhs.m_components),
                                                        std::begin(rhs.m_components), std::end(rhs.m_components), m_less);
                }
            };
        private:
            static const String& separators();

            StringList m_components;
            bool m_absolute;

            Path(bool absolute, const StringList& components);
        public:
            explicit Path(const String& path = "");

            Path operator+(const Path& rhs) const;
            int compare(const Path& rhs, bool caseSensitive = true) const;
            bool operator==(const Path& rhs) const;
            bool operator!= (const Path& rhs) const;
            bool operator<(const Path& rhs) const;
            bool operator>(const Path& rhs) const;

            String asString(const String& sep = separator()) const;
            static StringList asStrings(const Path::List& paths, const String& sep = separator());
            static List asPaths(const StringList& strs);

            size_t length() const;
            bool isEmpty() const;
            Path firstComponent() const;
            Path deleteFirstComponent() const;
            Path lastComponent() const;
            Path deleteLastComponent() const;
            Path prefix(size_t count) const;
            Path suffix(size_t count) const;
            Path subPath(size_t index, size_t count) const;
            const StringList& components() const;

            String filename() const;
            String basename() const;
            String extension() const;

            bool hasPrefix(const Path& prefix, bool caseSensitive) const;
            bool hasFilename(const String& filename, bool caseSensitive) const;
            bool hasFilename(const StringList& filenames, bool caseSensitive) const;
            bool hasBasename(const String& basename, bool caseSensitive) const;
            bool hasBasename(const StringList& basenames, bool caseSensitive) const;
            bool hasExtension(const String& extension, bool caseSensitive) const;
            bool hasExtension(const StringList& extensions, bool caseSensitive) const;

            Path deleteExtension() const;
            Path addExtension(const String& extension) const;
            Path replaceExtension(const String& extension) const;

            Path replaceBasename(const String& basename) const;

            bool isAbsolute() const;
            bool canMakeRelative(const Path& absolutePath) const;
            Path makeAbsolute(const Path& relativePath) const;
            Path makeRelative(const Path& absolutePath) const;
            Path makeCanonical() const;
            Path makeLowerCase() const;

            static List makeAbsoluteAndCanonical(const List& paths, const Path& relativePath);
        private:
            static bool hasDriveSpec(const StringList& components);
            static bool hasDriveSpec(const String& component);
            StringList resolvePath(bool absolute, const StringList& components) const;
        };

        std::ostream& operator<<(std::ostream& stream, const Path& path);
    }
}

#endif /* defined(TrenchBroom_Path) */

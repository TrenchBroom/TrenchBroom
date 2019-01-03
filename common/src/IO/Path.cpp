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

#include "Path.h"

#include "Exceptions.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace IO {
        const Path::List Path::EmptyList = Path::List(0);

        char Path::separator() {
#ifdef _WIN32
            static const char sep = '\\';
#else
            static const char sep = '/';
#endif
            return sep;
        }
        
        const String& Path::separators() {
            static const String sep("/\\");
            return sep;
        }

        Path::Path(bool absolute, const StringList& components) :
        m_components(components),
        m_absolute(absolute) {}

        Path::Path(const String& path) {
            const auto trimmed = StringUtils::trim(path);
            m_components = StringUtils::split(trimmed, separators());
#ifdef _WIN32
            m_absolute = (hasDriveSpec(m_components) ||
                          (!trimmed.empty() && trimmed[0] == '/') ||
                          (!trimmed.empty() && trimmed[0] == '\\'));
#else
            m_absolute = !trimmed.empty() && trimmed[0] == separator();
#endif
        }

        Path Path::operator+(const Path& rhs) const {
            if (rhs.isAbsolute()) {
                throw PathException("Cannot concatenate absolute path");
            }
            auto components = m_components;
            components.insert(std::end(components), std::begin(rhs.m_components), std::end(rhs.m_components));
            return Path(m_absolute, components);
        }

        int Path::compare(const Path& rhs, const bool caseSensitive) const {
            if (!isAbsolute() && rhs.isAbsolute()) {
                return -1;
            } else if (isAbsolute() && !rhs.isAbsolute()) {
                return 1;
            }

            const auto& rcomps = rhs.m_components;
            
            size_t i = 0;
            const auto max = std::min(m_components.size(), rcomps.size());
            while (i < max) {
                const auto& mcomp = m_components[i];
                const auto& rcomp = rcomps[i];
                const auto result = caseSensitive ? StringUtils::caseSensitiveCompare(mcomp, rcomp) : StringUtils::caseInsensitiveCompare(mcomp, rcomp);
                if (result < 0) {
                    return -1;
                } else if (result > 0) {
                    return 1;
                }
                ++i;
            }
            if (m_components.size() < rcomps.size()) {
                return -1;
            } else if (m_components.size() > rcomps.size()) {
                return 1;
            } else {
                return 0;
            }
        }

        bool Path::operator==(const Path& rhs) const {
            return compare(rhs) == 0;
        }

        bool Path::operator!= (const Path& rhs) const {
            return !(*this == rhs);
        }

        bool Path::operator<(const Path& rhs) const {
            return compare(rhs) < 0;
        }

        bool Path::operator>(const Path& rhs) const {
            return compare(rhs) > 0;
        }

        String Path::asString(const char separator) const {
            if (m_absolute) {
#ifdef _WIN32
                if (hasDriveSpec(m_components)) {
                    return StringUtils::join(m_components, separator);
                } else {
                    return separator + StringUtils::join(m_components, separator);
                }
#else
                return separator + StringUtils::join(m_components, separator);
#endif
            }
            return StringUtils::join(m_components, separator);
        }

        QString Path::asQString(const char separator) const {
            return QString::fromStdString(asString(separator));
        }

        String Path::asString(const String& separator) const {
            if (m_absolute) {
#ifdef _WIN32
                if (hasDriveSpec(m_components)) {
                    return StringUtils::join(m_components, separator);
                } else {
                    return separator + StringUtils::join(m_components, separator);
                }
#else
                return separator + StringUtils::join(m_components, separator);
#endif
            }
            return StringUtils::join(m_components, separator);
        }



        StringList Path::asStrings(const Path::List& paths, const char separator) {
            auto result = StringList();
            result.reserve(paths.size());
            std::transform(std::begin(paths), std::end(paths), std::back_inserter(result),
                           [separator](const Path& path) { return path.asString(separator); });
            return result;
        }

        Path::List Path::asPaths(const StringList& strs) {
            auto result = Path::List();
            result.reserve(strs.size());
            std::transform(std::begin(strs), std::end(strs), std::back_inserter(result),
                           [](const String& str) { return Path(str); });
            return result;
        }

        size_t Path::length() const {
            return m_components.size();
        }

        bool Path::isEmpty() const {
            return !m_absolute && m_components.empty();
        }

        Path Path::firstComponent() const {
            if (isEmpty()) {
                throw PathException("Cannot return first component of empty path");
            }

            if (!m_absolute) {
                return Path(m_components.front());
            }

#ifdef _WIN32
            if (hasDriveSpec(m_components)) {
                return Path(m_components.front());
            }

            return Path("\\");
#else
            return Path("/");
#endif
        }

        Path Path::deleteFirstComponent() const {
            if (isEmpty()) {
                throw PathException("Cannot delete first component of empty path");
            }
            if (!m_absolute) {
                auto components = StringList();
                components.reserve(m_components.size() - 1);
                components.insert(std::begin(components), std::begin(m_components) + 1, std::end(m_components));
                return Path(false, components);
            }
#ifdef _WIN32
            if (!m_components.empty() && hasDriveSpec(m_components[0])) {
                StringList components;
                components.reserve(m_components.size() - 1);
                components.insert(std::begin(components), std::begin(m_components) + 1, std::end(m_components));
                return Path(false, components);
            }
            return Path(false, m_components);
#else
            return Path(false, m_components);
#endif
        }

        Path Path::lastComponent() const {
            if (isEmpty())
                throw PathException("Cannot return last component of empty path");
            if (!m_components.empty()) {
                return Path(m_components.back());
            } else {
                return Path("");
            }
        }

        Path Path::deleteLastComponent() const {
            if (isEmpty()) {
                throw PathException("Cannot delete last component of empty path");
            }

            if (!m_components.empty()) {
                auto components = StringList();
                components.reserve(m_components.size() - 1);
                components.insert(std::begin(components), std::begin(m_components), std::end(m_components) - 1);
                return Path(m_absolute, components);
            } else {
                return Path(m_absolute, m_components);
            }
        }

        Path Path::prefix(const size_t count) const {
            return subPath(0, count);
        }
        
        Path Path::suffix(const size_t count) const {
            return subPath(m_components.size() - count, count);
        }
        
        Path Path::subPath(const size_t index, const size_t count) const {
            if (index + count > m_components.size()) {
                throw PathException("Sub path out of bounds");
            }

            if (count == 0) {
                return Path("");
            }

            auto begin = std::begin(m_components);
            std::advance(begin, static_cast<StringList::const_iterator::difference_type>(index));
            auto end = begin;
            std::advance(end, static_cast<StringList::const_iterator::difference_type>(count));
            auto newComponents = StringList(count);
            std::copy(begin, end, std::begin(newComponents));
            return Path(m_absolute && index == 0, newComponents);
        }

        String Path::filename() const {
            if (isEmpty()) {
                throw PathException("Cannot get filename of empty path");
            }

            if (m_components.empty()) {
                return "";
            } else {
                return m_components.back();
            }
        }
        
        String Path::basename() const {
            if (isEmpty()) {
                throw PathException("Cannot get basename of empty path");
            }

            const auto filename = this->filename();
            const auto dotIndex = filename.rfind('.');
            if (dotIndex == String::npos) {
                return filename;
            } else {
                return filename.substr(0, dotIndex);
            }
        }

        String Path::extension() const {
            if (isEmpty()) {
                throw PathException("Cannot get extension of empty path");
            }

            const auto filename = this->filename();
            const auto dotIndex = filename.rfind('.');
            if (dotIndex == String::npos) {
                return "";
            } else {
                return filename.substr(dotIndex + 1);
            }
        }

        bool Path::hasPrefix(const Path& prefix, bool caseSensitive) const {
            if (prefix.length() > length()) {
                return false;
            }

            return this->prefix(prefix.length()).compare(prefix, caseSensitive) == 0;
        }

        bool Path::hasFilename(const String& filename, const bool caseSensitive) const {
            if (caseSensitive) {
                return StringUtils::caseSensitiveEqual(filename, this->filename());
            } else {
                return StringUtils::caseInsensitiveEqual(filename, this->filename());
            }
        }

        bool Path::hasFilename(const StringList& filenames, const bool caseSensitive) const {
            for (const auto& filename : filenames) {
                if (hasFilename(filename, caseSensitive)) {
                    return true;
                }
            }
            return false;
        }

        bool Path::hasBasename(const String& basename, const bool caseSensitive) const {
            if (caseSensitive) {
                return StringUtils::caseSensitiveEqual(basename, this->basename());
            } else {
                return StringUtils::caseInsensitiveEqual(basename, this->basename());
            }
        }

        bool Path::hasBasename(const StringList& basenames, const bool caseSensitive) const {
            for (const auto& basename : basenames) {
                if (hasBasename(basename, caseSensitive)) {
                    return true;
                }
            }
            return false;
        }

        bool Path::hasExtension(const String& extension, const bool caseSensitive) const {
            if (caseSensitive) {
                return StringUtils::caseSensitiveEqual(extension, this->extension());
            } else {
                return StringUtils::caseInsensitiveEqual(extension, this->extension());
            }
        }

        bool Path::hasExtension(const StringList& extensions, const bool caseSensitive) const {
            for (const auto& extension : extensions) {
                if (hasExtension(extension, caseSensitive)) {
                    return true;
                }
            }
            return false;
        }

        Path Path::deleteExtension() const {
            return deleteLastComponent() + Path(basename());
        }

        Path Path::addExtension(const String& extension) const {
            if (isEmpty()) {
                throw PathException("Cannot add extension to empty path");
            }

            auto components = m_components;
            if (components.empty()
#ifdef _WIN32
                || hasDriveSpec(m_components.back())
#endif
                ) {
                components.push_back("." + extension);
            } else {
                components.back() += "." + extension;
            }
            return Path(m_absolute, components);
        }

        Path Path::replaceExtension(const String& extension) const {
            return deleteExtension().addExtension(extension);
        }

        bool Path::isAbsolute() const {
            return m_absolute;
        }

        bool Path::canMakeRelative(const Path& absolutePath) const {
            return (!isEmpty() && !absolutePath.isEmpty() &&
                    isAbsolute() && absolutePath.isAbsolute()
#ifdef _WIN32
                    && 
                    !m_components.empty() && !absolutePath.m_components.empty()
                    &&
                    m_components[0] == absolutePath.m_components[0]
#endif
            );
        }

        Path Path::makeAbsolute(const Path& relativePath) const {
            if (!isAbsolute()) {
                throw PathException("Cannot make absolute path from relative path");
            }

            if (relativePath.isAbsolute()) {
                throw PathException("Cannot make absolute path with absolute sub path");
            }

            return *this + relativePath;
        }

        Path Path::makeRelative(const Path& absolutePath) const {
            if (isEmpty()) {
                throw PathException("Cannot make relative path from an empty reference path");
            }

            if (absolutePath.isEmpty()) {
                throw PathException("Cannot make relative path with empty sub path");
            }

            if (!isAbsolute()) {
                throw PathException("Cannot make relative path from relative reference path");
            }

            if (!absolutePath.isAbsolute()) {
                throw PathException("Cannot make relative path with relative sub path");
            }

#ifdef _WIN32
            if (m_components.empty()) {
                throw PathException("Cannot make relative path from an reference path with no drive spec");
            }
            if (absolutePath.m_components.empty()) {
                throw PathException("Cannot make relative path with sub path with no drive spec");
            }
            if (m_components[0] != absolutePath.m_components[0]) {
                throw PathException("Cannot make relative path if reference path has different drive spec");
            }
#endif
            
            const auto myResolved = resolvePath(true, m_components);
            const auto theirResolved = resolvePath(true, absolutePath.m_components);
            
            // cross off all common prefixes
            size_t p = 0;
            while (p < std::min(myResolved.size(), theirResolved.size())) {
                if (myResolved[p] != theirResolved[p]) {
                    break;
                }
                ++p;
            }

            auto components = StringList();
            for (size_t i = p; i < myResolved.size(); ++i) {
                components.push_back("..");
            }
            for (size_t i = p; i < theirResolved.size(); ++i) {
                components.push_back(theirResolved[i]);
            }

            return Path(false, components);
        }

        Path Path::makeCanonical() const {
            return Path(m_absolute, resolvePath(m_absolute, m_components));
        }

        Path Path::makeLowerCase() const {
            auto lcComponents = StringList();
            lcComponents.reserve(m_components.size());
            std::transform(std::begin(m_components), std::end(m_components), std::back_inserter(lcComponents),
                           [](const String& component) { return StringUtils::toLower(component); });
            return Path(m_absolute, lcComponents);
        }

        Path::List Path::makeAbsoluteAndCanonical(const List& paths, const Path& relativePath) {
            auto result = List();
            result.reserve(paths.size());
            std::transform(std::begin(paths), std::end(paths), std::back_inserter(result),
                           [&relativePath](const Path& path) { return path.makeAbsolute(relativePath).makeCanonical(); });
            return result;
        }

        bool Path::hasDriveSpec(const StringList& components) {
#ifdef _WIN32
            if (components.empty()) {
                return false;
            } else {
                return hasDriveSpec(components[0]);
            }
#else
            return false;
#endif
        }

        bool Path::hasDriveSpec(const String& component) {
#ifdef _WIN32
            if (component.size() <= 1) {
                return false;
            } else {
                return component[1] == ':';
            }
#else
            return false;
#endif
        }

        StringList Path::resolvePath(const bool absolute, const StringList& components) const {
            auto resolved = StringList();
            for (const auto& comp : components) {
                if (comp == ".") {
                    continue;
                }
                if (comp == "..") {
                    if (resolved.empty()) {
                        throw PathException("Cannot resolve path");
                    }

#ifdef _WIN32
                    if (absolute && hasDriveSpec(resolved[0]) && resolved.size() < 2) {
                        throw PathException("Cannot resolve path");
                    }
#endif
                    resolved.pop_back();
                    continue;
                }
                resolved.push_back(comp);
            }
            return resolved;
        }

        std::ostream& operator<<(std::ostream& stream, const Path& path) {
            stream << path.asString();
            return stream;
        }
    }
}

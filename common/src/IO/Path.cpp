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

#include "Path.h"

#include "Exceptions.h"

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace IO {
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
            const String trimmed = StringUtils::trim(path);
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
            if (rhs.isAbsolute())
                throw PathException("Cannot concatenate absolute path");
            StringList components = m_components;
            components.insert(components.end(), rhs.m_components.begin(), rhs.m_components.end());
            return Path(m_absolute, components);
        }

        int Path::compare(const Path& rhs) const {
            if (!isAbsolute() && rhs.isAbsolute())
                return -1;
            if (isAbsolute() && !rhs.isAbsolute())
                return 1;
            
            const StringList& rcomps = rhs.m_components;
            
            size_t i = 0;
            const size_t max = std::min(m_components.size(), rcomps.size());
            while (i < max) {
                const String& mcomp = m_components[i];
                const String& rcomp = rcomps[i];
                const int result = mcomp.compare(rcomp);
                if (result < 0)
                    return -1;
                if (result > 0)
                    return 1;
                ++i;
            }
            if (m_components.size() < rcomps.size())
                return -1;
            if (m_components.size() > rcomps.size())
                return 1;
            return 0;
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
                if (hasDriveSpec(m_components))
                    return StringUtils::join(m_components, separator);
                else
                    return separator + StringUtils::join(m_components, separator);
#else
                return separator + StringUtils::join(m_components, separator);
#endif
            }
            return StringUtils::join(m_components, separator);
        }

        String Path::asString(const String& separator) const {
            if (m_absolute) {
#ifdef _WIN32
                if (hasDriveSpec(m_components))
                    return StringUtils::join(m_components, separator);
                else
                    return separator + StringUtils::join(m_components, separator);
#else
                return separator + StringUtils::join(m_components, separator);
#endif
            }
            return StringUtils::join(m_components, separator);
        }

        StringList Path::asStrings(const Path::List& paths, const char separator) {
            StringList result;
            Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it)
                result.push_back(it->asString(separator));
            return result;
        }

        size_t Path::length() const {
            return m_components.size();
        }

        bool Path::isEmpty() const {
            return !m_absolute && m_components.empty();
        }

        Path Path::firstComponent() const {
            if (isEmpty())
                throw PathException("Cannot return first component of empty path");
            if (!m_absolute)
                return Path(m_components.front());
#ifdef _WIN32
            if (hasDriveSpec(m_components))
                return Path(m_components.front());
            return Path("\\");
#else
            return Path("/");
#endif
        }

        Path Path::deleteFirstComponent() const {
            if (isEmpty())
                throw PathException("Cannot delete first component of empty path");
            if (!m_absolute) {
                StringList components;
                components.reserve(m_components.size() - 1);
                components.insert(components.begin(), m_components.begin() + 1, m_components.end());
                return Path(false, components);
            }
#ifdef _WIN32
            if (!m_components.empty() && hasDriveSpec(m_components[0])) {
                StringList components;
                components.reserve(m_components.size() - 1);
                components.insert(components.begin(), m_components.begin() + 1, m_components.end());
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
            return Path(m_components.back());
        }

        Path Path::deleteLastComponent() const {
            if (isEmpty())
                throw PathException("Cannot delete last component of empty path");
            StringList components;
            components.reserve(m_components.size() - 1);
            components.insert(components.begin(), m_components.begin(), m_components.end() - 1);
            return Path(m_absolute, components);
        }

        Path Path::prefix(const size_t count) const {
            return subPath(0, count);
        }
        
        Path Path::suffix(const size_t count) const {
            return subPath(m_components.size() - count, count);
        }
        
        Path Path::subPath(const size_t index, const size_t count) const {
            if (isEmpty())
                throw PathException("Cannot get sub path of empty path");
            if (index + count > m_components.size())
                throw PathException("Sub path out of bounds");
            if (count == 0)
                return Path("");
            
            StringList::const_iterator begin = m_components.begin();
            std::advance(begin, index);
            StringList::const_iterator end = begin;
            std::advance(end, count);
            StringList newComponents(count);
            std::copy(begin, end, newComponents.begin());
            return Path(m_absolute && index == 0, newComponents);
        }

        const String Path::extension() const {
            if (isEmpty())
                throw PathException("Cannot get extension of empty path");
            const String& lastComponent = m_components.back();
            const size_t dotIndex = lastComponent.rfind('.');
            if (dotIndex == String::npos)
                return "";
            return lastComponent.substr(dotIndex + 1);
        }
        
        Path Path::deleteExtension() const {
            if (isEmpty())
                throw PathException("Cannot get extension of empty path");
            const String& lastComponent = m_components.back();
            const size_t dotIndex = lastComponent.rfind('.');
            if (dotIndex == String::npos)
                return *this;
            return deleteLastComponent() + Path(lastComponent.substr(0, dotIndex));
        }

        Path Path::addExtension(const String& extension) const {
            if (isEmpty())
                throw PathException("Cannot get extension of empty path");
            StringList components = m_components;
            String& lastComponent = components.back();
            lastComponent += "." + extension;
            return Path(m_absolute, components);
        }

        bool Path::isAbsolute() const {
            return m_absolute;
        }

        bool Path::canMakeRelative(const Path& absolutePath) const {
            return (!isEmpty() && !absolutePath.isEmpty() &&
                    isAbsolute() && absolutePath.isAbsolute()
#ifdef _WIN32
                    &&
                    m_components[0] == absolutePath.m_components[0]
#endif
            );
        }

        Path Path::makeAbsolute(const Path& relativePath) const {
            if (!isAbsolute())
                throw PathException("Cannot make absolute path from relative path");
            if (relativePath.isAbsolute())
                throw PathException("Cannot make absolute path with absolute sub path");
            return *this + relativePath;
        }

        Path Path::makeRelative(const Path& absolutePath) const {
            if (isEmpty())
                throw PathException("Cannot make relative path from an empty reference path");
            if (absolutePath.isEmpty())
                throw PathException("Cannot make relative path with empty sub path");
            if (!isAbsolute())
                throw PathException("Cannot make relative path from relative reference path");
            if (!absolutePath.isAbsolute())
                throw PathException("Cannot make relative path with relative sub path");

#ifdef _WIN32
            if (m_components[0] != absolutePath.m_components[0])
                throw PathException("Cannot make relative path if reference path has different drive spec");
#endif
            
            const StringList myResolved = resolvePath(true, m_components);
            const StringList theirResolved = resolvePath(true, absolutePath.m_components);
            
            // cross off all common prefixes
            size_t p = 0;
            while (p < std::min(myResolved.size(), theirResolved.size())) {
                if (myResolved[p] != theirResolved[p])
                    break;
                ++p;
            }

            StringList components;
            for (size_t i = p; i < myResolved.size(); ++i)
                components.push_back("..");
            for (size_t i = p; i < theirResolved.size(); ++i)
                components.push_back(theirResolved[i]);
            
            return Path(false, components);
        }

        Path Path::makeCanonical() const {
            return Path(m_absolute, resolvePath(m_absolute, m_components));
        }

        Path Path::makeLowerCase() const {
            StringList lcComponents;
            StringList::const_iterator it, end;
            for (it = m_components.begin(), end = m_components.end(); it != end; ++it) {
                const String& component = *it;
                lcComponents.push_back(StringUtils::toLower(component));
            }
            return Path(m_absolute, lcComponents);
        }

        Path::List Path::makeAbsoluteAndCanonical(const List& paths, const String& relativePath) {
            List result;
            List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const Path& path = *it;
                const Path absPath = path.makeAbsolute(relativePath);
                const Path canPath = path.makeCanonical();
                result.push_back(canPath);
            }
            return result;
        }

        bool Path::hasDriveSpec(const StringList& components) {
#ifdef _WIN32
            if (components.empty())
                return false;
            return hasDriveSpec(components[0]);
#else
            return false;
#endif
        }

        bool Path::hasDriveSpec(const String& component) {
#ifdef _WIN32
            if (component.size() <= 1)
                return false;
            return component[1] == ':';
#else
            return false;
#endif
        }

        StringList Path::resolvePath(const bool absolute, const StringList& components) const {
            StringList::const_iterator it, end;
            StringList resolved;
            for (it = components.begin(), end = components.end(); it != end; ++it) {
                const String& comp = *it;
                if (comp == ".")
                    continue;
                if (comp == "..") {
#ifdef _WIN32
                    if (resolved.empty())
                        throw PathException("Cannot resolve path");
                    if (absolute && hasDriveSpec(resolved[0]) && resolved.size() < 2)
                            throw PathException("Cannot resolve path");
#else
                    if (resolved.empty())
                        throw PathException("Cannot resolve path");
#endif
                    resolved.pop_back();
                    continue;
                }
                resolved.push_back(comp);
            }
            return resolved;
        }
    }
}

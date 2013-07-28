/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Path.h"

#include "Exceptions.h"

#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace IO {
#ifdef _WIN32
        const char Path::Separator = '\\';
#else
        const char Path::Separator = '/';
#endif
        const String Path::Separators("/\\");

        Path::Path(bool absolute, const StringList& components) :
        m_components(components),
        m_absolute(absolute) {}

        Path::Path(const String& path) {
            const String trimmed = StringUtils::trim(path);
            m_components = StringUtils::split(trimmed, Separators);
#ifdef _WIN32
            m_absolute = trimmed.size() > 1 && trimmed[1] == ':';
#else
            m_absolute = !trimmed.empty() && trimmed[0] == Separator;
#endif
        }

        Path Path::operator+ (const Path& rhs) const {
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

        bool Path::operator== (const Path& rhs) const {
            return compare(rhs) == 0;
        }

        bool Path::operator< (const Path& rhs) const {
            return compare(rhs) < 0;
        }

        bool Path::operator> (const Path& rhs) const {
            return compare(rhs) > 0;
        }
        
        String Path::asString() const {
            if (m_absolute)
#ifdef _WIN32
                return StringUtils::join(m_components, Separator);
#else
                return Separator + StringUtils::join(m_components, Separator);
#endif
            return StringUtils::join(m_components, Separator);
        }

        Path::operator String() const {
            return asString();
        }

        bool Path::isEmpty() const {
            return !m_absolute && m_components.empty();
        }

        String Path::firstComponent() const {
            if (isEmpty())
                throw PathException("Cannot return last component of empty path");
            return m_components.front();
        }

        Path Path::deleteFirstComponent() const {
            if (isEmpty())
                throw PathException("Cannot delete last component of empty path");
            StringList components;
            components.reserve(m_components.size() - 1);
            components.insert(components.begin(), m_components.begin() + 1, m_components.end());
            return Path(false, components);
        }

        String Path::lastComponent() const {
            if (isEmpty())
                throw PathException("Cannot return last component of empty path");
            return m_components.back();
        }

        Path Path::deleteLastComponent() const {
            if (isEmpty())
                throw PathException("Cannot delete last component of empty path");
            StringList components;
            components.reserve(m_components.size() - 1);
            components.insert(components.begin(), m_components.begin(), m_components.end() - 1);
            return Path(m_absolute, components);
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

        Path Path::makeAbsolute(const Path& relativePath) const {
            if (!isAbsolute())
                throw PathException("Cannot make absolute path from relative path");
            if (relativePath.isAbsolute())
                throw PathException("Cannot make absolute path with absolute sub path");
            return *this + relativePath;
        }

        Path Path::makeRelative(const Path& absolutePath) const {
            if (!isAbsolute())
                throw PathException("Cannot make relative path from relative reference path");
            if (!absolutePath.isAbsolute())
                throw PathException("Cannot make relative path with relative sub path");

            const StringList resolved = resolvePath(m_absolute, m_components);
            StringList prefix;
            bool prefixEqual = false;
            
            const StringList& theirComponents = absolutePath.m_components;
            StringList::const_iterator theirIt = theirComponents.begin();
            const StringList::const_iterator theirEnd = theirComponents.end();
            
            while (theirIt != theirEnd && !prefixEqual) {
                const String& comp = *theirIt;
                if (comp != ".") {
                    if (comp == "..") {
                        if (prefix.empty())
                            throw PathException("Cannot resolve sub path");
                        prefix.pop_back();
                    } else {
                        prefix.push_back(comp);
                    }
                    prefixEqual = resolved.size() == prefix.size() && std::equal(resolved.begin(), resolved.end(), prefix.begin());
                }
                ++theirIt;
            }
            
            if (!prefixEqual)
                throw PathException("Sub path is not relative to reference path");
            

            StringList components;
            components.insert(components.end(), theirIt, theirEnd);
            return Path(false, components);
        }

        Path Path::makeCanonical() const {
            return Path(m_absolute, resolvePath(m_absolute, m_components));
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

        StringList Path::resolvePath(const bool absolute, const StringList& components) const {
            StringList::const_iterator it, end;
            StringList resolved;
            for (it = m_components.begin(), end = m_components.end(); it != end; ++it) {
                const String& comp = *it;
                if (comp == ".")
                    continue;
                if (comp == "..") {
#ifdef _WIN32
                    if (absolute && resolved.size() < 2 || resolved.empty())
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

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

namespace TrenchBroom {
    namespace IO {
#ifdef WIN32
        const char Path::Separator = '\\';
#else
        const char Path::Separator = '/';
#endif
        const String Path::Separators("/\\");

        Path::Path(bool absolute, const StringList& components) :
        m_absolute(absolute),
        m_components(components) {}

        Path::Path(const String& path) {
            const String trimmed = StringUtils::trim(path);
            m_components = StringUtils::split(trimmed, Separators);
#ifdef WIN32
            m_absolute = trimmed.size() > 1 && trimmed[1] == ':';
#else
            m_absolute = !trimmed.empty() && trimmed[0] == Separator;
#endif
        }

        const Path Path::operator+ (const Path& rhs) const {
            if (rhs.isAbsolute())
                throw PathException("Cannot concatenate absolute path");
            StringList components = m_components;
            components.insert(components.end(), rhs.m_components.begin(), rhs.m_components.end());
            return Path(m_absolute, components);
        }

        const bool Path::operator== (const Path& rhs) const {
            return (m_absolute == rhs.m_absolute &&
                    m_components.size() == rhs.m_components.size() &&
                    std::equal(m_components.begin(), m_components.end(), rhs.m_components.begin()));
        }

        String Path::asString() const {
            if (m_absolute)
#ifdef WIN32
                return StringUtils::join(m_components, Separator);
#else
                return Separator + StringUtils::join(m_components, Separator);
#endif
            return StringUtils::join(m_components, Separator);
        }

        Path::operator String() const {
            return asString();
        }

        String Path::lastComponent() const {
            if (m_components.empty())
                throw PathException("Cannot return last component of empty path");
            return m_components.back();
        }

        Path Path::deleteLastComponent() const {
            if (m_components.empty())
                throw PathException("Cannot delete last component of empty path");
            StringList components = m_components;
            components.pop_back();
            return Path(m_absolute, components);
        }

        const String Path::extension() const {
            if (m_components.empty())
                throw PathException("Cannot get extension of empty path");
            const String& lastComponent = m_components.back();
            const size_t dotIndex = lastComponent.rfind('.');
            if (dotIndex == String::npos)
                return "";
            return lastComponent.substr(dotIndex + 1);
        }

        Path Path::addExtension(const String& extension) const {
            if (m_components.empty())
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

        StringList Path::resolvePath(const bool absolute, const StringList& components) const {
            StringList::const_iterator it, end;
            StringList resolved;
            for (it = m_components.begin(), end = m_components.end(); it != end; ++it) {
                const String& comp = *it;
                if (comp == ".")
                    continue;
                if (comp == "..") {
#ifdef WIN32
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

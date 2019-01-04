/*
 Copyright (C) 2019-2019 Eric Wasylishen

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

#include "ModifiableMenuItem.h"

namespace TrenchBroom {
    namespace View {
        ModifiableMenuItem::ModifiableMenuItem() = default;

        ModifiableMenuItem::ModifiableMenuItem(const IO::Path& configPath, const QString& defaultKey)
        : m_configPath(configPath),
        m_defaultKey(defaultKey) {}

        const TrenchBroom::IO::Path& ModifiableMenuItem::configPath() const {
            return m_configPath;
        }

        const QString& ModifiableMenuItem::defaultKey() const {
            return m_defaultKey;
        }
    }
}

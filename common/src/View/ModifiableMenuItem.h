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

#ifndef TRENCHBROOM_MODIFIABLEMENUITEM_H
#define TRENCHBROOM_MODIFIABLEMENUITEM_H

#include <QMetaType>

#include "IO/Path.h"

namespace TrenchBroom {
    namespace View {
        class ModifiableMenuItem {
        private:
            IO::Path m_configPath;
            QString m_defaultKey;
        public:
            ModifiableMenuItem();
            ModifiableMenuItem(const IO::Path& configPath, const QString& defaultKey);

            const IO::Path& configPath() const;
            const QString &defaultKey() const;
        };
    }
}

Q_DECLARE_METATYPE(TrenchBroom::View::ModifiableMenuItem)

#endif //TRENCHBROOM_MODIFIABLEMENUITEM_H

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

#ifndef TRENCHBROOM_TAGMANAGER_H
#define TRENCHBROOM_TAGMANAGER_H

#include <set>

namespace TrenchBroom {
    namespace Model {
        class Tag;
        class SmartTag;
        class Taggable;

        class TagManager {
        private:
            std::set<Tag> m_tags;
            std::set<SmartTag> m_smartTags;
        public:
            void updateTags(Taggable& taggable) const;
        };
    }
}

#endif //TRENCHBROOM_TAGMANAGER_H

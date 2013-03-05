/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_Map_h
#define TrenchBroom_Map_h

#include <map>

namespace TrenchBroom {
    namespace Utility {
        template <typename Key, typename Value>
        inline void insertOrReplace(std::map<Key, Value>& map, const Key& key, Value& value) {
            typedef std::map<Key, Value> Map;
            typename Map::key_compare compare = map.key_comp();
            typename Map::iterator insertPos = map.lower_bound(key);
            if (insertPos == map.end() || compare(key, insertPos->first)) {
                // the two keys are not equal (key is less than insertPos' key), so we must insert the value
                map.insert(insertPos, std::pair<Key, Value>(key, value));
            } else {
                // the two keys are equal because insertPos either points to the pair with the same key or the one
                // right after the position where the given pair would be inserted
                insertPos->second = value;
            }
        }

        template <typename Key, typename Value>
        inline void insertOrReplace(std::map<Key, Value*>& map, const Key& key, Value* value) {
            typedef std::map<Key, Value*> Map;
            typename Map::key_compare compare = map.key_comp();
            typename Map::iterator insertPos = map.lower_bound(key);
            if (insertPos == map.end() || compare(key, insertPos->first)) {
                // the two keys are not equal (key is less than insertPos' key), so we must insert the value
                map.insert(insertPos, std::pair<Key, Value*>(key, value));
            } else {
                // the two keys are equal because insertPos either points to the pair with the same key or the one
                // right after the position where the given pair would be inserted
                delete insertPos->second;
                insertPos->second = value;
            }
        }
        
        template <typename Key, typename Value>
        inline void deleteAll(std::map<Key, Value*>& map) {
            typedef std::map<Key, Value*> Map;
            typename Map::iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it)
                delete it->second;
            map.clear();
        }
    }
}

#endif

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

#ifndef TrenchBroom_CollectionUtils_h
#define TrenchBroom_CollectionUtils_h

#include <algorithm>
#include <map>
#include <vector>

namespace VectorUtils {
    template <typename T>
    struct Deleter {
    public:
        void operator()(const T* ptr) const {
            delete ptr;
        }
    };

    template <typename T>
    inline void eraseAndDelete(std::vector<T*>& vec, typename std::vector<T*>::iterator first, typename std::vector<T*>::iterator last) {
        std::for_each(first, last, Deleter<T>());
        vec.erase(first, last);
    }
    
    template <typename T>
    inline void eraseAndDelete(std::vector<T*>& vec, typename std::vector<T*>::iterator first) {
        eraseAndDelete(vec, first, vec.end());
    }
    
    template <typename T>
    inline void clearAndDelete(std::vector<T*>& vec) {
        std::for_each(vec.begin(), vec.end(), Deleter<T>());
        vec.clear();
    }
    
    template <typename T>
    inline void remove(std::vector<T*>& vec, const T* item) {
        vec.erase(std::remove(vec.begin(), vec.end(), item), vec.end());
    }

    template <typename T>
    inline void removeAndDelete(std::vector<T*>& vec, const T* item) {
        remove(vec, item);
        delete item;
    }
}

namespace MapUtils {
    template <typename K, typename V>
    struct Deleter {
    public:
        void operator()(const std::pair<K, V*> entry) const {
            delete entry.second;
        }
    };

    template <typename K, typename V>
    inline void insertOrReplace(std::map<K, V>& map, const K& key, V& value) {
        typedef std::map<K, V> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            map.insert(insertPos, std::pair<K, V>(key, value));
        } else {
            // the two keys are equal because insertPos either points to the pair with the same key or the one
            // right after the position where the given pair would be inserted
            insertPos->second = value;
        }
    }

    template <typename K, typename V>
    inline void insertOrReplace(std::map<K, V*>& map, const K& key, V* value) {
        typedef std::map<K, V*> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            map.insert(insertPos, std::pair<K, V*>(key, value));
        } else {
            // the two keys are equal because insertPos either points to the pair with the same key or the one
            // right after the position where the given pair would be inserted
            delete insertPos->second;
            insertPos->second = value;
        }
    }

    template <typename K, typename V>
    inline void clearAndDelete(std::map<K, V*>& map) {
        std::for_each(map.begin(), map.end(), Deleter<K,V>());
        map.clear();
    }
}

#endif

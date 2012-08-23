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

#ifndef TrenchBroom_InstancePool_h
#define TrenchBroom_InstancePool_h

#include <cassert>
#include <list>

namespace TrenchBroom {
    namespace Utility {
		template <typename T>
		class Pool {
		private:
			size_t m_maxSize;
			std::list<T*> m_items;
		public:
			Pool(size_t maxSize = 25) : m_maxSize(maxSize) {}
			~Pool() {
				while (!m_items.empty()) {
					free(m_items.back());
					m_items.pop_back();
				}
			}
            
			inline bool empty() {
				return m_items.empty();
			}
            
			inline size_t size() {
				return m_items.size();
			}
            
			inline bool push(T* item) {
				if (m_items.size() == m_maxSize)
					return false;
				m_items.push_back(item);
				return true;
			}
            
			inline T* pop() {
				assert(!m_items.empty());
				T* item = m_items.front();
				m_items.pop_front();
				return item;
			}
		};
    }
}

#endif

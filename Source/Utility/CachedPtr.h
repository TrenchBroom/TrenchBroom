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

#ifndef TrenchBroom_CachedPtr_h
#define TrenchBroom_CachedPtr_h

#include <cassert>

namespace TrenchBroom {
    namespace Utility {
        template <class Element>
        class CachedPtr {
        public:
            class Cache {
            public:
                virtual ~Cache() {}
                virtual void deleteElement(Element* element) = 0;
            };
        private:
            struct Counter {
                Cache* cache;
                Element* ptr;
                unsigned count;
                
                Counter(Cache* i_cache, Element* i_ptr = NULL, unsigned i_count = 1) :
                cache(i_cache),
                ptr(i_ptr),
                count(i_count) {
                    assert(cache != NULL);
                }
            }* m_counter;
            
            void acquire(Counter* c) throw() { // increment the count
                m_counter = c;
                if (c)
                    ++c->count;
            }
            
            void release() { // decrement the count, delete if it is 0
                if (m_counter) {
                    m_counter->count--;
                    if (m_counter->count == 1)
                        m_counter->cache->deleteElement(m_counter->ptr);
                    else if (m_counter->count == 0)
                        delete m_counter;
                    m_counter = NULL;
                }
            }

        public:
            explicit CachedPtr(Cache* cache, Element* p = NULL) // allocate a new counter
            : m_counter(NULL) {
                if (p) {
                    assert(cache != NULL);
                    m_counter = new Counter(cache, p);
                }
            }
            
            ~CachedPtr() {
                release();
            }
            
            CachedPtr(const CachedPtr& r) throw() {
                acquire(r.m_counter);
            }
            
            CachedPtr& operator=(const CachedPtr& r) {
                if (this != &r) {
                    release();
                    acquire(r.m_counter);
                }
                return *this;
            }
            
            Element& operator*() const throw() {
                return *m_counter->ptr;
            }
            
            Element* operator->() const throw() {
                return m_counter->ptr;
            }
            
            Element* get() const throw() {
                return m_counter ? m_counter->ptr : 0;
            }
            
            bool unique() const throw() {
                return (m_counter ? m_counter->count == 1 : true);
            }
        };
    }
}

#endif

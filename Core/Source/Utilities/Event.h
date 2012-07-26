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

#ifndef TrenchBroom_Observer_h
#define TrenchBroom_Observer_h

#include <cstdlib>
#include <string>
#include <vector>
#include <map>

namespace TrenchBroom {
    template <typename Arg1>
    class Event  {
    private:
        class ListenerBase {
        public:
            virtual ~ListenerBase() {}
            virtual void operator()(Arg1 arg1) = 0;
            virtual bool equals(const void* other) const = 0;
            bool operator==(const ListenerBase& other) const {
                return other.equals(this);
            }
        };

		std::vector<ListenerBase*> m_ptrs;
        bool m_modified;
        size_t m_modifiedIndex;
    public:
        template <typename Class>
        class Listener : public ListenerBase {
            typedef void (Class::*Func)(Arg1);
        private:
            Class* m_target; // Pointer to the object we are delegating to.
            Func   m_function; // Address of the function on the delegate object.
        public:
            Listener(Class* target, Func function) : m_target(target), m_function(function) {}
            void operator()(Arg1 arg1) {
                return (m_target->*m_function)(arg1);
            }
            
            bool equals(const void* other) const {
                return *this == *static_cast<const Listener<Class>*>(other);
            }
            
            bool operator==(const Listener<Class>& other) const {
                return this == &other || (m_target == other.m_target && m_function == other.m_function);
            }
        };
        
        Event& operator+=(ListenerBase* ptr) {
            m_modified = true;
            m_modifiedIndex = m_ptrs.size();
            m_ptrs.push_back(ptr);
            return *this;
        }
        
        Event& operator-=(ListenerBase* ptr) {
            typename std::vector<ListenerBase*>::iterator it;
            unsigned int i = 0;
            for (it = m_ptrs.begin(); it != m_ptrs.end(); ++it, i++) {
                if (**it == *ptr) {
                    delete *it;
                    m_ptrs.erase(it);
                    m_modified = true;
                    m_modifiedIndex = i;
                    break;
                }
            }
            delete ptr;
            return *this;
        }
        
        void operator()(Arg1 arg1) {
            m_modified = false;
            m_modifiedIndex = 0;
            unsigned int i = 0;
            while (i < m_ptrs.size()) {
                (*m_ptrs[i])(arg1);
                if (!m_modified || m_modifiedIndex > i)
                    i++;
                m_modified = false;
                m_modifiedIndex = 0;
            }
        }

		Event() {
            m_modified = false;
            m_modifiedIndex = 0;
		}

        ~Event() {
            while (!m_ptrs.empty()) delete m_ptrs.back(), m_ptrs.pop_back();
        }
    };
}

#endif

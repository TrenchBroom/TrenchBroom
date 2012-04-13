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

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace TrenchBroom {
    namespace Model {
        template <typename Arg1>
        class Event  {
        private:
            class Base {
            public:
                virtual ~Base() {}
                virtual void operator()(Arg1) = 0;
            };
            vector<Base*> m_ptrs;
        public: 
            template <typename Class>
            class T : public Base {
                typedef void (Class::*Func)(Arg1);
            private:
                Class* m_target; // Pointer to the object we are delegating to.
                Func   m_function; // Address of the function on the delegate object.
            public:
                T(Class* target, Func function) : m_target(target), m_function(function) {}
                virtual void operator()(Arg1 arg1) {
                    return (m_target->*m_function)(arg1);
                }
                virtual bool operator==(const T& other) const {
                    return this == &other || (m_target == other.m_target && m_function == other.m_function);
                }
            };
            class S : public Base {
                typedef void (*Func)(Arg1);
            private:
                Func m_function; 
                
            public:
                S(Func function) : m_function(function) {}
                virtual void operator()(Arg1 arg1) {
                    return m_function(arg1);
                }
                virtual bool operator==(const S& other) const {
                    return this == &other || m_function == other.m_function;
                }
            };
            
            Event& operator+=(Base* ptr) {
                m_ptrs.push_back(ptr);
                return *this;
            }
            
            Event& operator-=(Base* ptr) {
                typename vector<Base*>::iterator it = find(m_ptrs.begin(), m_ptrs.end(), ptr);
                if (it != m_ptrs.end()) {
                    delete *it;
                    m_ptrs.erase(it);
                }
                delete ptr;
                return *this;
            }
            
            void operator()(Arg1 arg1) {
                typename std::vector<Base*>::iterator end = m_ptrs.end();
                for (typename std::vector<Base*>::iterator i = m_ptrs.begin(); i != end; ++i) {
                    (*(*i))(arg1); 
                }
            }
        };
    }
}

#endif

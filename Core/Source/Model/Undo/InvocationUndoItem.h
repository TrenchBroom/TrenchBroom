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

#ifndef TrenchBroom_InvocationUndoItem_h
#define TrenchBroom_InvocationUndoItem_h

#include "Model/Undo/UndoItem.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class Functor {
        public:
            virtual ~Functor() {};
            virtual void operator()() = 0;
        };
        
        template <class Target>
        class NoArgFunctor : public Functor {
        public:
            typedef void (Target::*Func)();
        private:
            Target* m_target;
            Func m_function;
        public:
            NoArgFunctor(Target* target, Func function) : m_target(target), m_function(function) {}
            void operator()() {
                (m_target->*m_function)();
            }
        };
        
        template <class Target, typename Arg1>
        class OneArgFunctor : public Functor {
        public:
            typedef void (Target::*Func)(Arg1);
        private:
            Target* m_target;
            Func m_function;
            Arg1 m_arg1;
        public:
            OneArgFunctor(Target* target, Func function, Arg1 arg1) : m_target(target), m_function(function), m_arg1(arg1) {}
            void operator()() {
                (m_target->*m_function)(m_arg1);
            }
        };
        
        template <class Target, typename Arg1, typename Arg2>
        class TwoArgFunctor : public Functor {
        public:
            typedef void (Target::*Func)(Arg1, Arg2);
        private:
            Target* m_target;
            Func m_function;
            Arg1 m_arg1;
            Arg2 m_arg2;
        public:
            TwoArgFunctor(Target* target, Func function, Arg1 arg1, Arg2 arg2) : m_target(target), m_function(function), m_arg1(arg1), m_arg2(arg2) {}
            void operator()() {
                (m_target->*m_function)(m_arg1, m_arg2);
            }
        };
        
        template <class Target, typename Arg1, typename Arg2, typename Arg3>
        class ThreeArgFunctor : public Functor {
        public:
            typedef void (Target::*Func)(Arg1, Arg2, Arg3);
        private:
            Target* m_target;
            Func m_function;
            Arg1 m_arg1;
            Arg2 m_arg2;
            Arg3 m_arg3;
        public:
            ThreeArgFunctor(Target* target, Func function, Arg1 arg1, Arg2 arg2, Arg3 arg3) : m_target(target), m_function(function), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3) {}
            void operator()() {
                (m_target->*m_function)(m_arg1, m_arg2, m_arg3);
            }
        };
        
        class FunctorUndoItem : public SelectionUndoItem {
        protected:
            Functor* m_functor;
        public:
            FunctorUndoItem(Map& map, Functor* functor);
            ~FunctorUndoItem();
            void performUndo();
        };
    }
}

#endif

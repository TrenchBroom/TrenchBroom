/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Functor
#define TrenchBroom_Functor

#include <cassert>
#include <iostream> // for NULL

namespace TrenchBroom {
    // ====== Function pointer with 0 arguments ======
    template <typename R>
    class FuncBase0 {
    public:
        virtual ~FuncBase0() {}
        virtual R operator()() const = 0;
    };
    
    template <typename R>
    class FuncPtr0 : public FuncBase0<R> {
    public:
        typedef R (*F)();
    private:
        F m_function;
    public:
        FuncPtr0(F function) :
        m_function(function) {}
        
        R operator()() const {
            return (*m_function)();
        }
    };
    
#ifdef _MSC_VER
    template <typename R>
    class StdCallFuncPtr0 : public FuncBase0<R> {
    public:
        typedef R (__stdcall *F)();
    private:
        F m_function;
    public:
        StdCallFuncPtr0(F function) :
        m_function(function) {}
        
        R operator()() const {
            return (*m_function)();
        }
    };
#endif
    
    template <class C, typename R>
    class MemFuncPtr0 : public FuncBase0<R> {
    public:
        typedef R (C::*F)();
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr0(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()() const {
            return (m_receiver->*m_function)();
        }
    };
    
    template <typename R>
    class Func0 {
    private:
        FuncBase0<R>* m_func;
    public:
        Func0() :
        m_func(NULL) {}
        
        ~Func0() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr0<R>::F func) {
            delete m_func;
            m_func = new FuncPtr0<R>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr0<R>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr0<R>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr0<C,R>::F func) {
            delete m_func;
            m_func = new MemFuncPtr0<C,R>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()() {
            assert(m_func != NULL);
            return (*m_func)();
        }
    };

    // ====== Function pointer with 1 argument ======
    template <typename R, typename A1>
    class FuncBase1 {
    public:
        virtual ~FuncBase1() {}
        virtual R operator()(A1 a1) const = 0;
    };
    
    template <typename R, typename A1>
    class FuncPtr1 : public FuncBase1<R,A1> {
    public:
        typedef R (*F)(A1 a1);
    private:
        F m_function;
    public:
        FuncPtr1(F function) :
        m_function(function) {}
        
        R operator()(A1 a1) const {
            return (*m_function)(a1);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1>
    class StdCallFuncPtr1 : public FuncBase1<R,A1> {
    public:
        typedef R (__stdcall *F)(A1 a1);
    private:
        F m_function;
    public:
        StdCallFuncPtr1(F function) :
        m_function(function) {}
        
        R operator()(A1 a1) const {
            return (*m_function)(a1);
        }
    };
#endif
    
    template <class C, typename R, typename A1>
    class MemFuncPtr1 : public FuncBase1<R,A1> {
    public:
        typedef R (C::*F)(A1 a1);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr1(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1) const {
            return (m_receiver->*m_function)(a1);
        }
    };
    
    template <typename R, typename A1>
    class Func1 {
    private:
        FuncBase1<R,A1>* m_func;
    public:
        Func1() :
        m_func(NULL) {}
        
        ~Func1() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr1<R,A1>::F func) {
            delete m_func;
            m_func = new FuncPtr1<R,A1>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr1<R,A1>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr1<R,A1>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr1<C,R,A1>::F func) {
            delete m_func;
            m_func = new MemFuncPtr1<C,R,A1>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1) {
            assert(m_func != NULL);
            return (*m_func)(a1);
        }
    };
    
    // ====== Function pointer with 2 arguments ======
    template <typename R, typename A1, typename A2>
    class FuncBase2 {
    public:
        virtual ~FuncBase2() {}
        virtual R operator()(A1 a1, A2 a2) const = 0;
    };
    
    template <typename R, typename A1, typename A2>
    class FuncPtr2 : public FuncBase2<R,A1,A2> {
    public:
        typedef R (*F)(A1 a1, A2 a2);
    private:
        F m_function;
    public:
        FuncPtr2(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2) const {
            return (*m_function)(a1, a2);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2>
    class StdCallFuncPtr2 : public FuncBase2<R,A1,A2> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2);
    private:
        F m_function;
    public:
        StdCallFuncPtr2(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2) const {
            return (*m_function)(a1, a2);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2>
    class MemFuncPtr2 : public FuncBase2<R,A1,A2> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr2(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2) const {
            return (m_receiver->*m_function)(a1, a2);
        }
    };
    
    template <typename R, typename A1, typename A2>
    class Func2 {
    private:
        FuncBase2<R,A1,A2>* m_func;
    public:
        Func2() :
        m_func(NULL) {}
        
        ~Func2() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr2<R,A1,A2>::F func) {
            delete m_func;
            m_func = new FuncPtr2<R,A1,A2>(func);
        }

#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr2<R,A1,A2>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr2<R,A1,A2>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr2<C,R,A1,A2>::F func) {
            delete m_func;
            m_func = new MemFuncPtr2<C,R,A1,A2>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2);
        }
    };
    
    // ====== Function pointer with 3 arguments ======
    template <typename R, typename A1, typename A2, typename A3>
    class FuncBase3 {
    public:
        virtual ~FuncBase3() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3>
    class FuncPtr3 : public FuncBase3<R,A1,A2,A3> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3);
    private:
        F m_function;
    public:
        FuncPtr3(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3) const {
            return (*m_function)(a1, a2, a3);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3>
    class StdCallFuncPtr3 : public FuncBase3<R,A1,A2,A3> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3);
    private:
        F m_function;
    public:
        StdCallFuncPtr3(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3) const {
            return (*m_function)(a1, a2, a3);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3>
    class MemFuncPtr3 : public FuncBase3<R,A1,A2,A3> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr3(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3) const {
            return (m_receiver->*m_function)(a1, a2, a3);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3>
    class Func3 {
    private:
        FuncBase3<R,A1,A2,A3>* m_func;
    public:
        Func3() :
        m_func(NULL) {}
        
        ~Func3() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr3<R,A1,A2,A3>::F func) {
            delete m_func;
            m_func = new FuncPtr3<R,A1,A2,A3>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr3<R,A1,A2,A3>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr3<R,A1,A2,A3>(func);
        }
#endif

        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr3<C,R,A1,A2,A3>::F func) {
            delete m_func;
            m_func = new MemFuncPtr3<C,R,A1,A2,A3>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3);
        }
    };
    
    // ====== Function pointer with 4 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4>
    class FuncBase4 {
    public:
        virtual ~FuncBase4() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4>
    class FuncPtr4 : public FuncBase4<R,A1,A2,A3,A4> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4);
    private:
        F m_function;
    public:
        FuncPtr4(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const {
            return (*m_function)(a1, a2, a3, a4);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4>
    class StdCallFuncPtr4 : public FuncBase4<R,A1,A2,A3,A4> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4);
    private:
        F m_function;
    public:
        StdCallFuncPtr4(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const {
            return (*m_function)(a1, a2, a3, a4);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4>
    class MemFuncPtr4 : public FuncBase4<R,A1,A2,A3,A4> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr4(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4>
    class Func4 {
    private:
        FuncBase4<R,A1,A2,A3,A4>* m_func;
    public:
        Func4() :
        m_func(NULL) {}
        
        ~Func4() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr4<R,A1,A2,A3,A4>::F func) {
            delete m_func;
            m_func = new FuncPtr4<R,A1,A2,A3,A4>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr4<R,A1,A2,A3,A4>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr4<R,A1,A2,A3,A4>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr4<C,R,A1,A2,A3,A4>::F func) {
            delete m_func;
            m_func = new MemFuncPtr4<C,R,A1,A2,A3,A4>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4);
        }
    };
    
    // ====== Function pointer with 5 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    class FuncBase5 {
    public:
        virtual ~FuncBase5() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    class FuncPtr5 : public FuncBase5<R,A1,A2,A3,A4,A5> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
    private:
        F m_function;
    public:
        FuncPtr5(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const {
            return (*m_function)(a1, a2, a3, a4, a5);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    class StdCallFuncPtr5 : public FuncBase5<R,A1,A2,A3,A4,A5> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
    private:
        F m_function;
    public:
        StdCallFuncPtr5(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const {
            return (*m_function)(a1, a2, a3, a4, a5);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    class MemFuncPtr5 : public FuncBase5<R,A1,A2,A3,A4,A5> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr5(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4, a5);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
    class Func5 {
    private:
        FuncBase5<R,A1,A2,A3,A4,A5>* m_func;
    public:
        Func5() :
        m_func(NULL) {}
        
        ~Func5() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr5<R,A1,A2,A3,A4,A5>::F func) {
            delete m_func;
            m_func = new FuncPtr5<R,A1,A2,A3,A4,A5>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr5<R,A1,A2,A3,A4,A5>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr5<R,A1,A2,A3,A4,A5>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr5<C,R,A1,A2,A3,A4,A5>::F func) {
            delete m_func;
            m_func = new MemFuncPtr5<C,R,A1,A2,A3,A4,A5>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4, a5);
        }
    };
    
    // ====== Function pointer with 6 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    class FuncBase6 {
    public:
        virtual ~FuncBase6() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    class FuncPtr6 : public FuncBase6<R,A1,A2,A3,A4,A5,A6> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
    private:
        F m_function;
    public:
        FuncPtr6(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    class StdCallFuncPtr6 : public FuncBase6<R,A1,A2,A3,A4,A5,A6> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
    private:
        F m_function;
    public:
        StdCallFuncPtr6(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    class MemFuncPtr6 : public FuncBase6<R,A1,A2,A3,A4,A5,A6> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr6(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4, a5, a6);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    class Func6 {
    private:
        FuncBase6<R,A1,A2,A3,A4,A5,A6>* m_func;
    public:
        Func6() :
        m_func(NULL) {}
        
        ~Func6() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr6<R,A1,A2,A3,A4,A5,A6>::F func) {
            delete m_func;
            m_func = new FuncPtr6<R,A1,A2,A3,A4,A5,A6>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr6<R,A1,A2,A3,A4,A5,A6>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr6<R,A1,A2,A3,A4,A5,A6>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr6<C,R,A1,A2,A3,A4,A5,A6>::F func) {
            delete m_func;
            m_func = new MemFuncPtr6<C,R,A1,A2,A3,A4,A5,A6>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4, a5, a6);
        }
    };
    
    // ====== Function pointer with 7 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    class FuncBase7 {
    public:
        virtual ~FuncBase7() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    class FuncPtr7 : public FuncBase7<R,A1,A2,A3,A4,A5,A6,A7> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
    private:
        F m_function;
    public:
        FuncPtr7(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    class StdCallFuncPtr7 : public FuncBase7<R,A1,A2,A3,A4,A5,A6,A7> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
    private:
        F m_function;
    public:
        StdCallFuncPtr7(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    class MemFuncPtr7 : public FuncBase7<R,A1,A2,A3,A4,A5,A6,A7> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr7(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4, a5, a6, a7);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    class Func7 {
    private:
        FuncBase7<R,A1,A2,A3,A4,A5,A6,A7>* m_func;
    public:
        Func7() :
        m_func(NULL) {}
        
        ~Func7() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr7<R,A1,A2,A3,A4,A5,A6,A7>::F func) {
            delete m_func;
            m_func = new FuncPtr7<R,A1,A2,A3,A4,A5,A6,A7>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr7<R,A1,A2,A3,A4,A5,A6,A7>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr7<R,A1,A2,A3,A4,A5,A6,A7>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr7<C,R,A1,A2,A3,A4,A5,A6,A7>::F func) {
            delete m_func;
            m_func = new MemFuncPtr7<C,R,A1,A2,A3,A4,A5,A6,A7>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4, a5, a6, a7);
        }
    };
    
    // ====== Function pointer with 8 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    class FuncBase8 {
    public:
        virtual ~FuncBase8() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    class FuncPtr8 : public FuncBase8<R,A1,A2,A3,A4,A5,A6,A7,A8> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
    private:
        F m_function;
    public:
        FuncPtr8(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7, a8);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    class StdCallFuncPtr8 : public FuncBase8<R,A1,A2,A3,A4,A5,A6,A7,A8> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
    private:
        F m_function;
    public:
        StdCallFuncPtr8(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7, a8);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    class MemFuncPtr8 : public FuncBase8<R,A1,A2,A3,A4,A5,A6,A7,A8> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr8(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4, a5, a6, a7, a8);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    class Func8 {
    private:
        FuncBase8<R,A1,A2,A3,A4,A5,A6,A7,A8>* m_func;
    public:
        Func8() :
        m_func(NULL) {}
        
        ~Func8() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr8<R,A1,A2,A3,A4,A5,A6,A7,A8>::F func) {
            delete m_func;
            m_func = new FuncPtr8<R,A1,A2,A3,A4,A5,A6,A7,A8>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr8<R,A1,A2,A3,A4,A5,A6,A7,A8>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr8<R,A1,A2,A3,A4,A5,A6,A7,A8>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr8<C,R,A1,A2,A3,A4,A5,A6,A7,A8>::F func) {
            delete m_func;
            m_func = new MemFuncPtr8<C,R,A1,A2,A3,A4,A5,A6,A7,A8>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4, a5, a6, a7, a8);
        }
    };
    
    // ====== Function pointer with 9 arguments ======
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    class FuncBase9 {
    public:
        virtual ~FuncBase9() {}
        virtual R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const = 0;
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    class FuncPtr9 : public FuncBase9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9> {
    public:
        typedef R (*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
    private:
        F m_function;
    public:
        FuncPtr9(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        }
    };
    
#ifdef _MSC_VER
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    class StdCallFuncPtr9 : public FuncBase9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9> {
    public:
        typedef R (__stdcall *F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
    private:
        F m_function;
    public:
        StdCallFuncPtr9(F function) :
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const {
            return (*m_function)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        }
    };
#endif
    
    template <class C, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    class MemFuncPtr9 : public FuncBase9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9> {
    public:
        typedef R (C::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
    private:
        C* m_receiver;
        F m_function;
    public:
        MemFuncPtr9(C* receiver, F function) :
        m_receiver(receiver),
        m_function(function) {}
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const {
            return (m_receiver->*m_function)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        }
    };
    
    template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    class Func9 {
    private:
        FuncBase9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>* m_func;
    public:
        Func9() :
        m_func(NULL) {}
        
        ~Func9() {
            delete m_func;
            m_func = NULL;
        }
        
        void bindFunc(typename FuncPtr9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>::F func) {
            delete m_func;
            m_func = new FuncPtr9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(func);
        }
        
#ifdef _MSC_VER
        void bindFunc(typename StdCallFuncPtr9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>::F func) {
            delete m_func;
            m_func = new StdCallFuncPtr9<R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(func);
        }
#endif
        
        template <class C>
        void bindMemFunc(C* receiver, typename MemFuncPtr9<C,R,A1,A2,A3,A4,A5,A6,A7,A8,A9>::F func) {
            delete m_func;
            m_func = new MemFuncPtr9<C,R,A1,A2,A3,A4,A5,A6,A7,A8,A9>(receiver, func);
        }
        
        void unbindFunc() {
            delete m_func;
            m_func = 0;
        }
        
        R operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) {
            assert(m_func != NULL);
            return (*m_func)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        }
    };
}

#endif /* defined(TrenchBroom_Functor) */

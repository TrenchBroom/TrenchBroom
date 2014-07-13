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

#ifndef TrenchBroom_Notifier_h
#define TrenchBroom_Notifier_h

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "SetBool.h"

#include <algorithm>
#include <cassert>
#include <list>

namespace TrenchBroom {
    class Notifier0 {
    private:
        class Observer {
        public:
            typedef std::list<Observer*> List;
        public:
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()() = 0;
            
            bool operator==(const Observer& rhs) const {
                if (receiver() != rhs.receiver())
                    return false;
                return compareFunctions(rhs);
            }
        private:
            virtual bool compareFunctions(const Observer& rhs) const = 0;
        };
        
        template <typename R>
        class CObserver : public Observer {
        private:
            typedef void (R::*F)();
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()() {
                (m_receiver->*m_function)();
            }

            void* receiver() const {
                return static_cast<void*>(m_receiver);
            }
            
            F function() const {
                return m_function;
            }

            bool compareFunctions(const Observer& rhs) const {
                const CObserver<R>& rhsR = static_cast<const CObserver<R>&>(rhs);
                return m_function == rhsR.function();
            }
        };
        
        struct CompareObservers {
        private:
            const Observer& m_lhs;
        public:
            CompareObservers(const Observer& lhs) : m_lhs(lhs) {}
            bool operator()(const Observer* rhs) const {
                return m_lhs == (*rhs);
            }
        };
    private:
        Observer::List m_observers;
        bool m_notifying;
    public:
        Notifier0() :
        m_notifying(false) {}
        
        ~Notifier0() {
            ListUtils::clearAndDelete(m_observers);
        }
        
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)()) {
            assert(!m_notifying);

            CObserver<R>* observer = new CObserver<R>(receiver, function);
            Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(*observer));
            if (it != m_observers.end()) {
                delete observer;
                return false;
            }
            
            m_observers.push_back(observer);
            return true;
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)()) {
            assert(!m_notifying);
            
            CObserver<R> test(receiver, function);
            
            Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(test));
            if (it == m_observers.end())
                return false;
            
            delete *it;
            m_observers.erase(it);
            return true;
        }
        
        bool addObserver(Notifier0& notifier) {
            return addObserver(&notifier, &Notifier0::operator());
        }
        
        bool removeObserver(Notifier0& notifier) {
            return removeObserver(&notifier, &Notifier0::operator());
        }
        
        void notify() {
            const SetBool notifying(m_notifying);
            
            Observer::List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                Observer& observer = **it;
                observer();
            }
        }
        
        void operator()() {
            notify();
        }
    };
    
    template <typename A1>
    class Notifier1 {
    private:
        class Observer {
        public:
            typedef std::list<Observer*> List;
        public:
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()(A1 a1) = 0;

            bool operator==(const Observer& rhs) const {
                if (receiver() != rhs.receiver())
                    return false;
                return compareFunctions(rhs);
            }
        private:
            virtual bool compareFunctions(const Observer& rhs) const = 0;
        };
        
        template <typename R>
        class CObserver : public Observer {
        private:
            typedef void (R::*F)(A1 a1);
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()(A1 a1) {
                (m_receiver->*m_function)(a1);
            }
            
            void* receiver() const {
                return static_cast<void*>(m_receiver);
            }
            
            F function() const {
                return m_function;
            }
            
            bool compareFunctions(const Observer& rhs) const {
                const CObserver<R>& rhsR = static_cast<const CObserver<R>&>(rhs);
                return m_function == rhsR.function();
            }
        };
        
        struct CompareObservers {
        private:
            const Observer& m_lhs;
        public:
            CompareObservers(const Observer& lhs) : m_lhs(lhs) {}
            bool operator()(const Observer* rhs) const {
                return m_lhs == (*rhs);
            }
        };
    private:
        typename Observer::List m_observers;
        bool m_notifying;
    public:
        Notifier1() :
        m_notifying(false) {}
        
        ~Notifier1() {
            ListUtils::clearAndDelete(m_observers);
        }
        
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1)) {
            assert(!m_notifying);
            
            CObserver<R>* observer = new CObserver<R>(receiver, function);
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(*observer));
            if (it != m_observers.end()) {
                delete observer;
                return false;
            }
            
            m_observers.push_back(observer);
            return true;
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1)) {
            assert(!m_notifying);
            
            CObserver<R> test(receiver, function);
            
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(test));
            if (it == m_observers.end())
                return false;
            
            delete *it;
            m_observers.erase(it);
            return true;
        }
        
        bool addObserver(Notifier1& notifier) {
            return addObserver(&notifier, &Notifier1::operator());
        }
        
        bool removeObserver(Notifier1& notifier) {
            return removeObserver(&notifier, &Notifier1::operator());
        }

        template <typename A>
        void notify(A a1) {
            const SetBool notifying(m_notifying);
            
            typename Observer::List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                Observer& observer = **it;
                observer(a1);
            }
        }

        template <typename A>
        void operator()(A a1) {
            notify(a1);
        }
        
        template <typename I>
        void notify(I it, I end) {
            while (it != end) {
                notify(*it);
                ++it;
            }
        }
        
        template <typename I>
        void operator()(I it, I end) {
            while (it != end) {
                notify(*it);
                ++it;
            }
        }
    };
    
    template <typename A1, typename A2>
    class Notifier2 {
    private:
        class Observer {
        public:
            typedef std::list<Observer*> List;
        public:
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()(A1 a1, A2 a2) = 0;
            
            bool operator==(const Observer& rhs) const {
                if (receiver() != rhs.receiver())
                    return false;
                return compareFunctions(rhs);
            }
        private:
            virtual bool compareFunctions(const Observer& rhs) const = 0;
        };
        
        template <typename R>
        class CObserver : public Observer {
        private:
            typedef void (R::*F)(A1 a1, A2 a2);
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()(A1 a1, A2 a2) {
                (m_receiver->*m_function)(a1, a2);
            }
            
            void* receiver() const {
                return static_cast<void*>(m_receiver);
            }
            
            F function() const {
                return m_function;
            }
            
            bool compareFunctions(const Observer& rhs) const {
                const CObserver<R>& rhsR = static_cast<const CObserver<R>&>(rhs);
                return m_function == rhsR.function();
            }
        };
        
        struct CompareObservers {
        private:
            const Observer& m_lhs;
        public:
            CompareObservers(const Observer& lhs) : m_lhs(lhs) {}
            bool operator()(const Observer* rhs) const {
                return m_lhs == (*rhs);
            }
        };
    private:
        typename Observer::List m_observers;
        bool m_notifying;
    public:
        Notifier2() :
        m_notifying(false) {}
        
        ~Notifier2() {
            ListUtils::clearAndDelete(m_observers);
        }
        
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2)) {
            assert(!m_notifying);
            
            CObserver<R>* observer = new CObserver<R>(receiver, function);
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(*observer));
            if (it != m_observers.end()) {
                delete observer;
                return false;
            }
            
            m_observers.push_back(observer);
            return true;
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2)) {
            assert(!m_notifying);
            
            CObserver<R> test(receiver, function);
            
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(test));
            if (it == m_observers.end())
                return false;
            
            delete *it;
            m_observers.erase(it);
            return true;
        }
        
        bool addObserver(Notifier2& notifier) {
            return addObserver(&notifier, &Notifier2::operator());
        }
        
        bool removeObserver(Notifier2& notifier) {
            return removeObserver(&notifier, &Notifier2::operator());
        }

        void notify(A1 a1, A2 a2) {
            const SetBool notifying(m_notifying);
            
            typename Observer::List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                Observer& observer = **it;
                observer(a1, a2);
            }
        }
        
        void operator()(A1 a1, A2 a2) {
            notify(a1, a2);
        }
        
        template <typename I>
        void notify(I it, I end, A2 a2) {
            while (it != end) {
                notify(*it, a2);
                ++it;
            }
        }
        
        template <typename I>
        void operator()(I it, I end, A2 a2) {
            while (it != end) {
                notify(*it, a2);
                ++it;
            }
        }
    };

    template <typename A1, typename A2, typename A3>
    class Notifier3 {
    private:
        class Observer {
        public:
            typedef std::list<Observer*> List;
        public:
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()(A1 a1, A2 a2, A3 a3) = 0;
            
            bool operator==(const Observer& rhs) const {
                if (receiver() != rhs.receiver())
                    return false;
                return compareFunctions(rhs);
            }
        private:
            virtual bool compareFunctions(const Observer& rhs) const = 0;
        };
        
        template <typename R>
        class CObserver : public Observer {
        private:
            typedef void (R::*F)(A1 a1, A2 a2, A3 a3);
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()(A1 a1, A2 a2, A3 a3) {
                (m_receiver->*m_function)(a1, a2, a3);
            }
            
            void* receiver() const {
                return static_cast<void*>(m_receiver);
            }
            
            F function() const {
                return m_function;
            }
            
            bool compareFunctions(const Observer& rhs) const {
                const CObserver<R>& rhsR = static_cast<const CObserver<R>&>(rhs);
                return m_function == rhsR.function();
            }
        };
        
        struct CompareObservers {
        private:
            const Observer& m_lhs;
        public:
            CompareObservers(const Observer& lhs) : m_lhs(lhs) {}
            bool operator()(const Observer* rhs) const {
                return m_lhs == (*rhs);
            }
        };
    private:
        typename Observer::List m_observers;
        bool m_notifying;
    public:
        Notifier3() :
        m_notifying(false) {}
        
        ~Notifier3() {
            ListUtils::clearAndDelete(m_observers);
        }
        
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2, A3)) {
            assert(!m_notifying);
            
            CObserver<R>* observer = new CObserver<R>(receiver, function);
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(*observer));
            if (it != m_observers.end()) {
                delete observer;
                return false;
            }
            
            m_observers.push_back(observer);
            return true;
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2, A3)) {
            assert(!m_notifying);
            
            CObserver<R> test(receiver, function);
            
            typename Observer::List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(test));
            if (it == m_observers.end())
                return false;
            
            delete *it;
            m_observers.erase(it);
            return true;
        }
        
        bool addObserver(Notifier3& notifier) {
            return addObserver(&notifier, &Notifier3::operator());
        }
        
        bool removeObserver(Notifier3& notifier) {
            return removeObserver(&notifier, &Notifier3::operator());
        }

        void notify(A1 a1, A2 a2, A3 a3) {
            const SetBool notifying(m_notifying);
            
            typename Observer::List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                Observer& observer = **it;
                observer(a1, a2, a3);
            }
        }
        
        void operator()(A1 a1, A2 a2, A3 a3) {
            notify(a1, a2, a3);
        }
        
        template <typename I>
        void notify(I it, I end, A2 a2, A3 a3) {
            while (it != end) {
                notify(*it, a2, a3);
                ++it;
            }
        }
        
        template <typename I>
        void operator()(I it, I end, A2 a2, A3 a3) {
            while (it != end) {
                notify(*it, a2, a3);
                ++it;
            }
        }
    };
}

#endif

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
#include "SetAny.h"

#include <algorithm>
#include <cassert>
#include <list>

namespace TrenchBroom {
    template <typename O>
    class NotifierState {
    private:
        struct CompareObservers {
        private:
            const O* m_lhs;
        public:
            CompareObservers(const O* lhs) : m_lhs(lhs) {}
            bool operator()(const O* rhs) const {
                return (*m_lhs) == (*rhs);
            }
        };

        typedef std::list<O*> List;
        
        List m_observers;
        List m_toAdd;
        List m_toRemove;
        
        bool m_notifying;
    public:
        NotifierState() : m_notifying(false) {}
        ~NotifierState() {
            ListUtils::clearAndDelete(m_observers);
            ListUtils::clearAndDelete(m_toAdd);
            ListUtils::clearAndDelete(m_toRemove);
        }
        
        bool addObserver(O* observer) {
            if (!m_observers.empty()) {
                typename List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(observer));
                if (it != m_observers.end()) {
                    delete observer;
                    return false;
                }
            }

            if (m_notifying)
                m_toAdd.push_back(observer);
            else
                m_observers.push_back(observer);
            return true;
        }
        
        bool removeObserver(O* observer) {
            typename List::iterator it = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(observer));
            if (it == m_observers.end()) {
                delete observer;
                return false;
            } else {
                (*it)->setSkip();
            }
            
            if (m_notifying) {
                m_toRemove.push_back(observer);
            } else {
                delete observer;
                delete *it;
                m_observers.erase(it);
            }
            
            return true;
        }
        
        void notify() {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer();
            }
            
            removePending();
            addPending();
        }
        
        template <typename A1>
        void notify(A1 a1) {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer(a1);
            }
            
            removePending();
            addPending();
        }
        
        template <typename A1, typename A2>
        void notify(A1 a1, A2 a2) {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer(a1, a2);
            }
            
            removePending();
            addPending();
        }
        
        template <typename A1, typename A2, typename A3>
        void notify(A1 a1, A2 a2, A3 a3) {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer(a1, a2, a3);
            }
            
            removePending();
            addPending();
        }
        
        template <typename A1, typename A2, typename A3, typename A4>
        void notify(A1 a1, A2 a2, A3 a3, A4 a4) {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer(a1, a2, a3, a4);
            }
            
            removePending();
            addPending();
        }
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        void notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
            const SetBool notifying(m_notifying);
            
            typename List::const_iterator it, end;
            for (it = m_observers.begin(), end = m_observers.end(); it != end; ++it) {
                O& observer = **it;
                if (!observer.skip())
                    observer(a1, a2, a3, a4, a5);
            }
            
            removePending();
            addPending();
        }
    private:
        void addPending() {
            m_observers.insert(m_observers.end(), m_toAdd.begin(), m_toAdd.end());
            m_toAdd.clear();
        }
        
        void removePending() {
            typename List::iterator it, end, elem;
            for (it = m_toRemove.begin(), end = m_toRemove.end(); it != end; ++it) {
                elem = std::find_if(m_observers.begin(), m_observers.end(), CompareObservers(*it));
                assert(elem != m_observers.end());
                delete *elem;
                m_observers.erase(elem);
            }

            ListUtils::clearAndDelete(m_toRemove);
        }
    };
    
    class Notifier0 {
    private:
        class Observer {
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }
            
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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)()) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)()) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier0& notifier) {
            return addObserver(&notifier, &Notifier0::operator());
        }
        
        bool removeObserver(Notifier0& notifier) {
            return removeObserver(&notifier, &Notifier0::operator());
        }
        
        void notify() {
            m_state.notify();
        }
        
        void operator()() {
            notify();
        }
    };
    
    template <typename A1>
    class Notifier1 {
    private:
        class Observer {
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }

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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1)) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1)) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier1& notifier) {
            return addObserver(&notifier, &Notifier1::operator());
        }
        
        bool removeObserver(Notifier1& notifier) {
            return removeObserver(&notifier, &Notifier1::operator());
        }

        void notify(A1 a1) {
            m_state.notify(a1);
        }

        void operator()(A1 a1) {
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
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }
            
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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2)) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2)) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier2& notifier) {
            return addObserver(&notifier, &Notifier2::operator());
        }
        
        bool removeObserver(Notifier2& notifier) {
            return removeObserver(&notifier, &Notifier2::operator());
        }

        void notify(A1 a1, A2 a2) {
            m_state.notify(a1, a2);
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
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }
            
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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2, A3)) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2, A3)) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier3& notifier) {
            return addObserver(&notifier, &Notifier3::operator());
        }
        
        bool removeObserver(Notifier3& notifier) {
            return removeObserver(&notifier, &Notifier3::operator());
        }

        void notify(A1 a1, A2 a2, A3 a3) {
            m_state.notify(a1, a2, a3);
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
    
    template <typename A1, typename A2, typename A3, typename A4>
    class Notifier4 {
    private:
        class Observer {
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }
            
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()(A1 a1, A2 a2, A3 a3, A4 a4) = 0;
            
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
            typedef void (R::*F)(A1 a1, A2 a2, A3 a3, A4 a4);
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()(A1 a1, A2 a2, A3 a3, A4 a4) {
                (m_receiver->*m_function)(a1, a2, a3, a4);
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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2, A3, A4)) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2, A3, A4)) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier4& notifier) {
            return addObserver(&notifier, &Notifier4::operator());
        }
        
        bool removeObserver(Notifier4& notifier) {
            return removeObserver(&notifier, &Notifier4::operator());
        }
        
        void notify(A1 a1, A2 a2, A3 a3, A4 a4) {
            m_state.notify(a1, a2, a3, a4);
        }
        
        void operator()(A1 a1, A2 a2, A3 a3, A4 a4) {
            notify(a1, a2, a3, a4);
        }
        
        template <typename I>
        void notify(I it, I end, A2 a2, A3 a3, A4 a4) {
            while (it != end) {
                notify(*it, a2, a3, a4);
                ++it;
            }
        }
        
        template <typename I>
        void operator()(I it, I end, A2 a2, A3 a3, A4 a4) {
            while (it != end) {
                notify(*it, a2, a3, a4);
                ++it;
            }
        }
    };
    
    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    class Notifier5 {
    private:
        class Observer {
        private:
            bool m_skip;
        public:
            Observer() : m_skip(false) {}
            
            bool skip() const {
                return m_skip;
            }
            
            void setSkip() {
                m_skip = true;
            }
            
            virtual ~Observer() {}
            
            virtual void* receiver() const = 0;
            
            virtual void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) = 0;
            
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
            typedef void (R::*F)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
            
            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}
            
            void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
                (m_receiver->*m_function)(a1, a2, a3, a4, a5);
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
    private:
        NotifierState<Observer> m_state;
    public:
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A1, A2, A3, A4, A5)) {
            return m_state.addObserver(new CObserver<R>(receiver, function));
        }
        
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A1, A2, A3, A4, A5)) {
            return m_state.removeObserver(new CObserver<R>(receiver, function));
        }
        
        bool addObserver(Notifier5& notifier) {
            return addObserver(&notifier, &Notifier5::operator());
        }
        
        bool removeObserver(Notifier5& notifier) {
            return removeObserver(&notifier, &Notifier5::operator());
        }
        
        void notify(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
            m_state.notify(a1, a2, a3, a4, a5);
        }
        
        void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
            notify(a1, a2, a3, a4, a5);
        }
        
        template <typename I>
        void notify(I it, I end, A2 a2, A3 a3, A4 a4, A5 a5) {
            while (it != end) {
                notify(*it, a2, a3, a4, a5);
                ++it;
            }
        }
        
        template <typename I>
        void operator()(I it, I end, A2 a2, A3 a3, A4 a4, A5 a5) {
            while (it != end) {
                notify(*it, a2, a3, a4, a5);
                ++it;
            }
        }
    };
}

#endif

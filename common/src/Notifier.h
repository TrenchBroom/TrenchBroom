/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include <kdl/set_temp.h>

#include <cassert>
#include <memory>
#include <vector>

namespace TrenchBroom {
    /**
     * Encapsulates the internal state of a notifier. Handles adding and removing observers during notification.
     *
     * @tparam O the type of the observers registered with the notifier
     */
    template <typename O>
    class NotifierState {
    private:
        std::vector<std::unique_ptr<O>> m_observers;
        std::vector<std::unique_ptr<O>> m_toAdd;
        std::vector<std::unique_ptr<O>> m_toRemove;

        bool m_notifying;
    public:
        /**
         * Creates a new notifier state.
         */
        NotifierState() : m_notifying(false) {}

        /**
         * Adds the given observer to this notifier state. If the notifier is currently notifying, then the given
         * observer will be added when notification is finished. A given observer can be added only once.
         *
         * @param observer the observer to add
         * @return true if the given observer was successfully registered and false otherwise
         */
        bool addObserver(std::unique_ptr<O> observer) {
            if (!m_observers.empty()) {
                auto it = findObserver(observer);
                if (it != std::end(m_observers)) {
                    return false;
                }
            }

            if (m_notifying) {
                m_toAdd.push_back(std::move(observer));
            } else {
                m_observers.push_back(std::move(observer));
            }
            return true;
        }

        /**
         * Removes the given observer from this notifier state. If the notifier is currently, notifying, then the given
         * observer will be removed when notification is finished.
         *
         * @param observer the observer to remove
         * @return true if the given observer could be removed successfully and false otherwise
         */
        bool removeObserver(std::unique_ptr<O> observer) {
            auto it = findObserver(observer);
            if (it == std::end(m_observers)) {
                return false;
            } else {
                (*it)->setSkip();
            }

            if (m_notifying) {
                m_toRemove.push_back(std::move(observer));
            } else {
                m_observers.erase(it);
            }

            return true;
        }

        /**
         * Notifies all registered observers and passes the given arguments.
         *
         * @tparam A the argument types
         * @param a the arguments
         */
        template <typename... A>
        void notify(A... a) {
            {
                const kdl::set_temp notifying(m_notifying);
                for (auto& observer : m_observers) {
                    if (!observer->skip()) {
                        (*observer)(a...);
                    }
                }
            }

            removePending();
            addPending();
        }
    private:
        void addPending() {
            assert(!m_notifying);

            for (auto it = std::begin(m_toAdd), end = std::end(m_toAdd); it != end; ++it) {
                m_observers.push_back(std::move(*it));
            }
            m_toAdd.clear();
        }

        void removePending() {
            assert(!m_notifying);

            for (const auto& observer : m_toRemove) {
                auto it = findObserver(observer);
                assert(it != std::end(m_observers));
                m_observers.erase(it);
            }

            m_toRemove.clear();
        }

        auto findObserver(const std::unique_ptr<O>& observer) const {
            for (auto it = std::begin(m_observers), end = std::end(m_observers); it != end; ++it) {
                if (*observer == **it) {
                    return it;
                }
            }
            return std::end(m_observers);
        }
    };

    /**
     * A notifier allows registering observers and notifying them. An observer is a member function whose signature
     * matches the argument types given as parameters to this template.
     *
     * @tparam A the observer callback type parameter types
     */
    template <typename... A>
    class Notifier {
    private:
        using N = Notifier<A...>;

        class Observer {
        private:
            bool m_skip;
        public:
            Observer() :
            m_skip(false) {}

            virtual ~Observer()= default;

            bool skip() const {
                return m_skip;
            }

            void setSkip() {
                m_skip = true;
            }

            virtual void* receiver() const = 0;
            virtual void operator()(A... a) = 0;

            bool operator==(const Observer& rhs) const {
                return receiver() == rhs.receiver() && compareFunctions(rhs);
            }
        private:
            virtual bool compareFunctions(const Observer& rhs) const = 0;
        };

        template <typename R>
        class CObserver : public Observer {
        private:
            using F = void(R::*)(A...);

            R* m_receiver;
            F m_function;
        public:
            CObserver(R* receiver, F function) :
            m_receiver(receiver),
            m_function(function) {}

            void operator()(A... a) override {
                (m_receiver->*m_function)(a...);
            }

            void* receiver() const override {
                return static_cast<void*>(m_receiver);
            }

            F function() const {
                return m_function;
            }

            bool compareFunctions(const Observer& rhs) const override {
                auto& rhsR = static_cast<const CObserver<R>&>(rhs);
                return m_function == rhsR.function();
            }
        };
    private:
        NotifierState<Observer> m_state;
    public:
        /**
         * RAII style helper tht notifies the given notifier immediately, passing the given arguments. This class in and
         * of itself is not very useful and was only added for reasons of symmetry.
         */
        class NotifyBefore {
        public:
            /**
             * Creates a new instance and immediately notifies the given notifier with the given parameters
             *
             * @param notify controls whether or not the notifications should be sent
             * @param before the notifier to notify
             * @param a the arguments to pass to the notifier
             */
            explicit NotifyBefore(const bool notify, N& before, A... a) {
                if (notify) {
                    before(a...);
                }
            }
        };

        /**
         * RAII style helper tht notifies the given notifier when it is destroyed, passing the given arguments.
         */
        class NotifyAfter {
        private:
            /**
             * Holds a lambda; used in favor of std::function to keep compilation times low.
             */
            class BaseHolder {
            public:
                virtual ~BaseHolder() = default;
                virtual void apply() = 0;
            };

            template <typename T>
            class Holder : public BaseHolder {
            private:
                bool m_notify;
                T m_func;
            public:
                explicit Holder(const bool notify, T&& func) : 
                m_notify(notify),
                m_func(std::move(func)) {}

                void apply() override {
                    if (m_notify) {
                        m_func();
                    }
                }
            };

            std::unique_ptr<BaseHolder> m_after;
        public:
            /**
             * Creates a new instance to notify the given notifier. The given arguments are passed to the notifier.
             *
             * @param notify controls whether or not the notifications should be sent
             * @param after the notifier to notify
             * @param a the arguments to pass to the notifier
             */
            explicit NotifyAfter(const bool notify, N& after, A... a) :
            m_after(createLambda(notify, after, std::move(a)...)) {}

            virtual ~NotifyAfter() {
                m_after->apply();
            }
        private:
            static std::unique_ptr<BaseHolder> createLambda(const bool notify, N& after, A... a) {
                auto lambda = [&]() { after(a...); };
                return std::make_unique<Holder<decltype(lambda)>>(notify, std::move(lambda));
            }
        };

        /**
         * RAII style helper tht notifies a given notifier immediately and another notifier when it is destroyed,
         * passing the given arguments to either notifier.
         */
        class NotifyBeforeAndAfter : public NotifyBefore, public NotifyAfter {
        public:
            /**
             * Creates a new instance that notifies the given notifiers.
             *
             * @param notify controls whether or not the notifications should be sent
             * @param before the notifier to notify immediately
             * @param after the notifier to notify later when this object is destroyed
             * @param a the arguments to pass to either notifier
             */
            NotifyBeforeAndAfter(const bool notify, N& before, N& after, A... a) :
            NotifyBefore(notify, before, a...),
            NotifyAfter(notify, after, a...) {}

            /**
             * Creates a new instance that notifies the given notifiers.
             *
             * @param before the notifier to notify immediately
             * @param after the notifier to notify later when this object is destroyed
             * @param a the arguments to pass to either notifier
             */
            NotifyBeforeAndAfter(N& before, N& after, A... a) :
            NotifyBeforeAndAfter(true, before, after, std::forward<A>(a)...) {}
        };

        /**
         * Adds the given observer to this notifier.
         *
         * @tparam R the receiver type
         * @param receiver the receiver
         * @param function the member of the receiver to notify
         * @return true if the given observer was successfully added to this notifier and false otherwise
         */
        template <typename R>
        bool addObserver(R* receiver, void (R::*function)(A...)) {
            return m_state.addObserver(std::make_unique<CObserver<R>>(receiver, function));
        }

        /**
         * Removes the given observer from this notifier.
         *
         * @tparam R the receiver type
         * @param receiver the receiver
         * @param function the member of the receiver to notify
         * @return true if the given observer was successfully removed from this notifier and false otherwise
         */
        template <typename R>
        bool removeObserver(R* receiver, void (R::*function)(A...)) {
            return m_state.removeObserver(std::make_unique<CObserver<R>>(receiver, function));
        }

        /**
         * Adds the given notifier as an observer to this notifier.
         *
         * @param notifier the notifier to add
         * @return true if the given notifier was successfully added to this notifier and false otherwise
         */
        bool addObserver(Notifier<A...>& notifier) {
            return addObserver(&notifier, &Notifier<A...>::operator());
        }

        /**
         * Removes the given notifier as an observer from this notifier.
         *
         * @param notifier the notifier to remove
         * @return true if the given notifier was successfully removed from this notifier and false otherwise
         */
        bool removeObserver(Notifier& notifier) {
            return removeObserver(&notifier, &Notifier<A...>::operator());
        }

        /**
         * Notifies all observers of this notifier with the given arguments.
         *
         * @param a the arguments to pass to each notifier
         */
        void notify(A... a) {
            m_state.notify(a...);
        }

        /**
         * Notifies all observers of this notifier with the given arguments.
         *
         * @param a the arguments to pass to each notifier
         */
        void operator()(A... a) {
            notify(a...);
        }
    };
}

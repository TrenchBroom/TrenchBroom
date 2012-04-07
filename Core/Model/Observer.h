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
        template <typename T> 
        struct Notifier
        { 
            typedef void (*Function)(const vector<T>&); 
        };

        class Observable;
        class Observer {
        protected:
            virtual void notify(const string& name, const void* data) {};
            friend class ObservableSlot;
        };
        
        class ObservableSlot {
        private:
            bool m_postNotifications;
            vector<Observer*> m_observers;
        public:
            void addObserver(Observer& observer, Notifier::Function& function);
        };
        
        class Observable {
        private:
            bool m_postNotifications;
            multimap<const string, Observer&> m_observers;
        protected:
            void postNotification(const string& name, const void* data);
        public:
            Observable() : m_postNotifications(true) {};
            void addObserver(const string& name, Observer& observer);
            void removeObserver(const string& name, Observer& observer);
            void removeObserver(Observer& observer);
            void setPostNotifications(bool postNotifications);
            bool postNotifications();
        };
    }
}

#endif

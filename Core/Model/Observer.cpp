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

#include "Observer.h"

namespace TrenchBroom {
    void Observable::addObserver(const string& name, Observer& observer) {
        m_observers.insert(pair<const string, Observer&>(name, observer));
    }
    
    void Observable::removeObserver(const string& name, Observer& observer) {
        multimap<const string, Observer&>::iterator start, end;
        start = m_observers.lower_bound(name);
        end = m_observers.upper_bound(name);
        while (start != end) {
            if (&start->second == &observer) {
                m_observers.erase(start);
                break;
            }
            start++;
        }
    }
    
    void Observable::removeObserver(Observer& observer) {
        multimap<const string, Observer&>::iterator it, eraseIt;
        for (it = m_observers.begin(); it != m_observers.end(); it++) {
            if (&it->second == &observer) {
                eraseIt = it;
                m_observers.erase(eraseIt);
            }
        }
    }
    
    void Observable::postNotification(const string& name, const void* data) {
        if (m_postNotifications) {
            multimap<const string, Observer&>::iterator start, end;
            start = m_observers.lower_bound(name);
            end = m_observers.upper_bound(name);
            while (start != end) {
                start->second.notify(name, data);
                start++;
            }
        }
    }

    void Observable::setPostNotifications(bool postNotifications) {
        m_postNotifications = postNotifications;
    }
    
    bool Observable::postNotifications() {
        return m_postNotifications;
    }
}
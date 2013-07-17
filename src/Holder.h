/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef TrenchBroom_Holder_h
#define TrenchBroom_Holder_h

#include "SharedPointer.h"

class BaseHolder {
public:
    typedef std::tr1::shared_ptr<BaseHolder> Ptr;
    virtual ~BaseHolder() {}
};

template <typename T>
class Holder : public BaseHolder {
private:
    T m_object;
public:
    static BaseHolder::Ptr newHolder(T object) {
        return BaseHolder::Ptr(new Holder(object));
    }
    
    inline T object() const {
        return m_object;
    }
private:
    Holder(T object) :
    m_object(object) {}
    
};

#endif

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

#ifndef TrenchBroom_TemporarilySetAny
#define TrenchBroom_TemporarilySetAny

#include "Ensure.h"

namespace TrenchBroom {
    template <typename T>
    class TemporarilySetAny {
    private:
        T& m_value;
        T m_oldValue;
    public:
        TemporarilySetAny(T& value, T newValue) :
        m_value(value),
        m_oldValue(m_value) {
            m_value = newValue;
        }

        virtual ~TemporarilySetAny() {
            m_value = m_oldValue;
        }
    };

    template <typename T>
    class SetLate {
    private:
        T& m_value;
        T m_newValue;
    public:
        SetLate(T& value, T newValue) :
        m_value(value),
        m_newValue(newValue) {}

        ~SetLate() {
            m_value = m_newValue;
        }
    };

    class TemporarilySetBool : public TemporarilySetAny<bool> {
    public:
        explicit TemporarilySetBool(bool& value, bool newValue = true);
    };

    template <typename R>
    class TemporarilySetBoolFun {
    private:
        using F = void (R::*)(bool);
        R* m_receiver;
        F m_function;
        bool m_setTo;
    public:
        TemporarilySetBoolFun(R* receiver, F function, bool setTo = true) :
        m_receiver(receiver),
        m_function(function),
        m_setTo(setTo) {
            ensure(m_receiver != nullptr, "receiver is null");
            (m_receiver->*m_function)(m_setTo);
        }

        ~TemporarilySetBoolFun() {
            (m_receiver->*m_function)(!m_setTo);
        }
    };
}

#endif /* defined(TrenchBroom_TemporarilySetAny) */

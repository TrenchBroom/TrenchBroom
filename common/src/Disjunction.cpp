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

#include "Disjunction.h"

#include <cassert>

namespace TrenchBroom {
    Disjunction::Disjunction() : m_count(0) {}

    Disjunction& Disjunction::operator=(const bool value) {
        if (value) {
            ++m_count;
        } else {
            assert(m_count > 0);
            if (m_count > 0)
                --m_count;
        }
        return *this;
    }

    Disjunction::operator bool() const {
        return m_count > 0;
    }

    Disjunction::Set::Set(Disjunction& disjunction) :
    m_disjunction(disjunction) {
        m_disjunction = true;
    }

    Disjunction::Set::~Set() {
        m_disjunction = false;
    }

}

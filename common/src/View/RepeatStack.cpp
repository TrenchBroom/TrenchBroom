/*
 Copyright (C) 2020 Kristian Duske

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

#include "RepeatStack.h"

#include <kdl/set_temp.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        RepeatStack::RepeatStack() :
        m_clearOnNextPush(false),
        m_repeating(false) {}

        size_t RepeatStack::size() const {
            return m_stack.size();
        }

        void RepeatStack::push(RepeatableAction repeatableAction) {
            if (!m_repeating) {
                if (m_clearOnNextPush) {
                    m_clearOnNextPush = false;
                    clear();
                }
                m_stack.push_back(std::move(repeatableAction));
            }
        }

        void RepeatStack::repeat() const {
            const kdl::set_temp repeating(m_repeating);
            for (const auto& repeatable : m_stack) {
                repeatable();
            }
        }

        void RepeatStack::clear() {
            assert(!m_repeating);
            m_stack.clear();
        }

        void RepeatStack::clearOnNextPush() {
            m_clearOnNextPush = true;
        }
    }
}

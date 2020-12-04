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

#pragma once

#include <functional>
#include <vector>

namespace TrenchBroom {
    namespace View {
        /**
         * A stack of actions (C++ callables) that can be repeatedly executed as a whole.
         *
         * Actions are repeated in the order in which they have been added to the stack. If the
         * stack is currently repeating, requests for adding an action are ignored so that repeating
         * actions doesn't put the repeated actions on the stack again.
         *
         * The stack can be cleared immediately or it can be primed to be cleared automatically when
         * the next action is pushed to the stack.
         */ 
        class RepeatStack {
        public:
            using RepeatableAction = std::function<void()>;
        private:
            std::vector<RepeatableAction> m_stack;
            bool m_clearOnNextPush;
            mutable bool m_repeating;
        public:
            /**
             * Creates a new instance.
             */
            RepeatStack();

            /**
             * Returns the number of repeatable actions on this repeat stack.
             */
            size_t size() const;

            /**
             * Adds the given repeatable action to this repeat stack.
             * 
             * If this stack is currently repeating actions, the given action is not added.
             * If clearOnNextPush() was called, the repeat stack will be cleared before the given action
             * is added.
             * 
             * @param repeatableAction the action to add
             */
            void push(RepeatableAction repeatableAction);

            /**
             * Repeats the actions on this stack in the order in which they were added.
             *
             * No new actions will be added to the stack while it is repeating, so the list of
             * actions on the stack will be the same when this function finishes.
             */
            void repeat() const;

            /**
             * Clears all repeatable actions on this stack.
             * 
             * The stack must not be repeating actions when this function is called.
             */ 
            void clear();

            /**
             * Prime the stack so that it is cleared when the next action is pushed.
             * 
             * The effect is that the action currently on the stack can still be repeated
             * (even multiple times). But when the next action is pushed onto the stack, it
             * is cleared.
             */
            void clearOnNextPush();
        };
    }
}

#endif /* defined(TrenchBroom_RepeatStack) */

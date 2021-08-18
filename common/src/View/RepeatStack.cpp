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

#include "Ensure.h"

#include <kdl/overload.h>
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

        void RepeatStack::push(CompositeAction repeatableAction) {
            if (!m_repeating) {
                if (m_openTransactionsStack.empty()) {
                    if (m_clearOnNextPush) {
                        m_clearOnNextPush = false;
                        clear();
                    }

                    m_stack.push_back(std::move(repeatableAction));
                } else {
                    auto& openTransaction = m_openTransactionsStack.back();
                    openTransaction->actions.push_back(std::move(repeatableAction));
                }
            }
        }

        void RepeatStack::repeat() const {
            ensure(m_openTransactionsStack.empty(), "must not be called with open transactions");

            const kdl::set_temp repeating(m_repeating);

            const std::function<void(const CompositeAction&)> performComposite = [&](const CompositeAction& c) {
                std::visit(kdl::overload(
                    [](const RepeatableAction& r) {
                        r();
                    },
                    [&](const std::unique_ptr<Transaction>& t) {
                        for (const auto& transactionAction : t->actions) {
                            performComposite(transactionAction);
                        }
                    }
                ), c);
            };

            for (const auto& repeatable : m_stack) {
                performComposite(repeatable);
            }
        }

        void RepeatStack::clear() {
            ensure(!m_repeating, "The stack must not be repeating actions when this function is called");
            ensure(m_openTransactionsStack.empty(), "must not be called with open transactions");
            m_stack.clear();
        }

        void RepeatStack::clearOnNextPush() {
            if (!m_openTransactionsStack.empty()) {
                return;
            }
            m_clearOnNextPush = true;
        }

        void RepeatStack::startTransaction() {
            if (m_repeating) {
                return;
            }
            m_openTransactionsStack.push_back(std::make_unique<Transaction>());
        }

        void RepeatStack::commitTransaction() {
            if (m_repeating) {
                return;
            }
            ensure(!m_openTransactionsStack.empty(), "a transaction is open");

            auto transaction = std::move(m_openTransactionsStack.back());
            m_openTransactionsStack.pop_back();

            // discard empty transactions
            if (transaction->actions.empty()) {
                return;
            }

            // push it onto the next open transaction (or the main stack)
            push(std::move(transaction));
        }

        void RepeatStack::rollbackTransaction() {
            if (m_repeating) {
                return;
            }
            ensure(!m_openTransactionsStack.empty(), "a transaction is open");

            auto& openTransaction = m_openTransactionsStack.back();
            openTransaction->actions.clear();
        }
    }
}

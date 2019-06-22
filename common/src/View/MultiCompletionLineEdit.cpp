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

#include "MultiCompletionLineEdit.h"

#include <QtGlobal>
#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QKeySequence>
#include <QScrollBar>
#include <QShortcut>

namespace TrenchBroom {
    namespace View {
        MultiCompletionLineEdit::MultiCompletionLineEdit(QWidget* parent) :
        MultiCompletionLineEdit(QString(), parent) {}

        MultiCompletionLineEdit::MultiCompletionLineEdit(const QString& contents, QWidget* parent) :
        QLineEdit(contents, parent),
        m_multiCompleter(nullptr)  {
            auto* shortcut = new QShortcut(QKeySequence(
#ifdef __APPLE__
                Qt::META
#else
                Qt::CTRL
#endif
                + Qt::Key_Space), this);
            connect(shortcut, &QShortcut::activated, this, &MultiCompletionLineEdit::triggerCompletion);
        }

        MultiCompletionLineEdit::~MultiCompletionLineEdit() {
            delete m_multiCompleter;
            m_multiCompleter = nullptr;
        }

        void MultiCompletionLineEdit::setWordDelimiter(const QRegularExpression& delimiters) {
            setWordDelimiters(delimiters, delimiters);
        }

        void MultiCompletionLineEdit::setWordDelimiters(const QRegularExpression& leftDelimiter, const QRegularExpression& rightDelimiter) {
            m_leftDelimiter = leftDelimiter;
            m_rightDelimiter = rightDelimiter;
        }

        void MultiCompletionLineEdit::setMultiCompleter(QCompleter* completer) {
            delete m_multiCompleter;
            m_multiCompleter = completer;
            if (m_multiCompleter != nullptr) {
                m_multiCompleter->setWidget(this);
                connect(m_multiCompleter, QOverload<const QString&>::of(&QCompleter::activated), this, &MultiCompletionLineEdit::insertCompletion);
            }
        }

        void MultiCompletionLineEdit::keyPressEvent(QKeyEvent* event) {
            QLineEdit::keyPressEvent(event);

            const auto t = event->text();
            updateCompleter(!t.isEmpty() && t[0].isPrint());
        }

        void MultiCompletionLineEdit::updateCompleter(const bool showCompleter) {
            if (!m_multiCompleter) {
                return;
            }

            const auto leftBoundary = findLeftBoundary();
            const auto rightBoundary = findRightBoundary();

            if (leftBoundary > rightBoundary) {
                return;
            }

            const auto& t = this->text();
            const auto completionPrefix = t.mid(leftBoundary, cursorPosition() - leftBoundary);

            m_multiCompleter->setCompletionPrefix(completionPrefix);
            if (m_multiCompleter->completionPrefix().length() < 1) {
                m_multiCompleter->popup()->hide();
                return;
            }

            if (showCompleter) {
                QRect cr = cursorRect();
                cr.setWidth(m_multiCompleter->popup()->sizeHintForColumn(0) + m_multiCompleter->popup()->verticalScrollBar()->sizeHint().width());
                m_multiCompleter->complete(cr);
            }
        }

        int MultiCompletionLineEdit::findLeftBoundary() const {
            if (cursorPosition() == 0 || m_leftDelimiter.pattern().isEmpty() || m_rightDelimiter.pattern().isEmpty()) {
                return 0;
            }

            const auto prefix = this->text().left(cursorPosition());

            const auto lastLeftDelimiter  = findLastMatch(prefix, m_leftDelimiter);
            const auto lastRightDelimiter = findLastMatch(prefix, m_rightDelimiter);

            if (lastLeftDelimiter == -1) {
                return cursorPosition();
            } else if (lastRightDelimiter == -1) {
                return lastLeftDelimiter;
            } else if (lastRightDelimiter > lastLeftDelimiter) {
                return cursorPosition();
            } else {
                return lastLeftDelimiter;
            }
        }

        int MultiCompletionLineEdit::findRightBoundary() const {
            const auto& t = this->text();
            if (cursorPosition() == t.length() || m_leftDelimiter.pattern().isEmpty() || m_rightDelimiter.pattern().isEmpty()) {
                return t.length();
            }

            const auto suffix = this->text().mid(cursorPosition());

            const auto firstLeftDelimiter  = findFirstMatch(suffix, m_leftDelimiter);
            const auto firstRightDelimiter = findFirstMatch(suffix, m_rightDelimiter);

            if (firstRightDelimiter == -1) {
                return cursorPosition();
            } else if (firstLeftDelimiter == -1) {
                return cursorPosition() + firstRightDelimiter + 1;
            } else if (firstLeftDelimiter < firstRightDelimiter) {
                return cursorPosition();
            } else {
                return cursorPosition() + firstRightDelimiter + 1;
            }
        }

        int MultiCompletionLineEdit::findFirstMatch(const QString& str, const QRegularExpression& expression) const {
            auto matches = expression.globalMatch(str);
            if (!matches.hasNext() || !matches.isValid()) {
                return -1;
            }

            return matches.next().capturedStart();
        }

        int MultiCompletionLineEdit::findLastMatch(const QString& str, const QRegularExpression& expression) const {
            auto matches = expression.globalMatch(str);
            if (!matches.hasNext() || !matches.isValid()) {
                return -1;
            }

            auto lastMatch = matches.next();
            while (matches.hasNext()) {
                lastMatch = matches.next();
            }

            return lastMatch.capturedStart();
        }

        void MultiCompletionLineEdit::triggerCompletion() {
            updateCompleter(true);
        }

        void MultiCompletionLineEdit::insertCompletion(const QString& string) {
            const auto leftBoundary = findLeftBoundary();
            const auto rightBoundary = findRightBoundary();

            if (leftBoundary > rightBoundary) {
                return;
            }

            auto oldText = this->text();
            setText(oldText.replace(leftBoundary, rightBoundary - leftBoundary, string));
            setCursorPosition(leftBoundary + string.length());
        }
    }
}

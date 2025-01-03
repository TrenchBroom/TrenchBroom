/*
 Copyright (C) 2010 Kristian Duske

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

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QKeySequence>
#include <QScrollBar>
#include <QShortcut>
#include <QtGlobal>

namespace tb::ui
{

MultiCompletionLineEdit::MultiCompletionLineEdit(QWidget* parent)
  : MultiCompletionLineEdit(QString{}, parent)
{
}

MultiCompletionLineEdit::MultiCompletionLineEdit(const QString& contents, QWidget* parent)
  : QLineEdit{contents, parent}
{
  auto* shortcut = new QShortcut(
    QKeySequence(
#ifdef __APPLE__
      +Qt::META
#else
      +Qt::CTRL
#endif
      + Qt::Key_Space),
    this);
  connect(
    shortcut, &QShortcut::activated, this, &MultiCompletionLineEdit::triggerCompletion);
}

MultiCompletionLineEdit::~MultiCompletionLineEdit()
{
  delete m_multiCompleter;
  m_multiCompleter = nullptr;
}

void MultiCompletionLineEdit::setWordDelimiter(const QRegularExpression& delimiters)
{
  setWordDelimiters(delimiters, delimiters);
}

void MultiCompletionLineEdit::setWordDelimiters(
  const QRegularExpression& leftDelimiter, const QRegularExpression& rightDelimiter)
{
  m_leftDelimiter = leftDelimiter;
  m_rightDelimiter = rightDelimiter;
}

void MultiCompletionLineEdit::setMultiCompleter(QCompleter* completer)
{
  delete m_multiCompleter;
  m_multiCompleter = completer;
  if (m_multiCompleter)
  {
    m_multiCompleter->setWidget(this);
    connect(
      m_multiCompleter,
      QOverload<const QString&>::of(&QCompleter::activated),
      this,
      &MultiCompletionLineEdit::insertCompletion);
  }
}

void MultiCompletionLineEdit::keyPressEvent(QKeyEvent* event)
{
  QLineEdit::keyPressEvent(event);

  const auto t = event->text();
  updateCompleter(!t.isEmpty() && t[0].isPrint());
}

void MultiCompletionLineEdit::updateCompleter(const bool showCompleter)
{
  if (!m_multiCompleter)
  {
    return;
  }

  const auto leftBoundary = findLeftBoundary();
  const auto rightBoundary = findRightBoundary();

  if (leftBoundary > rightBoundary)
  {
    return;
  }

  const auto& t = this->text();
  const auto completionPrefix = t.mid(leftBoundary, cursorPosition() - leftBoundary);

  m_multiCompleter->setCompletionPrefix(completionPrefix);
  if (m_multiCompleter->completionPrefix().length() < 1)
  {
    m_multiCompleter->popup()->hide();
    return;
  }

  if (showCompleter)
  {
    auto cr = cursorRect();
    cr.setWidth(
      m_multiCompleter->popup()->sizeHintForColumn(0)
      + m_multiCompleter->popup()->verticalScrollBar()->sizeHint().width());
    m_multiCompleter->complete(cr);
  }
}

int MultiCompletionLineEdit::findLeftBoundary() const
{
  if (
    cursorPosition() == 0 || m_leftDelimiter.pattern().isEmpty()
    || m_rightDelimiter.pattern().isEmpty())
  {
    return 0;
  }

  const auto prefix = this->text().left(cursorPosition());

  const auto lastLeftDelimiter = findLastMatch(prefix, m_leftDelimiter);
  const auto lastRightDelimiter = findLastMatch(prefix, m_rightDelimiter);

  return lastLeftDelimiter == -1                  ? cursorPosition()
         : lastRightDelimiter == -1               ? lastLeftDelimiter
         : lastRightDelimiter > lastLeftDelimiter ? cursorPosition()
                                                  : lastLeftDelimiter;
}

int MultiCompletionLineEdit::findRightBoundary() const
{
  const auto& t = this->text();
  if (
    cursorPosition() == t.length() || m_leftDelimiter.pattern().isEmpty()
    || m_rightDelimiter.pattern().isEmpty())
  {
    return int(t.length());
  }

  const auto suffix = this->text().mid(cursorPosition());

  const auto firstLeftDelimiter = findFirstMatch(suffix, m_leftDelimiter);
  const auto firstRightDelimiter = findFirstMatch(suffix, m_rightDelimiter);

  return firstRightDelimiter == -1  ? cursorPosition()
         : firstLeftDelimiter == -1 ? cursorPosition() + firstRightDelimiter + 1
         : firstLeftDelimiter < firstRightDelimiter
           ? cursorPosition()
           : cursorPosition() + firstRightDelimiter + 1;
}

int MultiCompletionLineEdit::findFirstMatch(
  const QString& str, const QRegularExpression& expression) const
{
  auto matches = expression.globalMatch(str);
  return matches.hasNext() && matches.isValid() ? int(matches.next().capturedStart())
                                                : -1;
}

int MultiCompletionLineEdit::findLastMatch(
  const QString& str, const QRegularExpression& expression) const
{
  auto matches = expression.globalMatch(str);
  if (matches.hasNext() && matches.isValid())
  {
    auto lastMatch = matches.next();
    while (matches.hasNext())
    {
      lastMatch = matches.next();
    }

    return int(lastMatch.capturedStart());
  }
  return -1;
}

void MultiCompletionLineEdit::triggerCompletion()
{
  updateCompleter(true);
}

void MultiCompletionLineEdit::insertCompletion(const QString& string)
{
  const auto leftBoundary = findLeftBoundary();
  const auto rightBoundary = findRightBoundary();

  if (leftBoundary > rightBoundary)
  {
    return;
  }

  auto oldText = this->text();
  setText(oldText.replace(leftBoundary, rightBoundary - leftBoundary, string));
  setCursorPosition(leftBoundary + int(string.length()));
}

} // namespace tb::ui

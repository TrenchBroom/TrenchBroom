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

#include "Console.h"

#include <QDebug>
#include <QMutexLocker>
#include <QScrollBar>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

#include "Ensure.h"
#include "FileLogger.h"
#include "Macros.h"
#include "View/ViewConstants.h"

#include <string>

namespace TrenchBroom::View
{
namespace
{

auto getForegroundBrush(const LogLevel level, const QPalette& palette)
{
  // NOTE: QPalette::Text is the correct color role for contrast against QPalette::Base
  // which is the background of text entry widgets

  switch (level)
  {
  case LogLevel::Debug:
    return QBrush{palette.color(QPalette::Disabled, QPalette::Text)};
  case LogLevel::Info:
    return QBrush{palette.color(QPalette::Normal, QPalette::Text)};
  case LogLevel::Warn:
    return QBrush{palette.color(QPalette::Active, QPalette::Text)};
  case LogLevel::Error:
    return QBrush{QColor{250, 30, 60}};
    switchDefault();
  }
}

} // namespace

Console::Console(QWidget* parent)
  : TabBookPage{parent}
  , m_timer{new QTimer{this}}
{
  m_textView = new QTextEdit{};
  m_textView->setReadOnly(true);
  m_textView->setWordWrapMode(QTextOption::NoWrap);

  auto* sizer = new QVBoxLayout{};
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->addWidget(m_textView);
  setLayout(sizer);

  connect(m_timer, &QTimer::timeout, this, &Console::logCachedMessages);
  m_timer->start(50);
}

void Console::doLog(const LogLevel level, const std::string_view message)
{
  if (!message.empty())
  {
    auto lock = QMutexLocker{&m_cacheMutex};
    m_cache.cacheMessage(level, message);
  }
}

void Console::logToDebugOut(const LogLevel /* level */, const std::string& message)
{
  qDebug("%s", message.c_str());
}

void Console::logToConsole(const LogLevel level, const std::string& message)
{
  ensure(
    m_textView->thread() == QThread::currentThread(),
    "Can only log to console from main thread");

  auto format = QTextCharFormat{};
  format.setForeground(getForegroundBrush(level, m_textView->palette()));
  format.setFont(Fonts::fixedWidthFont());

  auto cursor = QTextCursor{m_textView->document()};
  cursor.movePosition(QTextCursor::MoveOperation::End);

  cursor.insertText(QString::fromStdString(message), format);
  cursor.insertText("\n");

  m_textView->moveCursor(QTextCursor::MoveOperation::End);
}

void Console::logCachedMessages()
{
  auto lock = QMutexLocker{&m_cacheMutex};

  m_cache.getCachedMessages([this](const auto level, const auto& message) {
    const auto messageStr = std::string{message};
    logToDebugOut(level, messageStr);
    logToConsole(level, messageStr);
    FileLogger::instance().log(level, message);
  });
}

} // namespace TrenchBroom::View

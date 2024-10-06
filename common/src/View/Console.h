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

#pragma once

#include <QMutex>

#include "Logger.h"
#include "LoggerCache.h"
#include "View/TabBook.h"

#include <string_view>

class QTextEdit;
class QTimer;
class QWidget;

namespace tb::View
{
class Console : public TabBookPage, public Logger
{
private:
  QTextEdit* m_textView = nullptr;
  QTimer* m_timer = nullptr;

  LoggerCache m_cache;
  QMutex m_cacheMutex;

public:
  explicit Console(QWidget* parent = nullptr);

private:
  void doLog(LogLevel level, std::string_view message) override;
  void logToDebugOut(LogLevel level, const std::string& message);
  void logToConsole(LogLevel level, const std::string& message);

  void logCachedMessages();
};

} // namespace tb::View

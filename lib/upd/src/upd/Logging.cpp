/*
 Copyright (C) 2025 Kristian Duske

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

#include "Logging.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace upd
{

void logToFile(const std::optional<QString>& logFilePath, const QString& msg)
{
  if (logFilePath)
  {
    if (auto logFile = QFile{*logFilePath};
        logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
      const auto timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

      auto out = QTextStream{&logFile};
      out << "[" << timestamp << "] " << msg << "\n";
    }
    else
    {
      qDebug() << "Failed to open log file:" << *logFilePath;
    }
  }
}

} // namespace upd

/*
 Copyright (C) 2023 Kristian Duske

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

#include <QObject>
#include <QSharedMemory>
#include <QString>
#include <QSystemSemaphore>

#include <kdl/reflection_decl.h>
#include <kdl/result.h>

#include <string>

class QSharedMemory;
class QLocalServer;
class QLocalSocket;

// See https://stackoverflow.com/a/28172162

namespace TrenchBroom::View
{
enum class CmdServerMode
{
  Async,
  Deferred
};

struct CmdError
{
  std::string msg;

  kdl_reflect_decl(CmdError, msg);
};

class CmdServer : public QObject
{
  Q_OBJECT
private:
  QString m_name;
  QLocalServer* m_server;

public:
  explicit CmdServer(QString name, CmdServerMode mode);

  bool start();
  void stop();
  bool processCommand();
  QString serverName() const;

signals:
  void commandReceived(const QString& cmd);

private:
  void onNewConnection(CmdServerMode mode);
};

kdl::result<void, CmdError> sendCommand(const QString& serverName, const QString& cmd);

class RunGuard : public QObject
{
  Q_OBJECT
private:
  const QString m_key;
  const QString m_memLockKey;
  const QString m_sharedMemKey;
  QSystemSemaphore m_memLock;
  QSharedMemory m_sharedMem;

  CmdServer m_server;

public:
  explicit RunGuard(QString key);
  ~RunGuard() override;

  bool isAnotherRunning();
  bool tryToRun();
  void sendCommandToMainInstance(const QString& cmd);

signals:
  void commandReceived(const QString& cmd);

private:
  void release();

  Q_DISABLE_COPY(RunGuard)
};
} // namespace TrenchBroom::View

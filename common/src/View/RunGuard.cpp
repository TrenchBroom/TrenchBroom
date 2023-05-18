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

#include "RunGuard.h"

#include <QCryptographicHash>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include <kdl/reflection_impl.h>

namespace TrenchBroom::View
{
namespace
{
const auto SocketName = QString{"TrenchBroom RunGuard"};

QString generateKeyHash(const QString& key, const QString& salt)
{
  auto data = QByteArray{};

  data.append(key.toUtf8());
  data.append(salt.toUtf8());
  data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

  return data;
}
} // namespace

kdl_reflect_impl(CmdError);

CmdServer::CmdServer(QString name, const CmdServerMode mode)
  : m_name{std::move(name)}
  , m_server{new QLocalServer{this}}
{
  if (mode == CmdServerMode::Async)
  {
    connect(m_server, &QLocalServer::newConnection, [&]() {
      onNewConnection(CmdServerMode::Async);
    });
  }
}

bool CmdServer::start()
{
  if (m_server->isListening())
  {
    return true;
  }

  QLocalServer::removeServer(m_name);

  if (!m_server->listen(m_name))
  {
    return false;
  }

  return true;
}

void CmdServer::stop()
{
  if (m_server->isListening())
  {
    m_server->close();
  }
}

bool CmdServer::processCommand()
{
  if (m_server->waitForNewConnection())
  {
    onNewConnection(CmdServerMode::Deferred);
    return true;
  }
  return false;
}

QString CmdServer::serverName() const
{
  return m_server->fullServerName();
}

void CmdServer::onNewConnection(const CmdServerMode mode)
{
  auto* socket = m_server->nextPendingConnection();
  connect(socket, &QLocalSocket::readyRead, [&]() {
    const auto message = socket->readAll();
    emit commandReceived(QString::fromUtf8(message));
  });

  if (mode == CmdServerMode::Deferred)
  {
    socket->waitForReadyRead();
  }
}


kdl::result<void, CmdError> sendCommand(const QString& serverName, const QString& cmd)
{
  auto socket = QLocalSocket{};
  socket.connectToServer(serverName);

  if (!socket.waitForConnected())
  {
    return CmdError{("Could not connect to server at " + socket.fullServerName() + " ("
                     + socket.errorString() + ")")
                      .toStdString()};
  }

  socket.write(cmd.toUtf8());
  if (!socket.waitForBytesWritten())
  {
    return CmdError{("Could not send to server at " + socket.fullServerName() + " ("
                     + socket.errorString() + ")")
                      .toStdString()};
  }

  socket.disconnectFromServer();
  if (socket.state() != QLocalSocket::UnconnectedState && !socket.waitForDisconnected())
  {
    return CmdError{("Could not disconnect from server at " + socket.fullServerName()
                     + " (" + socket.errorString() + ")")
                      .toStdString()};
  }

  return kdl::void_success;
}

RunGuard::RunGuard(QString key)
  : m_key{std::move(key)}
  , m_memLockKey{generateKeyHash(m_key, "_memLockKey")}
  , m_sharedMemKey{generateKeyHash(m_key, "_sharedMemKey")}
  , m_memLock{m_memLockKey, 1}
  , m_sharedMem{m_sharedMemKey}
  , m_server{SocketName, CmdServerMode::Async}
{
  m_memLock.acquire();
  {
    auto fix = QSharedMemory{m_sharedMemKey};
    fix.attach();
  }
  m_memLock.release();
}

RunGuard::~RunGuard()
{
  release();
}

bool RunGuard::isAnotherRunning()
{
  if (m_sharedMem.isAttached())
  {
    return false;
  }

  m_memLock.acquire();
  const auto isRunning = m_sharedMem.attach();
  if (isRunning)
  {
    m_sharedMem.detach();
  }
  m_memLock.release();

  return isRunning;
}

bool RunGuard::tryToRun()
{
  if (isAnotherRunning())
  { // Extra check
    return false;
  }

  m_memLock.acquire();
  const auto result = m_sharedMem.create(sizeof(quint64));
  m_memLock.release();

  if (!result || !m_server.start())
  {
    release();
    return false;
  }

  qInfo() << "Listening for connections at " << m_server.serverName();

  connect(&m_server, &CmdServer::commandReceived, [&](const auto& receivedCmd) {
    emit commandReceived(receivedCmd);
  });

  return true;
}

void RunGuard::sendCommandToMainInstance(const QString& cmd)
{
  qInfo() << "Sending command " << cmd << " to main instance: " << m_server.serverName();

  sendCommand(SocketName, cmd).or_else([](const auto& error) {
    qWarning() << "Could not connect to main instance: "
               << QString::fromStdString(error.msg);
    return kdl::void_success;
  });
}

void RunGuard::release()
{
  m_memLock.acquire();
  if (m_sharedMem.isAttached())
  {
    m_sharedMem.detach();
  }
  m_memLock.release();
}

} // namespace TrenchBroom::View

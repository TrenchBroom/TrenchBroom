/*
 Copyright (C) 2026 Kristian Duske

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

#include "singleinstance/SingleInstance.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDeadlineTimer>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QThread>
#include <QtGlobal>

#include <algorithm>
#include <optional>
#include <ostream>

#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace tb::singleinstance
{
namespace
{

using namespace std::chrono_literals;

// how long to wait for a single connection attempt
constexpr auto ConnectTimeout = 100ms;

// how long to wait between attempts to establish a role
constexpr auto RetryInterval = 50ms;

// messages larger than this are discarded
constexpr auto MaxMessageSize = qint64(1024 * 1024);

constexpr auto ArgumentsKey = QLatin1StringView{"arguments"};

int remainingMsecs(const QDeadlineTimer& deadline)
{
  return int(std::max(deadline.remainingTime(), qint64{0}));
}

std::unique_ptr<QLockFile> tryLock(const QString& key)
{
  auto lockFile =
    std::make_unique<QLockFile>(QDir{QDir::tempPath()}.filePath(key + ".lock"));

  // only consider the lock stale if the process that holds it is not running anymore
  lockFile->setStaleLockTime(0);

  return lockFile->tryLock(0) ? std::move(lockFile) : nullptr;
}

std::unique_ptr<QLocalSocket> tryConnect(const QString& key)
{
  auto socket = std::make_unique<QLocalSocket>();
  socket->connectToServer(key);
  return socket->waitForConnected(int(ConnectTimeout.count())) ? std::move(socket)
                                                               : nullptr;
}

QByteArray encodeMessage(const QStringList& arguments)
{
  const auto message =
    QJsonObject{{QString{ArgumentsKey}, QJsonArray::fromStringList(arguments)}};
  return QJsonDocument{message}.toJson(QJsonDocument::Compact) + '\n';
}

std::optional<QStringList> decodeMessage(const QByteArray& data)
{
  auto error = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject())
  {
    return std::nullopt;
  }

  const auto argumentsValue = document.object().value(ArgumentsKey);
  if (!argumentsValue.isArray())
  {
    return std::nullopt;
  }

  auto arguments = QStringList{};
  for (const auto& argument : argumentsValue.toArray())
  {
    if (!argument.isString())
    {
      return std::nullopt;
    }
    arguments.push_back(argument.toString());
  }
  return arguments;
}

bool writeAll(
  QLocalSocket& socket, const QByteArray& data, const QDeadlineTimer& deadline)
{
  if (socket.write(data) != data.size())
  {
    return false;
  }

  while (socket.bytesToWrite() > 0)
  {
    if (
      !socket.waitForBytesWritten(remainingMsecs(deadline)) && socket.bytesToWrite() > 0)
    {
      return false;
    }
  }

  return true;
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const Role rhs)
{
  switch (rhs)
  {
  case Role::Primary:
    lhs << "Primary";
    break;
  case Role::Secondary:
    lhs << "Secondary";
    break;
  case Role::Standalone:
    lhs << "Standalone";
    break;
  }
  return lhs;
}

SingleInstance::SingleInstance(
  QString key, const std::chrono::milliseconds electionTimeout, QObject* parent)
  : QObject{parent}
  , m_key{std::move(key)}
{
  const auto deadline = QDeadlineTimer{electionTimeout};
  while (true)
  {
    if (auto lockFile = tryLock(m_key))
    {
      m_role = listen(std::move(lockFile)) ? Role::Primary : Role::Standalone;
      return;
    }

    // Another instance holds the lock, but it may not be listening yet, or it may be
    // terminating and release the lock soon.
    if (auto socket = tryConnect(m_key))
    {
      m_socket = std::move(socket);
      m_role = Role::Secondary;
      return;
    }

    if (deadline.hasExpired())
    {
      break;
    }

    QThread::msleep(static_cast<unsigned long>(
      std::min(int(RetryInterval.count()), remainingMsecs(deadline))));
  }

  m_role = Role::Standalone;
}

SingleInstance::~SingleInstance()
{
  // Stop accepting connections before releasing the lock so that no other instance can
  // connect to this instance after it has become the primary instance.
  m_server.reset();
  m_lockFile.reset();
}

Role SingleInstance::role() const
{
  return m_role;
}

bool SingleInstance::sendMessage(
  const QStringList& arguments, const std::chrono::milliseconds timeout)
{
  if (m_role != Role::Secondary || !m_socket)
  {
    return false;
  }

#if defined(Q_OS_WIN)
  // Windows only allows a process to bring its windows to the foreground if it was
  // started by the foreground process or if the foreground process allows it.
  AllowSetForegroundWindow(ASFW_ANY);
#endif

  const auto deadline = QDeadlineTimer{timeout};
  if (!writeAll(*m_socket, encodeMessage(arguments), deadline))
  {
    return false;
  }

  // Wait for the acknowledgement so that the connection isn't closed before the primary
  // instance has read the message. If the primary instance is busy, it will still read
  // the message later.
  while (!m_socket->canReadLine() && m_socket->state() == QLocalSocket::ConnectedState)
  {
    if (!m_socket->waitForReadyRead(remainingMsecs(deadline)))
    {
      break;
    }
  }

  m_socket->disconnectFromServer();
  m_socket.reset();
  return true;
}

bool SingleInstance::listen(std::unique_ptr<QLockFile> lockFile)
{
  auto server = std::make_unique<QLocalServer>();
  server->setSocketOptions(QLocalServer::UserAccessOption);

  // Holding the lock implies that a remaining socket file was left by a crashed process.
  QLocalServer::removeServer(m_key);

  if (!server->listen(m_key))
  {
    qWarning() << "Failed to listen on" << m_key << ":" << server->errorString();
    return false;
  }

  m_lockFile = std::move(lockFile);
  m_server = std::move(server);

  connect(
    m_server.get(),
    &QLocalServer::newConnection,
    this,
    &SingleInstance::acceptConnections);

  if (auto* app = QCoreApplication::instance())
  {
    // Messages that arrive while the application quits would be lost, so stop accepting
    // connections. Another instance becomes the primary instance once this instance has
    // terminated.
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() {
      if (m_server)
      {
        m_server->close();
      }
    });
  }

  return true;
}

void SingleInstance::acceptConnections()
{
  while (auto* socket = m_server->nextPendingConnection())
  {
    connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
      readMessages(*socket);
    });
    connect(socket, &QLocalSocket::disconnected, this, [this, socket]() {
      // read any messages that arrived together with the disconnection
      readMessages(*socket);
      socket->deleteLater();
    });

    readMessages(*socket);
  }
}

void SingleInstance::readMessages(QLocalSocket& socket)
{
  while (socket.canReadLine())
  {
    const auto line = socket.readLine(MaxMessageSize + 1);
    if (const auto arguments = decodeMessage(line))
    {
      socket.write("\n");
      socket.flush();
      emit messageReceived(*arguments);
    }
    else
    {
      qWarning() << "Discarding malformed message on" << m_key;
      socket.abort();
      return;
    }
  }

  if (socket.bytesAvailable() > MaxMessageSize)
  {
    qWarning() << "Discarding oversized message on" << m_key;
    socket.abort();
  }
}

QString currentUserId()
{
#if defined(Q_OS_WIN)
  return qEnvironmentVariable("USERDOMAIN") + "\\" + qEnvironmentVariable("USERNAME");
#else
  return QString::number(::getuid());
#endif
}

QString makeKey(
  const QStringView appName, const QStringView userId, const QStringList& scope)
{
  auto components = QJsonArray{appName.toString(), userId.toString()};
  for (const auto& component : scope)
  {
    components.push_back(component);
  }

  const auto hash = QCryptographicHash::hash(
    QJsonDocument{components}.toJson(QJsonDocument::Compact), QCryptographicHash::Sha1);

  auto prefix = QString{};
  for (const auto c : appName)
  {
    if (prefix.size() < 32 && c.unicode() < 128 && c.isLetterOrNumber())
    {
      prefix.push_back(c);
    }
  }

  const auto hashStr = QString::fromLatin1(hash.toHex().left(16));
  return prefix.isEmpty() ? hashStr : prefix + "-" + hashStr;
}

QString makeKey(const QStringView appName, const QStringList& scope)
{
  return makeKey(appName, currentUserId(), scope);
}

} // namespace tb::singleinstance

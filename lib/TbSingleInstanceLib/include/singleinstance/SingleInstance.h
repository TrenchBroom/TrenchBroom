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

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QStringView>

#include <chrono>
#include <iosfwd>
#include <memory>

class QLocalServer;
class QLocalSocket;
class QLockFile;

namespace tb::singleinstance
{

enum class Role
{
  /**
   * This instance is the only one running. It receives messages sent by other
   * instances.
   */
  Primary,
  /**
   * Another instance is running and this instance is connected to it. It should send
   * its message and terminate.
   */
  Secondary,
  /**
   * Neither role could be established, e.g. because another instance holds the lock,
   * but doesn't accept connections. This instance should run on its own.
   */
  Standalone,
};

std::ostream& operator<<(std::ostream& lhs, Role rhs);

/**
 * Ensures that only one instance of an application runs at a time.
 *
 * All instances that are created with the same key compete for the primary role. The
 * first one becomes the primary instance and receives the messages that are sent by any
 * later instances, which become secondary instances.
 *
 * The primary role is determined by a lock file in the temporary directory, and the
 * messages are sent using a local socket (a named pipe on Windows). The lock file is
 * released when the primary instance is destroyed or its process terminates.
 */
class SingleInstance : public QObject
{
  Q_OBJECT
private:
  QString m_key;
  Role m_role = Role::Standalone;
  std::unique_ptr<QLockFile> m_lockFile;
  std::unique_ptr<QLocalServer> m_server;
  std::unique_ptr<QLocalSocket> m_socket;

public:
  /**
   * Determines the role of this instance.
   *
   * If another instance holds the lock, but doesn't accept connections yet, this keeps
   * trying to establish a role until the given timeout has expired. If no role could be
   * established by then, the role of this instance is Role::Standalone.
   *
   * On Unix, the local socket is created in the temporary directory, and its path must
   * not exceed 103 bytes on macOS and 107 bytes on Linux. Keys created by makeKey are
   * short enough for common temporary directories.
   *
   * @param key the name of the lock file and the local socket, see makeKey
   * @param electionTimeout how long to keep trying to establish a role
   * @param parent the parent object
   */
  SingleInstance(
    QString key, std::chrono::milliseconds electionTimeout, QObject* parent = nullptr);
  ~SingleInstance() override;

  Role role() const;

  /**
   * Sends the given arguments to the primary instance.
   *
   * Only a secondary instance can send messages. The message counts as delivered once it
   * has been written to the primary instance, even if the primary instance doesn't
   * acknowledge it within the timeout.
   *
   * @param arguments the arguments to send
   * @param timeout how long to wait for the primary instance to receive the message
   * @return true if the message was delivered and false otherwise
   */
  bool sendMessage(const QStringList& arguments, std::chrono::milliseconds timeout);

signals:
  /**
   * Emitted by the primary instance when it receives a message from another instance.
   */
  void messageReceived(const QStringList& arguments);

private:
  bool listen(std::unique_ptr<QLockFile> lockFile);
  void acceptConnections();
  void readMessages(QLocalSocket& socket);
};

/**
 * Returns an identifier of the user running this process.
 */
QString currentUserId();

/**
 * Creates a key that can be passed to SingleInstance.
 *
 * The key begins with the ASCII letters and digits of the given application name,
 * followed by a hash of the application name, the given user ID and the given scope.
 * Instances with different keys don't affect each other, so the scope should contain
 * anything that should distinguish separate instances of the application, e.g. the
 * canonical path to the application's executable.
 *
 * The user ID must be part of the key because named pipes are not separated per user on
 * Windows.
 *
 * @param appName the name of the application
 * @param userId the user ID, see currentUserId
 * @param scope any additional strings that distinguish separate instances
 */
QString makeKey(QStringView appName, QStringView userId, const QStringList& scope);

/**
 * Creates a key for the current user, see makeKey and currentUserId.
 */
QString makeKey(QStringView appName, const QStringList& scope);

} // namespace tb::singleinstance

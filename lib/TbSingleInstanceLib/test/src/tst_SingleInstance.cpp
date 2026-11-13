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

#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QDir>
#include <QLocalSocket>
#include <QLockFile>
#include <QThread>
#include <QUuid>
#include <QtTest/QSignalSpy>

#include "singleinstance/SingleInstance.h"

#include <chrono>
#include <future>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::singleinstance
{
namespace
{

using namespace std::chrono_literals;

constexpr auto Timeout = 5s;

QString makeUniqueKey()
{
  // keep the key short, as the socket path must fit into 104 bytes on macOS
  return "TbSiTest-" + QUuid::createUuid().toString(QUuid::Id128);
}

/**
 * Processes events until the given future is ready, so that a primary instance on this
 * thread can serve a secondary instance on another thread.
 */
template <typename T>
T waitFor(std::future<T>& future)
{
  const auto deadline = QDeadlineTimer{Timeout};
  while (future.wait_for(0ms) != std::future_status::ready && !deadline.hasExpired())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
  }
  return future.get();
}

/**
 * Creates a secondary instance on another thread and sends the given arguments from it.
 */
std::future<bool> sendFromSecondary(const QString& key, const QStringList& arguments)
{
  return std::async(std::launch::async, [key, arguments]() {
    auto secondary = SingleInstance{key, Timeout};
    return secondary.role() == Role::Secondary
           && secondary.sendMessage(arguments, Timeout);
  });
}

/**
 * Connects to the primary instance on another thread and writes the given data.
 */
std::future<void> sendRaw(const QString& key, const QByteArray& data)
{
  return std::async(std::launch::async, [key, data]() {
    auto socket = QLocalSocket{};
    socket.connectToServer(key);
    if (socket.waitForConnected(int(Timeout / 1ms)))
    {
      socket.write(data);
      while (socket.bytesToWrite() > 0 && socket.waitForBytesWritten(int(Timeout / 1ms)))
      {
      }
      socket.waitForDisconnected(int(Timeout / 1ms));
    }
  });
}

} // namespace

TEST_CASE("SingleInstance")
{
  auto argc = 0;
  auto app = QCoreApplication{argc, nullptr};

  const auto key = makeUniqueKey();

  SECTION("SingleInstance")
  {
    SECTION("The first instance becomes the primary instance")
    {
      auto primary = SingleInstance{key, 0ms};
      CHECK(primary.role() == Role::Primary);

      SECTION("Another instance becomes a secondary instance")
      {
        auto secondary = SingleInstance{key, Timeout};
        CHECK(secondary.role() == Role::Secondary);
      }

      SECTION("An instance with a different key becomes a primary instance")
      {
        auto other = SingleInstance{makeUniqueKey(), 0ms};
        CHECK(other.role() == Role::Primary);
      }
    }

    SECTION("Another instance becomes primary after the primary instance is destroyed")
    {
      {
        auto primary = SingleInstance{key, 0ms};
        REQUIRE(primary.role() == Role::Primary);
      }

      auto next = SingleInstance{key, 0ms};
      CHECK(next.role() == Role::Primary);
    }

    SECTION(
      "An instance becomes standalone if the lock is held by an unreachable "
      "instance")
    {
      auto lockFile = QLockFile{QDir{QDir::tempPath()}.filePath(key + ".lock")};
      REQUIRE(lockFile.tryLock(0));

      auto instance = SingleInstance{key, 200ms};
      CHECK(instance.role() == Role::Standalone);
    }
  }

  SECTION("sendMessage")
  {
    auto primary = SingleInstance{key, 0ms};
    REQUIRE(primary.role() == Role::Primary);

    auto spy = QSignalSpy{&primary, &SingleInstance::messageReceived};

    SECTION("The primary instance receives the arguments")
    {
      const auto arguments = GENERATE(
        QStringList{},
        QStringList{"a"},
        QStringList{"C:\\maps\\some map.map", "/home/user/ümlaut.map", "line\nbreak"});

      auto delivered = sendFromSecondary(key, arguments);
      CHECK(waitFor(delivered));

      REQUIRE(spy.count() == 1);
      CHECK(spy.at(0).at(0).toStringList() == arguments);
    }

    SECTION("The primary instance receives the arguments when it is busy at first")
    {
      auto delivered = sendFromSecondary(key, {"a"});
      QThread::msleep(500);

      CHECK(waitFor(delivered));

      REQUIRE(spy.count() == 1);
      CHECK(spy.at(0).at(0).toStringList() == QStringList{"a"});
    }

    SECTION("The primary instance receives arguments from several instances")
    {
      auto delivered1 = sendFromSecondary(key, {"a"});
      auto delivered2 = sendFromSecondary(key, {"b"});

      CHECK(waitFor(delivered1));
      CHECK(waitFor(delivered2));

      REQUIRE(spy.count() == 2);

      auto received = QStringList{};
      for (const auto& signal : spy)
      {
        received += signal.at(0).toStringList();
      }
      received.sort();
      CHECK(received == QStringList{"a", "b"});
    }

    SECTION("The primary instance discards invalid messages")
    {
      const auto data = GENERATE(
        QByteArray{"not json\n"},
        QByteArray{"[\"a\"]\n"},
        QByteArray{"{\"arguments\":\"a\"}\n"},
        QByteArray{"{\"arguments\":[1]}\n"},
        QByteArray(2 * 1024 * 1024, 'a'));

      auto sent = sendRaw(key, data);
      waitFor(sent);
      CHECK(spy.count() == 0);

      // the primary instance keeps working
      auto delivered = sendFromSecondary(key, {"a"});
      CHECK(waitFor(delivered));

      REQUIRE(spy.count() == 1);
      CHECK(spy.at(0).at(0).toStringList() == QStringList{"a"});
    }

    SECTION("Only a secondary instance can send messages")
    {
      CHECK_FALSE(primary.sendMessage({"a"}, Timeout));
    }
  }
}

TEST_CASE("SingleInstance functions")
{
  SECTION("currentUserId")
  {
    CHECK(!currentUserId().isEmpty());
    CHECK(currentUserId() == currentUserId());
  }

  SECTION("makeKey")
  {
    const auto key = makeKey(u"TrenchBroom", u"user", {"C:/path/TrenchBroom.exe"});
    CHECK(key.startsWith("TrenchBroom-"));
    CHECK(key == makeKey(u"TrenchBroom", u"user", {"C:/path/TrenchBroom.exe"}));

    CHECK(key != makeKey(u"TrenchBroom2", u"user", {"C:/path/TrenchBroom.exe"}));
    CHECK(key != makeKey(u"TrenchBroom", u"other", {"C:/path/TrenchBroom.exe"}));
    CHECK(key != makeKey(u"TrenchBroom", u"user", {"C:/other/TrenchBroom.exe"}));
    CHECK(key != makeKey(u"TrenchBroom", u"user", {"C:/path/TrenchBroom.exe", ""}));
    CHECK(key != makeKey(u"TrenchBroom", u"user", {}));

    // The components are separated unambiguously
    CHECK(makeKey(u"a", u"b", {"c"}) != makeKey(u"a", u"bc", {}));

    // Only ASCII letters and digits are used from the application name
    CHECK(makeKey(u"Trench Bröom/1", u"user", {}).startsWith("TrenchBrom1-"));
    CHECK(!makeKey(QStringView{}, u"user", {}).isEmpty());
    CHECK(!makeKey(QStringView{}, u"user", {}).startsWith("-"));
    CHECK(makeKey(u"!!!", u"user", {}) != makeKey(u"???", u"user", {}));

    // The key is short enough for Unix socket paths
    CHECK(makeKey(QString(1000, 'a'), u"user", {QString(1000, 'b')}).size() <= 64);

    CHECK(
      makeKey(u"TrenchBroom", {"x"}) == makeKey(u"TrenchBroom", currentUserId(), {"x"}));
  }
}

} // namespace tb::singleinstance

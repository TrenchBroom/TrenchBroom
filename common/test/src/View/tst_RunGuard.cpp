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

#include <QCoreApplication>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include "View/RunGuard.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

namespace
{
bool isSocketInUse(const QString& serverName)
{
  auto server = QLocalServer{};
  return !server.listen(serverName);
}
} // namespace

TEST_CASE("CmdServer")
{
  static const auto ServerName = "TestServer";
  SECTION("start")
  {
    auto server = CmdServer{ServerName, CmdServerMode::Deferred};
    CHECK(server.start());
    CHECK(server.start());

    SECTION("second instance always works")
    {
      auto other = CmdServer{ServerName, CmdServerMode::Deferred};
      CHECK(other.start());
    }
  }

#ifndef _WIN32
  SECTION("stop")
  {
    auto server = CmdServer{ServerName, CmdServerMode::Deferred};
    REQUIRE(server.start());
    REQUIRE(isSocketInUse(ServerName));

    server.stop();
    CHECK(!isSocketInUse(ServerName));
  }
#endif

  SECTION("sendCommand")
  {
    auto server = CmdServer{ServerName, CmdServerMode::Deferred};

    auto lastCommand = QString{};
    QObject::connect(&server, &CmdServer::commandReceived, [&](const auto& command) {
      lastCommand = command;
    });

    REQUIRE(server.start());
    CAPTURE(server.serverName());

    REQUIRE(sendCommand(ServerName, "test command") == kdl::result<void, CmdError>{});
    REQUIRE(server.processCommand());

    CHECK(lastCommand == "test command");
  }
}
} // namespace TrenchBroom::View

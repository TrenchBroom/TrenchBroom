/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTest>
#include <QUuid>

#include "ui/CatchConfig.h"
#include "ui/LocalJsonRpcServer.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

class TestRequestHandler : public JsonRpcRequestHandler
{
public:
  struct Request
  {
    QString method;
    QJsonValue params;
  };

  std::vector<Request> requests;

  JsonRpcResponse handleRequest(const QString& method, const QJsonValue& params) override
  {
    requests.push_back({method, params});

    if (method == "missing")
    {
      return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
    }
    if (method == "invalidParams")
    {
      return JsonRpcResponse::error({JsonRpcError::InvalidParams, "Invalid params"});
    }
    if (method == "fail")
    {
      throw std::runtime_error{"test failure"};
    }

    return JsonRpcResponse::success(params);
  }
};

QString uniqueServerName()
{
  // QLocalServer uses a Unix-domain socket on macOS; keep the name below the
  // platform's socket-path limit even when TMPDIR itself is long.
  return "tb-rpc-" + QUuid::createUuid().toRfc4122().toHex().left(12);
}

std::unique_ptr<QLocalSocket> connectClient(const QString& serverName)
{
  auto socket = std::make_unique<QLocalSocket>();
  socket->connectToServer(serverName);
  REQUIRE(socket->waitForConnected(1000));
  return socket;
}

QJsonObject readResponse(QLocalSocket& socket)
{
  auto timeout = QElapsedTimer{};
  timeout.start();
  while (!socket.canReadLine() && timeout.elapsed() < 1000)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    QTest::qWait(1);
  }

  REQUIRE(socket.canReadLine());
  auto parseError = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(socket.readLine().trimmed(), &parseError);
  REQUIRE(parseError.error == QJsonParseError::NoError);
  REQUIRE(document.isObject());
  return document.object();
}

void writeMessage(QLocalSocket& socket, const QByteArray& message)
{
  REQUIRE(socket.write(message) == message.size());
  REQUIRE(socket.waitForBytesWritten(1000));
  QTest::qWait(10);
}

bool waitForClientCount(const LocalJsonRpcServer& server, const qsizetype count)
{
  auto timeout = QElapsedTimer{};
  timeout.start();
  while (server.clientCount() != count && timeout.elapsed() < 1000)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    QTest::qWait(1);
  }

  return server.clientCount() == count;
}

} // namespace

TEST_CASE("LocalJsonRpcServer")
{
  const auto serverName = uniqueServerName();
  QLocalServer::removeServer(serverName);

  auto handler = TestRequestHandler{};
  auto server = LocalJsonRpcServer{&handler};
  const auto listenSuccessful = server.listen(serverName);
  INFO(server.errorString());
  REQUIRE(listenSuccessful);

  SECTION("processes compact requests split across writes")
  {
    auto client = connectClient(serverName);
    REQUIRE(waitForClientCount(server, 1));

    writeMessage(*client, R"({"jsonrpc":"2.0","method":"echo",)");
    QTest::qWait(20);
    CHECK(handler.requests.empty());

    writeMessage(
      *client,
      R"("params":{"value":42},"id":"request-1"})"
      "\n");

    const auto response = readResponse(*client);
    CHECK(response.value("jsonrpc") == "2.0");
    CHECK(response.value("id") == "request-1");
    CHECK(response.value("result") == QJsonObject{{"value", 42}});
    REQUIRE(handler.requests.size() == 1u);
    CHECK(handler.requests.front().method == "echo");
  }

  SECTION("serves multiple clients and multiple messages in a single write")
  {
    auto firstClient = connectClient(serverName);
    auto secondClient = connectClient(serverName);
    REQUIRE(waitForClientCount(server, 2));

    writeMessage(
      *firstClient,
      R"({"jsonrpc":"2.0","method":"echo","params":[1],"id":1})"
      "\n"
      R"({"jsonrpc":"2.0","method":"echo","params":[2],"id":2})"
      "\n");
    writeMessage(
      *secondClient,
      R"({"jsonrpc":"2.0","method":"echo","params":[3],"id":3})"
      "\n");

    CHECK(readResponse(*firstClient).value("result") == QJsonArray{1});
    CHECK(readResponse(*firstClient).value("result") == QJsonArray{2});
    CHECK(readResponse(*secondClient).value("result") == QJsonArray{3});
    CHECK(server.clientCount() == 2);
    CHECK(handler.requests.size() == 3u);
  }

  SECTION("returns standard errors for malformed and invalid requests")
  {
    auto client = connectClient(serverName);

    writeMessage(*client, "not json\n");
    auto response = readResponse(*client);
    CHECK(response.value("id").isNull());
    CHECK(response.value("error").toObject().value("code") == JsonRpcError::ParseError);

    writeMessage(
      *client,
      R"({"jsonrpc":"1.0","method":"echo","id":7})"
      "\n");
    response = readResponse(*client);
    CHECK(response.value("id").isNull());
    CHECK(
      response.value("error").toObject().value("code") == JsonRpcError::InvalidRequest);

    writeMessage(
      *client,
      R"({"jsonrpc":"2.0","method":"missing","id":8})"
      "\n");
    response = readResponse(*client);
    CHECK(response.value("id") == 8);
    CHECK(
      response.value("error").toObject().value("code") == JsonRpcError::MethodNotFound);

    writeMessage(
      *client,
      R"({"jsonrpc":"2.0","method":"invalidParams","id":9})"
      "\n");
    response = readResponse(*client);
    CHECK(response.value("id") == 9);
    CHECK(
      response.value("error").toObject().value("code") == JsonRpcError::InvalidParams);

    writeMessage(
      *client,
      R"({"jsonrpc":"2.0","method":"fail","id":10})"
      "\n");
    response = readResponse(*client);
    CHECK(response.value("id") == 10);
    CHECK(
      response.value("error").toObject().value("code") == JsonRpcError::InternalError);
  }

  SECTION("dispatches notifications without sending a response")
  {
    auto client = connectClient(serverName);

    writeMessage(
      *client,
      R"({"jsonrpc":"2.0","method":"echo","params":{"notification":true}})"
      "\n");
    QTest::qWait(50);

    REQUIRE(handler.requests.size() == 1u);
    CHECK(handler.requests.front().method == "echo");
    CHECK(handler.requests.front().params == QJsonObject{{"notification", true}});
    CHECK(client->bytesAvailable() == 0);
  }

  SECTION("closes connected clients and can release its endpoint")
  {
    auto client = connectClient(serverName);
    REQUIRE(waitForClientCount(server, 1));

    server.close();

    CHECK_FALSE(server.isListening());
    CHECK(server.clientCount() == 0);
    REQUIRE(client->waitForDisconnected(1000));

    auto replacement = LocalJsonRpcServer{};
    CHECK(replacement.listen(serverName));
  }

  server.close();
  QLocalServer::removeServer(serverName);
}

} // namespace tb::ui

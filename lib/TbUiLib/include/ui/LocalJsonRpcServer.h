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

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <memory>
#include <optional>
#include <vector>

class QLocalServer;
class QLocalSocket;

namespace tb::ui
{

/**
 * A JSON-RPC 2.0 error returned by a request handler.
 *
 * The standard error codes are provided as named constants, but handlers may return
 * application-specific errors in the -32000 to -32099 reserved-server-error range.
 */
struct JsonRpcError
{
  static constexpr int ParseError = -32700;
  static constexpr int InvalidRequest = -32600;
  static constexpr int MethodNotFound = -32601;
  static constexpr int InvalidParams = -32602;
  static constexpr int InternalError = -32603;

  JsonRpcError(int code, QString message, std::optional<QJsonValue> data = std::nullopt);

  int code;
  QString message;
  std::optional<QJsonValue> data;
};

/** A successful result or an error returned by a JSON-RPC request handler. */
class JsonRpcResponse
{
public:
  static JsonRpcResponse success(QJsonValue result = QJsonValue{QJsonValue::Null});
  static JsonRpcResponse error(JsonRpcError error);

  bool isSuccess() const;
  const QJsonValue& result() const;
  const JsonRpcError& error() const;

private:
  explicit JsonRpcResponse(QJsonValue result);
  explicit JsonRpcResponse(JsonRpcError error);

private:
  std::optional<QJsonValue> m_result;
  std::optional<JsonRpcError> m_error;
};

/**
 * Implemented by the service which owns the JSON-RPC methods.
 *
 * A handler is not owned by LocalJsonRpcServer and must outlive it, or be removed by
 * calling setRequestHandler(nullptr) before it is destroyed.
 */
class JsonRpcRequestHandler
{
public:
  virtual ~JsonRpcRequestHandler() = default;

  virtual JsonRpcResponse handleRequest(
    const QString& method, const QJsonValue& params) = 0;
};

/**
 * A local, newline-delimited JSON-RPC 2.0 server.
 *
 * Each line received through a QLocalSocket is a compact JSON request. The server
 * accepts multiple concurrent clients and sends one compact JSON response followed by
 * a newline for each request. Notifications (requests without an id) are dispatched
 * but never receive a response.
 */
class LocalJsonRpcServer : public QObject
{
public:
  explicit LocalJsonRpcServer(QObject* parent = nullptr);
  LocalJsonRpcServer(JsonRpcRequestHandler* requestHandler, QObject* parent = nullptr);
  ~LocalJsonRpcServer() override;

  LocalJsonRpcServer(const LocalJsonRpcServer&) = delete;
  LocalJsonRpcServer& operator=(const LocalJsonRpcServer&) = delete;

  void setRequestHandler(JsonRpcRequestHandler* requestHandler);

  bool listen(const QString& serverName);
  void close();

  bool isListening() const;
  QString serverName() const;
  QString errorString() const;
  qsizetype clientCount() const;

private:
  struct Client;

  void acceptNewConnection();
  void readClient(QLocalSocket* socket);
  void handleMessage(QLocalSocket* socket, const QByteArray& message);
  void removeClient(QLocalSocket* socket);
  void sendResponse(QLocalSocket* socket, const QJsonObject& response);

private:
  JsonRpcRequestHandler* m_requestHandler = nullptr;
  std::unique_ptr<QLocalServer> m_server;
  std::vector<std::unique_ptr<Client>> m_clients;
};

} // namespace tb::ui

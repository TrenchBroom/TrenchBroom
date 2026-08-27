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

#include "ui/LocalJsonRpcServer.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>

#include <algorithm>
#include <exception>
#include <utility>

namespace tb::ui
{
namespace
{

constexpr auto MaxMessageBytes = 64 * 1024 * 1024;

QJsonObject makeErrorResponse(const QJsonValue& id, const JsonRpcError& error)
{
  auto errorObject = QJsonObject{{"code", error.code}, {"message", error.message}};
  if (error.data)
  {
    errorObject.insert("data", *error.data);
  }

  return QJsonObject{{"jsonrpc", "2.0"}, {"id", id}, {"error", errorObject}};
}

QJsonObject makeSuccessResponse(const QJsonValue& id, const QJsonValue& result)
{
  return QJsonObject{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

bool isValidId(const QJsonValue& id)
{
  return id.isNull() || id.isString() || id.isDouble();
}

bool isValidParams(const QJsonValue& params)
{
  return params.isUndefined() || params.isArray() || params.isObject();
}

} // namespace

JsonRpcError::JsonRpcError(
  const int errorCode, QString errorMessage, std::optional<QJsonValue> errorData)
  : code{errorCode}
  , message{std::move(errorMessage)}
  , data{std::move(errorData)}
{
}

JsonRpcResponse JsonRpcResponse::success(QJsonValue result)
{
  return JsonRpcResponse{std::move(result)};
}

JsonRpcResponse JsonRpcResponse::error(JsonRpcError error)
{
  return JsonRpcResponse{std::move(error)};
}

bool JsonRpcResponse::isSuccess() const
{
  return m_result.has_value();
}

const QJsonValue& JsonRpcResponse::result() const
{
  return *m_result;
}

const JsonRpcError& JsonRpcResponse::error() const
{
  return *m_error;
}

JsonRpcResponse::JsonRpcResponse(QJsonValue result)
  : m_result{std::move(result)}
{
}

JsonRpcResponse::JsonRpcResponse(JsonRpcError error)
  : m_error{std::move(error)}
{
}

struct LocalJsonRpcServer::Client
{
  explicit Client(QLocalSocket* clientSocket)
    : socket{clientSocket}
  {
  }

  QPointer<QLocalSocket> socket;
  QByteArray buffer;
};

LocalJsonRpcServer::LocalJsonRpcServer(QObject* parent)
  : QObject{parent}
  , m_server{std::make_unique<QLocalServer>()}
{
  connect(m_server.get(), &QLocalServer::newConnection, this, [this]() {
    acceptNewConnection();
  });
}

LocalJsonRpcServer::LocalJsonRpcServer(
  JsonRpcRequestHandler* requestHandler, QObject* parent)
  : LocalJsonRpcServer{parent}
{
  setRequestHandler(requestHandler);
}

LocalJsonRpcServer::~LocalJsonRpcServer()
{
  close();
}

void LocalJsonRpcServer::setRequestHandler(JsonRpcRequestHandler* requestHandler)
{
  m_requestHandler = requestHandler;
}

bool LocalJsonRpcServer::listen(const QString& serverName)
{
  close();
  m_server->setSocketOptions(QLocalServer::UserAccessOption);
  return m_server->listen(serverName);
}

void LocalJsonRpcServer::close()
{
  m_server->close();
  for (const auto& client : m_clients)
  {
    if (client->socket)
    {
      client->socket->disconnect(this);
      client->socket->disconnectFromServer();
      client->socket->deleteLater();
    }
  }
  m_clients.clear();
}

bool LocalJsonRpcServer::isListening() const
{
  return m_server->isListening();
}

QString LocalJsonRpcServer::serverName() const
{
  return m_server->serverName();
}

QString LocalJsonRpcServer::errorString() const
{
  return m_server->errorString();
}

qsizetype LocalJsonRpcServer::clientCount() const
{
  return static_cast<qsizetype>(m_clients.size());
}

void LocalJsonRpcServer::acceptNewConnection()
{
  while (m_server->hasPendingConnections())
  {
    auto* socket = m_server->nextPendingConnection();
    socket->setParent(m_server.get());
    m_clients.push_back(std::make_unique<Client>(socket));

    connect(
      socket, &QLocalSocket::readyRead, this, [this, socket]() { readClient(socket); });
    connect(socket, &QLocalSocket::disconnected, this, [this, socket]() {
      removeClient(socket);
    });
  }
}

void LocalJsonRpcServer::readClient(QLocalSocket* socket)
{
  const auto clientIt =
    std::find_if(m_clients.begin(), m_clients.end(), [socket](const auto& client) {
      return client->socket == socket;
    });
  if (clientIt == m_clients.end())
  {
    return;
  }

  auto& buffer = (*clientIt)->buffer;
  buffer.append(socket->readAll());
  if (buffer.size() > MaxMessageBytes && !buffer.contains('\n'))
  {
    sendResponse(
      socket,
      makeErrorResponse(
        QJsonValue{QJsonValue::Null},
        {JsonRpcError::InvalidRequest, "Request exceeds 64 MiB limit"}));
    socket->disconnectFromServer();
    return;
  }

  while (true)
  {
    const auto newlineIndex = buffer.indexOf('\n');
    if (newlineIndex < 0)
    {
      return;
    }
    if (newlineIndex > MaxMessageBytes)
    {
      sendResponse(
        socket,
        makeErrorResponse(
          QJsonValue{QJsonValue::Null},
          {JsonRpcError::InvalidRequest, "Request exceeds 64 MiB limit"}));
      socket->disconnectFromServer();
      return;
    }

    const auto message = buffer.left(newlineIndex);
    buffer.remove(0, newlineIndex + 1);
    if (!message.trimmed().isEmpty())
    {
      handleMessage(socket, message);
    }
  }
}

void LocalJsonRpcServer::handleMessage(QLocalSocket* socket, const QByteArray& message)
{
  auto parseError = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(message, &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    sendResponse(
      socket,
      makeErrorResponse(
        QJsonValue{QJsonValue::Null}, {JsonRpcError::ParseError, "Parse error"}));
    return;
  }

  if (!document.isObject())
  {
    sendResponse(
      socket,
      makeErrorResponse(
        QJsonValue{QJsonValue::Null}, {JsonRpcError::InvalidRequest, "Invalid Request"}));
    return;
  }

  const auto request = document.object();
  const auto idIt = request.constFind("id");
  const auto isNotification = idIt == request.end();
  const auto id = isNotification ? QJsonValue{QJsonValue::Null} : *idIt;
  const auto method = request.value("method");
  const auto params = request.value("params");

  if (
    request.value("jsonrpc") != "2.0" || !method.isString() || !isValidId(id)
    || !isValidParams(params))
  {
    if (!isNotification)
    {
      sendResponse(
        socket,
        makeErrorResponse(
          QJsonValue{QJsonValue::Null},
          {JsonRpcError::InvalidRequest, "Invalid Request"}));
    }
    return;
  }

  if (m_requestHandler == nullptr)
  {
    if (!isNotification)
    {
      sendResponse(
        socket,
        makeErrorResponse(id, {JsonRpcError::MethodNotFound, "Method not found"}));
    }
    return;
  }

  try
  {
    const auto response = m_requestHandler->handleRequest(method.toString(), params);
    if (!isNotification)
    {
      sendResponse(
        socket,
        response.isSuccess() ? makeSuccessResponse(id, response.result())
                             : makeErrorResponse(id, response.error()));
    }
  }
  catch (const std::exception&)
  {
    if (!isNotification)
    {
      sendResponse(
        socket, makeErrorResponse(id, {JsonRpcError::InternalError, "Internal error"}));
    }
  }
  catch (...)
  {
    if (!isNotification)
    {
      sendResponse(
        socket, makeErrorResponse(id, {JsonRpcError::InternalError, "Internal error"}));
    }
  }
}

void LocalJsonRpcServer::removeClient(QLocalSocket* socket)
{
  socket->deleteLater();
  std::erase_if(
    m_clients, [socket](const auto& client) { return client->socket == socket; });
}

void LocalJsonRpcServer::sendResponse(QLocalSocket* socket, const QJsonObject& response)
{
  socket->write(QJsonDocument{response}.toJson(QJsonDocument::Compact));
  socket->write("\n");
  socket->flush();
}

} // namespace tb::ui

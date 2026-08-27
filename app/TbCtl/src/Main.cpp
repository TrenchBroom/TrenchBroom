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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <memory>
#include <optional>

namespace
{

enum class ExitCode
{
  Success = 0,
  Usage = 2,
  ConnectionFailed = 3,
  TransportError = 4,
  Timeout = 5,
  InvalidResponse = 6,
  RpcError = 7,
};

void printError(const QString& message)
{
  QTextStream{stderr} << "tbctl: " << message << '\n';
}

int remainingTimeout(const QElapsedTimer& timer, const int timeout)
{
  return std::max(0, timeout - static_cast<int>(timer.elapsed()));
}

std::optional<QJsonValue> parseParams(const QString& value, QString& error)
{
  auto parseError = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(value.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    error = "--params must be valid JSON: " + parseError.errorString();
    return std::nullopt;
  }

  if (document.isObject())
  {
    return document.object();
  }
  if (document.isArray())
  {
    return document.array();
  }

  error = "--params must be a JSON object or array";
  return std::nullopt;
}

std::optional<QString> readDiscoveryFile(const QString& path, QString& error)
{
  auto file = QFile{path};
  if (!file.open(QIODevice::ReadOnly))
  {
    error = "could not read discovery file '" + path + "'";
    return std::nullopt;
  }
  const auto document = QJsonDocument::fromJson(file.readAll());
  const auto socket = document.object().value("socket").toString();
  if (!document.isObject() || socket.isEmpty())
  {
    error = "discovery file does not contain a socket name: '" + path + "'";
    return std::nullopt;
  }
  return socket;
}

QStringList discoverSockets(QString& error)
{
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
  const auto directory = QDir::homePath() + "/.TrenchBroom/automation";
#else
  const auto directory =
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
    + "/TrenchBroom/automation";
#endif
  auto dir = QDir{directory};
  const auto files =
    dir.entryInfoList({"*.json"}, QDir::Files | QDir::Readable, QDir::Time);
  if (files.empty())
  {
    error =
      "no running TrenchBroom automation service was discovered in '" + directory + "'";
    return {};
  }
  auto result = QStringList{};
  for (const auto& file : files)
  {
    auto fileError = QString{};
    if (const auto socket = readDiscoveryFile(file.absoluteFilePath(), fileError))
    {
      result.push_back(*socket);
    }
  }
  if (result.empty())
  {
    error = "no valid TrenchBroom discovery files were found in '" + directory + "'";
  }
  return result;
}

std::optional<QJsonObject> readResponse(
  QLocalSocket& socket, const QElapsedTimer& timer, const int timeout, QString& error)
{
  auto buffer = QByteArray{};
  while (true)
  {
    buffer.append(socket.readAll());
    if (const auto newline = buffer.indexOf('\n'); newline >= 0)
    {
      auto parseError = QJsonParseError{};
      const auto document = QJsonDocument::fromJson(buffer.left(newline), &parseError);
      if (parseError.error != QJsonParseError::NoError || !document.isObject())
      {
        error = "received an invalid JSON-RPC response";
        return std::nullopt;
      }
      return document.object();
    }

    const auto remaining = remainingTimeout(timer, timeout);
    if (remaining == 0)
    {
      error = "timed out waiting for the JSON-RPC response";
      return std::nullopt;
    }
    if (!socket.waitForReadyRead(remaining))
    {
      error = socket.state() == QLocalSocket::UnconnectedState
                ? "connection closed before a JSON-RPC response was received"
                : "timed out waiting for the JSON-RPC response";
      return std::nullopt;
    }
  }
}

bool isValidResponse(const QJsonObject& response)
{
  const auto id = response.value("id");
  if (response.value("jsonrpc") != "2.0" || !id.isDouble() || id.toInt() != 1)
  {
    return false;
  }

  const auto hasResult = response.contains("result");
  const auto hasError = response.value("error").isObject();
  return hasResult != hasError;
}

} // namespace

int main(int argc, char* argv[])
{
  auto app = QCoreApplication{argc, argv};
  QCoreApplication::setApplicationName("tbctl");
  QCoreApplication::setApplicationVersion("1");

  auto parser = QCommandLineParser{};
  parser.setApplicationDescription("Send one JSON-RPC 2.0 request to TrenchBroom.");
  parser.addHelpOption();
  parser.addVersionOption();

  const auto socketOption = QCommandLineOption{
    "socket", "Local socket name (auto-discovered by default).", "name"};
  const auto discoveryOption = QCommandLineOption{
    "discovery", "Read the socket name from a discovery JSON file.", "path"};
  const auto methodOption =
    QCommandLineOption{"method", "Required JSON-RPC method name.", "name"};
  const auto paramsOption =
    QCommandLineOption{"params", "JSON object or array parameters.", "json", "{}"};
  const auto notificationOption = QCommandLineOption{
    "notification", "Send a JSON-RPC notification and do not wait for a response."};
  const auto timeoutOption = QCommandLineOption{
    "timeout",
    "Connection and response timeout in milliseconds.",
    "milliseconds",
    "5000"};
  const auto prettyOption = QCommandLineOption{
    "pretty", "Print an indented JSON response instead of compact JSON."};
  parser.addOption(socketOption);
  parser.addOption(discoveryOption);
  parser.addOption(methodOption);
  parser.addOption(paramsOption);
  parser.addOption(notificationOption);
  parser.addOption(timeoutOption);
  parser.addOption(prettyOption);

  if (!parser.parse(app.arguments()))
  {
    printError(parser.errorText());
    return static_cast<int>(ExitCode::Usage);
  }
  if (parser.isSet("help"))
  {
    parser.showHelp(static_cast<int>(ExitCode::Success));
  }
  if (parser.isSet("version"))
  {
    parser.showVersion();
  }

  auto socketNames = QStringList{};
  auto discoveryError = QString{};
  if (parser.isSet(socketOption))
  {
    socketNames.push_back(parser.value(socketOption));
  }
  else if (parser.isSet(discoveryOption))
  {
    const auto discovered =
      readDiscoveryFile(parser.value(discoveryOption), discoveryError);
    if (discovered)
    {
      socketNames.push_back(*discovered);
    }
  }
  else
  {
    socketNames = discoverSockets(discoveryError);
  }
  if (socketNames.empty())
  {
    printError(discoveryError);
    return static_cast<int>(ExitCode::ConnectionFailed);
  }

  const auto method = parser.value(methodOption);
  if (method.isEmpty())
  {
    printError("--method is required");
    return static_cast<int>(ExitCode::Usage);
  }

  auto timeoutOk = false;
  const auto timeout = parser.value(timeoutOption).toInt(&timeoutOk);
  if (!timeoutOk || timeout <= 0)
  {
    printError("--timeout must be a positive integer");
    return static_cast<int>(ExitCode::Usage);
  }

  auto paramsError = QString{};
  const auto params = parseParams(parser.value(paramsOption), paramsError);
  if (!params)
  {
    printError(paramsError);
    return static_cast<int>(ExitCode::Usage);
  }

  auto request = QJsonObject{{"jsonrpc", "2.0"}, {"method", method}, {"params", *params}};
  const auto notification = parser.isSet(notificationOption);
  if (!notification)
  {
    request.insert("id", 1);
  }

  auto timer = QElapsedTimer{};
  timer.start();
  auto socket = std::make_unique<QLocalSocket>();
  auto connectedName = QString{};
  for (qsizetype i = 0; i < socketNames.size(); ++i)
  {
    socket->connectToServer(socketNames[i]);
    const auto attemptTimeout = i + 1 == socketNames.size()
                                  ? remainingTimeout(timer, timeout)
                                  : std::min(250, remainingTimeout(timer, timeout));
    if (socket->waitForConnected(attemptTimeout))
    {
      connectedName = socketNames[i];
      break;
    }
    socket = std::make_unique<QLocalSocket>();
  }
  if (connectedName.isEmpty())
  {
    printError("could not connect to any discovered TrenchBroom instance");
    return static_cast<int>(ExitCode::ConnectionFailed);
  }

  const auto requestBytes = QJsonDocument{request}.toJson(QJsonDocument::Compact) + '\n';
  if (socket->write(requestBytes) != requestBytes.size())
  {
    printError("could not write the JSON-RPC request: " + socket->errorString());
    return static_cast<int>(ExitCode::TransportError);
  }
  if (!socket->waitForBytesWritten(remainingTimeout(timer, timeout)))
  {
    printError("could not finish writing the JSON-RPC request: " + socket->errorString());
    return static_cast<int>(ExitCode::TransportError);
  }

  if (notification)
  {
    return static_cast<int>(ExitCode::Success);
  }

  auto responseError = QString{};
  const auto response = readResponse(*socket, timer, timeout, responseError);
  if (!response)
  {
    printError(responseError);
    if (responseError.startsWith("timed out"))
    {
      return static_cast<int>(ExitCode::Timeout);
    }
    return responseError.startsWith("connection closed")
             ? static_cast<int>(ExitCode::TransportError)
             : static_cast<int>(ExitCode::InvalidResponse);
  }
  if (!isValidResponse(*response))
  {
    printError("received an invalid JSON-RPC response");
    return static_cast<int>(ExitCode::InvalidResponse);
  }

  const auto format =
    parser.isSet(prettyOption) ? QJsonDocument::Indented : QJsonDocument::Compact;
  QTextStream{stdout} << QJsonDocument{*response}.toJson(format);
  return response->contains("error") ? static_cast<int>(ExitCode::RpcError)
                                     : static_cast<int>(ExitCode::Success);
}

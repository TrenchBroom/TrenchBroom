/*
 Copyright (C) 2025 Kristian Duske

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

#include "QtHttpClient.h"

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>

#include <cassert>


namespace upd
{

namespace
{

auto formatError(const QNetworkReply& reply)
{
  return QString{"%1: %2"}.arg(reply.error()).arg(reply.errorString());
}

class QtHttpOperation : public HttpOperation
{
protected:
  QNetworkReply* m_reply;

  explicit QtHttpOperation(QNetworkReply* reply)
    : HttpOperation{reply}
    , m_reply{reply}
  {
  }

  void cancel() override { m_reply->abort(); }
};

class QtHttpGet : public QtHttpOperation
{
private:
  HttpClient::GetCallback m_getCallback;
  HttpClient::ErrorCallback m_errorCallback;

public:
  explicit QtHttpGet(
    QNetworkReply* reply,
    HttpClient::GetCallback getCallback,
    HttpClient::ErrorCallback errorCallback)
    : QtHttpOperation{reply}
    , m_getCallback{std::move(getCallback)}
    , m_errorCallback{std::move(errorCallback)}
  {
    connect(m_reply, &QNetworkReply::finished, [this] {
      m_reply->deleteLater();

      if (m_reply->error() == QNetworkReply::NoError)
      {
        m_getCallback(m_reply->readAll());
      }
      else if (m_reply->error() != QNetworkReply::OperationCanceledError)
      {
        m_errorCallback(formatError(*m_reply));
      }
    });
  }
};

class QtHttpDownload : public QtHttpOperation
{
private:
  HttpClient::DownloadCallback m_downloadCallback;
  HttpClient::ErrorCallback m_errorCallback;
  std::optional<float> m_progress;

  QTemporaryFile m_file;

public:
  explicit QtHttpDownload(
    QNetworkReply* reply,
    HttpClient::DownloadCallback downloadCallback,
    HttpClient::ErrorCallback errorCallback)
    : QtHttpOperation{reply}
    , m_downloadCallback{std::move(downloadCallback)}
    , m_errorCallback{std::move(errorCallback)}
  {
    const auto tempDirectory =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const auto fileInfo = QFileInfo{reply->url().fileName()};
    const auto baseName = fileInfo.baseName();
    const auto extension = fileInfo.suffix();

    m_file.setFileTemplate(
      QString{"%1/%2-XXXXXX.%3"}.arg(tempDirectory).arg(baseName).arg(extension));
    m_file.open();

    connect(m_reply, &QNetworkReply::finished, [this] {
      m_reply->deleteLater();
      m_file.close();

      if (m_reply->error() == QNetworkReply::NoError)
      {
        m_file.setAutoRemove(false);
        m_downloadCallback(m_file);
      }
      else if (m_reply->error() != QNetworkReply::OperationCanceledError)
      {
        m_errorCallback(formatError(*m_reply));
      }
    });
    connect(
      m_reply,
      &QNetworkReply::downloadProgress,
      [this](const qint64 bytesReceived, const qint64 bytesTotal) {
        m_progress = static_cast<float>(bytesReceived) / static_cast<float>(bytesTotal);
      });
    connect(m_reply, &QNetworkReply::readyRead, [this] {
      if (m_file.write(m_reply->readAll()) == -1)
      {
        m_reply->abort();
      }
    });
  }

  std::optional<float> progress() const override { return m_progress; }
};

} // namespace

QtHttpClient::QtHttpClient(QNetworkAccessManager& networkManager)
  : HttpClient{&networkManager}
  , m_networkManager{networkManager}
{
}

HttpOperation* QtHttpClient::get(
  const QUrl& url, GetCallback getCallback, ErrorCallback errorCallback) const
{
  return new QtHttpGet{
    m_networkManager.get(QNetworkRequest{url}),
    std::move(getCallback),
    std::move(errorCallback)};
}

HttpOperation* QtHttpClient::download(
  const QUrl& url, DownloadCallback downloadCallback, ErrorCallback errorCallback) const
{
  return new QtHttpDownload{
    m_networkManager.get(QNetworkRequest{url}),
    std::move(downloadCallback),
    std::move(errorCallback)};
}

} // namespace upd

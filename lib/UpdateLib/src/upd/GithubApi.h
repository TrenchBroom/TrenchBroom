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

#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QList>
#include <QString>

#include "upd/Asset.h"
#include "upd/HttpClient.h"
#include "upd/Release.h"
#include "upd/Version.h"

#include <algorithm>
#include <functional>
#include <variant>

namespace upd
{

namespace detail
{

struct Error
{
  QString msg;
};

std::variant<Asset, Error> parseAsset(const QJsonValue& json);
std::variant<QList<Asset>, Error> parseAssets(const QJsonValue& json);

template <typename Version>
std::variant<Release<Version>, Error> parseRelease(
  const QJsonValue& json, const ParseVersion<Version>& parseVersion)
{
  if (!json.isObject())
  {
    return Error{"invalid response body, expected object"};
  }

  const auto versionValue = json["tag_name"];
  const auto prereleaseValue = json["prerelease"];
  const auto draftValue = json["draft"];
  const auto nameValue = json["name"];
  const auto browserUrlValue = json["html_url"];
  const auto assetsValue = json["assets"];

  if (!versionValue.isString())
  {
    return Error{"invalid response body, expected string for 'tag_name'"};
  }

  if (!prereleaseValue.isBool())
  {
    return Error{"invalid response body, expected boolean for 'prerelease'"};
  }

  if (!draftValue.isBool())
  {
    return Error{"invalid response body, expected boolean for 'draft'"};
  }

  if (!nameValue.isString())
  {
    return Error{"invalid response body, expected string for 'name'"};
  }

  if (!browserUrlValue.isString())
  {
    return Error{"invalid response body, expected string for 'html_url'"};
  }

  if (!assetsValue.isArray())
  {
    return Error{"invalid response body, expected array for 'assets'"};
  }

  const auto version = parseVersion(versionValue.toString());
  if (!version)
  {
    return Error{"invalid response body, failed to parse version"};
  }

  const auto assets = parseAssets(assetsValue);
  if (const auto* error = std::get_if<Error>(&assets))
  {
    return *error;
  }

  return Release<Version>{
    *version,
    prereleaseValue.toBool(),
    draftValue.toBool(),
    nameValue.toString(),
    browserUrlValue.toString(),
    std::get<QList<Asset>>(assets),
  };
}

template <typename Version>
std::variant<QList<Release<Version>>, Error> parseReleases(
  const QJsonValue& json, const ParseVersion<Version>& parseVersion)
{
  if (!json.isArray())
  {
    return Error{"invalid response body, expected array"};
  }

  auto releases = QList<Release<Version>>{};
  for (const auto& releaseValue : json.toArray())
  {
    const auto release = parseRelease(releaseValue, parseVersion);
    if (const auto* error = std::get_if<Error>(&release))
    {
      return *error;
    }

    releases += std::get<Release<Version>>(release);
  }

  return releases;
}

template <typename Version>
std::variant<QList<Release<Version>>, Error> parseReleases(
  const QByteArray& str, const ParseVersion<Version>& parseVersion)
{
  auto error = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(str, &error);

  if (error.error != QJsonParseError::NoError)
  {
    return Error{error.errorString()};
  }

  if (!document.isArray())
  {
    return Error{"invalid response body, expected array"};
  }

  return parseReleases(document.array(), parseVersion);
}

} // namespace detail

template <typename Version>
using GetReleasesCallback = std::function<void(const QList<Release<Version>>&)>;

template <typename Version>
HttpOperation* getReleases(
  const HttpClient& httpClient,
  const QString& ghOrgName,
  const QString& ghRepoName,
  ParseVersion<Version> parseVersion_,
  GetReleasesCallback<Version> getReleasesCallback_,
  HttpClient::ErrorCallback errorCallback)
{
  const auto url =
    QUrl{QString{"https://api.github.com/repos/%1/%2/releases"}.arg(ghOrgName).arg(
      ghRepoName)};
  return httpClient.get(
    url,
    [parseVersion = std::move(parseVersion_),
     getReleasesCallback = std::move(getReleasesCallback_),
     errorCallback](auto body) {
      const auto releases = detail::parseReleases<Version>(body, parseVersion);
      if (const auto* error = std::get_if<detail::Error>(&releases))
      {
        errorCallback(error->msg);
      }
      else
      {
        getReleasesCallback(std::get<QList<Release<Version>>>(releases));
      }
    },
    errorCallback);
}

template <typename Version>
using GetLatestReleaseCallback =
  std::function<void(const std::optional<Release<Version>>&)>;

template <typename Version>
HttpOperation* getLatestRelease(
  const HttpClient& httpClient,
  const QString& ghOrgName,
  const QString& ghRepoName,
  const Version currentVersion,
  const bool includePreReleases,
  const bool includeDraftReleases,
  ParseVersion<Version> parseVersion,
  GetLatestReleaseCallback<Version> getLatestReleaseCallback_,
  HttpClient::ErrorCallback errorCallback)
{
  auto getReleasesCallback =
    [=, getLatestReleaseCallback = std::move(getLatestReleaseCallback_)](auto releases) {
      erase_if(releases, [&](const auto& release) {
        return (!includeDraftReleases && release.draft)
               || (!includePreReleases && release.prerelease);
      });

      if (auto iLatestRelease = std::ranges::max_element(releases);
          iLatestRelease != releases.end() && currentVersion < iLatestRelease->version)
      {
        getLatestReleaseCallback(*iLatestRelease);
      }
      else
      {
        getLatestReleaseCallback(std::nullopt);
      }
    };

  return getReleases<Version>(
    httpClient,
    ghOrgName,
    ghRepoName,
    std::move(parseVersion),
    std::move(getReleasesCallback),
    std::move(errorCallback));
}

HttpOperation* downloadAsset(
  const HttpClient& httpClient,
  const Asset& asset,
  HttpClient::DownloadCallback downloadCallback,
  HttpClient::ErrorCallback errorCallback);

} // namespace upd

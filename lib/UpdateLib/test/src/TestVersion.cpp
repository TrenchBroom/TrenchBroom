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

#include "TestVersion.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace upd
{
namespace
{

auto makeAssetJson(const Asset& asset)
{
  auto json = QJsonObject{};
  json["name"] = asset.name;
  json["browser_download_url"] = asset.url.toString();
  json["size"] = static_cast<double>(asset.size);
  return json;
}

auto makeAssetsJson(const QList<Asset>& assets)
{
  auto json = QJsonArray{};
  for (const auto& asset : assets)
  {
    json << makeAssetJson(asset);
  }
  return json;
}

auto makeVersionTag(const TestVersion& version)
{
  return QString{"v%1"}.arg(version.v);
}

auto makeReleaseJson(const Release<TestVersion>& release)
{
  auto json = QJsonObject{};
  json["tag_name"] = makeVersionTag(release.version);
  json["name"] = release.name;
  json["prerelease"] = release.prerelease;
  json["draft"] = release.draft;
  json["html_url"] = release.browserUrl;
  json["assets"] = makeAssetsJson(release.assets);
  return json;
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const TestVersion& rhs)
{
  return lhs << "TestVersion{v: " << rhs.v << "}";
}


std::optional<TestVersion> parseVersion(const QString& str)
{
  static const auto pattern = QRegularExpression{R"(v(\d+))"};

  const auto match = pattern.match(str);
  if (!match.hasMatch())
  {
    return std::nullopt;
  }

  return TestVersion{match.captured(1).toInt()};
}

QString describeVersion(const TestVersion& version)
{
  return QString{"Version %1"}.arg(version.v);
}

Asset chooseFirstAsset(const QList<Asset>& assets)
{
  return assets.front();
}

QByteArray makeGetReleasesJson(const QList<Release<TestVersion>>& releases)
{
  auto json = QJsonArray{};
  for (const auto& release : releases)
  {
    json << makeReleaseJson(release);
  }

  const auto document = QJsonDocument{json};
  return document.toJson(QJsonDocument::Compact);
}

} // namespace upd

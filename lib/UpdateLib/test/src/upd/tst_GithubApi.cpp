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

#include <QList>

#include "upd/GithubApi.h"
#include "upd/TestHttpClient.h"
#include "upd/TestUtils.h"
#include "upd/TestVersion.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace upd
{

TEST_CASE("GithubApi")
{
  auto httpClient = TestHttpClient{};

  SECTION("getLatestRelease")
  {
    SECTION("calls errorCallback if getReleases fails")
    {
      getLatestRelease<TestVersion>(
        httpClient,
        "some_org",
        "some_repo",
        TestVersion{1},
        false,
        false,
        parseVersion,
        [](const auto&) { FAIL("getLatestReleaseCallback should not be called"); },
        [](const auto& error) { CHECK(error == "some error"); });

      REQUIRE(httpClient.pendingGetOperation != nullptr);
      httpClient.pendingGetOperation->errorCallback("some error");
    }

    SECTION("passes expected release if getReleases succeeds")
    {
      using T = std::tuple<
        QList<Release<TestVersion>>,
        TestVersion,
        bool,
        bool,
        std::optional<Release<TestVersion>>>;

      const auto
        [availableReleases,
         currentVersion,
         includePreReleases,
         includeDraftReleases,
         expectedRelease] = GENERATE(values<T>({
          {{}, TestVersion{1}, false, false, std::nullopt},
          {{
             Release<TestVersion>{TestVersion{3}, false, false, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           false,
           false,
           Release<TestVersion>{TestVersion{3}, false, false, "v3", "", {}}},
          {{
             Release<TestVersion>{TestVersion{3}, false, true, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           false,
           false,
           std::nullopt},
          {{
             Release<TestVersion>{TestVersion{3}, true, false, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           false,
           false,
           std::nullopt},
          {{
             Release<TestVersion>{TestVersion{5}, false, true, "v5", "", {}},
             Release<TestVersion>{TestVersion{4}, true, true, "v4", "", {}},
             Release<TestVersion>{TestVersion{3}, true, false, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           true,
           false,
           Release<TestVersion>{TestVersion{3}, true, false, "v3", "", {}}},

          {{
             Release<TestVersion>{TestVersion{5}, false, true, "v5", "", {}},
             Release<TestVersion>{TestVersion{4}, true, true, "v4", "", {}},
             Release<TestVersion>{TestVersion{3}, true, false, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           true,
           true,
           Release<TestVersion>{TestVersion{5}, false, true, "v5", "", {}}},

          {{
             Release<TestVersion>{TestVersion{5}, true, true, "v5", "", {}},
             Release<TestVersion>{TestVersion{4}, false, true, "v4", "", {}},
             Release<TestVersion>{TestVersion{3}, true, false, "v3", "", {}},
             Release<TestVersion>{TestVersion{2}, false, false, "v2", "", {}},
             Release<TestVersion>{TestVersion{1}, false, false, "v1", "", {}},
           },
           TestVersion{2},
           false,
           true,
           Release<TestVersion>{TestVersion{4}, false, true, "v4", "", {}}},
        }));

      CAPTURE(
        availableReleases, currentVersion, includePreReleases, includeDraftReleases);

      getLatestRelease<TestVersion>(
        httpClient,
        "some_org",
        "some_repo",
        currentVersion,
        includePreReleases,
        includeDraftReleases,
        parseVersion,
        [&, expectedRelease_ = expectedRelease](const auto& release) {
          CHECK(release == expectedRelease_);
        },
        [](const auto&) { FAIL("errorCallback should not be called"); });

      REQUIRE(httpClient.pendingGetOperation != nullptr);
      httpClient.pendingGetOperation->successCallback(
        makeGetReleasesJson(availableReleases));
    }
  }

  SECTION("getReleases")
  {
    SECTION("calls errorCallback if get fails")
    {
      getReleases<TestVersion>(
        httpClient,
        "some_org",
        "some_repo",
        parseVersion,
        [](const auto&) { FAIL("successCallback should not be called"); },
        [](const auto& error) { CHECK(error == "some error"); });

      REQUIRE(httpClient.pendingGetOperation != nullptr);
      httpClient.pendingGetOperation->errorCallback("some error");
    }

    SECTION("calls errorCallback if get returns invalid JSON")
    {
      using T = std::tuple<QByteArray, QString>;

      const auto [body, expectedError] = GENERATE(values<T>({
        {R"()", "illegal value"},
        {R"([)", "unterminated array"},
        {R"(asdf)", "illegal number"},
        {R"({})", "invalid response body, expected array"},
      }));

      CAPTURE(QString{body});

      getReleases<TestVersion>(
        httpClient,
        "some_org",
        "some_repo",
        parseVersion,
        [](const auto&) { FAIL("successCallback should not be called"); },
        [&, expectedError_ = expectedError](const auto& error) {
          CHECK(error == expectedError_);
        });

      REQUIRE(httpClient.pendingGetOperation != nullptr);
      httpClient.pendingGetOperation->successCallback(body);
    }

    SECTION("passes expected releases if get returns valid JSON")
    {
      using T = std::tuple<QByteArray, QList<Release<TestVersion>>>;

      const auto [body, expectedReleases] = GENERATE(values<T>({
        {R"([])", {}},
        {
          R"([{
            "tag_name": "v2",
            "name": "v2 Stable Release",
            "prerelease": false,
            "draft": false,
            "html_url": "https://github.com/owner/repo/releases/tag/v2",
            "assets": [
                {
                    "name": "app-v2.zip",
                    "size": 1048576,
                    "browser_download_url":
                    "https://github.com/owner/repo/releases/download/v2/app-v2.zip"
                }
            ]
          }])",
          {
            Release<TestVersion>{
              TestVersion{2},
              false,
              false,
              "v2 Stable Release",
              "https://github.com/owner/repo/releases/tag/v2",
              {
                {
                  "app-v2.zip",
                  QUrl{"https://github.com/owner/repo/releases/download/v2/app-v2.zip"},
                  1048576,
                },
              },
            },
          }},
      }));

      CAPTURE(body);

      getReleases<TestVersion>(
        httpClient,
        "some_org",
        "some_repo",
        parseVersion,
        [&, expectedReleases_ = expectedReleases](const auto& releases) {
          CHECK(releases == expectedReleases_);
        },
        [](const auto&) { FAIL("errorCallback should not be called"); });

      REQUIRE(httpClient.pendingGetOperation != nullptr);
      httpClient.pendingGetOperation->successCallback(body);
    }
  }

  SECTION("downloadAsset")
  {
    SECTION("calls errorCallback if download fails")
    {
      downloadAsset(
        httpClient,
        Asset{},
        [](const auto&) { FAIL("callback should not be called"); },
        [](const auto& error) { CHECK(error == "some error"); });

      REQUIRE(httpClient.pendingDownloadOperation != nullptr);
      httpClient.pendingDownloadOperation->errorCallback("some error");
    }

    SECTION("passes file if download succeeds")
    {
      auto expectedFile = QTemporaryFile{};
      expectedFile.open();
      expectedFile.write("some content");
      expectedFile.close();

      downloadAsset(
        httpClient,
        Asset{},
        [](auto& file) { CHECK(readFileIntoString(file) == "some content"); },
        [](const auto&) { FAIL("errorCallback should not be called"); });

      REQUIRE(httpClient.pendingDownloadOperation != nullptr);
      httpClient.pendingDownloadOperation->successCallback(expectedFile);
    }
  }
}

} // namespace upd

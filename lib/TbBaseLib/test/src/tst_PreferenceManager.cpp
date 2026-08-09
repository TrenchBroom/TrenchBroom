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

#include "base/Color.h"
#include "base/KeySequence.h"
#include "base/PreferenceManager.h"
#include "base/PreferenceStore.h"

#include "kd/k.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <unordered_map>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{
namespace
{

using Value = std::variant<bool, int, float, std::string, std::filesystem::path, Color>;

[[maybe_unused]] std::ostream& operator<<(std::ostream& lhs, const Value& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

struct MockPreferenceStore : public PreferenceStore
{
  bool load(const std::filesystem::path& path, bool& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<bool>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path& path, int& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<int>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path& path, float& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<float>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path& path, std::string& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<std::string>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path& path, std::filesystem::path& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<std::filesystem::path>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path& path, Color& value) override
  {
    if (const auto iValue = values.find(path); iValue != values.end())
    {
      value = std::get<Color>(iValue->second);
      return true;
    }

    return false;
  }

  bool load(const std::filesystem::path&, std::vector<KeySequence>&) override
  {
    // not covered by this mock; see tst_QPreferenceStore.cpp for shortcut-specific
    // coverage
    return false;
  }

  void save(const std::filesystem::path& path, const bool value) override
  {
    values[path] = value;
  }

  void save(const std::filesystem::path& path, const int value) override
  {
    values[path] = value;
  }

  void save(const std::filesystem::path& path, const float value) override
  {
    values[path] = value;
  }

  void save(const std::filesystem::path& path, const std::string& value) override
  {
    values[path] = value;
  }

  void save(
    const std::filesystem::path& path, const std::filesystem::path& value) override
  {
    values[path] = value;
  }

  void save(const std::filesystem::path& path, const Color& value) override
  {
    values[path] = value;
  }

  void save(const std::filesystem::path&, const std::vector<KeySequence>&) override
  {
    // not covered by this mock; see tst_QPreferenceStore.cpp for shortcut-specific
    // coverage
  }

  std::unordered_map<std::filesystem::path, Value> values;
};

/**
 * Creates the global PreferenceManager instance and destroys it again when it goes out
 * of scope, so that no instance leaks into other test cases.
 */
struct PreferenceManagerInstance
{
  template <typename... Args>
  explicit PreferenceManagerInstance(Args&&... args)
  {
    PreferenceManager::createInstance(std::forward<Args>(args)...);
  }

  ~PreferenceManagerInstance() { PreferenceManager::destroyInstance(); }
};

} // namespace

TEST_CASE("PreferenceManager")
{
  using namespace std::string_literals;

  auto stringPref = Preference<std::string>{"some/path", "asdf"};

  auto preferenceStoreOwner = std::make_unique<MockPreferenceStore>();
  auto& preferenceStore = *preferenceStoreOwner;

  SECTION("when saveInstantly is true")
  {
    auto preferenceManager =
      PreferenceManager{std::move(preferenceStoreOwner), K(saveInstantly)};

    SECTION("getValue / getPendingValue")
    {
      SECTION("returns default value if no value is stored")
      {
        CHECK(preferenceManager.get(stringPref) == "asdf");
        CHECK(preferenceManager.getPendingValue(stringPref) == "asdf");
      }

      SECTION("returns stored value")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);

        CHECK(preferenceManager.get(stringPref) == "fdsa");
        CHECK(preferenceManager.getPendingValue(stringPref) == "fdsa");
      }

      SECTION("returns changed value")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);
        preferenceManager.set(stringPref, "qwer");

        CHECK(preferenceManager.get(stringPref) == "qwer");
        CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");
      }
    }

    SECTION("setValue")
    {
      SECTION("sets and saves the new value")
      {
        preferenceManager.set(stringPref, "qwer");

        CHECK(
          preferenceStore.values
          == std::unordered_map<std::filesystem::path, Value>{
            {"some/path", "qwer"s},
          });
        CHECK(preferenceManager.get(stringPref) == "qwer");
      }

      SECTION("transient preferences are not saved")
      {
        auto transientPref = Preference<std::string>{
          "other/path", "default", PreferencePersistencePolicy::Transient};

        preferenceManager.set(transientPref, "qwer");

        CHECK(
          preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
        CHECK(preferenceManager.get(transientPref) == "qwer");
      }
    }

    SECTION("when preference store is reloaded")
    {
      preferenceManager.set(stringPref, "fdsa");
      REQUIRE(
        preferenceStore.values
        == std::unordered_map<std::filesystem::path, Value>{
          {"some/path", "fdsa"s},
        });
      REQUIRE(preferenceManager.get(stringPref) == "fdsa");
      REQUIRE(preferenceManager.getPendingValue(stringPref) == "fdsa");

      preferenceStore.values["some/path"] = "qwer"s;
      preferenceStore.preferencesWereReloadedNotifier(
        std::vector<std::filesystem::path>{"some/path"});

      CHECK(preferenceManager.get(stringPref) == "qwer");
      CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");
    }
  }

  SECTION("when saveInstantly is false")
  {
    auto preferenceManager =
      PreferenceManager{std::move(preferenceStoreOwner), !K(saveInstantly)};

    SECTION("getValue / getPendingValue")
    {
      SECTION("returns default value if no value is stored")
      {
        CHECK(preferenceManager.get(stringPref) == "asdf");
        CHECK(preferenceManager.getPendingValue(stringPref) == "asdf");
      }

      SECTION("returns stored value")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);

        CHECK(preferenceManager.get(stringPref) == "fdsa");
        CHECK(preferenceManager.getPendingValue(stringPref) == "fdsa");
      }

      SECTION("getValue returns stored value even if changed")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);
        preferenceManager.set(stringPref, "qwer");

        CHECK(preferenceManager.get(stringPref) == "fdsa");
      }

      SECTION("getPendingValue returns changed valu")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);
        preferenceManager.set(stringPref, "qwer");

        CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");
      }
    }

    SECTION("setValue")
    {
      SECTION("doesn't set the value")
      {
        preferenceManager.set(stringPref, "qwer");

        CHECK(
          preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
      }

      SECTION("clears pending value when set to current value")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);

        preferenceManager.set(stringPref, "qwer");
        REQUIRE(preferenceManager.hasUnsavedChanges());
        REQUIRE(preferenceManager.get(stringPref) == "fdsa");
        REQUIRE(preferenceManager.getPendingValue(stringPref) == "qwer");

        preferenceManager.set(stringPref, "fdsa");
        CHECK(!preferenceManager.hasUnsavedChanges());
        CHECK(preferenceManager.get(stringPref) == "fdsa");
        CHECK(preferenceManager.getPendingValue(stringPref) == "fdsa");
      }

      SECTION("saveChanges")
      {
        SECTION("saves persistent preferences")
        {
          preferenceManager.set(stringPref, "qwer");

          REQUIRE(
            preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
          REQUIRE(preferenceManager.get(stringPref) == "asdf");
          REQUIRE(preferenceManager.getPendingValue(stringPref) == "qwer");

          preferenceManager.saveChanges();
          CHECK(
            preferenceStore.values
            == std::unordered_map<std::filesystem::path, Value>{
              {"some/path", "qwer"s},
            });
          CHECK(preferenceManager.get(stringPref) == "qwer");
          CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");
        }

        SECTION("does not save transient preferences")
        {
          auto transientPref = Preference<std::string>{
            "other/path", "default", PreferencePersistencePolicy::Transient};

          preferenceManager.set(transientPref, "qwer");

          REQUIRE(
            preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
          REQUIRE(preferenceManager.get(transientPref) == "default");
          REQUIRE(preferenceManager.getPendingValue(transientPref) == "qwer");

          preferenceManager.saveChanges();
          CHECK(
            preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
          CHECK(preferenceManager.get(transientPref) == "qwer");
          CHECK(preferenceManager.getPendingValue(transientPref) == "qwer");
        }
      }

      SECTION("discardChanges")
      {
        preferenceStore.values.emplace("some/path", "fdsa"s);
        preferenceManager.set(stringPref, "qwer");

        REQUIRE(
          preferenceStore.values
          == std::unordered_map<std::filesystem::path, Value>{
            {"some/path", "fdsa"s},
          });
        REQUIRE(preferenceManager.get(stringPref) == "fdsa");
        REQUIRE(preferenceManager.getPendingValue(stringPref) == "qwer");

        preferenceManager.discardChanges();
        CHECK(
          preferenceStore.values
          == std::unordered_map<std::filesystem::path, Value>{
            {"some/path", "fdsa"s},
          });
        CHECK(preferenceManager.get(stringPref) == "fdsa");
        REQUIRE(preferenceManager.getPendingValue(stringPref) == "fdsa");
      }
    }

    SECTION("hasUnsavedChanges")
    {
      CHECK(!preferenceManager.hasUnsavedChanges());

      preferenceManager.set(stringPref, "qwer");
      CHECK(preferenceManager.hasUnsavedChanges());

      preferenceManager.saveChanges();
      CHECK(!preferenceManager.hasUnsavedChanges());

      preferenceManager.set(stringPref, "fdsa");
      CHECK(preferenceManager.hasUnsavedChanges());

      preferenceManager.discardChanges();
      CHECK(!preferenceManager.hasUnsavedChanges());
    }

    SECTION("when preference store is reloaded")
    {
      preferenceManager.set(stringPref, "fdsa");
      REQUIRE(
        preferenceStore.values == std::unordered_map<std::filesystem::path, Value>{});
      REQUIRE(preferenceManager.get(stringPref) == "asdf");
      REQUIRE(preferenceManager.getPendingValue(stringPref) == "fdsa");

      preferenceStore.values["some/path"] = "qwer"s;
      preferenceStore.preferencesWereReloadedNotifier(
        std::vector<std::filesystem::path>{"some/path"});

      CHECK(preferenceManager.get(stringPref) == "qwer");
      CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");

      preferenceManager.saveChanges();
      CHECK(
        preferenceStore.values
        == std::unordered_map<std::filesystem::path, Value>{
          {"some/path", "qwer"s},
        });
      CHECK(preferenceManager.get(stringPref) == "qwer");
      CHECK(preferenceManager.getPendingValue(stringPref) == "qwer");
    }
  }

  SECTION("shouldSaveInstantly")
  {
#ifdef __APPLE__
    CHECK(PreferenceManager::shouldSaveInstantly());
#else
    CHECK(!PreferenceManager::shouldSaveInstantly());
#endif
  }

  SECTION("saveInstantly")
  {
    const auto saveInstantly = GENERATE(true, false);
    CAPTURE(saveInstantly);

    const auto preferenceManager =
      PreferenceManager{std::move(preferenceStoreOwner), saveInstantly};

    CHECK(preferenceManager.saveInstantly() == saveInstantly);
  }

  SECTION("createInstance / instance")
  {
    const auto instance =
      PreferenceManagerInstance{std::move(preferenceStoreOwner), K(saveInstantly)};

    auto boolPref = Preference<bool>{"some/path", false};
    preferenceStore.values.emplace("some/path", true);

    CHECK(PreferenceManager::instance().saveInstantly());
    CHECK(PreferenceManager::instance().get(boolPref));

    // the same instance is returned every time
    CHECK(&PreferenceManager::instance() == &PreferenceManager::instance());
  }

  SECTION("destroyInstance")
  {
    SECTION("a new instance can be created after the previous one was destroyed")
    {
      PreferenceManager::createInstance(
        std::move(preferenceStoreOwner), K(saveInstantly));
      PreferenceManager::destroyInstance();

      const auto instance = PreferenceManagerInstance{
        std::make_unique<MockPreferenceStore>(), !K(saveInstantly)};

      CHECK(!PreferenceManager::instance().saveInstantly());
    }
  }
}

TEST_CASE("togglePref")
{
  auto preferenceStoreOwner = std::make_unique<MockPreferenceStore>();
  auto& preferenceStore = *preferenceStoreOwner;

  const auto saveInstantly = GENERATE(true, false);
  CAPTURE(saveInstantly);

  const auto instance =
    PreferenceManagerInstance{std::move(preferenceStoreOwner), saveInstantly};

  auto boolPref = Preference<bool>{"some/path", false};

  SECTION("toggles the preference and saves it")
  {
    togglePref(boolPref);

    CHECK(PreferenceManager::instance().get(boolPref));
    CHECK(
      preferenceStore.values
      == std::unordered_map<std::filesystem::path, Value>{
        {"some/path", true},
      });

    togglePref(boolPref);

    CHECK(!PreferenceManager::instance().get(boolPref));
    CHECK(
      preferenceStore.values
      == std::unordered_map<std::filesystem::path, Value>{
        {"some/path", false},
      });
  }

  SECTION("leaves no unsaved changes behind")
  {
    togglePref(boolPref);

    CHECK(!PreferenceManager::instance().hasUnsavedChanges());
  }
}

} // namespace tb

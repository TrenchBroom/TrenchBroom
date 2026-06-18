/*
 Copyright (C) 2026 Artsiom Trubchyk
 Copyright (C) 2026 Kristian Duske

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
#include <QKeyEvent>

#include "ui/ActionExecutionContext.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/KeyboardShortcutUtils.h"

#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{
constexpr auto CyrillicTse = 0x0426;

#if defined(Q_OS_WIN)
constexpr auto NativeVirtualW = quint32{'W'};
constexpr auto NativeScanW = quint32{0};
#elif defined(Q_OS_MACOS)
constexpr auto NativeVirtualW = quint32{0x0D};
constexpr auto NativeScanW = quint32{0};
#elif defined(Q_OS_LINUX)
constexpr auto NativeVirtualW = quint32{0};
constexpr auto NativeScanW = quint32{0x19};
#else
constexpr auto NativeVirtualW = quint32{0};
constexpr auto NativeScanW = quint32{0};
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
constexpr auto PlatformUsesPhysicalShortcutFallback = true;
#else
constexpr auto PlatformUsesPhysicalShortcutFallback = false;
#endif

QKeyEvent makePhysicalWEvent(
  const Qt::KeyboardModifiers modifiers = Qt::NoModifier, const int key = CyrillicTse)
{
  return QKeyEvent{QEvent::KeyPress, key, modifiers, NativeScanW, NativeVirtualW, 0};
}

QKeySequence keySequence(const Qt::KeyboardModifiers modifiers, const Qt::Key key)
{
  return QKeySequence{static_cast<int>(modifiers.toInt()) | static_cast<int>(key)};
}

#if defined(Q_OS_MACOS)
Qt::KeyboardModifier physicalCommandModifier()
{
  return QCoreApplication::testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)
           ? Qt::MetaModifier
           : Qt::ControlModifier;
}
#endif

Action makeAction(
  const std::filesystem::path& preferencePath,
  const QKeySequence& shortcut,
  const bool enabled = true)
{
  return Action{
    preferencePath,
    "Action",
    ActionContext::Any,
    shortcut,
    [](auto&) {},
    [enabled](const auto&) { return enabled; }};
}

} // namespace

TEST_CASE("eventMatchesPhysicalKey")
{
  SECTION("matches physical latin letter key on non-latin layout")
  {
    const auto event = makePhysicalWEvent();
    const auto shortcut = QKeySequence{Qt::Key_W};

    CHECK(eventMatchesPhysicalKey(event, shortcut));
  }

  SECTION("falls back to logical latin letter key when native key data is unavailable")
  {
#if !defined(Q_OS_MACOS)
    // On macOS, kVK_ANSI_A == 0x00, so nativeVirtualKey 0 is a valid key code
    // and cannot be used to represent an event without native key data.
    const auto event = QKeyEvent{QEvent::KeyPress, Qt::Key_W, Qt::NoModifier};
    const auto shortcut = QKeySequence{Qt::Key_W};

    CHECK(eventMatchesPhysicalKey(event, shortcut));
#endif
  }

  SECTION("falls back to logical key for non-layout-dependent keys")
  {
    const auto event = QKeyEvent{QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier};
    const auto shortcut = QKeySequence{Qt::Key_Escape};

    CHECK(eventMatchesPhysicalKey(event, shortcut));
  }
}

TEST_CASE("shouldUsePhysicalShortcutFallback")
{
  SECTION("uses platform-specific fallback policy")
  {
    const auto event = makePhysicalWEvent();

    CHECK(
      shouldUsePhysicalShortcutFallback(event) == PlatformUsesPhysicalShortcutFallback);
  }

#if defined(Q_OS_MACOS)
  SECTION("does not use fallback when physical Command is pressed")
  {
    const auto event = makePhysicalWEvent(physicalCommandModifier());

    CHECK_FALSE(shouldUsePhysicalShortcutFallback(event));
  }
#endif
}

TEST_CASE("eventMatchesPhysicalShortcutFallback")
{
  SECTION("matches physical single-key shortcut only on platforms that need fallback")
  {
    const auto event = makePhysicalWEvent();
    const auto shortcut = QKeySequence{Qt::Key_W};

    CHECK(
      eventMatchesPhysicalShortcutFallback(event, shortcut)
      == PlatformUsesPhysicalShortcutFallback);
  }

  SECTION("matches physical modified shortcut on Linux")
  {
    const auto event = makePhysicalWEvent(Qt::ControlModifier);
    const auto shortcut = keySequence(Qt::ControlModifier, Qt::Key_W);

#if defined(Q_OS_LINUX)
    CHECK(eventMatchesPhysicalShortcutFallback(event, shortcut));
#else
    CHECK_FALSE(eventMatchesPhysicalShortcutFallback(event, shortcut));
#endif
  }

#if defined(Q_OS_MACOS)
  SECTION("does not match physical modified shortcut when physical Command is pressed")
  {
    const auto commandModifier = physicalCommandModifier();
    const auto event = makePhysicalWEvent(commandModifier);
    const auto shortcut = keySequence(commandModifier, Qt::Key_W);

    CHECK_FALSE(eventMatchesPhysicalShortcutFallback(event, shortcut));
  }
#endif

  SECTION("does not report fallback when logical key already matches")
  {
    const auto event = makePhysicalWEvent(Qt::NoModifier, Qt::Key_W);
    const auto shortcut = QKeySequence{Qt::Key_W};

    CHECK_FALSE(eventMatchesPhysicalShortcutFallback(event, shortcut));
  }
}

TEST_CASE("findFallbackAction")
{
  auto appControllerFixture = AppControllerFixture{};
  auto& appController = appControllerFixture.appController();

  const auto event = makePhysicalWEvent();
  const auto context = ActionExecutionContext{appController, nullptr, nullptr};

  auto matchingAction = makeAction("matching", QKeySequence{Qt::Key_W});
  auto nonMatchingAction = makeAction("nonMatching", QKeySequence{Qt::Key_X});
  auto disabledMatchingAction =
    makeAction("disabledMatching", QKeySequence{Qt::Key_W}, false);

  SECTION("returns no match if no action matches")
  {
    const auto actions =
      std::vector<Action*>{&nonMatchingAction, &disabledMatchingAction};

    const auto match = findFallbackAction(event, context, actions);

    CHECK(std::holds_alternative<NoFallbackActionMatch>(match));
  }

  SECTION("returns the unique matching enabled action only when fallback is enabled")
  {
    const auto actions =
      std::vector<Action*>{&nonMatchingAction, &matchingAction, &disabledMatchingAction};

    const auto match = findFallbackAction(event, context, actions);

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    REQUIRE(std::holds_alternative<UniqueFallbackActionMatch>(match));
    CHECK(&std::get<UniqueFallbackActionMatch>(match).action == &matchingAction);
#else
    CHECK(std::holds_alternative<NoFallbackActionMatch>(match));
#endif
  }

  SECTION(
    "returns the first matching action if the match is ambiguous and fallback is enabled")
  {
    auto otherMatchingAction = makeAction("otherMatching", QKeySequence{Qt::Key_W});
    const auto actions =
      std::vector<Action*>{&nonMatchingAction, &matchingAction, &otherMatchingAction};

    const auto match = findFallbackAction(event, context, actions);

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    REQUIRE(std::holds_alternative<AmbiguousFallbackActionMatch>(match));
    CHECK(&std::get<AmbiguousFallbackActionMatch>(match).action == &matchingAction);
#else
    CHECK(std::holds_alternative<NoFallbackActionMatch>(match));
#endif
  }
}


} // namespace tb::ui

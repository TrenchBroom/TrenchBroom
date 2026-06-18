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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/KeyboardShortcutUtils.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QtGlobal>

#if defined(Q_OS_MACOS)
#include <Carbon/Carbon.h>
#endif

#if defined(Q_OS_LINUX)
#include <linux/input-event-codes.h>
#endif

#include "kd/reflection_impl.h"

#include <cstddef>

namespace tb::ui
{
namespace
{

constexpr auto ShortcutModifierMask =
  Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;

bool isLatinLetterOrDigit(const Qt::Key key)
{
  return (key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9);
}

struct NativeKeyMap
{
  quint32 native;
  Qt::Key key;
};

template <std::size_t N>
Qt::Key lookupNativeKey(const quint32 native, const NativeKeyMap (&map)[N])
{
  for (const auto& item : map)
  {
    if (item.native == native)
    {
      return item.key;
    }
  }

  return Qt::Key_unknown;
}

Qt::Key physicalLatinLetterOrDigitKey(const QKeyEvent& event)
{
#if defined(Q_OS_WIN)
  const auto nativeVirtualKey = event.nativeVirtualKey();
  if (nativeVirtualKey >= 'A' && nativeVirtualKey <= 'Z')
  {
    return static_cast<Qt::Key>(
      static_cast<int>(Qt::Key_A) + static_cast<int>(nativeVirtualKey - 'A'));
  }
  if (nativeVirtualKey >= '0' && nativeVirtualKey <= '9')
  {
    return static_cast<Qt::Key>(
      static_cast<int>(Qt::Key_0) + static_cast<int>(nativeVirtualKey - '0'));
  }

#elif defined(Q_OS_MACOS)
#define MAC_KEY(x) {static_cast<quint32>(kVK_ANSI_##x), Qt::Key_##x}

  static constexpr NativeKeyMap map[] = {
    MAC_KEY(A), MAC_KEY(B), MAC_KEY(C), MAC_KEY(D), MAC_KEY(E), MAC_KEY(F), MAC_KEY(G),
    MAC_KEY(H), MAC_KEY(I), MAC_KEY(J), MAC_KEY(K), MAC_KEY(L), MAC_KEY(M), MAC_KEY(N),
    MAC_KEY(O), MAC_KEY(P), MAC_KEY(Q), MAC_KEY(R), MAC_KEY(S), MAC_KEY(T), MAC_KEY(U),
    MAC_KEY(V), MAC_KEY(W), MAC_KEY(X), MAC_KEY(Y), MAC_KEY(Z),

    MAC_KEY(0), MAC_KEY(1), MAC_KEY(2), MAC_KEY(3), MAC_KEY(4), MAC_KEY(5), MAC_KEY(6),
    MAC_KEY(7), MAC_KEY(8), MAC_KEY(9),
  };

#undef MAC_KEY

  const auto key = lookupNativeKey(event.nativeVirtualKey(), map);
  if (key != Qt::Key_unknown)
  {
    return key;
  }

#elif defined(Q_OS_LINUX)
  // Qt/X11 and Qt/Wayland expose XKB keycodes here.
  // Linux evdev KEY_* -> XKB keycode = KEY_* + 8.
  constexpr auto XkbOffset = quint32{8};

#define LINUX_KEY(x) {static_cast<quint32>(KEY_##x + XkbOffset), Qt::Key_##x}

  static constexpr NativeKeyMap map[] = {
    LINUX_KEY(A), LINUX_KEY(B), LINUX_KEY(C), LINUX_KEY(D), LINUX_KEY(E), LINUX_KEY(F),
    LINUX_KEY(G), LINUX_KEY(H), LINUX_KEY(I), LINUX_KEY(J), LINUX_KEY(K), LINUX_KEY(L),
    LINUX_KEY(M), LINUX_KEY(N), LINUX_KEY(O), LINUX_KEY(P), LINUX_KEY(Q), LINUX_KEY(R),
    LINUX_KEY(S), LINUX_KEY(T), LINUX_KEY(U), LINUX_KEY(V), LINUX_KEY(W), LINUX_KEY(X),
    LINUX_KEY(Y), LINUX_KEY(Z),

    LINUX_KEY(0), LINUX_KEY(1), LINUX_KEY(2), LINUX_KEY(3), LINUX_KEY(4), LINUX_KEY(5),
    LINUX_KEY(6), LINUX_KEY(7), LINUX_KEY(8), LINUX_KEY(9),
  };

#undef LINUX_KEY

  const auto key = lookupNativeKey(event.nativeScanCode(), map);
  if (key != Qt::Key_unknown)
  {
    return key;
  }
#endif

  return Qt::Key_unknown;
}

#if defined(Q_OS_MACOS)
bool hasPhysicalCommandModifier(const QKeyEvent& event)
{
  const auto commandModifier =
    QCoreApplication::testAttribute(Qt::AA_MacDontSwapCtrlAndMeta) ? Qt::MetaModifier
                                                                   : Qt::ControlModifier;

  return (event.modifiers() & commandModifier) != 0;
}
#endif

bool matchesShortcut(
  const QKeyEvent& event,
  const QKeySequence& shortcut,
  const bool compareModifiers,
  const bool usePhysicalKey)
{
  if (shortcut.count() != 1)
  {
    return false;
  }

  const auto keyCombination = shortcut[0];
  const auto shortcutKey = keyCombination.key();
  const auto logicalEventKey = static_cast<Qt::Key>(event.key());
  auto eventKey = logicalEventKey;
  if (usePhysicalKey && isLatinLetterOrDigit(shortcutKey))
  {
    if (const auto physicalEventKey = physicalLatinLetterOrDigitKey(event);
        physicalEventKey != Qt::Key_unknown)
    {
      eventKey = physicalEventKey;
    }
  }

  if (shortcutKey != eventKey)
  {
    return false;
  }

  return !compareModifiers
         || (keyCombination.keyboardModifiers() & ShortcutModifierMask)
              == (event.modifiers() & ShortcutModifierMask);
}

} // namespace

bool eventMatchesPhysicalKey(const QKeyEvent& event, const QKeySequence& shortcut)
{
  return matchesShortcut(event, shortcut, false, true);
}

bool shouldUsePhysicalShortcutFallback(const QKeyEvent& event)
{
  // Windows already handles QAction/QShortcut for these keys. On Linux it doesn't
  // for non-Latin layouts. On macOS, Qt handles Command shortcuts but not
  // single-key shortcuts in the same layout-independent way.
#if defined(Q_OS_WIN)
  (void)event;
  return false;
#elif defined(Q_OS_MACOS)
  return !hasPhysicalCommandModifier(event);
#elif defined(Q_OS_LINUX)
  (void)event;
  return true;
#else
  (void)event;
  return false;
#endif
}

bool eventMatchesPhysicalShortcutFallback(
  const QKeyEvent& event, const QKeySequence& shortcut)
{
  return shouldUsePhysicalShortcutFallback(event)
         && matchesShortcut(event, shortcut, true, true)
         && !matchesShortcut(event, shortcut, true, false);
}

kdl_reflect_impl(NoFallbackActionMatch);

std::ostream& operator<<(std::ostream& lhs, const UniqueFallbackActionMatch& rhs)
{
  return lhs << "UniqueFallbackActionMatch{action: " << rhs.action.label().toStdString()
             << "}";
}

std::ostream& operator<<(std::ostream& lhs, const AmbiguousFallbackActionMatch& rhs)
{
  return lhs << "AmbiguousFallbackActionMatch{action: "
             << rhs.action.label().toStdString() << "}";
}

std::ostream& operator<<(std::ostream& lhs, const FallbackActionMatch& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

} // namespace tb::ui

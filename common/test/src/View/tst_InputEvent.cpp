/*
Copyright (C) 2020 Kristian Duske

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

#include <QKeyEvent>
#include <QtGlobal>

#include "View/InputEvent.h"

#include "kdl/overload.h"

#include <array>
#include <chrono>
#include <list>
#include <thread>
#include <variant>

#include "Catch2.h" // IWYU pragma: keep

namespace TrenchBroom::View
{

TEST_CASE("KeyEvent")
{
  SECTION("collateWith")
  {
    const auto lhsType = GENERATE(KeyEvent::Type::Down, KeyEvent::Type::Up);
    const auto rhsType = GENERATE(KeyEvent::Type::Down, KeyEvent::Type::Up);

    auto lhs = KeyEvent{lhsType};
    const auto rhs = KeyEvent{rhsType};
    CHECK_FALSE(lhs.collateWith(rhs));
  }
}

TEST_CASE("MouseEvent")
{
  SECTION("collateWith")
  {
    SECTION("can collate")
    {
      constexpr std::array<std::array<bool, 9>, 9> expectedResult = {{
        // Down   Up     Click  DClick Motion Scroll DragSt Drag   DragEnd
        {false, false, false, false, false, false, false, false, false}, // Down
        {false, false, false, false, false, false, false, false, false}, // Up
        {false, false, false, false, false, false, false, false, false}, // Click
        {false, false, false, false, false, false, false, false, false}, // DClick
        {false, false, false, false, true, false, false, false, false},  // Motion
        {false, false, false, false, false, true, false, false, false},  // Scroll
        {false, false, false, false, false, false, false, false, false}, // DragStart
        {false, false, false, false, false, false, false, true, false},  // Drag
        {false, false, false, false, false, false, false, false, false}, // DragEnd
      }};

      using Type = MouseEvent::Type;
      const auto lhsType = GENERATE(
        Type::Down,
        Type::Up,
        Type::Click,
        Type::DoubleClick,
        Type::Motion,
        Type::Scroll,
        Type::DragStart,
        Type::Drag,
        Type::DragEnd);
      const auto rhsType = GENERATE(
        Type::Down,
        Type::Up,
        Type::Click,
        Type::DoubleClick,
        Type::Motion,
        Type::Scroll,
        Type::DragStart,
        Type::Drag,
        Type::DragEnd);

      auto lhs = MouseEvent{
        lhsType, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 0, 0, 0.0f};
      const auto rhs = MouseEvent{
        rhsType, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 0, 0, 0.0f};

      CHECK(lhs.collateWith(rhs) == expectedResult[size_t(lhsType)][size_t(rhsType)]);
    }

    SECTION("motion collation")
    {
      auto lhs = MouseEvent{
        MouseEvent::Type::Motion,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::None,
        2,
        3,
        0.0f};
      const auto rhs = MouseEvent{
        MouseEvent::Type::Motion,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::None,
        5,
        5,
        0.0f};
      CHECK(lhs.collateWith(rhs));
      CHECK(lhs.posX == 5);
      CHECK(lhs.posY == 5);
    }

    SECTION("drag collation")
    {
      auto lhs = MouseEvent{
        MouseEvent::Type::Drag,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::None,
        2,
        3,
        0.0f};
      const auto rhs = MouseEvent{
        MouseEvent::Type::Drag,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::None,
        5,
        5,
        0.0f};
      CHECK(lhs.collateWith(rhs));
      CHECK(lhs.posX == 5);
      CHECK(lhs.posY == 5);
    }

    SECTION("horizontal wheel collation")
    {
      using Axis = MouseEvent::WheelAxis;
      const auto lhsWheelAxis = GENERATE(Axis::Horizontal, Axis::Vertical);
      const auto rhsWheelAxis = GENERATE(Axis::Horizontal, Axis::Vertical);

      // clang-format off
      const auto expectedScrollDistances = std::array<std::array<std::optional<float>, 2>, 2>{{
        // H           V
        {-2.0f,        std::nullopt}, // H
        {std::nullopt, -2.0f},        // V
      }};
      // clang-format on

      const auto expectedScrollDistance =
        expectedScrollDistances[size_t(lhsWheelAxis) - 1][size_t(rhsWheelAxis) - 1];

      auto lhs = MouseEvent{
        MouseEvent::Type::Scroll, MouseEvent::Button::None, lhsWheelAxis, 0, 0, 3.0f};
      const auto rhs = MouseEvent{
        MouseEvent::Type::Scroll, MouseEvent::Button::None, rhsWheelAxis, 0, 0, -5.0f};

      CHECK(lhs.collateWith(rhs) == expectedScrollDistance.has_value());
      if (expectedScrollDistance)
      {
        CHECK(lhs.scrollDistance == expectedScrollDistance);
      }
    }
  }
}

namespace
{
class TestEventProcessor : public InputEventProcessor
{
private:
  using Event = std::variant<KeyEvent, MouseEvent, CancelEvent>;
  std::list<Event> m_expectedEvents;

public:
  template <typename... Args>
  explicit TestEventProcessor(Args&&... args)
  {
    (m_expectedEvents.emplace_back(std::forward<Args>(args)), ...);
  }

  void processEvent(const KeyEvent& act) override
  {
    CHECK_FALSE(m_expectedEvents.empty());
    std::visit(
      kdl::overload(
        [&](const KeyEvent& exp) { CHECK(act == exp); },
        [&](const auto&) { CHECK(false); }),
      m_expectedEvents.front());
    m_expectedEvents.pop_front();
  }

  void processEvent(const MouseEvent& act) override
  {
    CHECK_FALSE(m_expectedEvents.empty());
    std::visit(
      kdl::overload(
        [&](const MouseEvent& exp) {
          CHECK(exp.type == act.type);
          CHECK(exp.button == act.button);
          CHECK(exp.wheelAxis == act.wheelAxis);
          CHECK(exp.posX == act.posX);
          CHECK(exp.posY == act.posY);
          CHECK(exp.scrollDistance == Approx(act.scrollDistance));
        },
        [&](const auto&) { CHECK(false); }),
      m_expectedEvents.front());
    m_expectedEvents.pop_front();
  }

  void processEvent(const CancelEvent& act) override
  {
    CHECK_FALSE(m_expectedEvents.empty());
    std::visit(
      kdl::overload(
        [&](const CancelEvent& exp) { CHECK(act == exp); },
        [&](const auto&) { CHECK(false); }),
      m_expectedEvents.front());
    m_expectedEvents.pop_front();
  }

  bool allConsumed() const { return m_expectedEvents.empty(); }
};

template <typename... Args>
void checkEventQueue(InputEventRecorder& r, Args&&... args)
{
  auto p = TestEventProcessor{std::forward<Args>(args)...};
  r.processEvents(p);
  CHECK(p.allConsumed());
}

QWheelEvent makeWheelEvent(const QPoint& angleDelta)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
  return {{}, {}, {}, angleDelta, Qt::NoButton, nullptr, Qt::ScrollUpdate, false};
#else
  return {{}, {}, {}, angleDelta, 0, Qt::Orientation::Horizontal, Qt::NoButton, 0};
#endif
}

} // namespace

TEST_CASE("InputEventRecorder")
{
  auto r = InputEventRecorder{};

  SECTION("recordKeyEvents")
  {
    r.recordEvent(QKeyEvent{QEvent::KeyPress, 0, nullptr, nullptr, 0});
    r.recordEvent(QKeyEvent{QEvent::KeyRelease, 0, nullptr, nullptr, 0});

    checkEventQueue(r, KeyEvent{KeyEvent::Type::Down}, KeyEvent{KeyEvent::Type::Up});
  }

  SECTION("recordLeftClick")
  {
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f});
  }

  SECTION("recordLeftDoubleClick")
  {
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonDblClick,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DoubleClick,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f});
  }

  SECTION("recordCtrlLeftClick")
  {
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      Qt::MetaModifier});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f});
  }

  SECTION("recordRightClick")
  {
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::RightButton,
      Qt::RightButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {2.0f, 5.0f},
      {},
      {},
      Qt::RightButton,
      Qt::RightButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Right,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f});
  }

  SECTION("recordMotionWithCollation")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {6.0f, 3.0f}, {}, {}, Qt::NoButton, Qt::NoButton, nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {12.0f, 8.0f}, {}, {}, Qt::NoButton, Qt::NoButton, nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Motion,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::None,
        12,
        8,
        0.0f});
  }

  SECTION("recordHScrollWithCollation")
  {
    const auto qWheel1 = makeWheelEvent({2, 0});
    const auto qWheel2 = makeWheelEvent({3, 0});

    const auto expectedScrollLines =
      float((InputEventRecorder::scrollLinesForEvent(qWheel1)
             + InputEventRecorder::scrollLinesForEvent(qWheel2))
              .x());
    REQUIRE(expectedScrollLines > 0.0f);

    using namespace std::chrono_literals;
    r.recordEvent(qWheel1);
    r.recordEvent(qWheel2);

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Scroll,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::Horizontal,
        0,
        0,
        expectedScrollLines});
  }

  SECTION("recordVScrollWithCollation")
  {
    const auto qWheel1 = makeWheelEvent({0, 3});
    const auto qWheel2 = makeWheelEvent({0, 4});

    const auto expectedScrollLines =
      float((InputEventRecorder::scrollLinesForEvent(qWheel1)
             + InputEventRecorder::scrollLinesForEvent(qWheel2))
              .y());
    REQUIRE(expectedScrollLines > 0.0f);

    using namespace std::chrono_literals;
    r.recordEvent(qWheel1);
    r.recordEvent(qWheel2);

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Scroll,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::Vertical,
        0,
        0,
        expectedScrollLines});
  }

  SECTION("recordDiagonalScroll")
  {
    const auto qWheel1 = makeWheelEvent({1, 3});
    const auto qWheel2 = makeWheelEvent({3, 0});

    const auto expectedScrollLines1 = InputEventRecorder::scrollLinesForEvent(qWheel1);
    REQUIRE(expectedScrollLines1.x() > 0.0f);
    REQUIRE(expectedScrollLines1.y() > 0.0f);

    const auto expectedScrollLines2 = InputEventRecorder::scrollLinesForEvent(qWheel2);
    REQUIRE(expectedScrollLines2.x() > 0.0f);
    REQUIRE(0.0f == expectedScrollLines2.y());

    using namespace std::chrono_literals;
    r.recordEvent(qWheel1);
    r.recordEvent(qWheel2);

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Scroll,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::Horizontal,
        0,
        0,
        float(expectedScrollLines1.x())},
      MouseEvent{
        MouseEvent::Type::Scroll,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::Vertical,
        0,
        0,
        float(expectedScrollLines1.y())},
      MouseEvent{
        MouseEvent::Type::Scroll,
        MouseEvent::Button::None,
        MouseEvent::WheelAxis::Horizontal,
        0,
        0,
        float(expectedScrollLines2.x())});
  }

  SECTION("recordLeftClickWithQuickSmallMotion")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {4.0f, 3.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {4.0f, 3.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Motion,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        4,
        3,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        4,
        3,
        0.0f});
  }

  SECTION("recordLeftClickWithSlowSmallMotion")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {4.0f, 3.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    std::this_thread::sleep_for(200ms);
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {4.0f, 3.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Motion,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        4,
        3,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Click,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        4,
        3,
        0.0f});
  }

  SECTION("recordLeftClickWithAccidentalDrag")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {6.0f, 3.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {6.0f, 3.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DragStart,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Drag,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        6,
        3,
        0.0f},
      CancelEvent(),
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        6,
        3,
        0.0f});
  }

  SECTION("recordLeftDrag")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {6.0f, 3.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    std::this_thread::sleep_for(200ms);
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {6.0f, 3.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DragStart,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Drag,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        6,
        3,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DragEnd,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        6,
        3,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        6,
        3,
        0.0f});
  }

  SECTION("recordLeftDragWithCollation")
  {
    using namespace std::chrono_literals;
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonPress,
      {2.0f, 5.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {6.0f, 3.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    std::this_thread::sleep_for(200ms);
    r.recordEvent(QMouseEvent{
      QEvent::MouseMove, {12.0f, 8.0f}, {}, {}, Qt::LeftButton, Qt::LeftButton, nullptr});
    r.recordEvent(QMouseEvent{
      QEvent::MouseButtonRelease,
      {12.0f, 8.0f},
      {},
      {},
      Qt::LeftButton,
      Qt::LeftButton,
      nullptr});

    checkEventQueue(
      r,
      MouseEvent{
        MouseEvent::Type::Down,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DragStart,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        2,
        5,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Drag,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        12,
        8,
        0.0f},
      MouseEvent{
        MouseEvent::Type::DragEnd,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        12,
        8,
        0.0f},
      MouseEvent{
        MouseEvent::Type::Up,
        MouseEvent::Button::Left,
        MouseEvent::WheelAxis::None,
        12,
        8,
        0.0f});
  }
}


} // namespace TrenchBroom::View

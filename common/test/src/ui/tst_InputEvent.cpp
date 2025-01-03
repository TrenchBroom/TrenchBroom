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

#include "ui/InputEvent.h"

#include <array>
#include <thread>
#include <variant>

#include "Catch2.h" // IWYU pragma: keep

namespace tb::ui
{
namespace
{
using Event = std::variant<KeyEvent, MouseEvent, ScrollEvent, GestureEvent, CancelEvent>;

[[maybe_unused]] std::ostream& operator<<(std::ostream& lhs, const Event& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

class TestEventProcessor : public InputEventProcessor
{
private:
  std::vector<Event> m_events;

public:
  const std::vector<Event>& events() const { return m_events; }

  void processEvent(const KeyEvent& event) override { m_events.emplace_back(event); }

  void processEvent(const MouseEvent& event) override { m_events.emplace_back(event); }

  void processEvent(const ScrollEvent& event) override { m_events.emplace_back(event); }

  void processEvent(const GestureEvent& event) override { m_events.emplace_back(event); }

  void processEvent(const CancelEvent& event) override { m_events.emplace_back(event); }
};

auto getEvents(InputEventRecorder& r)
{
  auto p = TestEventProcessor{};
  r.processEvents(p);
  return p.events();
}

QKeyEvent makeKeyEvent(const QEvent::Type type)
{
  return QKeyEvent{type, 0, Qt::NoModifier};
}

QMouseEvent makeMouseEvent(
  QEvent::Type type,
  const QPointF& pos,
  const Qt::MouseButton button = Qt::NoButton,
  const Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
  return QMouseEvent{type, pos, {}, button, button, modifiers};
}

QWheelEvent makeWheelEvent(const QPoint& angleDelta)
{
  return QWheelEvent{
    {}, {}, {}, angleDelta, Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false};
}

} // namespace

TEST_CASE("MouseEvent")
{
  SECTION("collateWith")
  {
    SECTION("can collate")
    {
      constexpr std::array<std::array<bool, 8>, 8> expectedResult = {{
        // Down   Up   Click  DClick Motion DragSt Drag   DragEnd
        {false, false, false, false, false, false, false, false}, // Down
        {false, false, false, false, false, false, false, false}, // Up
        {false, false, false, false, false, false, false, false}, // Click
        {false, false, false, false, false, false, false, false}, // DClick
        {false, false, false, false, true, false, false, false},  // Motion
        {false, false, false, false, false, false, false, false}, // DragStart
        {false, false, false, false, false, false, true, false},  // Drag
        {false, false, false, false, false, false, false, false}, // DragEnd
      }};

      using Type = MouseEvent::Type;
      const auto lhsType = GENERATE(
        Type::Down,
        Type::Up,
        Type::Click,
        Type::DoubleClick,
        Type::Motion,
        Type::DragStart,
        Type::Drag,
        Type::DragEnd);
      const auto rhsType = GENERATE(
        Type::Down,
        Type::Up,
        Type::Click,
        Type::DoubleClick,
        Type::Motion,
        Type::DragStart,
        Type::Drag,
        Type::DragEnd);

      auto lhs = MouseEvent{lhsType, MouseEvent::Button::None, 0, 0};
      const auto rhs = MouseEvent{rhsType, MouseEvent::Button::None, 0, 0};

      CHECK(lhs.collateWith(rhs) == expectedResult[size_t(lhsType)][size_t(rhsType)]);
    }

    SECTION("motion collation")
    {
      auto lhs = MouseEvent{MouseEvent::Type::Motion, MouseEvent::Button::None, 2, 3};
      const auto rhs =
        MouseEvent{MouseEvent::Type::Motion, MouseEvent::Button::None, 5, 5};
      CHECK(lhs.collateWith(rhs));
      CHECK(lhs.posX == 5);
      CHECK(lhs.posY == 5);
    }

    SECTION("drag collation")
    {
      auto lhs = MouseEvent{MouseEvent::Type::Drag, MouseEvent::Button::None, 2, 3};
      const auto rhs = MouseEvent{MouseEvent::Type::Drag, MouseEvent::Button::None, 5, 5};
      CHECK(lhs.collateWith(rhs));
      CHECK(lhs.posX == 5);
      CHECK(lhs.posY == 5);
    }
  }
}

TEST_CASE("ScrollEvent")
{
  SECTION("collateWith")
  {
    using Source = ScrollEvent::Source;
    const auto lhsSource = GENERATE(Source::Mouse, Source::Trackpad);
    const auto rhsSource = GENERATE(Source::Mouse, Source::Trackpad);

    using Axis = ScrollEvent::Axis;
    const auto lhsWheelAxis = GENERATE(Axis::Horizontal, Axis::Vertical);
    const auto rhsWheelAxis = GENERATE(Axis::Horizontal, Axis::Vertical);

    const auto canCollate = lhsSource == rhsSource && lhsWheelAxis == rhsWheelAxis;
    const auto expectedScrollDistance = canCollate ? std::optional{-2.0f} : std::nullopt;

    auto lhs = ScrollEvent{lhsSource, lhsWheelAxis, 3.0f};
    const auto rhs = ScrollEvent{rhsSource, rhsWheelAxis, -5.0f};

    CHECK(lhs.collateWith(rhs) == expectedScrollDistance.has_value());
    if (expectedScrollDistance)
    {
      CHECK(lhs.distance == expectedScrollDistance);
    }
  }
}

TEST_CASE("GestureEvent")
{
  SECTION("collateWith")
  {
    using Type = GestureEvent::Type;
    const auto lhsType = GENERATE(Type::Pan, Type::Zoom, Type::Rotate);
    const auto rhsType = GENERATE(Type::Pan, Type::Zoom, Type::Rotate);

    const auto expected = std::array<std::array<bool, 3>, 3>{{
      // Pan  Zoom Rotate
      {true, false, false}, // Pan
      {false, true, false}, // Zoom
      {false, false, true}, // Rotate
    }};

    auto lhs = GestureEvent{lhsType, 0, 0, 0.0f};
    const auto rhs = GestureEvent{rhsType, 0, 0, 0.0f};
    CHECK(lhs.collateWith(rhs) == expected[size_t(lhsType) - 2][size_t(rhsType) - 2]);
  }

  SECTION("value collation")
  {
    using Type = GestureEvent::Type;
    const auto type = GENERATE(Type::Pan, Type::Zoom, Type::Rotate);

    auto lhs = GestureEvent{type, 1, 2, 3.0f};
    const auto rhs = GestureEvent{type, 4, 5, 6.0f};

    REQUIRE(lhs.collateWith(rhs));
    CHECK(lhs.posX == 4);
    CHECK(lhs.posY == 5);
    CHECK(lhs.value == 6);
  }
}

TEST_CASE("InputEventRecorder")
{
  auto r = InputEventRecorder{};

  SECTION("recordKeyEvents")
  {
    r.recordEvent(makeKeyEvent(QEvent::KeyPress));
    r.recordEvent(makeKeyEvent(QEvent::KeyRelease));

    CHECK(
      getEvents(r)
      == std::vector<Event>{{
        KeyEvent{KeyEvent::Type::Down},
        KeyEvent{KeyEvent::Type::Up},
      }});
  }

  SECTION("recordLeftClick")
  {
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {2.0f, 5.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 2, 5},
      });
  }

  SECTION("recordLeftDoubleClick")
  {
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonDblClick, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {2.0f, 5.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::DoubleClick, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 2, 5},
      });
  }

  SECTION("recordCtrlLeftClick")
  {
    r.recordEvent(makeMouseEvent(
      QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton, Qt::MetaModifier));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {2.0f, 5.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Right, 2, 5},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Right, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Right, 2, 5},
      });
  }

  SECTION("recordRightClick")
  {
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::RightButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {2.0f, 5.0f}, Qt::RightButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Right, 2, 5},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Right, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Right, 2, 5},
      });
  }

  SECTION("recordMotionWithCollation")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {6.0f, 3.0f}));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {12.0f, 8.0f}));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Motion, MouseEvent::Button::None, 12, 8},
      });
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

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        ScrollEvent{
          ScrollEvent::Source::Mouse,
          ScrollEvent::Axis::Horizontal,
          expectedScrollLines,
        },
      });
  }

  SECTION("recordVScrollWithCollation")
  {
    const auto qWheel1 = makeWheelEvent({0, 4});
    const auto qWheel2 = makeWheelEvent({0, 6});

    const auto expectedScrollLines =
      float((InputEventRecorder::scrollLinesForEvent(qWheel1)
             + InputEventRecorder::scrollLinesForEvent(qWheel2))
              .y());
    REQUIRE(expectedScrollLines > 0.0f);

    using namespace std::chrono_literals;
    r.recordEvent(qWheel1);
    r.recordEvent(qWheel2);

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        ScrollEvent{
          ScrollEvent::Source::Mouse,
          ScrollEvent::Axis::Vertical,
          expectedScrollLines,
        },
      });
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

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        ScrollEvent{
          ScrollEvent::Source::Mouse,
          ScrollEvent::Axis::Horizontal,
          float(expectedScrollLines1.x()),
        },
        ScrollEvent{
          ScrollEvent::Source::Mouse,
          ScrollEvent::Axis::Vertical,
          float(expectedScrollLines1.y()),
        },
        ScrollEvent{
          ScrollEvent::Source::Mouse,
          ScrollEvent::Axis::Horizontal,
          float(expectedScrollLines2.x()),
        },
      });
  }

  SECTION("recordLeftClickWithQuickSmallMotion")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {4.0f, 3.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {4.0f, 3.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Motion, MouseEvent::Button::Left, 4, 3},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 4, 3},
      });
  }

  SECTION("recordLeftClickWithSlowSmallMotion")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {4.0f, 3.0f}, Qt::LeftButton));
    std::this_thread::sleep_for(200ms);
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {4.0f, 3.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Motion, MouseEvent::Button::Left, 4, 3},
        MouseEvent{MouseEvent::Type::Click, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 4, 3},
      });
  }

  SECTION("recordLeftClickWithAccidentalDrag")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {6.0f, 3.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {6.0f, 3.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::DragStart, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Drag, MouseEvent::Button::Left, 6, 3},
        CancelEvent(),
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 6, 3},
      });
  }

  SECTION("recordLeftDrag")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {6.0f, 3.0f}, Qt::LeftButton));
    std::this_thread::sleep_for(200ms);
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {6.0f, 3.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::DragStart, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Drag, MouseEvent::Button::Left, 6, 3},
        MouseEvent{MouseEvent::Type::DragEnd, MouseEvent::Button::Left, 6, 3},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 6, 3},
      });
  }

  SECTION("recordLeftDragWithCollation")
  {
    using namespace std::chrono_literals;
    r.recordEvent(makeMouseEvent(QEvent::MouseButtonPress, {2.0f, 5.0f}, Qt::LeftButton));
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {6.0f, 3.0f}, Qt::LeftButton));
    std::this_thread::sleep_for(200ms);
    r.recordEvent(makeMouseEvent(QEvent::MouseMove, {12.0f, 8.0f}, Qt::LeftButton));
    r.recordEvent(
      makeMouseEvent(QEvent::MouseButtonRelease, {12.0f, 8.0f}, Qt::LeftButton));

    CHECK(
      getEvents(r)
      == std::vector<Event>{
        MouseEvent{MouseEvent::Type::Down, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::DragStart, MouseEvent::Button::Left, 2, 5},
        MouseEvent{MouseEvent::Type::Drag, MouseEvent::Button::Left, 12, 8},
        MouseEvent{MouseEvent::Type::DragEnd, MouseEvent::Button::Left, 12, 8},
        MouseEvent{MouseEvent::Type::Up, MouseEvent::Button::Left, 12, 8},
      });
  }
}

} // namespace tb::ui

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

#include "View/InputEvent.h"

#include <kdl/overload.h>

#include <array>
#include <chrono>
#include <list>
#include <thread>
#include <variant>

#include <QtGlobal>
#include <QKeyEvent>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE("KeyEventTest.collateWith", "[KeyEventTest]") {
            static const std::array<KeyEvent::Type, 2> eventTypes = { KeyEvent::Type::Down, KeyEvent::Type::Up };
            
            for (std::size_t i = 0; i < 2; ++i) {
                for (std::size_t j = 0; j < 2; ++j) {
                    auto lhs = KeyEvent(eventTypes[i]);
                    const auto rhs = KeyEvent(eventTypes[j]);
                    CHECK_FALSE(lhs.collateWith(rhs));
                }
            }
        }
        
        TEST_CASE("MouseEventTest.collateWith", "[MouseEventTest]") {
            static const std::array<MouseEvent::Type, 9> eventTypes = {
                MouseEvent::Type::Down,
                MouseEvent::Type::Up,
                MouseEvent::Type::Click,
                MouseEvent::Type::DoubleClick,
                MouseEvent::Type::Motion,
                MouseEvent::Type::Scroll,
                MouseEvent::Type::DragStart,
                MouseEvent::Type::Drag,
                MouseEvent::Type::DragEnd
            };
            static const std::array<std::array<bool, 9>, 9> collationMatrix = {{
                // Down   Up     Click  DClick Motion Scroll DragSt Drag   DragEnd
                {  false, false, false, false, false, false, false, false, false }, // Down
                {  false, false, false, false, false, false, false, false, false }, // Up
                {  false, false, false, false, false, false, false, false, false }, // Click
                {  false, false, false, false, false, false, false, false, false }, // DClick
                {  false, false, false, false,  true, false, false, false, false }, // Motion
                {  false, false, false, false, false,  true, false, false, false }, // Scroll
                {  false, false, false, false, false, false, false, false, false }, // DragStart
                {  false, false, false, false, false, false, false,  true, false }, // Drag
                {  false, false, false, false, false, false, false, false, false }, // DragEnd
            }};

            for (std::size_t i = 0; i < 9; ++i) {
                for (std::size_t j = 0; j < 9; ++j) {
                    auto lhs = MouseEvent(eventTypes[i], MouseEvent::Button::None, MouseEvent::WheelAxis::None, 0, 0, 0.0f);
                    const auto rhs = MouseEvent(eventTypes[j], MouseEvent::Button::None, MouseEvent::WheelAxis::None, 0, 0, 0.0f);
                    
                    CHECK(lhs.collateWith(rhs) == collationMatrix[i][j]);
                }
            }
            
            {
                // motion collation
                auto lhs = MouseEvent(MouseEvent::Type::Motion, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 2, 3, 0.0f);
                const auto rhs = MouseEvent(MouseEvent::Type::Motion, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 5, 5, 0.0f);
                CHECK(lhs.collateWith(rhs));
                CHECK(lhs.posX == 5);
                CHECK(lhs.posY == 5);
            }
            
            {
                // drag collation
                auto lhs = MouseEvent(MouseEvent::Type::Drag, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 2, 3, 0.0f);
                const auto rhs = MouseEvent(MouseEvent::Type::Drag, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 5, 5, 0.0f);
                CHECK(lhs.collateWith(rhs));
                CHECK(lhs.posX == 5);
                CHECK(lhs.posY == 5);
            }
            
            {
                // horizontal wheel collation
                auto lhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, 3.0f);
                const auto rhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, -5.0f);
                CHECK(lhs.collateWith(rhs));
                CHECK(lhs.scrollDistance == -2.0f);
            }
            
            {
                // vertical wheel collation
                auto lhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Vertical, 0, 0, 3.0f);
                const auto rhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Vertical, 0, 0, -5.0f);
                CHECK(lhs.collateWith(rhs));
                CHECK(lhs.scrollDistance == -2.0f);
            }
            
            {
                // unmatched axis wheel collation
                auto lhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, 3.0f);
                const auto rhs = MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Vertical, 0, 0, -5.0f);
                CHECK_FALSE(lhs.collateWith(rhs));
                CHECK(lhs.scrollDistance == 3.0f);
            }
        }
        
        class TestEventProcessor : public InputEventProcessor {
        private:
            using Event = std::variant<KeyEvent, MouseEvent, CancelEvent>;
            std::list<Event> m_expectedEvents;
        public:
            template <typename... Args>
            TestEventProcessor(Args&&... args) {
                (m_expectedEvents.emplace_back(std::forward<Args>(args)), ...);
            }
            
            void processEvent(const KeyEvent& act) override {
                CHECK_FALSE(m_expectedEvents.empty());
                std::visit(kdl::overload(
                    [&](const KeyEvent& exp) { CHECK(act == exp); },
                    [&](const auto&)       { CHECK(false); }
                ), m_expectedEvents.front());
                m_expectedEvents.pop_front();
            }
            
            void processEvent(const MouseEvent& act) override {
                CHECK_FALSE(m_expectedEvents.empty());
                std::visit(kdl::overload(
                    [&](const MouseEvent& exp) {
                        CHECK(exp.type == act.type);
                        CHECK(exp.button == act.button);
                        CHECK(exp.wheelAxis == act.wheelAxis);
                        CHECK(exp.posX == act.posX);
                        CHECK(exp.posY == act.posY);
                        CHECK(exp.scrollDistance == Approx(act.scrollDistance));
                    },
                    [&](const auto&) { CHECK(false); }
                ), m_expectedEvents.front());
                m_expectedEvents.pop_front();
            }
            
            void processEvent(const CancelEvent& act) override {
                CHECK_FALSE(m_expectedEvents.empty());
                std::visit(kdl::overload(
                    [&](const CancelEvent& exp) { CHECK(act == exp); },
                    [&](const auto&)        { CHECK(false); }
                ), m_expectedEvents.front());
                m_expectedEvents.pop_front();
            }
            
            bool allConsumed() const {
                return m_expectedEvents.empty();
            }
        };
        
        template <typename... Args>
        void checkEventQueue(InputEventRecorder& r, Args&&... args) {
            TestEventProcessor p(std::forward<Args>(args)...);
            r.processEvents(p);
            CHECK(p.allConsumed());
        }
        
        inline QWheelEvent makeWheelEvent(const QPoint& angleDelta) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            return QWheelEvent({}, {}, {}, angleDelta, Qt::NoButton, 0, Qt::ScrollUpdate, false);
#else
            return QWheelEvent({}, {}, {}, angleDelta, 0, Qt::Orientation::Horizontal, Qt::NoButton, 0);
#endif
        }
        
        TEST_CASE("InputEventRecorderTest.recordKeyEvents", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qKeyPress = QKeyEvent(QEvent::KeyPress, 0, 0, 0, 0);
            const auto qKeyRelease = QKeyEvent(QEvent::KeyRelease, 0, 0, 0, 0);

            r.recordEvent(&qKeyPress);
            r.recordEvent(&qKeyRelease);
            
            checkEventQueue(r,
                KeyEvent(KeyEvent::Type::Down),
                KeyEvent(KeyEvent::Type::Up));
        }
        
        TEST_CASE("InputEventRecorderTest.recordLeftClick", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,  MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Click, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,    MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordLeftDoubleClick", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease1 = QMouseEvent(QEvent::MouseButtonRelease, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseDoubleClick = QMouseEvent(QEvent::MouseButtonDblClick, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease2 = QMouseEvent(QEvent::MouseButtonRelease, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseRelease1);
            r.recordEvent(&qMouseDoubleClick);
            r.recordEvent(&qMouseRelease2);

            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,        MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Click,       MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,          MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Down,        MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::DoubleClick, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,          MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f));
        }

        TEST_CASE("InputEventRecorderTest.recordCtrlLeftClick", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, Qt::MetaModifier);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,  MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Click, MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,    MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f));
        }

        TEST_CASE("InputEventRecorderTest.recordRightClick", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::RightButton, Qt::RightButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 2.0f, 5.0f }, {}, {}, Qt::RightButton, Qt::RightButton, 0);

            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,  MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Click, MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,    MouseEvent::Button::Right, MouseEvent::WheelAxis::None, 2, 5, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordMotionWithCollation", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMouseMotion1 = QMouseEvent(QEvent::MouseMove, { 6.0f, 3.0f }, {}, {}, Qt::NoButton, Qt::NoButton, 0);
            const auto qMouseMotion2 = QMouseEvent(QEvent::MouseMove, { 12.0f, 8.0f }, {}, {}, Qt::NoButton, Qt::NoButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMouseMotion1);
            r.recordEvent(&qMouseMotion2);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Motion, MouseEvent::Button::None, MouseEvent::WheelAxis::None, 12, 8, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordHScrollWithCollation", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qWheel1 = makeWheelEvent({ 2, 0 });
            const auto qWheel2 = makeWheelEvent({ 3, 0 });

            const float expectedScrollLines = \
                static_cast<float>((InputEventRecorder::scrollLinesForEvent(&qWheel1) +
                                    InputEventRecorder::scrollLinesForEvent(&qWheel2)).x());
            CHECK(expectedScrollLines > 0.0f);

            using namespace std::chrono_literals;
            r.recordEvent(&qWheel1);
            r.recordEvent(&qWheel2);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, expectedScrollLines));
        }
        
        TEST_CASE("InputEventRecorderTest.recordVScrollWithCollation", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qWheel1 = makeWheelEvent({ 0, 3 });
            const auto qWheel2 = makeWheelEvent({ 0, 4 });

            const float expectedScrollLines = \
                static_cast<float>((InputEventRecorder::scrollLinesForEvent(&qWheel1) +
                                    InputEventRecorder::scrollLinesForEvent(&qWheel2)).y());
            CHECK(expectedScrollLines > 0.0f);

            using namespace std::chrono_literals;
            r.recordEvent(&qWheel1);
            r.recordEvent(&qWheel2);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Vertical, 0, 0, expectedScrollLines));
        }
        
        TEST_CASE("InputEventRecorderTest.recordDiagonalScroll", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qWheel1 = makeWheelEvent({ 1, 3 });
            const auto qWheel2 = makeWheelEvent({ 3, 0 });

            const QPointF expectedScrollLines1 = InputEventRecorder::scrollLinesForEvent(&qWheel1);
            CHECK(expectedScrollLines1.x() > 0.0f);
            CHECK(expectedScrollLines1.y() > 0.0f);

            const QPointF expectedScrollLines2 = InputEventRecorder::scrollLinesForEvent(&qWheel2);
            CHECK(expectedScrollLines2.x() > 0.0f);
            CHECK(0.0f == expectedScrollLines2.y());

            using namespace std::chrono_literals;
            r.recordEvent(&qWheel1);
            r.recordEvent(&qWheel2);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, static_cast<float>(expectedScrollLines1.x())),
                MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Vertical,   0, 0, static_cast<float>(expectedScrollLines1.y())),
                MouseEvent(MouseEvent::Type::Scroll, MouseEvent::Button::None, MouseEvent::WheelAxis::Horizontal, 0, 0, static_cast<float>(expectedScrollLines2.x())));
        }

        TEST_CASE("InputEventRecorderTest.recordLeftClickWithQuickSmallMotion", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion = QMouseEvent(QEvent::MouseMove, { 4.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 4.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseMotion);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,   MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Motion, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 4, 3, 0.0f),
                MouseEvent(MouseEvent::Type::Click,  MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,     MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 4, 3, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordLeftClickWithSlowSmallMotion", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion = QMouseEvent(QEvent::MouseMove, { 4.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 4.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseMotion);
            std::this_thread::sleep_for(200ms);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,   MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Motion, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 4, 3, 0.0f),
                MouseEvent(MouseEvent::Type::Click,  MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Up,     MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 4, 3, 0.0f));
        }

        TEST_CASE("InputEventRecorderTest.recordLeftClickWithAccidentalDrag", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion = QMouseEvent(QEvent::MouseMove, { 6.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 6.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseMotion);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::DragStart, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Drag,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 6, 3, 0.0f),
                CancelEvent(),
                MouseEvent(MouseEvent::Type::Up,        MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 6, 3, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordLeftDrag", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion = QMouseEvent(QEvent::MouseMove, { 6.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 6.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseMotion);
            std::this_thread::sleep_for(200ms);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::DragStart, MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Drag,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 6, 3, 0.0f),
                MouseEvent(MouseEvent::Type::DragEnd,   MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 6, 3, 0.0f),
                MouseEvent(MouseEvent::Type::Up,        MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 6, 3, 0.0f));
        }
        
        TEST_CASE("InputEventRecorderTest.recordLeftDragWithCollation", "[InputEventRecorderTest]") {
            InputEventRecorder r;
            const auto qMousePress = QMouseEvent(QEvent::MouseButtonPress, { 2.0f, 5.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion1 = QMouseEvent(QEvent::MouseMove, { 6.0f, 3.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseMotion2 = QMouseEvent(QEvent::MouseMove, { 12.0f, 8.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);
            const auto qMouseRelease = QMouseEvent(QEvent::MouseButtonRelease, { 12.0f, 8.0f }, {}, {}, Qt::LeftButton, Qt::LeftButton, 0);

            using namespace std::chrono_literals;
            r.recordEvent(&qMousePress);
            r.recordEvent(&qMouseMotion1);
            std::this_thread::sleep_for(200ms);
            r.recordEvent(&qMouseMotion2);
            r.recordEvent(&qMouseRelease);
            
            checkEventQueue(r,
                MouseEvent(MouseEvent::Type::Down,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None,  2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::DragStart, MouseEvent::Button::Left, MouseEvent::WheelAxis::None,  2, 5, 0.0f),
                MouseEvent(MouseEvent::Type::Drag,      MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 12, 8, 0.0f),
                MouseEvent(MouseEvent::Type::DragEnd,   MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 12, 8, 0.0f),
                MouseEvent(MouseEvent::Type::Up,        MouseEvent::Button::Left, MouseEvent::WheelAxis::None, 12, 8, 0.0f));
        }
    }
}

/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "KeyboardShortcut.h"

#include <QKeyEvent>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcut::KeyboardShortcut(int qtKey) :
        m_qtKey(qtKey) {}

        KeyboardShortcut::KeyboardShortcut(const QKeySequence& keySequence) :
        m_qtKey(keySequence[0]) {}

        bool operator==(const KeyboardShortcut& lhs, const KeyboardShortcut& rhs) {
            return lhs.keySequence() == rhs.keySequence();
        }

        QKeySequence KeyboardShortcut::keySequence() const {
            return QKeySequence(m_qtKey);
        }

        bool KeyboardShortcut::matchesKeyDown(const QKeyEvent* event) const {
            const auto ourKey = keySequence();
            if (ourKey.isEmpty()) {
                return false;
            }

            const auto theirKeyInt = event->key();
            const auto ourKeyInt = keySequence()[0];
            return ourKeyInt == theirKeyInt;
        }

        bool KeyboardShortcut::matchesKeyUp(const QKeyEvent* event) const {
           const Qt::KeyboardModifiers AllModifiers =
                   Qt::ShiftModifier
                   | Qt::ControlModifier
                   | Qt::AltModifier
                   | Qt::MetaModifier
                   | Qt::KeypadModifier
                   | Qt::GroupSwitchModifier;

            const auto ourKey = keySequence();
            if (ourKey.isEmpty()) {
                return false;
            }

            const auto theirKeyInt = event->key();
            const auto ourKeyInt = keySequence()[0];

            if (ourKeyInt == theirKeyInt) {
                return true;
            }

            // if any of the modifiers were released, it matches
            const auto releasedModifiers = event->key() & AllModifiers;
            const auto ourModifiers = ourKeyInt & AllModifiers;
            return (releasedModifiers & ourModifiers) != 0;
        }

        int wxModifierToQt(const int wxMod) {
            const auto wxMOD_ALT         = 0x0001;
            const auto wxMOD_CONTROL     = 0x0002; // Command key on macOS
            const auto wxMOD_SHIFT       = 0x0004;
            const auto wxMOD_META        = 0x0008;
            const auto wxMOD_RAW_CONTROL = 0x0010; // Control key on macOS

            int result = 0;

            if (wxMod & wxMOD_ALT) {
                result |= Qt::AltModifier;
            }
            if (wxMod & wxMOD_CONTROL) {
                result |= Qt::ControlModifier;
            }
            if (wxMod & wxMOD_SHIFT) {
                result |= Qt::ShiftModifier;
            }
            // Both wxMOD_META and wxMOD_RAW_CONTROL map to Qt::MetaModifier, because wx uses wxMOD_RAW_CONTROL
            // for the Control key on macOS, and Qt uses Qt::MetaModifier
            if (wxMod & (wxMOD_META | wxMOD_RAW_CONTROL)) {
                result |= Qt::MetaModifier;
            }

            return result;
        }

        int wxKeyToQt(const int wxKey) {
            const auto WXK_BACK = 8;
            const auto WXK_TAB = 9;
            const auto WXK_RETURN = 13;
            const auto WXK_ESCAPE = 27;
            const auto WXK_SPACE = 32;
            const auto WXK_DELETE = 127;
            const auto WXK_START = 300;
            const auto WXK_LBUTTON = 301;
            const auto WXK_RBUTTON = 302;
            const auto WXK_CANCEL = 303;
            const auto WXK_MBUTTON = 304;
            const auto WXK_CLEAR = 305;
            const auto WXK_SHIFT = 306;
            const auto WXK_ALT = 307;
            const auto WXK_CONTROL = 308;
            const auto WXK_MENU = 309;
            const auto WXK_PAUSE = 310;
            const auto WXK_CAPITAL = 311;
            const auto WXK_END = 312;
            const auto WXK_HOME = 313;
            const auto WXK_LEFT = 314;
            const auto WXK_UP = 315;
            const auto WXK_RIGHT = 316;
            const auto WXK_DOWN = 317;
            const auto WXK_SELECT = 318;
            const auto WXK_PRINT = 319;
            const auto WXK_EXECUTE = 320;
            const auto WXK_SNAPSHOT = 321;
            const auto WXK_INSERT = 322;
            const auto WXK_HELP = 323;
            const auto WXK_NUMPAD0 = 324;
            const auto WXK_NUMPAD1 = 325;
            const auto WXK_NUMPAD2 = 326;
            const auto WXK_NUMPAD3 = 327;
            const auto WXK_NUMPAD4 = 328;
            const auto WXK_NUMPAD5 = 329;
            const auto WXK_NUMPAD6 = 330;
            const auto WXK_NUMPAD7 = 331;
            const auto WXK_NUMPAD8 = 332;
            const auto WXK_NUMPAD9 = 333;
            const auto WXK_MULTIPLY = 334;
            const auto WXK_ADD = 335;
            const auto WXK_SEPARATOR = 336;
            const auto WXK_SUBTRACT = 337;
            const auto WXK_DECIMAL = 338;
            const auto WXK_DIVIDE = 339;
            const auto WXK_F1 = 340;
            const auto WXK_F2 = 341;
            const auto WXK_F3 = 342;
            const auto WXK_F4 = 343;
            const auto WXK_F5 = 344;
            const auto WXK_F6 = 345;
            const auto WXK_F7 = 346;
            const auto WXK_F8 = 347;
            const auto WXK_F9 = 348;
            const auto WXK_F10 = 349;
            const auto WXK_F11 = 350;
            const auto WXK_F12 = 351;
            const auto WXK_F13 = 352;
            const auto WXK_F14 = 353;
            const auto WXK_F15 = 354;
            const auto WXK_F16 = 355;
            const auto WXK_F17 = 356;
            const auto WXK_F18 = 357;
            const auto WXK_F19 = 358;
            const auto WXK_F20 = 359;
            const auto WXK_F21 = 360;
            const auto WXK_F22 = 361;
            const auto WXK_F23 = 362;
            const auto WXK_F24 = 363;
            const auto WXK_NUMLOCK = 364;
            const auto WXK_SCROLL = 365;
            const auto WXK_PAGEUP = 366;
            const auto WXK_PAGEDOWN = 367;
            const auto WXK_NUMPAD_SPACE = 368;
            const auto WXK_NUMPAD_TAB = 369;
            const auto WXK_NUMPAD_ENTER = 370;
            const auto WXK_NUMPAD_F1 = 371;
            const auto WXK_NUMPAD_F2 = 372;
            const auto WXK_NUMPAD_F3 = 373;
            const auto WXK_NUMPAD_F4 = 374;
            const auto WXK_NUMPAD_HOME = 375;
            const auto WXK_NUMPAD_LEFT = 376;
            const auto WXK_NUMPAD_UP = 377;
            const auto WXK_NUMPAD_RIGHT = 378;
            const auto WXK_NUMPAD_DOWN = 379;
            const auto WXK_NUMPAD_PAGEUP = 380;
            const auto WXK_NUMPAD_PAGEDOWN = 381;
            const auto WXK_NUMPAD_END = 382;
            const auto WXK_NUMPAD_BEGIN = 383;
            const auto WXK_NUMPAD_INSERT = 384;
            const auto WXK_NUMPAD_DELETE = 385;
            const auto WXK_NUMPAD_EQUAL = 386;
            const auto WXK_NUMPAD_MULTIPLY = 387;
            const auto WXK_NUMPAD_ADD = 388;
            const auto WXK_NUMPAD_SEPARATOR = 389;
            const auto WXK_NUMPAD_SUBTRACT = 390;
            const auto WXK_NUMPAD_DECIMAL = 391;
            const auto WXK_NUMPAD_DIVIDE = 392;
            const auto WXK_WINDOWS_LEFT = 393;
            const auto WXK_WINDOWS_RIGHT = 394;
            const auto WXK_WINDOWS_MENU = 395;
            const auto WXK_BROWSER_BACK = 417;
            const auto WXK_BROWSER_FORWARD = 418;
            const auto WXK_BROWSER_REFRESH = 419;
            const auto WXK_BROWSER_STOP = 420;
            const auto WXK_BROWSER_SEARCH = 421;
            const auto WXK_BROWSER_FAVORITES = 422;
            const auto WXK_BROWSER_HOME = 423;
            const auto WXK_VOLUME_MUTE = 424;
            const auto WXK_VOLUME_DOWN = 425;
            const auto WXK_VOLUME_UP = 426;
            const auto WXK_MEDIA_NEXT_TRACK = 427;
            const auto WXK_MEDIA_PREV_TRACK = 428;
            const auto WXK_MEDIA_STOP = 429;
            const auto WXK_MEDIA_PLAY_PAUSE = 430;
            const auto WXK_LAUNCH_MAIL = 431;

            // special cases
            switch (wxKey) {
                case WXK_BACK: return Qt::Key_Backspace;
                case WXK_TAB: return Qt::Key_Tab;
                case WXK_RETURN: return Qt::Key_Return;
                case WXK_ESCAPE: return Qt::Key_Escape;
                case WXK_SPACE: return Qt::Key_Space;
                case WXK_DELETE: return Qt::Key_Delete;
                case WXK_START: return 0;
                case WXK_LBUTTON: return 0;
                case WXK_RBUTTON: return 0;
                case WXK_CANCEL: return Qt::Key_Cancel;
                case WXK_MBUTTON: return 0;
                case WXK_CLEAR: return Qt::Key_Clear;
                case WXK_SHIFT: return Qt::Key_Shift;
                case WXK_ALT: return Qt::Key_Alt;
                case WXK_CONTROL: return Qt::Key_Control;
                case WXK_MENU: return Qt::Key_Menu;
                case WXK_PAUSE: return Qt::Key_Pause;
                case WXK_CAPITAL: return 0;
                case WXK_END: return Qt::Key_End;
                case WXK_HOME: return Qt::Key_Home;
                case WXK_LEFT: return Qt::Key_Left;
                case WXK_UP: return Qt::Key_Up;
                case WXK_RIGHT: return Qt::Key_Right;
                case WXK_DOWN: return Qt::Key_Down;
                case WXK_SELECT: return Qt::Key_Select;
                case WXK_PRINT: return Qt::Key_Print;
                case WXK_EXECUTE: return Qt::Key_Execute;
                case WXK_SNAPSHOT: return 0;
                case WXK_INSERT: return Qt::Key_Insert;
                case WXK_HELP: return Qt::Key_Help;
                case WXK_NUMPAD0: return Qt::KeypadModifier | Qt::Key_0;
                case WXK_NUMPAD1: return Qt::KeypadModifier | Qt::Key_1;
                case WXK_NUMPAD2: return Qt::KeypadModifier | Qt::Key_2;
                case WXK_NUMPAD3: return Qt::KeypadModifier | Qt::Key_3;
                case WXK_NUMPAD4: return Qt::KeypadModifier | Qt::Key_4;
                case WXK_NUMPAD5: return Qt::KeypadModifier | Qt::Key_5;
                case WXK_NUMPAD6: return Qt::KeypadModifier | Qt::Key_6;
                case WXK_NUMPAD7: return Qt::KeypadModifier | Qt::Key_7;
                case WXK_NUMPAD8: return Qt::KeypadModifier | Qt::Key_8;
                case WXK_NUMPAD9: return Qt::KeypadModifier | Qt::Key_9;
                case WXK_MULTIPLY: return Qt::Key_multiply;
                case WXK_ADD: return Qt::Key_Plus;
                case WXK_SEPARATOR: return 0;
                case WXK_SUBTRACT: return Qt::Key_Minus;
                case WXK_DECIMAL: return Qt::Key_Period;
                case WXK_DIVIDE: return Qt::Key_division;
                case WXK_F1: return Qt::Key_F1;
                case WXK_F2: return Qt::Key_F2;
                case WXK_F3: return Qt::Key_F3;
                case WXK_F4: return Qt::Key_F4;
                case WXK_F5: return Qt::Key_F5;
                case WXK_F6: return Qt::Key_F6;
                case WXK_F7: return Qt::Key_F7;
                case WXK_F8: return Qt::Key_F8;
                case WXK_F9: return Qt::Key_F9;
                case WXK_F10: return Qt::Key_F10;
                case WXK_F11: return Qt::Key_F11;
                case WXK_F12: return Qt::Key_F12;
                case WXK_F13: return Qt::Key_F13;
                case WXK_F14: return Qt::Key_F14;
                case WXK_F15: return Qt::Key_F15;
                case WXK_F16: return Qt::Key_F16;
                case WXK_F17: return Qt::Key_F17;
                case WXK_F18: return Qt::Key_F18;
                case WXK_F19: return Qt::Key_F19;
                case WXK_F20: return Qt::Key_F20;
                case WXK_F21: return Qt::Key_F21;
                case WXK_F22: return Qt::Key_F22;
                case WXK_F23: return Qt::Key_F23;
                case WXK_F24: return Qt::Key_F24;
                case WXK_NUMLOCK: return Qt::Key_NumLock;
                case WXK_SCROLL: return Qt::Key_ScrollLock;
                case WXK_PAGEUP: return Qt::Key_PageUp;
                case WXK_PAGEDOWN: return Qt::Key_PageDown;
                case WXK_NUMPAD_SPACE: return Qt::KeypadModifier | Qt::Key_Space;
                case WXK_NUMPAD_TAB: return Qt::KeypadModifier | Qt::Key_Tab;
                case WXK_NUMPAD_ENTER: return Qt::KeypadModifier | Qt::Key_Enter;
                case WXK_NUMPAD_F1: return Qt::KeypadModifier | Qt::Key_F1;
                case WXK_NUMPAD_F2: return Qt::KeypadModifier | Qt::Key_F2;
                case WXK_NUMPAD_F3: return Qt::KeypadModifier | Qt::Key_F3;
                case WXK_NUMPAD_F4: return Qt::KeypadModifier | Qt::Key_F4;
                case WXK_NUMPAD_HOME: return Qt::KeypadModifier | Qt::Key_Home;
                case WXK_NUMPAD_LEFT: return Qt::KeypadModifier | Qt::Key_Left;
                case WXK_NUMPAD_UP: return Qt::KeypadModifier | Qt::Key_Up;
                case WXK_NUMPAD_RIGHT: return Qt::KeypadModifier | Qt::Key_Right;
                case WXK_NUMPAD_DOWN: return Qt::KeypadModifier | Qt::Key_Down;
                case WXK_NUMPAD_PAGEUP: return Qt::KeypadModifier | Qt::Key_PageUp;
                case WXK_NUMPAD_PAGEDOWN: return Qt::KeypadModifier | Qt::Key_PageDown;
                case WXK_NUMPAD_END: return Qt::KeypadModifier | Qt::Key_End;
                case WXK_NUMPAD_BEGIN: return 0;
                case WXK_NUMPAD_INSERT: return Qt::KeypadModifier | Qt::Key_Insert;
                case WXK_NUMPAD_DELETE: return Qt::KeypadModifier | Qt::Key_Delete;
                case WXK_NUMPAD_EQUAL: return Qt::KeypadModifier | Qt::Key_Equal;
                case WXK_NUMPAD_MULTIPLY: return Qt::KeypadModifier | Qt::Key_multiply;
                case WXK_NUMPAD_ADD: return Qt::KeypadModifier | Qt::Key_Plus;
                case WXK_NUMPAD_SEPARATOR: return 0;
                case WXK_NUMPAD_SUBTRACT: return Qt::KeypadModifier | Qt::Key_Minus;
                case WXK_NUMPAD_DECIMAL: return Qt::KeypadModifier | Qt::Key_Period;
                case WXK_NUMPAD_DIVIDE: return Qt::KeypadModifier | Qt::Key_division;
                case WXK_WINDOWS_LEFT: return Qt::Key_Meta;
                case WXK_WINDOWS_RIGHT: return Qt::Key_Meta;
                case WXK_WINDOWS_MENU: return Qt::Key_ApplicationRight;
                case WXK_BROWSER_BACK: return Qt::Key_Back;
                case WXK_BROWSER_FORWARD: return Qt::Key_Forward;
                case WXK_BROWSER_REFRESH: return Qt::Key_Refresh;
                case WXK_BROWSER_STOP: return Qt::Key_Stop;
                case WXK_BROWSER_SEARCH: return Qt::Key_Search;
                case WXK_BROWSER_FAVORITES: return Qt::Key_Favorites;
                case WXK_BROWSER_HOME: return Qt::Key_HomePage;
                case WXK_VOLUME_MUTE: return Qt::Key_VolumeMute;
                case WXK_VOLUME_DOWN: return Qt::Key_VolumeDown;
                case WXK_VOLUME_UP: return Qt::Key_VolumeUp;
                case WXK_MEDIA_NEXT_TRACK: return Qt::Key_MediaNext;
                case WXK_MEDIA_PREV_TRACK: return Qt::Key_MediaPrevious;
                case WXK_MEDIA_STOP: return Qt::Key_MediaStop;
                case WXK_MEDIA_PLAY_PAUSE: return Qt::Key_MediaPlay;
                case WXK_LAUNCH_MAIL: return Qt::Key_LaunchMail;
                default: break;
            }

            // Map lowercase letters to uppercase
            if (wxKey >= 'a' && wxKey <= 'z') {
                return 'A' + (wxKey - 'a');
            }

            // Pass through ASCII
            if (wxKey >= 0 && wxKey <= 127) {
                return wxKey;
            }

            return 0;
        }
    }
}

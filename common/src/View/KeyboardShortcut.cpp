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

#include <QGuiApplication>
#include <QKeyEvent>
#include <QString>
#include <QTextStream>
#include <QThread>

#include "Ensure.h"

#include <unordered_map>

namespace TrenchBroom {
namespace View {
static constexpr int WXK_BACK = 8;
static constexpr int WXK_TAB = 9;
static constexpr int WXK_RETURN = 13;
static constexpr int WXK_ESCAPE = 27;
static constexpr int WXK_SPACE = 32;
static constexpr int WXK_DELETE = 127;
static constexpr int WXK_START = 300;
static constexpr int WXK_LBUTTON = 301;
static constexpr int WXK_RBUTTON = 302;
static constexpr int WXK_CANCEL = 303;
static constexpr int WXK_MBUTTON = 304;
static constexpr int WXK_CLEAR = 305;
static constexpr int WXK_SHIFT = 306;
static constexpr int WXK_ALT = 307;
static constexpr int WXK_CONTROL = 308;
static constexpr int WXK_MENU = 309;
static constexpr int WXK_PAUSE = 310;
static constexpr int WXK_CAPITAL = 311;
static constexpr int WXK_END = 312;
static constexpr int WXK_HOME = 313;
static constexpr int WXK_LEFT = 314;
static constexpr int WXK_UP = 315;
static constexpr int WXK_RIGHT = 316;
static constexpr int WXK_DOWN = 317;
static constexpr int WXK_SELECT = 318;
static constexpr int WXK_PRINT = 319;
static constexpr int WXK_EXECUTE = 320;
static constexpr int WXK_SNAPSHOT = 321;
static constexpr int WXK_INSERT = 322;
static constexpr int WXK_HELP = 323;
static constexpr int WXK_NUMPAD0 = 324;
static constexpr int WXK_NUMPAD1 = 325;
static constexpr int WXK_NUMPAD2 = 326;
static constexpr int WXK_NUMPAD3 = 327;
static constexpr int WXK_NUMPAD4 = 328;
static constexpr int WXK_NUMPAD5 = 329;
static constexpr int WXK_NUMPAD6 = 330;
static constexpr int WXK_NUMPAD7 = 331;
static constexpr int WXK_NUMPAD8 = 332;
static constexpr int WXK_NUMPAD9 = 333;
static constexpr int WXK_MULTIPLY = 334;
static constexpr int WXK_ADD = 335;
static constexpr int WXK_SEPARATOR = 336;
static constexpr int WXK_SUBTRACT = 337;
static constexpr int WXK_DECIMAL = 338;
static constexpr int WXK_DIVIDE = 339;
static constexpr int WXK_F1 = 340;
static constexpr int WXK_F2 = 341;
static constexpr int WXK_F3 = 342;
static constexpr int WXK_F4 = 343;
static constexpr int WXK_F5 = 344;
static constexpr int WXK_F6 = 345;
static constexpr int WXK_F7 = 346;
static constexpr int WXK_F8 = 347;
static constexpr int WXK_F9 = 348;
static constexpr int WXK_F10 = 349;
static constexpr int WXK_F11 = 350;
static constexpr int WXK_F12 = 351;
static constexpr int WXK_F13 = 352;
static constexpr int WXK_F14 = 353;
static constexpr int WXK_F15 = 354;
static constexpr int WXK_F16 = 355;
static constexpr int WXK_F17 = 356;
static constexpr int WXK_F18 = 357;
static constexpr int WXK_F19 = 358;
static constexpr int WXK_F20 = 359;
static constexpr int WXK_F21 = 360;
static constexpr int WXK_F22 = 361;
static constexpr int WXK_F23 = 362;
static constexpr int WXK_F24 = 363;
static constexpr int WXK_NUMLOCK = 364;
static constexpr int WXK_SCROLL = 365;
static constexpr int WXK_PAGEUP = 366;
static constexpr int WXK_PAGEDOWN = 367;
static constexpr int WXK_NUMPAD_SPACE = 368;
static constexpr int WXK_NUMPAD_TAB = 369;
static constexpr int WXK_NUMPAD_ENTER = 370;
static constexpr int WXK_NUMPAD_F1 = 371;
static constexpr int WXK_NUMPAD_F2 = 372;
static constexpr int WXK_NUMPAD_F3 = 373;
static constexpr int WXK_NUMPAD_F4 = 374;
static constexpr int WXK_NUMPAD_HOME = 375;
static constexpr int WXK_NUMPAD_LEFT = 376;
static constexpr int WXK_NUMPAD_UP = 377;
static constexpr int WXK_NUMPAD_RIGHT = 378;
static constexpr int WXK_NUMPAD_DOWN = 379;
static constexpr int WXK_NUMPAD_PAGEUP = 380;
static constexpr int WXK_NUMPAD_PAGEDOWN = 381;
static constexpr int WXK_NUMPAD_END = 382;
static constexpr int WXK_NUMPAD_BEGIN = 383;
static constexpr int WXK_NUMPAD_INSERT = 384;
static constexpr int WXK_NUMPAD_DELETE = 385;
static constexpr int WXK_NUMPAD_EQUAL = 386;
static constexpr int WXK_NUMPAD_MULTIPLY = 387;
static constexpr int WXK_NUMPAD_ADD = 388;
static constexpr int WXK_NUMPAD_SEPARATOR = 389;
static constexpr int WXK_NUMPAD_SUBTRACT = 390;
static constexpr int WXK_NUMPAD_DECIMAL = 391;
static constexpr int WXK_NUMPAD_DIVIDE = 392;
static constexpr int WXK_WINDOWS_LEFT = 393;
static constexpr int WXK_WINDOWS_RIGHT = 394;
static constexpr int WXK_WINDOWS_MENU = 395;
static constexpr int WXK_BROWSER_BACK = 417;
static constexpr int WXK_BROWSER_FORWARD = 418;
static constexpr int WXK_BROWSER_REFRESH = 419;
static constexpr int WXK_BROWSER_STOP = 420;
static constexpr int WXK_BROWSER_SEARCH = 421;
static constexpr int WXK_BROWSER_FAVORITES = 422;
static constexpr int WXK_BROWSER_HOME = 423;
static constexpr int WXK_VOLUME_MUTE = 424;
static constexpr int WXK_VOLUME_DOWN = 425;
static constexpr int WXK_VOLUME_UP = 426;
static constexpr int WXK_MEDIA_NEXT_TRACK = 427;
static constexpr int WXK_MEDIA_PREV_TRACK = 428;
static constexpr int WXK_MEDIA_STOP = 429;
static constexpr int WXK_MEDIA_PLAY_PAUSE = 430;
static constexpr int WXK_LAUNCH_MAIL = 431;

int wxKeyToQt(const int wxKey) {
  // special cases
  switch (wxKey) {
    case WXK_BACK:
      return Qt::Key_Backspace;
    case WXK_TAB:
      return Qt::Key_Tab;
    case WXK_RETURN:
      return Qt::Key_Return;
    case WXK_ESCAPE:
      return Qt::Key_Escape;
    case WXK_SPACE:
      return Qt::Key_Space;
    case WXK_DELETE:
      return Qt::Key_Delete;
    case WXK_START:
      return 0;
    case WXK_LBUTTON:
      return 0;
    case WXK_RBUTTON:
      return 0;
    case WXK_CANCEL:
      return Qt::Key_Cancel;
    case WXK_MBUTTON:
      return 0;
    case WXK_CLEAR:
      return Qt::Key_Clear;
    case WXK_SHIFT:
      return Qt::Key_Shift;
    case WXK_ALT:
      return Qt::Key_Alt;
    case WXK_CONTROL:
      return Qt::Key_Control;
    case WXK_MENU:
      return Qt::Key_Menu;
    case WXK_PAUSE:
      return Qt::Key_Pause;
    case WXK_CAPITAL:
      return 0;
    case WXK_END:
      return Qt::Key_End;
    case WXK_HOME:
      return Qt::Key_Home;
    case WXK_LEFT:
      return Qt::Key_Left;
    case WXK_UP:
      return Qt::Key_Up;
    case WXK_RIGHT:
      return Qt::Key_Right;
    case WXK_DOWN:
      return Qt::Key_Down;
    case WXK_SELECT:
      return Qt::Key_Select;
    case WXK_PRINT:
      return Qt::Key_Print;
    case WXK_EXECUTE:
      return Qt::Key_Execute;
    case WXK_SNAPSHOT:
      return 0;
    case WXK_INSERT:
      return Qt::Key_Insert;
    case WXK_HELP:
      return Qt::Key_Help;
    case WXK_NUMPAD0:
      return Qt::KeypadModifier | Qt::Key_0;
    case WXK_NUMPAD1:
      return Qt::KeypadModifier | Qt::Key_1;
    case WXK_NUMPAD2:
      return Qt::KeypadModifier | Qt::Key_2;
    case WXK_NUMPAD3:
      return Qt::KeypadModifier | Qt::Key_3;
    case WXK_NUMPAD4:
      return Qt::KeypadModifier | Qt::Key_4;
    case WXK_NUMPAD5:
      return Qt::KeypadModifier | Qt::Key_5;
    case WXK_NUMPAD6:
      return Qt::KeypadModifier | Qt::Key_6;
    case WXK_NUMPAD7:
      return Qt::KeypadModifier | Qt::Key_7;
    case WXK_NUMPAD8:
      return Qt::KeypadModifier | Qt::Key_8;
    case WXK_NUMPAD9:
      return Qt::KeypadModifier | Qt::Key_9;
    case WXK_MULTIPLY:
      return Qt::Key_multiply;
    case WXK_ADD:
      return Qt::Key_Plus;
    case WXK_SEPARATOR:
      return 0;
    case WXK_SUBTRACT:
      return Qt::Key_Minus;
    case WXK_DECIMAL:
      return Qt::Key_Period;
    case WXK_DIVIDE:
      return Qt::Key_division;
    case WXK_F1:
      return Qt::Key_F1;
    case WXK_F2:
      return Qt::Key_F2;
    case WXK_F3:
      return Qt::Key_F3;
    case WXK_F4:
      return Qt::Key_F4;
    case WXK_F5:
      return Qt::Key_F5;
    case WXK_F6:
      return Qt::Key_F6;
    case WXK_F7:
      return Qt::Key_F7;
    case WXK_F8:
      return Qt::Key_F8;
    case WXK_F9:
      return Qt::Key_F9;
    case WXK_F10:
      return Qt::Key_F10;
    case WXK_F11:
      return Qt::Key_F11;
    case WXK_F12:
      return Qt::Key_F12;
    case WXK_F13:
      return Qt::Key_F13;
    case WXK_F14:
      return Qt::Key_F14;
    case WXK_F15:
      return Qt::Key_F15;
    case WXK_F16:
      return Qt::Key_F16;
    case WXK_F17:
      return Qt::Key_F17;
    case WXK_F18:
      return Qt::Key_F18;
    case WXK_F19:
      return Qt::Key_F19;
    case WXK_F20:
      return Qt::Key_F20;
    case WXK_F21:
      return Qt::Key_F21;
    case WXK_F22:
      return Qt::Key_F22;
    case WXK_F23:
      return Qt::Key_F23;
    case WXK_F24:
      return Qt::Key_F24;
    case WXK_NUMLOCK:
      return Qt::Key_NumLock;
    case WXK_SCROLL:
      return Qt::Key_ScrollLock;
    case WXK_PAGEUP:
      return Qt::Key_PageUp;
    case WXK_PAGEDOWN:
      return Qt::Key_PageDown;
    case WXK_NUMPAD_SPACE:
      return Qt::KeypadModifier | Qt::Key_Space;
    case WXK_NUMPAD_TAB:
      return Qt::KeypadModifier | Qt::Key_Tab;
    case WXK_NUMPAD_ENTER:
      return Qt::KeypadModifier | Qt::Key_Enter;
    case WXK_NUMPAD_F1:
      return Qt::KeypadModifier | Qt::Key_F1;
    case WXK_NUMPAD_F2:
      return Qt::KeypadModifier | Qt::Key_F2;
    case WXK_NUMPAD_F3:
      return Qt::KeypadModifier | Qt::Key_F3;
    case WXK_NUMPAD_F4:
      return Qt::KeypadModifier | Qt::Key_F4;
    case WXK_NUMPAD_HOME:
      return Qt::KeypadModifier | Qt::Key_Home;
    case WXK_NUMPAD_LEFT:
      return Qt::KeypadModifier | Qt::Key_Left;
    case WXK_NUMPAD_UP:
      return Qt::KeypadModifier | Qt::Key_Up;
    case WXK_NUMPAD_RIGHT:
      return Qt::KeypadModifier | Qt::Key_Right;
    case WXK_NUMPAD_DOWN:
      return Qt::KeypadModifier | Qt::Key_Down;
    case WXK_NUMPAD_PAGEUP:
      return Qt::KeypadModifier | Qt::Key_PageUp;
    case WXK_NUMPAD_PAGEDOWN:
      return Qt::KeypadModifier | Qt::Key_PageDown;
    case WXK_NUMPAD_END:
      return Qt::KeypadModifier | Qt::Key_End;
    case WXK_NUMPAD_BEGIN:
      return 0;
    case WXK_NUMPAD_INSERT:
      return Qt::KeypadModifier | Qt::Key_Insert;
    case WXK_NUMPAD_DELETE:
      return Qt::KeypadModifier | Qt::Key_Delete;
    case WXK_NUMPAD_EQUAL:
      return Qt::KeypadModifier | Qt::Key_Equal;
    case WXK_NUMPAD_MULTIPLY:
      return Qt::KeypadModifier | Qt::Key_multiply;
    case WXK_NUMPAD_ADD:
      return Qt::KeypadModifier | Qt::Key_Plus;
    case WXK_NUMPAD_SEPARATOR:
      return 0;
    case WXK_NUMPAD_SUBTRACT:
      return Qt::KeypadModifier | Qt::Key_Minus;
    case WXK_NUMPAD_DECIMAL:
      return Qt::KeypadModifier | Qt::Key_Period;
    case WXK_NUMPAD_DIVIDE:
      return Qt::KeypadModifier | Qt::Key_division;
    case WXK_WINDOWS_LEFT:
      return Qt::Key_Meta;
    case WXK_WINDOWS_RIGHT:
      return Qt::Key_Meta;
    case WXK_WINDOWS_MENU:
      return Qt::Key_ApplicationRight;
    case WXK_BROWSER_BACK:
      return Qt::Key_Back;
    case WXK_BROWSER_FORWARD:
      return Qt::Key_Forward;
    case WXK_BROWSER_REFRESH:
      return Qt::Key_Refresh;
    case WXK_BROWSER_STOP:
      return Qt::Key_Stop;
    case WXK_BROWSER_SEARCH:
      return Qt::Key_Search;
    case WXK_BROWSER_FAVORITES:
      return Qt::Key_Favorites;
    case WXK_BROWSER_HOME:
      return Qt::Key_HomePage;
    case WXK_VOLUME_MUTE:
      return Qt::Key_VolumeMute;
    case WXK_VOLUME_DOWN:
      return Qt::Key_VolumeDown;
    case WXK_VOLUME_UP:
      return Qt::Key_VolumeUp;
    case WXK_MEDIA_NEXT_TRACK:
      return Qt::Key_MediaNext;
    case WXK_MEDIA_PREV_TRACK:
      return Qt::Key_MediaPrevious;
    case WXK_MEDIA_STOP:
      return Qt::Key_MediaStop;
    case WXK_MEDIA_PLAY_PAUSE:
      return Qt::Key_MediaPlay;
    case WXK_LAUNCH_MAIL:
      return Qt::Key_LaunchMail;
    default:
      break;
  }

  // Pass through ASCII
  if (wxKey >= 0 && wxKey <= 127) {
    return wxKey;
  }

  return 0;
}

static std::unordered_map<int, int> qtKeyToWxMap() {
  std::unordered_map<int, int> qtToWx;

  // invert wxKeyToQt()
  for (int wxKey = 1; wxKey <= WXK_LAUNCH_MAIL; ++wxKey) {
    const int qtKey = wxKeyToQt(wxKey);

    if (qtKey != 0) {
      qtToWx[qtKey] = wxKey;
    }
  }

  return qtToWx;
}

int qtKeyToWx(const int qtKey) {
  ensure(
    qApp->thread() == QThread::currentThread(), "qtKeyToWx() can only be used on the main thread");

  static std::unordered_map<int, int> qtToWx;
  if (qtToWx.empty()) {
    qtToWx = qtKeyToWxMap();
  }

  auto it = qtToWx.find(qtKey);
  if (it != qtToWx.end()) {
    return it->second;
  }
  return 0;
}

std::optional<QKeySequence> keySequenceFromV1Settings(const QString& string) {
  auto inCopy = QString(string);
  auto inStream = QTextStream(&inCopy);

  int wxKey, mod1, mod2, mod3;
  char sep1, sep2, sep3;

  inStream >> wxKey >> sep1 >> mod1 >> sep2 >> mod2 >> sep3 >> mod3;

  if (wxKey < 0 || mod1 < 0 || mod2 < 0 || mod3 < 0) {
    return {};
  }
  if (sep1 != ':' || sep2 != ':' || sep3 != ':') {
    return {};
  }
  if (inStream.status() != QTextStream::Ok) {
    return {};
  }

  auto wxModToQt = [](const int wxMod) -> Qt::KeyboardModifier {
    switch (wxMod) {
      case WXK_SHIFT:
        return Qt::ShiftModifier;
      case WXK_CONTROL:
        return Qt::ControlModifier;
      case WXK_ALT:
        return Qt::AltModifier;
      default:
        return Qt::NoModifier;
    }
  };

  int qtKey = wxKeyToQt(wxKey);
  if (qtKey < 0) {
    return {};
  }

  qtKey |= wxModToQt(mod1);
  qtKey |= wxModToQt(mod2);
  qtKey |= wxModToQt(mod3);

  return {QKeySequence(qtKey)};
}

QString keySequenceToV1Settings(const QKeySequence& ks) {
  if (ks.count() != 1) {
    return "";
  }

  if (ks[0] < 0) {
    return "";
  }

  const auto qtKey = static_cast<unsigned int>(ks[0]);
  const auto QtShift = static_cast<unsigned int>(Qt::ShiftModifier);
  const auto QtCtrl = static_cast<unsigned int>(Qt::ControlModifier);
  const auto QtAlt = static_cast<unsigned int>(Qt::AltModifier);

  const auto qtKeyWithoutModifier = qtKey & ~(QtShift | QtCtrl | QtAlt);
  const int wxKey = qtKeyToWx(static_cast<int>(qtKeyWithoutModifier));

  std::vector<int> modifiers;
  if (qtKey & Qt::ControlModifier) {
    modifiers.push_back(WXK_CONTROL);
  }
  if (qtKey & Qt::AltModifier) {
    modifiers.push_back(WXK_ALT);
  }
  if (qtKey & Qt::ShiftModifier) {
    modifiers.push_back(WXK_SHIFT);
  }
  while (modifiers.size() < 3) {
    modifiers.push_back(0);
  }

  QString result;
  QTextStream stream(&result);

  stream << wxKey << ':' << modifiers[0] << ':' << modifiers[1] << ':' << modifiers[2];

  return result;
}
} // namespace View
} // namespace TrenchBroom

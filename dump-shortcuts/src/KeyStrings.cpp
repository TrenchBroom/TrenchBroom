/*
 Copyright (C) 2010 Kristian Duske

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

#include "KeyStrings.h"

#include <QKeySequence>

#include <cassert>

namespace tb::ui
{
KeyStrings::KeyStrings()
{
  putKey(Qt::Key_Escape);
  putKey(Qt::Key_Tab);
  putKey(Qt::Key_Backtab);
  putKey(Qt::Key_Backspace);
  putKey(Qt::Key_Return);
  putKey(Qt::Key_Enter);
  putKey(Qt::Key_Insert);
  putKey(Qt::Key_Delete);
  putKey(Qt::Key_Pause);
  putKey(Qt::Key_Print);
  putKey(Qt::Key_SysReq);
  putKey(Qt::Key_Clear);
  putKey(Qt::Key_Home);
  putKey(Qt::Key_End);
  putKey(Qt::Key_Left);
  putKey(Qt::Key_Up);
  putKey(Qt::Key_Right);
  putKey(Qt::Key_Down);
  putKey(Qt::Key_PageUp);
  putKey(Qt::Key_PageDown);
  putModifier(Qt::SHIFT);
  putModifier(Qt::CTRL);
  putModifier(Qt::META);
  putModifier(Qt::ALT);
  putKey(Qt::Key_CapsLock);
  putKey(Qt::Key_NumLock);
  putKey(Qt::Key_ScrollLock);
  putKey(Qt::Key_F1);
  putKey(Qt::Key_F2);
  putKey(Qt::Key_F3);
  putKey(Qt::Key_F4);
  putKey(Qt::Key_F5);
  putKey(Qt::Key_F6);
  putKey(Qt::Key_F7);
  putKey(Qt::Key_F8);
  putKey(Qt::Key_F9);
  putKey(Qt::Key_F10);
  putKey(Qt::Key_F11);
  putKey(Qt::Key_F12);
  putKey(Qt::Key_F13);
  putKey(Qt::Key_F14);
  putKey(Qt::Key_F15);
  putKey(Qt::Key_F16);
  putKey(Qt::Key_F17);
  putKey(Qt::Key_F18);
  putKey(Qt::Key_F19);
  putKey(Qt::Key_F20);
  putKey(Qt::Key_F21);
  putKey(Qt::Key_F22);
  putKey(Qt::Key_F23);
  putKey(Qt::Key_F24);
  putKey(Qt::Key_F25);
  putKey(Qt::Key_F26);
  putKey(Qt::Key_F27);
  putKey(Qt::Key_F28);
  putKey(Qt::Key_F29);
  putKey(Qt::Key_F30);
  putKey(Qt::Key_F31);
  putKey(Qt::Key_F32);
  putKey(Qt::Key_F33);
  putKey(Qt::Key_F34);
  putKey(Qt::Key_F35);
  putKey(Qt::Key_Super_L);
  putKey(Qt::Key_Super_R);
  putKey(Qt::Key_Menu);
  putKey(Qt::Key_Hyper_L);
  putKey(Qt::Key_Hyper_R);
  putKey(Qt::Key_Help);
  putKey(Qt::Key_Direction_L);
  putKey(Qt::Key_Direction_R);
  putKey(Qt::Key_Space);
  putKey(Qt::Key_Any);
  putKey(Qt::Key_Exclam);
  putKey(Qt::Key_QuoteDbl);
  putKey(Qt::Key_NumberSign);
  putKey(Qt::Key_Dollar);
  putKey(Qt::Key_Percent);
  putKey(Qt::Key_Ampersand);
  putKey(Qt::Key_Apostrophe);
  putKey(Qt::Key_ParenLeft);
  putKey(Qt::Key_ParenRight);
  putKey(Qt::Key_Asterisk);
  putKey(Qt::Key_Plus);
  putKey(Qt::Key_Comma);
  putKey(Qt::Key_Minus);
  putKey(Qt::Key_Period);
  putKey(Qt::Key_Slash);
  putKey(Qt::Key_0);
  putKey(Qt::Key_1);
  putKey(Qt::Key_2);
  putKey(Qt::Key_3);
  putKey(Qt::Key_4);
  putKey(Qt::Key_5);
  putKey(Qt::Key_6);
  putKey(Qt::Key_7);
  putKey(Qt::Key_8);
  putKey(Qt::Key_9);
  putKey(Qt::Key_Colon);
  putKey(Qt::Key_Semicolon);
  putKey(Qt::Key_Less);
  putKey(Qt::Key_Equal);
  putKey(Qt::Key_Greater);
  putKey(Qt::Key_Question);
  putKey(Qt::Key_At);
  putKey(Qt::Key_A);
  putKey(Qt::Key_B);
  putKey(Qt::Key_C);
  putKey(Qt::Key_D);
  putKey(Qt::Key_E);
  putKey(Qt::Key_F);
  putKey(Qt::Key_G);
  putKey(Qt::Key_H);
  putKey(Qt::Key_I);
  putKey(Qt::Key_J);
  putKey(Qt::Key_K);
  putKey(Qt::Key_L);
  putKey(Qt::Key_M);
  putKey(Qt::Key_N);
  putKey(Qt::Key_O);
  putKey(Qt::Key_P);
  putKey(Qt::Key_Q);
  putKey(Qt::Key_R);
  putKey(Qt::Key_S);
  putKey(Qt::Key_T);
  putKey(Qt::Key_U);
  putKey(Qt::Key_V);
  putKey(Qt::Key_W);
  putKey(Qt::Key_X);
  putKey(Qt::Key_Y);
  putKey(Qt::Key_Z);
  putKey(Qt::Key_BracketLeft);
  putKey(Qt::Key_Backslash);
  putKey(Qt::Key_BracketRight);
  putKey(Qt::Key_AsciiCircum);
  putKey(Qt::Key_Underscore);
  putKey(Qt::Key_QuoteLeft);
  putKey(Qt::Key_BraceLeft);
  putKey(Qt::Key_Bar);
  putKey(Qt::Key_BraceRight);
  putKey(Qt::Key_AsciiTilde);

  putKey(Qt::Key_nobreakspace);
  putKey(Qt::Key_exclamdown);
  putKey(Qt::Key_cent);
  putKey(Qt::Key_sterling);
  putKey(Qt::Key_currency);
  putKey(Qt::Key_yen);
  putKey(Qt::Key_brokenbar);
  putKey(Qt::Key_section);
  putKey(Qt::Key_diaeresis);
  putKey(Qt::Key_copyright);
  putKey(Qt::Key_ordfeminine);
  putKey(Qt::Key_guillemotleft);
  putKey(Qt::Key_notsign);
  putKey(Qt::Key_hyphen);
  putKey(Qt::Key_registered);
  putKey(Qt::Key_macron);
  putKey(Qt::Key_degree);
  putKey(Qt::Key_plusminus);
  putKey(Qt::Key_twosuperior);
  putKey(Qt::Key_threesuperior);
  putKey(Qt::Key_acute);
  putKey(Qt::Key_micro);
  putKey(Qt::Key_paragraph);
  putKey(Qt::Key_periodcentered);
  putKey(Qt::Key_cedilla);
  putKey(Qt::Key_onesuperior);
  putKey(Qt::Key_masculine);
  putKey(Qt::Key_guillemotright);
  putKey(Qt::Key_onequarter);
  putKey(Qt::Key_onehalf);
  putKey(Qt::Key_threequarters);
  putKey(Qt::Key_questiondown);
  putKey(Qt::Key_Agrave);
  putKey(Qt::Key_Aacute);
  putKey(Qt::Key_Acircumflex);
  putKey(Qt::Key_Atilde);
  putKey(Qt::Key_Adiaeresis);
  putKey(Qt::Key_Aring);
  putKey(Qt::Key_AE);
  putKey(Qt::Key_Ccedilla);
  putKey(Qt::Key_Egrave);
  putKey(Qt::Key_Eacute);
  putKey(Qt::Key_Ecircumflex);
  putKey(Qt::Key_Ediaeresis);
  putKey(Qt::Key_Igrave);
  putKey(Qt::Key_Iacute);
  putKey(Qt::Key_Icircumflex);
  putKey(Qt::Key_Idiaeresis);
  putKey(Qt::Key_ETH);
  putKey(Qt::Key_Ntilde);
  putKey(Qt::Key_Ograve);
  putKey(Qt::Key_Oacute);
  putKey(Qt::Key_Ocircumflex);
  putKey(Qt::Key_Otilde);
  putKey(Qt::Key_Odiaeresis);
  putKey(Qt::Key_multiply);
  putKey(Qt::Key_Ooblique);
  putKey(Qt::Key_Ugrave);
  putKey(Qt::Key_Uacute);
  putKey(Qt::Key_Ucircumflex);
  putKey(Qt::Key_Udiaeresis);
  putKey(Qt::Key_Yacute);
  putKey(Qt::Key_THORN);
  putKey(Qt::Key_ssharp);
  putKey(Qt::Key_division);
  putKey(Qt::Key_ydiaeresis);
}

KeyStrings::const_iterator KeyStrings::begin() const
{
  return std::begin(m_keys);
}

KeyStrings::const_iterator KeyStrings::end() const
{
  return std::end(m_keys);
}

void KeyStrings::putKey(const Qt::Key key)
{
  const auto keySequence = QKeySequence{key};

  m_keys.emplace_back(
    keySequence.toString(QKeySequence::PortableText),
    keySequence.toString(QKeySequence::NativeText));
}

void KeyStrings::putModifier(int key)
{
  const auto keySequence = QKeySequence{key};

  // QKeySequence doesn't totally support being given just a modifier
  // but it does seem to handle the key codes like Qt::SHIFT, which
  // it turns into native text as "Shift+" or the Shift symbol on macOS,
  // and portable text as "Shift+".

  auto portableLabel = keySequence.toString(QKeySequence::PortableText);
  assert(portableLabel.endsWith("+")); // This will be something like "Ctrl+"
  portableLabel.chop(1);               // Remove last character

  auto nativeLabel = keySequence.toString(QKeySequence::NativeText);
  if (nativeLabel.endsWith("+"))
  {
    // On Linux we get nativeLabel as something like "Ctrl+"
    // On macOS it's just the special Command character, with no +
    nativeLabel.chop(1); // Remove last character
  }

  m_keys.emplace_back(portableLabel, nativeLabel);
}

} // namespace tb::ui

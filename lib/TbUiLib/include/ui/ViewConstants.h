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

#pragma once

#include <QtSystemDetection>

namespace tb::ui
{
namespace LayoutConstants
{
#if defined(Q_OS_WIN)
static constexpr int DialogOuterMargin = 20;
static constexpr int DialogButtonTopMargin = 10;
static constexpr int DialogButtonLeftMargin = 7;
static constexpr int DialogButtonRightMargin = 7;
static constexpr int DialogButtonBottomMargin = 10;
static constexpr int WideHMargin = 8;
static constexpr int MediumHMargin = 6;
static constexpr int NarrowHMargin = 4;
static constexpr int WideVMargin = 8;
static constexpr int MediumVMargin = 4;
static constexpr int NarrowVMargin = 2;
static constexpr int TabBarBarLeftMargin = 10;
#elif defined(Q_OS_MACOS)
static constexpr int DialogOuterMargin = 20;
static constexpr int DialogButtonTopMargin = 10;
static constexpr int DialogButtonLeftMargin = 10;
static constexpr int DialogButtonRightMargin = 10;
static constexpr int DialogButtonBottomMargin = 10;
static constexpr int WideHMargin = 8;
static constexpr int MediumHMargin = 4;
static constexpr int NarrowHMargin = 2;
static constexpr int WideVMargin = 8;
static constexpr int MediumVMargin = 4;
static constexpr int NarrowVMargin = 2;
static constexpr int TabBarBarLeftMargin = 10;
#else
static constexpr int DialogOuterMargin = 20;
static constexpr int DialogButtonTopMargin = 10;
static constexpr int DialogButtonLeftMargin = 8;
static constexpr int DialogButtonRightMargin = 8;
static constexpr int DialogButtonBottomMargin = 10;
static constexpr int WideHMargin = 8;
static constexpr int MediumHMargin = 4;
static constexpr int NarrowHMargin = 2;
static constexpr int WideVMargin = 8;
static constexpr int MediumVMargin = 4;
static constexpr int NarrowVMargin = 2;
static constexpr int TabBarBarLeftMargin = 10;
#endif
} // namespace LayoutConstants

} // namespace tb::ui

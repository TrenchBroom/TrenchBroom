/*
 Copyright (C) 2010-2012 Kristian Duske

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

#include "WinPreferences.h"
#include "Controller/Tool.h"

namespace TrenchBroom {
	namespace Model {
			void WinPreferences::loadPlatformDefaults() {
				m_cameraKey = Controller::TB_MK_SHIFT;
				m_cameraOrbitKey = Controller::TB_MK_SHIFT | Controller::TB_MK_CTRL;
				m_quakePath = "C:\\Program Files\\Quake";
			}

			void WinPreferences::loadPreferences() {
			}

			void WinPreferences::saveInt(const string& key, int value) {

			}

			void WinPreferences::saveBool(const string& key, bool value) {
			}

			void WinPreferences::saveFloat(const string& key, float value) {
			}

			void WinPreferences::saveString(const string& key, const string& value) {
			}
			
			void WinPreferences::saveVec4f(const string& key, const Vec4f& value) {
			}

			bool WinPreferences::saveInstantly() {
				return false;
			}
	}
}

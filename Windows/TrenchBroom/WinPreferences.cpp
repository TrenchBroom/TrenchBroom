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
#include "stdafx.h"
#include "Controller/Tool.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdio>

namespace TrenchBroom {
	namespace Model {
		static std::string Section = "Preferences";

			void WinPreferences::loadPlatformDefaults() {
				m_cameraKey = Controller::TB_MK_SHIFT;
				m_cameraOrbitKey = Controller::TB_MK_SHIFT | Controller::TB_MK_CTRL;
				m_quakePath = "C:\\Program Files\\Quake";
			}

			bool WinPreferences::loadInt(const std::string& key, int& value) {
				value = AfxGetApp()->GetProfileIntA(Section.c_str(), key.c_str(), value);
				return true;
			}

            bool WinPreferences::loadFloat(const std::string& key, float& value) {
				std::stringstream sstrValue;
				sstrValue << value;
				std::string strValue = sstrValue.str();
				loadString(key, strValue);
				value = static_cast<float>(atof(strValue.c_str()));
				return true;
			}

            bool WinPreferences::loadBool(const std::string& key, bool& value) {
				int intVal = value ? 1 : 0;
				loadInt(key, intVal);
				value = intVal != 0;
				return true;
			}

            bool WinPreferences::loadString(const std::string& key, std::string& value) {
				value = AfxGetApp()->GetProfileStringA(Section.c_str(), key.c_str(), value.c_str());
				return true;
			}

            bool WinPreferences::loadVec3f(const std::string& key, Vec3f& value) {
				std::string strValue = value.asString();
				loadString(key, strValue);
				value = Vec3f(strValue);
				return true;
			}

            bool WinPreferences::loadVec4f(const std::string& key, Vec4f& value) {
				std::string strValue = value.asString();
				loadString(key, strValue);
				value = Vec4f(strValue);
				return true;
			}

			void WinPreferences::saveInt(const std::string& key, int value) {
				AfxGetApp()->WriteProfileInt(Section.c_str(), key.c_str(), value);
			}

			void WinPreferences::saveBool(const std::string& key, bool value) {
				saveInt(key, value ? 1 : 0);
			}

			void WinPreferences::saveFloat(const std::string& key, float value) {
				std::stringstream strValue;
				strValue << value;
				saveString(key, strValue.str());
			}

			void WinPreferences::saveString(const std::string& key, const std::string& value) {
				AfxGetApp()->WriteProfileStringA(Section.c_str(), key.c_str(), value.c_str());
			}
			
			void WinPreferences::saveVec3f(const std::string& key, const Vec3f& value) {
				saveString(key, value.asString());
			}

			void WinPreferences::saveVec4f(const std::string& key, const Vec4f& value) {
				saveString(key, value.asString());
			}

			bool WinPreferences::saveInstantly() {
				return false;
			}
	}
}

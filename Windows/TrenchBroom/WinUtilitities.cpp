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

#include "WinUtilities.h"

#include <cassert>

namespace TrenchBroom {
	namespace Gui {
		CMenu* createEntityMenu(const TrenchBroom::Model::EntityDefinitionList& definitions, unsigned int baseId) {
			TCHAR name[1024];
			MENUITEMINFO itemInfo;
			itemInfo.cbSize = sizeof(MENUITEMINFO);
			itemInfo.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_STRING;
			itemInfo.fType = MFT_STRING;
			itemInfo.fState = MFS_ENABLED;
			itemInfo.dwTypeData = name;

			BOOL success;
			CMenu* entityMenu = new CMenu();
			success = entityMenu->CreateMenu();
			assert(success);

			for (unsigned int i = 0; i < definitions.size(); i++) {
				TrenchBroom::Model::EntityDefinitionPtr definition = definitions[i];
				definition->name.copy(name, definition->name.length(), 0);
				name[definition->name.length()] = 0;

				itemInfo.wID = baseId++;
				itemInfo.cch = definition->name.length();
				success = entityMenu->InsertMenuItem(i, &itemInfo, TRUE);
				assert(success);
			}

			return entityMenu;
		}
	}
}
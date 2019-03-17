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

#include "PersistentSplitterWindow4.h"

#include "View/SplitterWindow4.h"

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        const double PersistentSplitterWindow4::Scaling = 10000.0;

        PersistentSplitterWindow4::PersistentSplitterWindow4(SplitterWindow4* obj) :
        wxPersistentWindow(obj) {}

        wxString PersistentSplitterWindow4::GetKind() const {
            return "SplitterWindow4";
        }

        void PersistentSplitterWindow4::Save() const {
            const SplitterWindow4* window = Get();
            const wxRealPoint ratios = getSplitRatios(window);
            const wxPoint scaledRatios(static_cast<int>(Scaling * ratios.x),
                                       static_cast<int>(Scaling * ratios.y));
            SaveValue("SplitRatioX", scaledRatios.x);
            SaveValue("SplitRatioY", scaledRatios.y);
        }

        bool PersistentSplitterWindow4::Restore() {
            wxPoint scaledRatios(static_cast<int>(-Scaling), static_cast<int>(-Scaling));
            if (!RestoreValue("SplitRatioX", &scaledRatios.x))
                return false;
            if (!RestoreValue("SplitRatioY", &scaledRatios.y))
                return false;

            SplitterWindow4* window = Get();
            window->m_initialSplitRatios = wxRealPoint(std::max(-1.0, std::min(1.0, static_cast<double>(scaledRatios.x) / Scaling)),
                                                       std::max(-1.0, std::min(1.0, static_cast<double>(scaledRatios.y) / Scaling)));
            return true;
        }

        wxRealPoint PersistentSplitterWindow4::getSplitRatios(const SplitterWindow4* window) const {
            wxRealPoint result;
            result.x = window->m_currentSplitRatios.x == -1.0 ? window->m_initialSplitRatios.x : window->m_currentSplitRatios.x;
            result.y = window->m_currentSplitRatios.y == -1.0 ? window->m_initialSplitRatios.y : window->m_currentSplitRatios.y;
            return result;
        }
    }
}

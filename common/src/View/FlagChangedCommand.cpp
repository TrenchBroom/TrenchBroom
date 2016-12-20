/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "FlagChangedCommand.h"

wxDEFINE_EVENT(FLAG_CHANGED_EVENT, TrenchBroom::View::FlagChangedCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(FlagChangedCommand, wxNotifyEvent)
        FlagChangedCommand::FlagChangedCommand() :
        wxNotifyEvent(FLAG_CHANGED_EVENT, wxID_ANY),
        m_index(0),
        m_flagSetValue(0),
        m_flagMixedValue(0) {}
        
        void FlagChangedCommand::setValues(const size_t index, const int flagSetValue, const int flagMixedValue) {
            m_index = index;
            m_flagSetValue = flagSetValue;
            m_flagMixedValue = flagMixedValue;
        }
        
        int FlagChangedCommand::flagSetValue() const {
            return m_flagSetValue;
        }
        
        int FlagChangedCommand::flagMixedValue() const {
            return m_flagMixedValue;
        }
        
        size_t FlagChangedCommand::index() const {
            return m_index;
        }
        
        bool FlagChangedCommand::flagSet() const {
            return (m_flagSetValue & (1 << m_index)) != 0;
        }

        wxEvent* FlagChangedCommand::Clone() const {
            return new FlagChangedCommand(*this);
        }
        
    }
}

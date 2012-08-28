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

#ifndef TrenchBroom_ProgressIndicator_h
#define TrenchBroom_ProgressIndicator_h

#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace Utility {
        class ProgressIndicator {
        private:
            int m_maxValue;
            float m_percent;
        protected:
            virtual void doReset() = 0;
            virtual void doUpdate() = 0;
        public:
            ProgressIndicator() : m_maxValue(100), m_percent(0.0f) {}
            virtual ~ProgressIndicator() {};
            
            int maxValue() {
                return m_maxValue;
            }
            
            float percent() {
                return m_percent;
            }
            
            void reset(int maxValue) {
                assert(maxValue > 0);
                m_maxValue = maxValue;
                doReset();
            };
            
            void update(int progress) {
                float percent = static_cast<float>(progress) / m_maxValue * 100.0f;
                if (static_cast<int>(m_percent) == static_cast<int>(percent)) return;
                m_percent = percent;
                doUpdate();
            }
            
            virtual void setText(const String& text) = 0;
        };
    }
}

#endif

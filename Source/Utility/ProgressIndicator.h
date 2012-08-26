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

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Utility {
        class ProgressIndicator {
        private:
            float m_maxValue;
            float m_percent;
        protected:
            virtual void doReset() = 0;
            virtual void doUpdate() = 0;
        public:
            ProgressIndicator(float maxValue) : m_maxValue(maxValue), m_percent(0) {
                assert(m_maxValue > 0);
            }
            virtual ~ProgressIndicator() {};
            
            float maxValue() {
                return m_maxValue;
            }
            
            float percent() {
                return m_percent;
            }
            
            void reset(float maxValue) {
                assert(maxValue > 0);
                m_maxValue = maxValue;
                doReset();
            };
            
            void update(float progress) {
                float percent = progress / m_maxValue * 100;
                if ((int)m_percent == (int)percent) return;
                m_percent = percent;
                doUpdate();
            }
            
            virtual void setText(const std::string& text) = 0;
        };
    }
}

#endif

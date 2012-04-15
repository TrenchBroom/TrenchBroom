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

using namespace std;

namespace TrenchBroom {
    namespace Controller {
        class ProgressIndicator {
        private:
            float m_max;
            float m_percent;
        protected:
            virtual void doReset() = 0;
            virtual void doUpdate() = 0;
        public:
            ProgressIndicator(float max) : m_max(max), m_percent(0) {
                assert(m_max > 0);
            }
            virtual ~ProgressIndicator() {};
            
            float max() {
                return m_max;
            }
            
            float percent() {
                return m_percent;
            }
            
            void reset(float max) {
                assert(max > 0);
                m_max = max;
                doReset();
            };
            
            void update(float progress) {
                float percent = progress / m_max * 100;
                if ((int)m_percent == (int)percent) return;
                m_percent = percent;
                doUpdate();
            }
            
            virtual void setText(const string& text) = 0;
        };
    }
}

#endif

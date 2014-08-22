/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__BrushContentType__
#define __TrenchBroom__BrushContentType__

#include "StringUtils.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeEvaluator;
        
        class BrushContentType {
        public:
            typedef int FlagType;
        private:
            String m_name;
            FlagType m_flagValue;
            BrushContentTypeEvaluator* m_evaluator;
        public:
            BrushContentType(const String& name, FlagType flagValue, BrushContentTypeEvaluator* evaluator);
            ~BrushContentType();
            
            const String& name() const;
            FlagType flagValue() const;
            
            bool evaluate(const Brush* brush) const;
        };
    }
}

#endif /* defined(__TrenchBroom__BrushContentType__) */

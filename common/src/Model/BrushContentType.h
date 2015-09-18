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

#ifndef TrenchBroom_BrushContentType
#define TrenchBroom_BrushContentType

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeEvaluator;
        
        class BrushContentType {
        public:
            typedef int FlagType;
            typedef std::vector<BrushContentType> List;
            static const List EmptyList;
        private:
            typedef std::tr1::shared_ptr<BrushContentTypeEvaluator> EvaluatorPtr;
            
            String m_name;
            bool m_transparent;
            FlagType m_flagValue;
            EvaluatorPtr m_evaluator;
        public:
            BrushContentType(const String& name, bool transparent, FlagType flagValue, BrushContentTypeEvaluator* evaluator);
            
            const String& name() const;
            bool transparent() const;
            FlagType flagValue() const;
            
            bool evaluate(const Brush* brush) const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushContentType) */

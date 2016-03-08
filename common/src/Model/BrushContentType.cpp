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

#include "BrushContentType.h"

#include "Model/BrushContentTypeEvaluator.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        const BrushContentType::List BrushContentType::EmptyList = BrushContentType::List();
        
        BrushContentType::BrushContentType(const String& name, const bool transparent, const FlagType flagValue, BrushContentTypeEvaluator* evaluator) :
        m_name(name),
        m_transparent(transparent),
        m_flagValue(flagValue),
        m_evaluator(evaluator) {
            assert(m_evaluator != NULL);
        }
        
        const String& BrushContentType::name() const {
            return m_name;
        }
        
        bool BrushContentType::transparent() const {
            return m_transparent;
        }
        
        BrushContentType::FlagType BrushContentType::flagValue() const {
            return m_flagValue;
        }
        
        bool BrushContentType::evaluate(const Brush* brush) const {
            return m_evaluator->evaluate(brush);
        }
    }
}

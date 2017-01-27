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

#include "Interpolator.h"

namespace TrenchBroom {
    namespace EL {
        Interpolator::Interpolator(const String& str) :
        ELParser(str) {}
        
        String Interpolator::interpolate(const EvaluationContext& context) {
            StringStream result;
            while (!m_tokenizer.eof()) {
                m_tokenizer.appendUntil("${", result);
                if (!m_tokenizer.eof()) {
                    Expression expression = parse();
                    result << expression.evaluate(context).convertTo(EL::Type_String).stringValue();
                    expect(IO::ELToken::CBrace, m_tokenizer.nextToken());
                }
            }
            
            return result.str();
        }

        String interpolate(const String& str, const EvaluationContext& context) {
            Interpolator interpolator(str);
            return interpolator.interpolate(context);
        }
    }
}

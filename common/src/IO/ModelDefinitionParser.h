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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ModelDefinitionParser_h
#define ModelDefinitionParser_h

#include "Assets/ModelDefinition.h"
#include "IO/ELParser.h"
#include "IO/LegacyModelDefinitionParser.h"

namespace TrenchBroom {
    namespace IO {
        class ParserStatus;
        
        template <typename T>
        Assets::ModelDefinition parseModelDefinition(ParserStatus& status, T& tokenizer) {
            const size_t line = tokenizer.line();
            const size_t column = tokenizer.column();
            
            try {
                LegacyModelDefinitionParser parser(tokenizer);
                EL::Expression expression = parser.parse(status);
                status.warn(line, column, "Legacy model expressions are deprecated, replace with '" + expression.asString() + "'");
                return Assets::ModelDefinition(expression);
            } catch (const ParserException&) {
                ELParser parser(tokenizer);
                return Assets::ModelDefinition(parser.parse());
            }
        }
    }
}

#endif /* ModelDefinitionParser_h */

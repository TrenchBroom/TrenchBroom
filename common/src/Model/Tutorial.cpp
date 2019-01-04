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

#include "Tutorial.h"

#include "Assets/AttributeDefinition.h"
#include "Model/BrushContentTypeEvaluator.h"

namespace TrenchBroom {
    namespace Model {
        namespace Tutorial {
            const AttributeValue Classname  = "_tb_tutorial";
            const AttributeName  Message    = "_tb_message";
            const Color EntityColor         = Color(1.0f, 0.5f, 0.0f, 1.0f);

            Assets::EntityDefinition* createTutorialEntityDefinition() {
                Assets::AttributeDefinitionPtr messageAttributeDefinition(new Assets::StringAttributeDefinition(Message, "Message to display", ""));
                
                Assets::AttributeDefinitionList attributeDefinitions;
                attributeDefinitions.push_back(messageAttributeDefinition);
                
                Assets::EntityDefinition* definition = new Assets::BrushEntityDefinition(Classname, EntityColor, "Tutorial helper entity", attributeDefinitions);
                
                return definition;
            }

            BrushContentType createTutorialBrushContentType(const BrushContentType::FlagType flag) {
                auto evaluator = BrushContentTypeEvaluator::entityClassnameEvaluator(Classname);
                return BrushContentType("Tutorial brush", true, flag, std::move(evaluator));
            }
        }
    }
}

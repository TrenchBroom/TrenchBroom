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

#include "EntityDefinitionParser.h"

#include "Macros.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/ParserStatus.h"
#include "Model/EntityAttributes.h"

#include <kdl/vector_utils.h>

#include <unordered_map>
#include <unordered_set>

namespace TrenchBroom {
    namespace IO {
        static const auto DefaultSize = vm::bbox3(-8, +8);
    
       EntityDefinitionParser::EntityDefinitionParser(const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor) {}
        
        EntityDefinitionParser::~EntityDefinitionParser() {}
        
        static std::shared_ptr<Assets::AttributeDefinition> mergeAttributes(const Assets::AttributeDefinition& inheritingClassAttribute, const Assets::AttributeDefinition& superClassAttribute) {
            assert(inheritingClassAttribute.name() == superClassAttribute.name());
        
            // for now, only merge spawnflags
            if (superClassAttribute.type() == Assets::AttributeDefinitionType::FlagsAttribute &&
                inheritingClassAttribute.type() == Assets::AttributeDefinitionType::FlagsAttribute &&
                superClassAttribute.name() == Model::AttributeNames::Spawnflags &&
                inheritingClassAttribute.name() == Model::AttributeNames::Spawnflags) {

                const auto& name = inheritingClassAttribute.name();
                auto result = std::make_shared<Assets::FlagsAttributeDefinition>(name);
                
                const auto& baseclassFlags = static_cast<const Assets::FlagsAttributeDefinition&>(superClassAttribute);
                const auto& classFlags     = static_cast<const Assets::FlagsAttributeDefinition&>(inheritingClassAttribute);

                for (int i = 0; i < 24; ++i) {
                    const auto* baseclassFlag = baseclassFlags.option(static_cast<int>(1 << i));
                    const auto* classFlag = classFlags.option(static_cast<int>(1 << i));

                    if (baseclassFlag != nullptr && classFlag == nullptr) {
                        result->addOption(baseclassFlag->value(), baseclassFlag->shortDescription(), baseclassFlag->longDescription(), baseclassFlag->isDefault());
                    } else if (classFlag != nullptr) {
                        result->addOption(classFlag->value(), classFlag->shortDescription(), classFlag->longDescription(), classFlag->isDefault());
                    }
                }
                
                return result;
            }
            
            return nullptr;
        }

        /**
         * Inherits the attributes from the super class to the inheriting class.
         *
         * Most attributes are only inherited if they are not already present in the inheriting class, except for the
         * following:
         * - spawnflags are merged together
         * - model definitions are merged together
         */
        static void inheritAttributes(EntityDefinitionClassInfo& inheritingClass, const EntityDefinitionClassInfo& superClass) {
            if (!inheritingClass.description) {
                inheritingClass.description = superClass.description;
            }
            if (!inheritingClass.color) {
                inheritingClass.color = superClass.color;
            }
            if (!inheritingClass.size) {
                inheritingClass.size = superClass.size;
            }

            for (const auto& attribute : superClass.attributes) {
                auto it = std::find_if(std::begin(inheritingClass.attributes), std::end(inheritingClass.attributes),
                    [&](const auto& a) { return a->name() == attribute->name(); });
                if (it == std::end(inheritingClass.attributes)) {
                    inheritingClass.attributes.push_back(attribute);
                } else {
                    auto mergedAttribute = mergeAttributes(**it, *attribute);
                    if (mergedAttribute != nullptr) {
                        *it = mergedAttribute;
                    }
                }
            }

            if (!inheritingClass.modelDefinition) {
                inheritingClass.modelDefinition = superClass.modelDefinition;
            } else if (superClass.modelDefinition) {
                inheritingClass.modelDefinition->append(*superClass.modelDefinition);
            }
        }
        
        /**
         * Resolves inheritance from the given inheriting class to the given super class, and recurses into the
         * super classes of the given super class.
         *
         * If the given super class has already been visited on the current path from the inheriting class to the super
         * class, then the inheritance hierarchy contains a cycle. In this case, an error is added to the given status
         * object and the recursion stops.
         *
         * Otherwise, the attributes from the given super class are copied to the inheriting class. For the exact
         * semantics of inheriting an attribute from a super class, see the inheritAttributes function. Afterwards, the
         * super classes of the given super class are recursively inherited from.
         *
         * By copying the attributes before recursing further into the super class hierarchy, the attributes inherited
         * from a class that is closer to the inheriting class in the inheritance hierarchy take precedence over the
         * attributes from a class that is further. This means that attributes from the further class get overridden
         * by attributes from the closer class.
         *
         * The following example illustrates this. Let A, B, C be classes such that A inherits from B and B inherits
         * from C. Then B has its attributes copied into A before C. And since attributes are only copied if they are
         * not present (with some exceptions), the attributes from B take precedence over the attributes from C.
         *
         * @param status the parser status to add errors to
         * @param inheritingClass class the class that is currently processed, i.e. the class that induces the
         * inheritance hierarchy that is currently being resolved
         * @param superClass the super class to inherit from
         * @param findClassInfos a function that finds class infos by their names
         * @param visited a set that contains the names of the classes visited so far on the path from the inheriting
         * class to the given super class
         *
         */
        template <typename F>
        static void inheritFromAndRecurse(ParserStatus& status, EntityDefinitionClassInfo& inheritingClass, const EntityDefinitionClassInfo& superClass, const F& findClassInfos, std::unordered_set<std::string>& visited) {
            if (!visited.insert(superClass.name).second) {
                status.error(inheritingClass.line, inheritingClass.column, "Entity definition class hierarchy contains a cycle");
                return;
            }

            inheritAttributes(inheritingClass, superClass);
            findSuperClassesAndInheritFrom(status, inheritingClass, superClass, findClassInfos, visited);
            
            visited.erase(superClass.name);
        }

        /**
         * Find the super classes to inherit from, and process each of them by callling `inheritFromAndRecurse`.
         *
         * The given `classWithSuperClasses` is used to determine the super classes to inherit from. This can be the
         * same as the given inheriting class, which is the class that induces the inheritance hierarchy and to which
         * the inherited attributes are added.
         *
         * For each super class name found at `classWithSuperClasses`, the function determines which class should be
         * inherited from. Since there can be multiple classes with the same name, but different types, the following
         * rules are used to resolve ambiguities:
         *
         * - If only one super class with the given name exists, then use that as a super class.
         * - If more than one super class with the given name exists:
         *   - if one of those potential super classes has the same type as the given inheriting class, then use it as
         *     a super class.
         *   - if the given inheriting class is not of type BaseClass, and one of the potential super classes is of type
         *     BaseClass, then use it as a super class.
         * Otherwise, no super class was found, return null.
         *
         * If a super class was found, inherit its attributes and recurse into its super classes again by calling
         * `inheritFromAndRecurse`.
         *
         * If the given `classWithSuperClasses` has multiple super classes, they are processed in the order in which
         * they were declared. This gives precedence to the attributes inherited from a super class that was declared
         * at a lower position than another super class.
         *
         * @param status the parser status to add errors to
         * @param inheritingClass class the class that is currently processed, i.e. the class that induces the
         * inheritance hierarchy that is currently being resolved
         * @param classWithSuperClasses the class that declares the super classes to inherit from
         * @param findClassInfos a function that finds class infos by their names
         * @param visited a set that contains the names of the classes visited so far on the path from the inheriting
         * class to the given super class
         */
        template <typename F>
        static void findSuperClassesAndInheritFrom(ParserStatus& status, EntityDefinitionClassInfo& inheritingClass, const EntityDefinitionClassInfo& classWithSuperClasses, const F& findClassInfos, std::unordered_set<std::string>& visited) {
            const auto selectSuperClass = [&](const auto& potentialSuperClasses) -> const EntityDefinitionClassInfo* {
                if (potentialSuperClasses.size() == 1u) {
                    return potentialSuperClasses.front();
                } else if (potentialSuperClasses.size() > 1u) {
                    // find a super class with the same class type as the inheriting class
                    for (const auto* potentialSuperClass : potentialSuperClasses) {
                        if (potentialSuperClass->type == inheritingClass.type) {
                            return potentialSuperClass;
                        }
                    }
                    
                    if (inheritingClass.type != EntityDefinitionClassType::BaseClass) {
                        // find a super class of type BaseClass
                        for (const auto* potentialSuperClass : potentialSuperClasses) {
                            if (potentialSuperClass->type == EntityDefinitionClassType::BaseClass) {
                                return potentialSuperClass;
                            }
                        }
                    }
                }
                
                return nullptr;
            };

            for (const auto& nextSuperClassName : classWithSuperClasses.superClasses) {
                const auto* nextSuperClass = selectSuperClass(findClassInfos(nextSuperClassName));
                if (nextSuperClass == nullptr) {
                    status.error(classWithSuperClasses.line, classWithSuperClasses.column, "No matching super class found for '" + nextSuperClassName + "'");
                } else {
                    inheritFromAndRecurse(status, inheritingClass, *nextSuperClass, findClassInfos, visited);
                }
            }
        }

        /**
         * Resolves the inheritance hierarchy induced the given inheriting class by recursively inheriting attributes
         * from its super classes.
         *
         * The super classes are explored in a depth first order, with super classes of a given class being explored
         * in the order in which they were declared. Once an attribute has been inherited from some super class, it
         * takes precedence over an attribute of the same name in some other super class that is visited later in the
         * process.
         *
         * @param status the parser status to add errors to
         * @param inheritingClass class the class that is currently processed, i.e. the class that induces the
         * inheritance hierarchy that is currently being resolved
         * @param findClassInfos a function that finds class infos by their names
         * @return a copy of the given inheriting class, with all attributes it inherits from its super classes added
         */
        template <typename F>
        static EntityDefinitionClassInfo resolveInheritance(ParserStatus& status, EntityDefinitionClassInfo inheritingClass, const F& findClassInfos) {
            auto visited = std::unordered_set<std::string>();
            findSuperClassesAndInheritFrom(status, inheritingClass, inheritingClass, findClassInfos, visited);
            return inheritingClass;
        }

        /**
         * Filter out redundant classes. A class is redundant if a class of the same name exists at an earlier position
         * in the given vector, unless the two classes each have one of the types point and brush each. That is, any
         * duplicate is redundant with the exception of overloaded point and brush classes.
         */
        static std::vector<EntityDefinitionClassInfo> filterRedundantClasses(ParserStatus& status, const std::vector<EntityDefinitionClassInfo>& classInfos) {
            std::vector<EntityDefinitionClassInfo> result;
            result.reserve(classInfos.size());
            
            const auto getMask = [](const auto type) {
                return 1 << static_cast<int>(type);
            };
            
            const auto baseClassMask = getMask(EntityDefinitionClassType::BaseClass);
            
            std::unordered_map<std::string, int> seen;
            for (const auto& classInfo : classInfos) {
                auto& seenMask = seen[classInfo.name];
                const auto classMask = getMask(classInfo.type);
                
                if (classMask & seenMask) {
                    status.warn(classInfo.line, classInfo.column, "Duplicate class info '" + classInfo.name + "'");
                } else if ((seenMask & baseClassMask) || (seenMask != 0 && (classMask & baseClassMask))) {
                    status.warn(classInfo.line, classInfo.column, "Redundant class info '" + classInfo.name + "'");
                } else {
                    result.push_back(classInfo);
                    seenMask |= classMask;
                }
            }
            
            return result;
        }

        /**
         * Resolves the inheritance for every class that is not of type BaseClass in the given vector and returns a
         * vector of copies where the inherited attributes are added to the inheriting classes.
         *
         * Exposed for testing.
         */
        std::vector<EntityDefinitionClassInfo> resolveInheritance(ParserStatus& status, const std::vector<EntityDefinitionClassInfo>& classInfos) {
            const auto filteredClassInfos = filterRedundantClasses(status, classInfos);
            const auto findClassInfos = [&](const auto& name) -> std::vector<const EntityDefinitionClassInfo*> {
                std::vector<const EntityDefinitionClassInfo*> result;
                for (const auto& classInfo : filteredClassInfos) {
                    if (classInfo.name == name) {
                        result.push_back(&classInfo);
                    }
                }
                return result;
            };

            std::vector<EntityDefinitionClassInfo> result;
            for (const auto& classInfo : filteredClassInfos) {
                if (classInfo.type != EntityDefinitionClassType::BaseClass) {
                    result.push_back(resolveInheritance(status, classInfo, findClassInfos));
                }
            }
            return result;
        }

        std::unique_ptr<Assets::EntityDefinition> EntityDefinitionParser::createDefinition(const EntityDefinitionClassInfo& classInfo) const {
            const auto& name = classInfo.name;
            const auto color = classInfo.color.value_or(m_defaultEntityColor);
            const auto size = classInfo.size.value_or(DefaultSize);
            auto description = classInfo.description.value_or("");
            auto& attributes = classInfo.attributes;
            auto modelDefinition = classInfo.modelDefinition.value_or(Assets::ModelDefinition());
            
            switch (classInfo.type) {
                case EntityDefinitionClassType::PointClass:
                    return std::make_unique<Assets::PointEntityDefinition>(name, color, size, std::move(description), std::move(attributes), std::move(modelDefinition));
                case EntityDefinitionClassType::BrushClass:
                    return std::make_unique<Assets::BrushEntityDefinition>(name, color, std::move(description), std::move(attributes));
                case EntityDefinitionClassType::BaseClass:
                    return nullptr;
                switchDefault()
            };
        }

        std::vector<Assets::EntityDefinition*> EntityDefinitionParser::createDefinitions(ParserStatus& status, const std::vector<EntityDefinitionClassInfo>& classInfos) const {
            const auto resolvedClasses = resolveInheritance(status, filterRedundantClasses(status, classInfos));

            std::vector<Assets::EntityDefinition*> result;
            for (const auto& classInfo : resolvedClasses) {
                if (auto definition = createDefinition(classInfo)) {
                    result.push_back(definition.release());
                }
            }
            
            return result;
        }

        EntityDefinitionParser::EntityDefinitionList EntityDefinitionParser::parseDefinitions(ParserStatus& status) {
            auto classInfos = parseClassInfos(status);
            return createDefinitions(status, std::move(classInfos));
        }
    }
}

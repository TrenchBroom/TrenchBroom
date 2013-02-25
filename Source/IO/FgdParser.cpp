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

#include "FgdParser.h"

#include "Utility/List.h"

using namespace TrenchBroom::IO::FgdTokenType;

namespace TrenchBroom {
    namespace IO {
        Token FgdTokenEmitter::doEmit(Tokenizer& tokenizer, size_t line, size_t column) {
            const size_t position = tokenizer.position();
            
            while (!tokenizer.eof()) {
                char c = tokenizer.nextChar();
                switch (c) {
                    case '/':
                        if (tokenizer.peekChar() == '/') {
                            // eat everything up to and including the next newline
                            while (tokenizer.nextChar() != '\n');
                            break;
                        }
                        error(line, column, c);
                        break;
                    case '(':
                        return Token(OParenthesis, "", position, tokenizer.position() - position, line, column);
                    case ')':
                        return Token(CParenthesis, "", position, tokenizer.position() - position, line, column);
                    case '[':
                        return Token(OBracket, "", position, tokenizer.position() - position, line, column);
                    case ']':
                        return Token(CBracket, "", position, tokenizer.position() - position, line, column);
                    case '=':
                        return Token(Equality, "", position, tokenizer.position() - position, line, column);
                    case ',':
                        return Token(Comma, "", position, tokenizer.position() - position, line, column);
                    case ':':
                        return Token(Colon, "", position, tokenizer.position() - position, line, column);
                    case '"': // quoted string
                        m_buffer.str(String());
                        while (!tokenizer.eof() && (c = tokenizer.nextChar()) != '"')
                            m_buffer << c;
                        return Token(QuotedString, m_buffer.str(), position, tokenizer.position() - position, line, column);
                    default: // integer, decimal or word
                        if (isWhitespace(c))
                            break;
                        
                        // clear the buffer
                        m_buffer.str(String());
                        
                        // try to read a number
                        if (c == '-' || isDigit(c)) {
                            m_buffer << c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer << c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(Integer, m_buffer.str(), position, tokenizer.position() - position, line, column);
                            }
                        }
                        
                        // try to read a decimal (may start with '.')
                        if (c == '.') {
                            m_buffer << c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer << c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(Decimal, m_buffer.str(), position, tokenizer.position() - position, line, column);
                            }
                        }
                        
                        // read a word
                        m_buffer << c;
                        while (!tokenizer.eof() && !isDelimiter(c = tokenizer.nextChar()))
                            m_buffer << c;
                        if (!tokenizer.eof())
                            tokenizer.pushChar();
                        return Token(Word, m_buffer.str(), position, tokenizer.position() - position, line, column);
                }
            }
            
            return Token(Eof, "", position, tokenizer.position() - position, line, column);
        }

        ClassInfo::ClassInfo() :
        hasDescription(false),
        hasColor(false),
        size(BBox(Vec3f(-8.0f, -8.0f, -8.0f), Vec3f(8.0f, 8.0f, 8.0f))),
        hasSize(false) {}
        
        
        ClassInfo::ClassInfo(size_t i_line, size_t i_column, const Color& defaultColor) :
        line(i_line),
        column(i_column),
        hasDescription(false),
        color(defaultColor),
        hasColor(false),
        size(BBox(Vec3f(-8.0f, -8.0f, -8.0f), Vec3f(8.0f, 8.0f, 8.0f))),
        hasSize(false) {}

        String FgdParser::typeNames(unsigned int types) {
            StringList names;
            if ((types & Integer) != 0)
                names.push_back("integer number");
            if ((types & Decimal) != 0)
                names.push_back("decimal number");
            if ((types & QuotedString) != 0)
                names.push_back("string");
            if ((types & OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((types & CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((types & OBracket) != 0)
                names.push_back("opening bracket");
            if ((types & CBracket) != 0)
                names.push_back("closing bracket");
            if ((types & Word) != 0)
                names.push_back("word");
            if ((types & Equality) != 0)
                names.push_back("equality sign");
            if ((types & Colon) != 0)
                names.push_back("colon");
            if ((types & Comma) != 0)
                names.push_back("comma");
            if ((types & Equality) != 0)
                names.push_back("equality sign");
            
            if (names.empty())
                return "unknown token type";
            if (names.size() == 1)
                return names[0];
            
            StringStream str;
            str << names[0];
            for (unsigned int i = 1; i < names.size() - 1; i++)
                str << ", " << names[i];
            str << ", or " << names[names.size() - 1];
            return str.str();
        }

        Model::PropertyDefinition* FgdParser::parseTargetSourceProperty(const String& propertyKey) {
            String description;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return new Model::PropertyDefinition(propertyKey, Model::PropertyDefinition::TargetSourceProperty, description);
        }
        
        Model::PropertyDefinition* FgdParser::parseTargetDestinationProperty(const String& propertyKey) {
            String description;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return new Model::PropertyDefinition(propertyKey, Model::PropertyDefinition::TargetDestinationProperty, description);
        }
        
        Model::PropertyDefinition* FgdParser::parseStringProperty(const String& propertyKey) {
            String description;
            String defaultValue;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();

                token = m_tokenizer.nextToken();
                if (token.type() == Colon) {
                    expect(QuotedString, token = m_tokenizer.nextToken());
                    defaultValue = token.data();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return new Model::StringPropertyDefinition(propertyKey, description, defaultValue);
        }
        
        Model::PropertyDefinition* FgdParser::parseIntegerProperty(const String& propertyKey) {
            String description;
            int defaultValue = 0;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();
                
                token = m_tokenizer.nextToken();
                if (token.type() == Colon) {
                    expect(Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return new Model::IntegerPropertyDefinition(propertyKey, description, defaultValue);
        }
        
        Model::PropertyDefinition* FgdParser::parseChoicesProperty(const String& propertyKey) {
            String description;
            int defaultValue;
            
            Token token;
            expect(Colon | Equality, token = m_tokenizer.nextToken());
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();

                expect(Colon | Equality, token = m_tokenizer.nextToken());
                if (token.type() == Colon) {
                    expect(Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger();
                    expect(Equality, token = m_tokenizer.nextToken());
                }
            }
            
            assert(token.type() == Equality);
            expect(OBracket, token = m_tokenizer.nextToken());
            expect(Integer | QuotedString | CBracket, token = m_tokenizer.nextToken());
            
            Model::ChoicePropertyDefinition* definition = new Model::ChoicePropertyDefinition(propertyKey, description, defaultValue);
            
            while (token.type() != CBracket) {
                const String value = token.data();
                expect(Colon, token = m_tokenizer.nextToken());
                expect(QuotedString, token = m_tokenizer.nextToken());
                const String caption = token.data();
                definition->addOption(value, caption);
                expect(Integer | QuotedString | CBracket, token = m_tokenizer.nextToken());
            }
            
            return definition;
        }
        
        Model::PropertyDefinition* FgdParser::parseFlagsProperty(const String& propertyKey) {
            String description;
            
            Token token;
            expect(Colon | Equality, token = m_tokenizer.nextToken());
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                description = token.data();
            }

            expect(OBracket, token = m_tokenizer.nextToken());
            expect(Integer | CBracket, token = m_tokenizer.nextToken());
            
            Model::FlagsPropertyDefinition* definition = new Model::FlagsPropertyDefinition(propertyKey, description);
            
            while (token.type() != CBracket) {
                const int value = token.toInteger();
                expect(Colon, token = m_tokenizer.nextToken());
                
                expect(QuotedString, token = m_tokenizer.nextToken());
                const String caption = token.data();

                bool defaultValue = false;
                expect(Colon | Integer | CBracket, token = m_tokenizer.nextToken());
                if (token.type() == Colon) {
                    expect(Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger() != 0;
                } else {
                    m_tokenizer.pushToken(token);
                }
                
                definition->addOption(value, caption, defaultValue);
                expect(Integer | CBracket, token = m_tokenizer.nextToken());
            }
            
            return definition;
        }

        Model::PropertyDefinition::Map FgdParser::parseProperties() {
            Model::PropertyDefinition::Map properties;
            
            Token token;
            expect(OBracket, token = m_tokenizer.nextToken());
            expect(Word | CBracket, token = m_tokenizer.nextToken());
            while (token.type() != CBracket) {
                const String propertyKey = token.data();
                
                expect(OParenthesis, token = m_tokenizer.nextToken());
                expect(Word, token = m_tokenizer.nextToken());
                const String typeName = token.data();
                expect(CParenthesis, token = m_tokenizer.nextToken());
                
                if (Utility::equalsString(typeName, "target_source", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseTargetSourceProperty(propertyKey);
                } else if (Utility::equalsString(typeName, "target_destination", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseTargetDestinationProperty(propertyKey);
                } else if (Utility::equalsString(typeName, "string", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseStringProperty(propertyKey);
                } else if (Utility::equalsString(typeName, "integer", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseIntegerProperty(propertyKey);
                } else if (Utility::equalsString(typeName, "choices", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseChoicesProperty(propertyKey);
                } else if (Utility::equalsString(typeName, "flags", false)) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitionsf or property " + propertyKey);
                    properties[propertyKey] = parseFlagsProperty(propertyKey);
                } else {
                    throw ParserException(token.line(), token.column(), "Unknown entity definition property " + typeName);
                }
                
                expect(Word | CBracket, token = m_tokenizer.nextToken());
            }
            
            return properties;
        }

        BBox FgdParser::parseSize() {
            BBox size;
            Token token;
            expect(OParenthesis, token = m_tokenizer.nextToken());
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            size.min.x = token.toFloat();
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            size.min.y = token.toFloat();
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            size.min.z = token.toFloat();
            
            expect(CParenthesis | Comma, token = m_tokenizer.nextToken());
            if (token.type() == Comma) {
                expect(Integer | Decimal, token = m_tokenizer.nextToken());
                size.max.x = token.toFloat();
                expect(Integer | Decimal, token = m_tokenizer.nextToken());
                size.max.y = token.toFloat();
                expect(Integer | Decimal, token = m_tokenizer.nextToken());
                size.max.z = token.toFloat();
                expect(CParenthesis, token = m_tokenizer.nextToken());
            } else {
                size.translate(size.size() / 2.0f);
            }
            
            return size;
        }

        Color FgdParser::parseColor() {
            float r, g, b;
            Token token;
            expect(OParenthesis, token = m_tokenizer.nextToken());
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            r = token.toFloat();
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            g = token.toFloat();
            expect(Integer | Decimal, token = m_tokenizer.nextToken());
            b = token.toFloat();
            expect(CParenthesis, token = m_tokenizer.nextToken());
            return Color(r, g, b, 1.0f);
        }

        StringList FgdParser::parseBaseClasses() {
            StringList baseClasses;
            Token token;
            expect(OParenthesis, token = m_tokenizer.nextToken());
            expect(Word | CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == Word) {
                m_tokenizer.pushToken(token);
                do {
                    expect(Word, token = m_tokenizer.nextToken());
                    baseClasses.push_back(token.data());
                    expect(Comma | CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == Comma);
            }
            return baseClasses;
        }

        Model::ModelDefinition::List FgdParser::parseModels() {
            Model::ModelDefinition::List result;
            Token token;
            expect(OParenthesis, token = m_tokenizer.nextToken());
            expect(QuotedString | CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == QuotedString) {
                m_tokenizer.pushToken(token);
                do {
                    expect(QuotedString, token = m_tokenizer.nextToken());
                    const String path = token.data();
                    
                    expect(Integer, token = m_tokenizer.nextToken());
                    unsigned int skinIndex = static_cast<unsigned int>(token.toInteger());
                    
                    expect(Integer, token = m_tokenizer.nextToken());
                    unsigned int frameIndex = static_cast<unsigned int>(token.toInteger());
                    
                    expect(Word | Comma | CParenthesis, token = m_tokenizer.nextToken());
                    if (token.type() == Word) {
                        const String propertyKey = token.data();
                        expect(Equality, token = m_tokenizer.nextToken());
                        expect(QuotedString | Integer, token = m_tokenizer.nextToken());
                        if (token.type() == QuotedString) {
                            const String propertyValue = token.data();
                            result.push_back(new Model::ModelDefinition(path, skinIndex, frameIndex, propertyKey, propertyValue));
                        } else {
                            const int flagValue = token.toInteger();
                            result.push_back(new Model::ModelDefinition(path, skinIndex, frameIndex, propertyKey, flagValue));
                        }
                        expect(Comma | CParenthesis, token = m_tokenizer.nextToken());
                    } else {
                        result.push_back(new Model::ModelDefinition(path, skinIndex, frameIndex));
                    }
                    
                } while (token.type() == Comma);
            }
            return result;
        }

        void FgdParser::resolveBaseClasses(const StringList& classnames, ClassInfo& classInfo) {
            StringList::const_reverse_iterator classnameIt, classnameEnd;
            for (classnameIt = classnames.rbegin(), classnameEnd = classnames.rend(); classnameIt != classnameEnd; ++classnameIt) {
                const String& classname = *classnameIt;
                ClassInfo::Map::const_iterator baseClassIt = m_baseClasses.find(classname);
                if (baseClassIt != m_baseClasses.end()) {
                    const ClassInfo& baseClass = baseClassIt->second;
                    if (!classInfo.hasDescription && baseClass.hasDescription)
                        classInfo.setDescription(baseClass.description);
                    if (!classInfo.hasColor && baseClass.hasColor)
                        classInfo.setColor(baseClass.color);
                    if (!classInfo.hasSize && baseClass.hasSize)
                        classInfo.setSize(baseClass.size);
                    
                    Model::PropertyDefinition::Map::const_iterator propertyIt, propertyEnd;
                    for (propertyIt = baseClass.properties.begin(), propertyEnd = baseClass.properties.end(); propertyIt != propertyEnd; ++propertyIt) {
                        const Model::PropertyDefinition* property = propertyIt->second;
                        const bool hasProperty = classInfo.properties.find(property->name()) != classInfo.properties.end();
                        if (!hasProperty)
                            classInfo.properties[property->name()] = new Model::PropertyDefinition(*property);
                    }
                    
                    Model::ModelDefinition::List::const_iterator modelIt, modelEnd;
                    for (modelIt = baseClass.models.begin(), modelEnd = baseClass.models.end(); modelIt != modelEnd; ++modelIt) {
                        const Model::ModelDefinition* model = *modelIt;
                        classInfo.models.push_back(new Model::ModelDefinition(*model));
                    }
                }
            }
        }

        ClassInfo FgdParser::parseClass() {
            Token token;
            expect(Word | Equality, token = m_tokenizer.nextToken());
            
            StringList baseClasses;
            ClassInfo classInfo(token.line(), token.column(), m_defaultEntityColor);
            
            while (token.type() == Word) {
                const String typeName = token.data();
                if (Utility::equalsString(typeName, "base", false)) {
                    if (!baseClasses.empty())
                        throw ParserException(token.line(), token.column(), "Found multiple base properties");
                    baseClasses = parseBaseClasses();
                } else if (Utility::equalsString(typeName, "color", false)) {
                    if (classInfo.hasColor)
                        throw ParserException(token.line(), token.column(), "Found multiple color properties");
                    classInfo.setColor(parseColor());
                } else if (Utility::equalsString(typeName, "size", false)) {
                    if (classInfo.hasSize)
                        throw ParserException(token.line(), token.column(), "Found multiple size properties");
                    classInfo.setSize(parseSize());
                } else if (Utility::equalsString(typeName, "model", false)) {
                    if (!classInfo.models.empty())
                        throw ParserException(token.line(), token.column(), "Found multiple model properties");
                    classInfo.models = parseModels();
                } else {
                    throw ParserException(token.line(), token.column(), "Unknown entity definition header property " + typeName);
                }
                expect(Equality | Word, token = m_tokenizer.nextToken());
            }
            
            expect(Word, token = m_tokenizer.nextToken());
            classInfo.name = token.data();
            
            expect(Colon | OBracket, token = m_tokenizer.nextToken());
            if (token.type() == Colon) {
                expect(QuotedString, token = m_tokenizer.nextToken());
                classInfo.setDescription(token.data());
            } else {
                m_tokenizer.pushToken(token);
            }
            
            classInfo.properties = parseProperties();
            resolveBaseClasses(baseClasses, classInfo);
            return classInfo;
        }

        Model::EntityDefinition* FgdParser::parseSolidClass() {
            ClassInfo classInfo = parseClass();
            if (classInfo.hasSize)
                throw ParserException(classInfo.line, classInfo.column, "Solid entity definition must not have a size");
            if (!classInfo.models.empty())
                throw ParserException(classInfo.line, classInfo.column, "Solid entity definition must not have model definitions");
            return new Model::BrushEntityDefinition(classInfo.name, classInfo.color, classInfo.description, classInfo.propertyList());
        }
        
        Model::EntityDefinition* FgdParser::parsePointClass() {
            ClassInfo classInfo = parseClass();
            return new Model::PointEntityDefinition(classInfo.name, classInfo.color, classInfo.size, classInfo.description, classInfo.propertyList(), classInfo.models);
        }
        
        void FgdParser::parseBaseClass() {
            ClassInfo classInfo = parseClass();
            if (m_baseClasses.count(classInfo.name) > 0) {
                classInfo.deleteProperties();
                classInfo.deleteModels();
                throw ParserException(classInfo.line, classInfo.column, "Redefinition of base class " + classInfo.name);
            }
            
            m_baseClasses[classInfo.name] = classInfo;
        }

        FgdParser::~FgdParser() {
            ClassInfo::Map::iterator infoIt, infoEnd;
            for (infoIt = m_baseClasses.begin(), infoEnd = m_baseClasses.end(); infoIt != infoEnd; ++infoIt) {
                ClassInfo& classInfo = infoIt->second;
                classInfo.deleteProperties();
                classInfo.deleteModels();
            }
            
            m_baseClasses.clear();
        }

        Model::EntityDefinition* FgdParser::nextDefinition() {
            Token token = m_tokenizer.nextToken();
            if (token.type() == Eof)
                return NULL;
            
            const String typeName = token.data();
            if (Utility::equalsString(typeName, "@SolidClass", false)) {
                return parseSolidClass();
            } else if (Utility::equalsString(typeName, "@PointClass", false)) {
                return parsePointClass();
            } else if (Utility::equalsString(typeName, "@BaseClass", false)) {
                parseBaseClass();
                return nextDefinition();
            } else {
                throw ParserException(token.line(), token.column(), "Unknown entity definition class " + typeName);
            }
        }
    }
}

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

#include "LegacyModelDefinitionParser.h"

#include "Assets/ModelDefinition.h"
#include "StringUtils.h"
#include "EL.h"

namespace TrenchBroom {
    namespace IO {
        LegacyModelDefinitionTokenizer::LegacyModelDefinitionTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", 0) {}
        
        LegacyModelDefinitionTokenizer::LegacyModelDefinitionTokenizer(const String& str) :
        Tokenizer(str, "", 0) {}
        
        const String LegacyModelDefinitionTokenizer::WordDelims = " \t\n\r()[]{};,=";
        
        LegacyModelDefinitionTokenizer::Token LegacyModelDefinitionTokenizer::emitToken() {
            while (!eof()) {
                const size_t startLine = line();
                const size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '=':
                        advance();
                        return Token(MdlToken::Equality, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(MdlToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(MdlToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        advance();
                        break;
                    case '"':
                        advance();
                        c = curPos();
                        return Token(MdlToken::String, c, readQuotedString(), offset(c), startLine, startColumn);
                    default: {
                        const char* e = readInteger(WordDelims);
                        if (e != NULL)
                            return Token(MdlToken::Integer, c, e, offset(c), startLine, startColumn);
                        e = readUntil(WordDelims);
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(MdlToken::Word, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(MdlToken::Eof, NULL, NULL, length(), line(), column());
        }

        LegacyModelDefinitionParser::LegacyModelDefinitionParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}
        
        LegacyModelDefinitionParser::LegacyModelDefinitionParser(const String& str) :
        m_tokenizer(str) {}
        
        EL::Expression LegacyModelDefinitionParser::parse(ParserStatus& status) {
            return parseModelDefinition(status);
        }
        
        EL::Expression LegacyModelDefinitionParser::parseModelDefinition(ParserStatus& status) {
            Token token = m_tokenizer.peekToken();
            const size_t startLine = token.line();
            const size_t startColumn = token.column();
            
            expect(status, MdlToken::String | MdlToken::Word | MdlToken::CParenthesis, token);
            if (token.hasType(MdlToken::CParenthesis))
                return EL::Expression(EL::LiteralExpression::create(EL::Value::Undefined, token.line(), token.column()));
            
            EL::ExpressionBase::List modelExpressions;
            try {
                do {
                    expect(status, MdlToken::String | MdlToken::Word, token = m_tokenizer.peekToken());
                    if (token.hasType(MdlToken::String))
                        modelExpressions.push_back(parseStaticModelDefinition(status));
                    else
                        modelExpressions.push_back(parseDynamicModelDefinition(status));
                    expect(status, MdlToken::Comma | MdlToken::CParenthesis, token = m_tokenizer.peekToken());
                    if (token.hasType(MdlToken::Comma))
                        m_tokenizer.nextToken();
                } while (token.hasType(MdlToken::Comma));
                
                // The legacy model expressions are evaluated back to front.
                modelExpressions.reverse();
                return EL::Expression(EL::SwitchOperator::create(modelExpressions, startLine, startColumn));
            } catch (...) {
                ListUtils::clearAndDelete(modelExpressions);
                throw;
            }
        }
        
        EL::ExpressionBase* LegacyModelDefinitionParser::parseStaticModelDefinition(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            expect(status, MdlToken::String, token);
            const size_t startLine = token.line();
            const size_t startColumn = token.column();

            EL::MapType map;
            map["path"] = EL::Value(token.data());
            
            std::vector<size_t> indices;
            
            expect(status, MdlToken::Integer | MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis, token = m_tokenizer.peekToken());
            if (token.hasType(MdlToken::Integer)) {
                token = m_tokenizer.nextToken();
                indices.push_back(token.toInteger<size_t>());
                expect(status, MdlToken::Integer | MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis, token = m_tokenizer.peekToken());
                if (token.hasType(MdlToken::Integer)) {
                    token = m_tokenizer.nextToken();
                    indices.push_back(token.toInteger<size_t>());
                    expect(status, MdlToken::Word | MdlToken::Comma | MdlToken::CParenthesis, token = m_tokenizer.peekToken());
                }
            }
            
            if (!indices.empty()) {
                map["skin"] = EL::Value(indices[0]);
                if (indices.size() > 1)
                    map["frame"] = EL::Value(indices[1]);
            }
            
            EL::ExpressionBase* modelExpression = EL::LiteralExpression::create(EL::Value(map), startLine, startColumn);
            
            if (token.hasType(MdlToken::Word)) {
                token = m_tokenizer.nextToken();
                
                const String attributeKey = token.data();
                const size_t line = token.line();
                const size_t column = token.column();
                EL::ExpressionBase* keyExpression = EL::VariableExpression::create(attributeKey, line, column);
                
                expect(status, MdlToken::Equality, token = m_tokenizer.nextToken());
                
                expect(status, MdlToken::String | MdlToken::Integer, token = m_tokenizer.nextToken());
                if (token.hasType(MdlToken::String)) {
                    const String attributeValue = token.data();
                    EL::ExpressionBase* valueExpression = EL::LiteralExpression::create(EL::Value(attributeValue), token.line(), token.column());
                    EL::ExpressionBase* premiseExpression = EL::ComparisonOperator::createEqual(keyExpression, valueExpression, line, column);
                    
                    return EL::CaseOperator::create(premiseExpression, modelExpression, startLine, startColumn);
                } else {
                    const int flagValue = token.toInteger<int>();
                    EL::ExpressionBase* valueExpression = EL::LiteralExpression::create(EL::Value(flagValue), token.line(), token.column());
                    EL::ExpressionBase* premiseExpression = EL::ComparisonOperator::createEqual(keyExpression, valueExpression, line, column);

                    return EL::CaseOperator::create(premiseExpression, modelExpression, startLine, startColumn);
                }
            } else {
                return modelExpression;
            }
        }
        
        EL::ExpressionBase* LegacyModelDefinitionParser::parseDynamicModelDefinition(ParserStatus& status) {
            Token token = m_tokenizer.peekToken();
            const size_t line = token.line();
            const size_t column = token.column();

            EL::ExpressionBase::Map map;
            map["path"] = parseNamedValue(status, "pathKey");

            expect(status, MdlToken::Word | MdlToken::CParenthesis, token = m_tokenizer.peekToken());
            
            if (!token.hasType(MdlToken::CParenthesis)) {
                do {
                    if (StringUtils::caseInsensitiveEqual("skinKey", token.data())) {
                        map["skin"] = parseNamedValue(status, "skinKey");
                    } else if (StringUtils::caseInsensitiveEqual("frameKey", token.data())) {
                        map["frame"] = parseNamedValue(status, "frameKey");
                    } else {
                        const String msg = "Expected 'skinKey' or 'frameKey', but found '" + token.data() + "'";
                        status.error(token.line(), token.column(), msg);
                        MapUtils::clearAndDelete(map);
                        throw ParserException(token.line(), token.column(), msg);
                    }
                } while (expect(status, MdlToken::Word | MdlToken::CParenthesis, token = m_tokenizer.peekToken()).hasType(MdlToken::Word));
            }
            
            return EL::MapExpression::create(map, line, column);
        }

        EL::ExpressionBase* LegacyModelDefinitionParser::parseNamedValue(ParserStatus& status, const String& name) {
            Token token;
            expect(status, MdlToken::Word, token = m_tokenizer.nextToken());
            
            const size_t line = token.line();
            const size_t column = token.column();
            if (!StringUtils::caseInsensitiveEqual(name, token.data()))
                throw ParserException(line, column, "Expected '" + name + "', but got '" + token.data() + "'");
            
            expect(status, MdlToken::Equality, token = m_tokenizer.nextToken());
            expect(status, MdlToken::String, token = m_tokenizer.nextToken());
            
            return EL::VariableExpression::create(token.data(), line, column);
        }

        LegacyModelDefinitionParser::TokenNameMap LegacyModelDefinitionParser::tokenNames() const {
            using namespace MdlToken;
            
            TokenNameMap names;
            names[Integer]      = "integer";
            names[String]       = "quoted string";
            names[Word]         = "word";
            names[Comma]        = "','";
            names[Equality]     = "'='";
            names[Eof]          = "end of file";
            return names;
        }
    }
}

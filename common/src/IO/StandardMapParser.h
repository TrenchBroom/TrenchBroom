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

#ifndef TrenchBroom_StandardMapParser
#define TrenchBroom_StandardMapParser

#include "TrenchBroom.h"
#include "VecMath.h"
#include "IO/MapParser.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"
#include "Model/MapFormat.h"

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        namespace QuakeMapToken {
            typedef unsigned int Type;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OParenthesis  = 1 <<  3; // opening parenthesis: (
            static const Type CParenthesis  = 1 <<  4; // closing parenthesis: )
            static const Type OBrace        = 1 <<  5; // opening brace: {
            static const Type CBrace        = 1 <<  6; // closing brace: }
            static const Type OBracket      = 1 <<  7; // opening bracket: [
            static const Type CBracket      = 1 <<  8; // closing bracket: ]
            static const Type Comment       = 1 <<  9; // line comment starting with ///
            static const Type Eof           = 1 << 10; // end of file
            static const Type Eol           = 1 << 11; // end of line
        }
        
        class QuakeMapTokenizer : public Tokenizer<QuakeMapToken::Type> {
        private:
            static const String NumberDelim;
            bool m_skipEol;
        public:
            QuakeMapTokenizer(const char* begin, const char* end);
            QuakeMapTokenizer(const String& str);
            
            void setSkipEol(bool skipEol);
        private:
            Token emitToken();
        };

        class StandardMapParser : public MapParser, public Parser<QuakeMapToken::Type> {
        private:
            typedef QuakeMapTokenizer::Token Token;

            QuakeMapTokenizer m_tokenizer;
            Logger* m_logger;
            Model::MapFormat::Type m_format;
        public:
            StandardMapParser(const char* begin, const char* end, Logger* logger = NULL);
            StandardMapParser(const String& str, Logger* logger = NULL);
            
            virtual ~StandardMapParser();
        protected:
            Logger* logger() const;


            Model::MapFormat::Type detectFormat();
            
            void parseEntities(Model::MapFormat::Type format);
            void parseBrushes(Model::MapFormat::Type format);
            void parseBrushFaces(Model::MapFormat::Type format);
            
            void reset();
        private:
            void setFormat(Model::MapFormat::Type format);
            
            void parseEntityOrBrush();
            void parseEntity();
            void parseEntityAttribute(Model::EntityAttribute::List& attributes);
            void parseBrush();
            void parseFace();

            Vec3 parseVector();
            void parseExtraAttributes(ExtraAttributes& extraAttributes);
        private: // implement Parser interface
            TokenNameMap tokenNames() const;
        };
    }
}

#endif /* defined(TrenchBroom_StandardMapParser) */

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

#ifndef TrenchBroom_TextureReader_h
#define TrenchBroom_TextureReader_h

#include "Macros.h"
#include "Assets/AssetTypes.h"
#include "IO/MappedFile.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class TextureReader {
        public:
            class NameStrategy {
            protected:
                NameStrategy();
            public:
                virtual ~NameStrategy();
                
                NameStrategy* clone() const;
                
                String textureName(const String& textureName, const Path& path) const;
            private:
                virtual NameStrategy* doClone() const = 0;
                virtual String doGetTextureName(const String& textureName, const Path& path) const = 0;
                
                deleteCopyAndAssignment(NameStrategy)
            };
            
            class TextureNameStrategy : public NameStrategy {
            public:
                TextureNameStrategy();
            private:
                NameStrategy* doClone() const;
                String doGetTextureName(const String& textureName, const Path& path) const;
                
                deleteCopyAndAssignment(TextureNameStrategy)
            };
            
            class PathSuffixNameStrategy : public NameStrategy {
            private:
                size_t m_suffixLength;
                bool m_deleteExtension;
            public:
                PathSuffixNameStrategy(size_t suffixLength, bool deleteExtension);
            private:
                NameStrategy* doClone() const;
                String doGetTextureName(const String& textureName, const Path& path) const;
                
                deleteCopyAndAssignment(PathSuffixNameStrategy)
            };
        private:
            NameStrategy* m_nameStrategy;
        protected:
            TextureReader(const NameStrategy& nameStrategy);
        public:
            virtual ~TextureReader();
            
            Assets::Texture* readTexture(MappedFile::Ptr file) const;
            Assets::Texture* readTexture(const char* const begin, const char* const end, const Path& path) const;
        protected:
            String textureName(const String& textureName, const Path& path) const;
        private:
            virtual Assets::Texture* doReadTexture(const char* const begin, const char* const end, const Path& path) const = 0;
        public:
            static size_t mipSize(size_t width, size_t height, size_t mipLevel);
            
            deleteCopyAndAssignment(TextureReader)
        };
    }
}

#endif

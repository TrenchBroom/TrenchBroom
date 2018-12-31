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
                
                deleteCopyAndMove(NameStrategy)
            };
            
            class TextureNameStrategy : public NameStrategy {
            public:
                TextureNameStrategy();
            private:
                NameStrategy* doClone() const override;
                String doGetTextureName(const String& textureName, const Path& path) const override;
                
                deleteCopyAndMove(TextureNameStrategy)
            };
            
            class PathSuffixNameStrategy : public NameStrategy {
            private:
                size_t m_suffixLength;
                bool m_deleteExtension;
            public:
                PathSuffixNameStrategy(size_t suffixLength, bool deleteExtension);
            private:
                NameStrategy* doClone() const override;
                String doGetTextureName(const String& textureName, const Path& path) const override;
                
                deleteCopyAndMove(PathSuffixNameStrategy)
            };
        private:
            NameStrategy* m_nameStrategy;
        protected:
            explicit TextureReader(const NameStrategy& nameStrategy);
        public:
            virtual ~TextureReader();
            
            Assets::Texture* readTexture(MappedFile::Ptr file) const;
        protected:
            String textureName(const String& textureName, const Path& path) const;
            String textureName(const Path& path) const;
        private:
            /**
             * Loads a texture and returns an Assets::Texture object allocated with new. Should not throw exceptions to
             * report errors loading textures except for unrecoverable errors (out of memory, bugs, etc.). In all other
             * cases, an empty placeholder texture is returned.
             *
             * @param file the file that contains the texture image
             * @return an Assets::Texture object allocated with new
             */
            virtual Assets::Texture* doReadTexture(MappedFile::Ptr file) const = 0;
        public:
            static size_t mipSize(size_t width, size_t height, size_t mipLevel);
            
            deleteCopyAndMove(TextureReader)
        };
    }
}

#endif

/*
Copyright (C) 2020 Kristian Duske

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

#ifndef TextureReference_h
#define TextureReference_h

namespace TrenchBroom {
    namespace Assets {
        class Texture;
        
        class TextureReference {
        private:
            Assets::Texture* m_texture;
        public:
            explicit TextureReference(Texture* texture = nullptr);
            
            TextureReference(const TextureReference& other);
            TextureReference(TextureReference&& other);
            
            ~TextureReference();
            
            TextureReference& operator=(TextureReference other);

            friend void swap(TextureReference& lhs, TextureReference& rhs);
            
            const Assets::Texture* texture() const;
        };
    }
}

#endif /* TextureReference_h */

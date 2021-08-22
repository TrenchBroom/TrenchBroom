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

#pragma once

#include "Color.h"
#include "IO/Path.h"
#include "Assets/TextureBuffer.h"
#include "Renderer/GL.h"

#include <vecmath/forward.h>

#include <atomic>
#include <iosfwd>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class TextureCollection;

        enum class TextureType {
            Opaque,
            /**
             * Modifies texture uploading to support mask textures.
             */
            Masked
        };

        enum class TextureCulling {
            CullDefault,
            CullNone,
            CullFront,
            CullBack,
            CullBoth
        };

        struct TextureBlendFunc {
            enum class Enable {
                /**
                 * Don't change GL_BLEND and don't change the blend function.
                 */
                UseDefault,
                /**
                 * Don't change GL_BLEND, but set the blend function.
                 */
                UseFactors,
                /**
                 * Set GL_BLEND to off.
                 */
                DisableBlend
            };
            
            Enable enable;
            GLenum srcFactor;
            GLenum destFactor;
        };

        struct Q2Data {
            int flags;
            int contents;
            int value;

            bool operator==(const Q2Data& other) const;
            bool operator!=(const Q2Data& other) const;
        };
        std::ostream& operator<<(std::ostream& str, const Q2Data& data);

        using GameData = std::variant<std::monostate, Q2Data>;

        class Texture {
        private:
            using Buffer = TextureBuffer;
            using BufferList = std::vector<Buffer>;
        private:
            std::string m_name;
            IO::Path m_absolutePath;
            IO::Path m_relativePath;

            size_t m_width;
            size_t m_height;
            Color m_averageColor;

            std::atomic<size_t> m_usageCount;
            bool m_overridden;

            GLenum m_format;
            TextureType m_type;

            // TODO: move these to a Q3Data variant case of m_gameData if possible
            // Quake 3 surface parameters; move these to materials when we add proper support for those.
            std::set<std::string> m_surfaceParms;

            // Quake 3 surface culling; move to materials
            TextureCulling m_culling;

            // Quake 3 blend function, move to materials
            TextureBlendFunc m_blendFunc;

            mutable GLuint m_textureId;
            mutable BufferList m_buffers;

            GameData m_gameData;
        public:
            Texture(const std::string& name, size_t width, size_t height, const Color& averageColor, Buffer&& buffer, GLenum format, TextureType type, GameData gameData = std::monostate{});
            Texture(const std::string& name, size_t width, size_t height, const Color& averageColor, BufferList&& buffers, GLenum format, TextureType type, GameData gameData = std::monostate{});
            Texture(const std::string& name, size_t width, size_t height, GLenum format = GL_RGB, TextureType type = TextureType::Opaque, GameData gameData = std::monostate{});

            Texture(const Texture&) = delete;
            Texture& operator=(const Texture&) = delete;
            
            Texture(Texture&& other);
            Texture& operator=(Texture&& other);

            ~Texture();

            static TextureType selectTextureType(bool masked);

            const std::string& name() const;

            /**
             * Absolute path of the texture
             *
             * Currently, only set for textures loaded by DirectoryTextureCollectionLoader
             */
            const IO::Path& absolutePath() const;
            void setAbsolutePath(const IO::Path& absolutePath);

            /**
             * Relative path of the texture in the game filesystem
             *
             * Currently, only set for textures loaded by DirectoryTextureCollectionLoader
             */
            const IO::Path& relativePath() const;
            void setRelativePath(const IO::Path& relativePath);

            size_t width() const;
            size_t height() const;
            const Color& averageColor() const;

            bool masked() const;
            void setOpaque();
            
            const std::set<std::string>& surfaceParms() const;
            void setSurfaceParms(const std::set<std::string>& surfaceParms);

            TextureCulling culling() const;
            void setCulling(TextureCulling culling);

            void setBlendFunc(GLenum srcFactor, GLenum destFactor);
            void disableBlend();

            const GameData& gameData() const;

            size_t usageCount() const;
            void incUsageCount();
            void decUsageCount();
            bool overridden() const;
            void setOverridden(bool overridden);

            bool isPrepared() const;
            void prepare(GLuint textureId, int minFilter, int magFilter);
            void setMode(int minFilter, int magFilter);

            void activate() const;
            void deactivate() const;
        public: // exposed for tests only
            /**
             * Returns the texture data in the format returned by format().
             * Once prepare() is called, this will be an empty vector.
             */
            const BufferList& buffersIfUnprepared() const;
            /**
             * Will be one of GL_RGB, GL_BGR, GL_RGBA, GL_BGRA.
             */
            GLenum format() const;
            TextureType type() const;
        };
    }
}


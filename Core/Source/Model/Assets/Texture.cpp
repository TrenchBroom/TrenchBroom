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

#include "Texture.h"
#include <cassert>
#include <algorithm>
#include <cstring>
#include "Utilities/Console.h"
#include "Utilities/Utils.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            bool compareByName(const Texture* texture1, const Texture* texture2) {
                return texture1->name <= texture2->name;
            }

            bool compareByUsageCount(const Texture* texture1, const Texture* texture2) {
                if (texture1->usageCount == texture2->usageCount)
                    return compareByName(texture1, texture2);
                return texture1->usageCount > texture2->usageCount;
            }

            void Texture::init(const std::string& name, unsigned int width, unsigned int height) {
                static int currentId = 1;
                this->uniqueId = currentId++;
                this->name = name;
                this->width = width;
                this->height = height;
                this->dummy = false;
                this->m_textureId = 0;
                this->usageCount = 0;
				m_textureBuffer = NULL;
            }

            void Texture::init(const std::string& name, const unsigned char* indexImage, unsigned int width, unsigned int height, const Palette* palette) {
                init(name, width, height);

                if (indexImage != NULL) {
                    unsigned int pixelCount = width * height;
                    m_textureBuffer = new unsigned char[pixelCount * 3];
                    palette->indexToRgb(indexImage, m_textureBuffer, pixelCount);

                    averageColor.x = averageColor.y = averageColor.z = 0;
                    averageColor.w = 1;
                    for (unsigned int i = 0; i < pixelCount; i++) {
                        averageColor.x += (m_textureBuffer[i * 3 + 0] / 255.0f);
                        averageColor.y += (m_textureBuffer[i * 3 + 1] / 255.0f);
                        averageColor.z += (m_textureBuffer[i * 3 + 2] / 255.0f);
                    }

                    averageColor.x /= pixelCount;
                    averageColor.y /= pixelCount;
                    averageColor.z /= pixelCount;
                }
            }

            Texture::Texture(const std::string& name, const unsigned char* rgbImage, unsigned int width, unsigned int height) {
                init(name, width, height);
                if (rgbImage != NULL) {
                    int pixelCount = width * height;
                    m_textureBuffer = new unsigned char[pixelCount * 3];
                    memcpy(m_textureBuffer, rgbImage, pixelCount * 3);
                }
            }

            Texture::Texture(const std::string& name, const unsigned char* indexImage, unsigned int width, unsigned int height, const Palette& palette) {
                init(name, indexImage, width, height, &palette);
            }

            Texture::Texture(const IO::Mip& mip, const Palette& palette) {
                init(mip.name, mip.mip0, mip.width, mip.height, &palette);
            }

            Texture::Texture(const std::string& name, const AliasSkin& skin, unsigned int skinIndex, const Palette& palette) {
                init(name, skin.pictures[skinIndex], skin.width, skin.height, &palette);
            }

            Texture::Texture(const std::string& name, const BspTexture& texture, const Palette& palette) {
                init(name, texture.image, texture.width, texture.height, &palette);
            }

            Texture::Texture(const std::string& name) {
                init(name, 1, 1);
                m_textureBuffer = new unsigned char[4];
                for (int i = 0; i < 4; i++)
                    m_textureBuffer[i] = 0;
                dummy = true;
            }

            Texture::~Texture() {
                if (m_textureId > 0)
                    glDeleteTextures(1, &m_textureId);
                if (m_textureBuffer != NULL)
                    delete[] m_textureBuffer;
            }

            void Texture::activate() {
                if (m_textureId == 0) {
                    if (m_textureBuffer != NULL) {
                        glGenTextures(1, &m_textureId);
                        glBindTexture(GL_TEXTURE_2D, m_textureId);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_textureBuffer);
                        delete[] m_textureBuffer;
                        m_textureBuffer = NULL;
                    } else {
                        log(TB_LL_WARN, "Cannot create texture '%s'", name.c_str());
                    }
                }

                glBindTexture(GL_TEXTURE_2D, m_textureId);
            }

            void Texture::deactivate() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            TextureCollection::TextureCollection(const std::string& name, IO::Wad& wad, const Palette& palette) : m_name(name) {
                for (unsigned int i = 0; i < wad.entries.size(); i++) {
                    IO::WadEntry& entry = wad.entries[i];
                    if (entry.type == IO::WT_MIP) {
                        IO::Mip* mip = wad.loadMipAtEntry(entry);
                        Texture* texture = new Texture(*mip, palette);
                        m_textures.push_back(texture);
                        delete mip;
                    }
                }
            }

            TextureCollection::~TextureCollection() {
                while (!m_textures.empty()) delete m_textures.back(), m_textures.pop_back();
            }

            const std::vector<Texture*>& TextureCollection::textures() const {
                return m_textures;
            }

            std::vector<Texture*> TextureCollection::textures(ETextureSortCriterion criterion) const {
                std::vector<Texture*> result = m_textures;
                if (criterion == TB_TS_USAGE) sort(result.begin(), result.end(), compareByUsageCount);
                else sort(result.begin(), result.end(), compareByName);
                return result;
            }

            const std::string& TextureCollection::name() const {
                return m_name;
            }

            void TextureManager::reloadTextures() {
                m_texturesCaseSensitive.clear();
                m_texturesCaseInsensitive.clear();
                for (unsigned int i = 0; i < m_collections.size(); i++) {
                    TextureCollection* collection = m_collections[i];
                    const std::vector<Texture*> textures = collection->textures();
                    for (unsigned int j = 0; j < textures.size(); j++) {
                        Texture* texture = textures[j];
                        m_texturesCaseSensitive.insert(std::pair<std::string, Texture*>(texture->name, texture));
                        m_texturesCaseInsensitive.insert(std::pair<std::string, Texture*>(toLower(texture->name), texture));
                    }
                }
            }

            TextureManager::~TextureManager() {
                clear();
            }

            void TextureManager::addCollection(TextureCollection* collection, unsigned int index) {
                assert(index <= m_collections.size());
                m_collections.insert(m_collections.begin() + index, collection);
                reloadTextures();
                textureManagerDidChange(*this);
            }

            void TextureManager::removeCollection(unsigned int index) {
                assert(index < m_collections.size());
                delete m_collections[index];
                m_collections.erase(m_collections.begin() + index);
                reloadTextures();
                textureManagerDidChange(*this);
            }

            void TextureManager::clear() {
                m_texturesCaseSensitive.clear();
                m_texturesCaseInsensitive.clear();
                while (!m_collections.empty()) delete m_collections.back(), m_collections.pop_back();
                textureManagerDidChange(*this);
            }

            const std::vector<TextureCollection*>& TextureManager::collections() {
                return m_collections;
            }

            const std::vector<Texture*> TextureManager::textures(ETextureSortCriterion criterion) {
                std::vector<Texture*> result;
                for (TextureMap::iterator it = m_texturesCaseSensitive.begin(); it != m_texturesCaseSensitive.end(); it++)
                    result.push_back(it->second);

                if (criterion == TB_TS_USAGE) sort(result.begin(), result.end(), compareByUsageCount);
                else sort(result.begin(), result.end(), compareByName);
                return result;
            }

            Texture* TextureManager::texture(const std::string& name) {
                TextureMap::iterator it = m_texturesCaseSensitive.find(name);
                if (it == m_texturesCaseSensitive.end()) {
                    it = m_texturesCaseInsensitive.find(toLower(name));
                    if (it == m_texturesCaseInsensitive.end())
                        return NULL;
                }
                return it->second;
            }

            void TextureManager::activateTexture(const std::string& name) {
                Texture* tex = texture(name);
                tex->activate();
            }

            void TextureManager::deactivateTexture() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}

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
#include <assert.h>

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            bool compareByName(const Texture* texture1, const Texture* texture2) {
                return texture1->name <= texture2->name;
            }
            
            bool compareByUsageCount(const Texture* texture1, const Texture* texture2) {
                if (texture1->usageCount == texture2->usageCount)
                    return compareByName(texture1, texture2);
                return texture1->usageCount < texture2->usageCount;
            }
            
            void Texture::init(const string& name, const unsigned char* image, int width, int height, const Palette* palette) {
                static int currentId = 0;
                this->uniqueId = currentId++;
                this->name = name;
                this->width = width;
                this->height = height;
                this->dummy = false;
                this->m_textureId = 0;
                this->usageCount = 0;
                
                if (image != NULL) {
                    int pixelCount = width * height;
                    m_textureBuffer = new unsigned char[pixelCount * 3];
                    palette->indexToRgb(image, m_textureBuffer, pixelCount);
                    
                    averageColor.x = averageColor.y = averageColor.z = 0;
                    averageColor.w = 1;
                    for (int i = 0; i < pixelCount; i++) {
                        averageColor.x += m_textureBuffer[i * 3 + 0] / 255.0;
                        averageColor.y += m_textureBuffer[i * 3 + 1] / 255.0;
                        averageColor.z += m_textureBuffer[i * 3 + 2] / 255.0;
                    }
                    
                    averageColor.x /= pixelCount;
                    averageColor.y /= pixelCount;
                    averageColor.z /= pixelCount;
                }
            }
            
            Texture::Texture(const string& name, const unsigned char* image, int width, int height, const Palette& palette) {
                init(name, image, width, height, &palette);
            }
            
            Texture::Texture(const IO::Mip& mip, const Palette& palette) {
                init(mip.name, mip.mip0, mip.width, mip.height, &palette);
            }
            
            Texture::Texture(const string& name, const AliasSkin& skin, int skinIndex, const Palette& palette) {
                init(name, skin.pictures[skinIndex], skin.width, skin.height, &palette);
            }
            
            Texture::Texture(const string& name, const BspTexture& texture, const Palette& palette) {
                init(name, texture.image, texture.width, texture.height, &palette);
            }
            
            Texture::Texture(const string& name) {
                init(name, NULL, 1, 1, NULL);
                dummy = true;
            }
            
            Texture::~Texture() {
                if (m_textureId > 0)
                    glDeleteTextures(1, &m_textureId);
                if (m_textureBuffer != NULL)
                    delete[] m_textureBuffer;
            }
            
            void Texture::activate() {
                if (dummy) return;
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
                        fprintf(stdout, "Warning: Cannot create texture '%s'", name.c_str());
                    }
                }
                
                glBindTexture(GL_TEXTURE_2D, m_textureId);
            }
            
            void Texture::deactivate() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            
            TextureCollection::TextureCollection(const string& name, IO::Wad& wad, const Palette& palette) {
                this->name = name;
                for (int i = 0; i < wad.entries.size(); i++) {
                    IO::WadEntry& entry = wad.entries[i];
                    if (entry.type == WT_MIP) {
                        IO::Mip* mip = wad.loadMipAtEntry(entry);
                        Texture* texture = new Texture(*mip, palette);
                        textures.push_back(texture);
                        delete mip;
                    }
                }
            }
            
            TextureCollection::~TextureCollection() {
                while (!textures.empty()) delete textures.back(), textures.pop_back();
            }
            
            void TextureManager::reloadTextures() {
                m_textures.clear();
                for (int i = 0; i < m_collections.size(); i++) {
                    TextureCollection* collection = m_collections[i];
                    for (int j = 0; j < collection->textures.size(); j++) {
                        Texture* texture = collection->textures[i];
                        m_textures[texture->name] = texture;
                    }
                }
            }
            
            TextureManager::~TextureManager() {
                setPostNotifications(false);
                clear();
            }
            
            void TextureManager::addCollection(TextureCollection* collection, int index) {
                assert(index >= 0 && index <= m_collections.size());
                m_collections.insert(m_collections.begin() + index, collection);
                reloadTextures();
                postNotification(TextureManagerChanged, NULL);
            }
            
            void TextureManager::removeCollection(int index) {
                assert(index >= 0 && index < m_collections.size());
                delete m_collections[index];
                m_collections.erase(m_collections.begin() + index);
                postNotification(TextureManagerChanged, NULL);
            }
            
            void TextureManager::clear() {
                m_dummies.clear();
                m_textures.clear();
                while (!m_collections.empty()) delete m_collections.back(), m_collections.pop_back();
                postNotification(TextureManagerChanged, NULL);
            }
            
            const vector<TextureCollection*> TextureManager::collections() {
                return m_collections;
            }
            
            const vector<Texture*> TextureManager::textures(ETextureSortCriterion criterion) {
                vector<Texture*> result;
                map<string, Texture*>::iterator it;
                for (it = m_textures.begin(); it != m_textures.end(); it++)
                    result.push_back(it->second);
                
                if (criterion == TS_USAGE) sort(result.begin(), result.end(), compareByUsageCount);
                else sort(result.begin(), result.end(), compareByName);
                return result;
            }
            
            Texture* TextureManager::texture(const string& name) {
                map<string, Texture*>::iterator it;
                if ((it = m_textures.find(name)) != m_textures.end()) return it->second;
                if ((it = m_dummies.find(name)) != m_dummies.end()) return it->second;
                
                Texture* dummy = new Texture(name);
                m_dummies[name] = dummy;
                return dummy;
            }
            
            void TextureManager::activateTexture(const string& name) {
                Texture* tex = texture(name);
                tex->activate();
            }
            
            void TextureManager::deactivateTexture() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}
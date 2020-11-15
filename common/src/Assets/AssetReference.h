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

#include <utility>

#ifndef AssetReference_h
#define AssetReference_h

namespace TrenchBroom {
    namespace Assets {
        template <typename T>
        class AssetReference {
        private:
            T* m_asset;
        public:
            explicit AssetReference(T* asset = nullptr) :
            m_asset(asset) {
                if (m_asset != nullptr) {
                    m_asset->incUsageCount();
                }
            }
            
            AssetReference(const AssetReference& other) :
            AssetReference(other.m_asset) {}

            AssetReference(AssetReference&& other) :
            m_asset(std::exchange(other.m_asset, nullptr)) {}
            
            ~AssetReference() {
                if (m_asset != nullptr) {
                    m_asset->decUsageCount();
                }
            }
            
            AssetReference& operator=(AssetReference other) {
                using std::swap;
                swap(*this, other);
                return *this;
            }

            friend void swap(AssetReference& lhs, AssetReference& rhs) {
                using std::swap;
                swap(lhs.m_asset, rhs.m_asset);
            }

            T* get() {
                return m_asset;
            }

            const T* get() const {
                return m_asset;
            }

            friend bool operator==(const AssetReference& lhs, const AssetReference& rhs) {
                return lhs.m_asset == rhs.m_asset;
            }

            friend bool operator!=(const AssetReference& lhs, const AssetReference& rhs) {
                return !(lhs == rhs);
            }
        };
    }
}

#endif /* AssetReference_h */

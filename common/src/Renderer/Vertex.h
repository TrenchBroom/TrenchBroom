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

#ifndef TrenchBroom_Vertex_h
#define TrenchBroom_Vertex_h

#include <cstddef>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        template <typename... Attrs>
        class VertexSpec;

        template <typename... Attrs>
        struct Vertex;

        template <typename Attr, typename... Attrs>
        struct Vertex<Attr, Attrs...> {
            using Spec = VertexSpec<Attr, Attrs...>;
            using List = std::vector<Vertex<Attr, Attrs...>>;

            typename Attr::ElementType attr;
            Vertex<Attrs...> rest;

            Vertex() = default;

            template <typename ElementType, typename... ElementTypes>
            explicit Vertex(ElementType&& i_attr, ElementTypes&&... i_attrs) :
            attr(std::forward<ElementType>(i_attr)),
            rest(std::forward<ElementTypes>(i_attrs)...) {}

            template <typename... I>
            static List toList(const size_t count, I... cur) {
                static_assert(sizeof...(I) == sizeof...(Attrs) + 1, "number of iterators must match number of vertex attributes");
                List result;
                result.reserve(count);
                for (size_t i = 0; i < count; ++i) {
                    result.emplace_back((*cur++)...);
                }
                return result;
            }
        };

        template <typename Attr>
        struct Vertex<Attr> {
            using Spec = VertexSpec<Attr>;
            using List = std::vector<Vertex<Attr>>;

            typename Attr::ElementType attr;

            Vertex() = default;

            template <typename ElementType>
            explicit Vertex(ElementType&& i_attr) :
            attr(std::forward<ElementType>(i_attr)) {}

            template <typename I>
            static List toList(const size_t count, I cur) {
                List result;
                result.reserve(count);
                for (size_t i = 0; i < count; ++i) {
                    result.emplace_back(*cur++);
                }
                return result;
            }
        };

        template <size_t I>
        struct GetVertexComponent {
            template <typename... Attrs>
            static const auto& get(const Vertex<Attrs...>& v) {
                return GetVertexComponent<I-1>::get(v.rest);
            }

            template <typename... Attrs>
            const auto& operator()(const Vertex<Attrs...>& v) const {
                return GetVertexComponent<I-1>::get(v.rest);
            }
        };

        template <>
        struct GetVertexComponent<0> {
            template <typename... Attrs>
            static const auto& get(const Vertex<Attrs...>& v) {
                return v.attr;
            }

            template <typename... Attrs>
            const auto& operator()(const Vertex<Attrs...>& v) const {
                return v.attr;
            }
        };

        /**
         * Helper to get access a vertex attribute value by index.
         * @tparam I the index of the vertex attribute
         * @tparam Attrs the vertex attribute types
         * @param v the vertex
         * @return a reference to the attribute value
         */
        template <size_t I, typename... Attrs>
        const auto& getVertexComponent(const Vertex<Attrs...>& v) {
            return GetVertexComponent<I>::get(v);
        }
    }
}

#endif

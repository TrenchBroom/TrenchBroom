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

#include "FloatType.h"
#include "Model/HitType.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        class Hit;

        class HitFilter {
        private:
            class Always;
            class Never;
        public:
            static std::unique_ptr<HitFilter> always();
            static std::unique_ptr<HitFilter> never();

            virtual ~HitFilter();

            std::unique_ptr<HitFilter> clone() const;

            bool matches(const Hit& hit) const;
        private:
            virtual std::unique_ptr<HitFilter> doClone() const = 0;
            virtual bool doMatches(const Hit& hit) const = 0;
        };

        class HitFilterChain : public HitFilter {
        private:
            const std::unique_ptr<const HitFilter> m_filter;
            const std::unique_ptr<const HitFilter> m_next;
        public:
            HitFilterChain(std::unique_ptr<const HitFilter> filter, std::unique_ptr<const HitFilter> next);
            ~HitFilterChain() override;
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };

        class TypedHitFilter : public HitFilter {
        private:
            HitType::Type m_typeMask;
        public:
            TypedHitFilter(HitType::Type typeMask);
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };

        class SelectionHitFilter : public HitFilter {
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };

        class TransitivelySelectedHitFilter : public HitFilter {
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };

        class MinDistanceHitFilter : public HitFilter {
        private:
            FloatType m_minDistance;
        public:
            MinDistanceHitFilter(FloatType minDistance);
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };

        class ContextHitFilter : public HitFilter {
        private:
            const EditorContext& m_context;
        public:
            ContextHitFilter(const EditorContext& context);
        private:
            std::unique_ptr<HitFilter> doClone() const override;
            bool doMatches(const Hit& hit) const override;
        };
    }
}



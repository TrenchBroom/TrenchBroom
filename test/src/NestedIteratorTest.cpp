/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include <gtest/gtest.h>

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "NestedIterator.h"
#include "Model/ModelTypes.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFacesIterator.h"
#include "Model/BrushFaceTypes.h"

namespace TrenchBroom {
    inline Model::Brush* makeBrush() {
        Model::BrushFace* left = new Model::QuakeBrushFace(Vec3(0.0, 0.0, 0.0),
                                                           Vec3(0.0, 1.0, 0.0),
                                                           Vec3(0.0, 0.0, 1.0));
        Model::BrushFace* right = new Model::QuakeBrushFace(Vec3(16.0, 0.0, 0.0),
                                                            Vec3(16.0, 0.0, 1.0),
                                                            Vec3(16.0, 1.0, 0.0));
        Model::BrushFace* front = new Model::QuakeBrushFace(Vec3(0.0, 0.0, 0.0),
                                                            Vec3(0.0, 0.0, 1.0),
                                                            Vec3(1.0, 0.0, 0.0));
        Model::BrushFace* back = new Model::QuakeBrushFace(Vec3(0.0, 16.0, 0.0),
                                                           Vec3(1.0, 16.0, 0.0),
                                                           Vec3(0.0, 16.0, 1.0));
        Model::BrushFace* top = new Model::QuakeBrushFace(Vec3(0.0, 0.0, 16.0),
                                                          Vec3(0.0, 1.0, 16.0),
                                                          Vec3(1.0, 0.0, 16.0));
        Model::BrushFace* bottom = new Model::QuakeBrushFace(Vec3(0.0, 0.0, 0.0),
                                                             Vec3(1.0, 0.0, 0.0),
                                                             Vec3(0.0, 1.0, 0.0));
        
        Model::BrushFaceList faces;
        faces.push_back(left);
        faces.push_back(right);
        faces.push_back(front);
        faces.push_back(back);
        faces.push_back(top);
        faces.push_back(bottom);
        
        BBox3 worldBounds(-8192.0, 8192.0);
        return new Model::Brush(worldBounds, faces);
    }
    
    TEST(NestedIteratorTest, testEmptyBrushFaceIterator) {
        Model::BrushList brushes;
        
        Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
        Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
        
        ASSERT_TRUE(begin == end);
    }

    TEST(NestedIteratorTest, testBrushEmptyFaceIterator) {
        Model::BrushList brushes;
        
        BBox3 worldBounds(-8192.0, 8192.0);
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        
        Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
        Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
        
        ASSERT_TRUE(begin == end);
        VectorUtils::clearAndDelete(brushes);
    }
    
    TEST(NestedIteratorTest, testOneBrushFacesIterator) {
        Model::BrushList brushes;
        brushes.push_back(makeBrush());
        
        Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
        Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
        
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_TRUE(begin == end);
        VectorUtils::clearAndDelete(brushes);
    }
    
    TEST(NestedIteratorTest, testTwoBrushesFacesIterator) {
        Model::BrushList brushes;
        brushes.push_back(makeBrush());
        brushes.push_back(makeBrush());
        
        Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
        Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
        
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_TRUE(begin == end);
        VectorUtils::clearAndDelete(brushes);
    }

    
    TEST(NestedIteratorTest, testTwoBrushesFacesIteratorWithEmptyBrushes) {
        BBox3 worldBounds(-8192.0, 8192.0);

        Model::BrushList brushes;
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        brushes.push_back(makeBrush());
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        brushes.push_back(makeBrush());
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        brushes.push_back(new Model::Brush(worldBounds, Model::EmptyBrushFaceList));
        
        Model::BrushFacesIterator::OuterIterator begin = Model::BrushFacesIterator::begin(brushes);
        Model::BrushFacesIterator::OuterIterator end = Model::BrushFacesIterator::end(brushes);
        
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_FALSE(begin == end);
        ++begin;
        ASSERT_TRUE(begin == end);
        VectorUtils::clearAndDelete(brushes);
    }
}

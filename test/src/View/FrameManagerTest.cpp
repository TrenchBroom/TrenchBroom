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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Exceptions.h"
#include "GL/GL.h"
#include "IO/Path.h"
#include "Model/QuakeGame.h"
#include "View/FrameManager.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        TEST(FrameManagerTest, sDINewFrame) {
            using namespace testing;
            GLMock = new NiceMock<CGLMock>(); // NiceMock suppresses "uninteresting call" warnings
            Mock::AllowLeak(GLMock);

            FrameManager manager(true);
            
            MapFrame* frame1 = manager.newFrame();
            ASSERT_TRUE(frame1 != NULL);
            
            MapFrame* frame2 = manager.newFrame();
            ASSERT_TRUE(frame2 != NULL);

            const FrameList& frames = manager.frames();
            ASSERT_EQ(1u, frames.size());
            ASSERT_EQ(frame1, frames[0]);
            ASSERT_EQ(frame2, frames[0]);
        }
        
        TEST(FrameManagerTest, mDINewFrame) {
            using namespace testing;
            GLMock = new NiceMock<CGLMock>(); // NiceMock suppresses "uninteresting call" warnings
            Mock::AllowLeak(GLMock);

            FrameManager manager(false);
            
            MapFrame* frame1 = manager.newFrame();
            ASSERT_TRUE(frame1 != NULL);
            
            MapFrame* frame2 = manager.newFrame();
            ASSERT_TRUE(frame2 != NULL);
            
            const FrameList& frames = manager.frames();
            ASSERT_EQ(2u, frames.size());
            ASSERT_EQ(frame1, frames[0]);
            ASSERT_EQ(frame2, frames[1]);
        }
        
        TEST(FrameManagerTest, closeAllFrames) {
            using namespace testing;
            GLMock = new NiceMock<CGLMock>(); // NiceMock suppresses "uninteresting call" warnings
            Mock::AllowLeak(GLMock);

            FrameManager manager(false);

            manager.newFrame();
            manager.newFrame();

            manager.closeAllFrames();
            ASSERT_TRUE(manager.frames().empty());
            ASSERT_TRUE(manager.allFramesClosed());
        }
    }
}

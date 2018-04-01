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

#include <gtest/gtest.h>

#include <memory>

#include "CollectionUtils.h"
#include "Model/ModelTypes.h"
#include "Model/PortalFile.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST(PortalFileTest, parseIncludedPortalFiles) {
            const auto basePath = IO::Disk::getCurrentWorkingDir() + IO::Path("data/Model/PortalFile");
            const auto prtFiles = IO::Disk::findItems(basePath, [] (const IO::Path& path, bool directory) {
                return !directory && StringUtils::caseInsensitiveEqual(path.extension(), "prt");
            });

            for (const auto& path : prtFiles) {
                std::unique_ptr<Model::PortalFile> portalFile;

                EXPECT_NO_THROW(portalFile = std::make_unique<Model::PortalFile>(path));
                if (portalFile) {
                    EXPECT_GT(portalFile->portals().size(), 0);
                }
            }
        }
    }
}

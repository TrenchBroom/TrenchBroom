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

#include "Exceptions.h"
#include "Assets/Texture.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/MapFormat.h"
#include "Model/PickResult.h"
#include "Model/WorldNode.h"

#include <kdl/collection_utils.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/segment.h>
#include <vecmath/polygon.h>
#include <vecmath/ray.h>

#include <memory>
#include <string>
#include <vector>

#include "Catch2.h"
#include "GTestCompat.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("BrushNodeTest.buildBrushFail", "[BrushNodeTest]") {
            /*
             See https://github.com/TrenchBroom/TrenchBroom/issues/1186
             This crash was caused by the correction of newly created vertices in Polyhedron::Edge::split - it would nudge vertices such that their plane status changed, resulting in problems when building the seam.
             */

            const std::string data("{\n"
                              "( 656 976 672 ) ( 656 1104 672 ) ( 656 976 800 ) black -976 672 0 1 1 //TX2\n"
                              "( 632 496.00295 640 ) ( 632 688.00137 768 ) ( 504 496.00295 640 ) doortrim2 632 331 0 -1 1.49999 //TX1\n"
                              "( 666.74516 848 928 ) ( 666.74516 826.95693 1054.25842 ) ( 794.74516 848 928 ) woodplank1 -941 667 90 0.98639 -1 //TX2\n"
                              "( 672 880 416 ) ( 672 880 544 ) ( 672 1008 416 ) wswamp2_1 -880 416 0 1 1 //TX1\n"
                              "( 656 754.57864 1021.42136 ) ( -84592 754.57864 1021.42136 ) ( 656 61034.01582 -59258.01582 ) skip 1 2 0 -666 470.93310 //TX2\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushFail2", "[BrushNodeTest]") {
            /*
             See https://github.com/TrenchBroom/TrenchBroom/issues/1185

             The cause for the endless loop was, like above, the vertex correction in Polyhedron::Edge::split.
             */

            const std::string data("{\n"
                              "( 32 1392 960 ) ( 32 1392 1088 ) ( 32 1264 960 ) black 1392 960 0 -1 1 //TX1\n"
                              "( 64 1137.02125 916.65252 ) ( 64 1243.52363 845.65079 ) ( -64 1137.02125 916.65252 ) woodplank1 64 1367 0 -1 0.83205 //TX1\n"
                              "( 5.25484 1296 864 ) ( 5.25484 1317.04307 990.25842 ) ( -122.74516 1296 864 ) woodplank1 -876 -5 90 0.98639 1 //TX2\n"
                              "( 64 1184 819.77710 ) ( 64 1184 947.77710 ) ( 64 1312 819.77710 ) woodplank1 -820 1184 90 1 -1 //TX2\n"
                              "( 16 1389.42136 957.42136 ) ( 85264 1389.42136 957.42136 ) ( 16 -58890.01582 -59322.01582 ) skip 0 -3 0 666 -470.93310 //TX2\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushFail3", "[BrushNodeTest]") {
            // From https://github.com/TrenchBroom/TrenchBroom/issues/1697

            /*
             This brush is broken beyond repair. When building the polyhedron, we run into problems where no seam can be
             computed. We opt to just throw an exception that case and expect it to fail without crashing.
             */

            /*
             Update after fixing issue https://github.com/TrenchBroom/TrenchBroom/issues/2611
             With the revised face sort order (sort by normal), this brush can now be built.
             */

            const std::string data("{\n"
                              "( -24 1844 112.527 ) ( -24 1844 112 ) ( -24 1844.27 113.544 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20 1848.53 112.527 ) ( -20 1848.53 112 ) ( -20 1847.47 112.526 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4797 1844 112.092 ) ( -23.4797 1844 112 ) ( -23.6766 1844 112.421 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -24 1852 112.526 ) ( -24 1852 112 ) ( -23.9258 1852 112.526 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4559 1851.73 112 ) ( -23.4732 1852 112 ) ( -21.5439 1851.2 112 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.4727 1848.53 116 ) ( -23.4727 1847.47 116 ) ( -24 1848.53 116 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.405 1844.27 113.439 ) ( -23.7974 1844 112.491 ) ( -23.7971 1844.27 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.7971 1844.27 113.544 ) ( -23.9311 1844 112.527 ) ( -24 1844.27 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.5781 1845.54 115.201 ) ( -23.6762 1844.8 114.456 ) ( -24 1845.54 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.5099 1846.46 115.728 ) ( -23.5792 1845.54 115.201 ) ( -24 1846.46 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.6779 1844.8 114.456 ) ( -23.798 1844.27 113.545 ) ( -24 1844.8 114.456 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4727 1847.47 116 ) ( -23.5085 1846.46 115.728 ) ( -24 1847.47 116 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 15.9999 ] 90 1 -1\n"
                              "( -23.5786 1850.46 115.201 ) ( -23.5092 1849.54 115.728 ) ( -24 1850.46 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.9316 1852 112.526 ) ( -23.7979 1851.73 113.545 ) ( -24 1852 112.526 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.5085 1849.54 115.728 ) ( -23.4726 1848.53 116 ) ( -24 1849.54 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 15.9999 ] 90 1 -1\n"
                              "( -23.4037 1851.73 113.439 ) ( -23.7965 1851.73 113.544 ) ( -23.7975 1852 112.491 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.6777 1851.2 114.457 ) ( -23.5797 1850.46 115.201 ) ( -24 1851.2 114.457 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -23.7974 1851.73 113.544 ) ( -23.6772 1851.2 114.457 ) ( -24 1851.73 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.7994 1848.53 114.456 ) ( -20.2725 1848.53 113.544 ) ( -20.7993 1847.47 114.456 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.544 1848.53 115.201 ) ( -20.7995 1848.53 114.456 ) ( -21.5442 1847.47 115.201 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2725 1848.53 113.544 ) ( -20 1848.53 112.527 ) ( -20.2726 1847.47 113.544 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4566 1847.47 115.728 ) ( -23.4727 1847.47 116 ) ( -22.4567 1848.53 115.728 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.4567 1848.53 115.728 ) ( -21.5439 1848.53 115.201 ) ( -22.4452 1847.46 115.721 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.5444 1844.8 112.324 ) ( -21.5444 1844.8 112 ) ( -22.456 1844.27 112.204 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2727 1846.46 112.491 ) ( -20.2727 1846.46 112 ) ( -20.799 1845.54 112.421 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.799 1845.54 112.421 ) ( -20.799 1845.54 112 ) ( -21.544 1844.8 112.323 ) O_METAL1_19AD [ 0 -1 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4559 1844.27 112.204 ) ( -22.4559 1844.27 112 ) ( -23.4738 1844 112.07 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20 1847.47 112.527 ) ( -20 1847.47 112 ) ( -20.2727 1846.46 112.491 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5608 1844.27 112.594 ) ( -22.4564 1844.27 112.205 ) ( -23.5091 1844 112.203 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5615 1846.46 115.474 ) ( -22.7649 1845.54 114.983 ) ( -23.5089 1846.46 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0512 1844.8 114.288 ) ( -23.677 1844.8 114.456 ) ( -22.7637 1845.54 114.982 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -23.5086 1846.46 115.727 ) ( -22.4567 1847.47 115.728 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.0364 1844.8 113.51 ) ( -21.7108 1844.8 112.946 ) ( -22.7661 1844.27 112.95 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.4392 1845.54 113.966 ) ( -21.0138 1846.47 114.293 ) ( -21.0168 1845.54 113.235 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7646 1844.27 112.948 ) ( -22.5612 1844.27 112.595 ) ( -23.5787 1844 112.323 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0176 1846.46 114.289 ) ( -20.7995 1847.47 114.456 ) ( -20.5267 1846.46 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7113 1844.8 112.948 ) ( -21.5438 1844.8 112.323 ) ( -22.5613 1844.27 112.596 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5159 1846.47 113.427 ) ( -20.27 1846.47 112.503 ) ( -21.0173 1845.54 113.236 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0171 1845.55 113.237 ) ( -20.7981 1845.55 112.42 ) ( -21.7127 1844.8 112.949 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.527 1846.46 113.439 ) ( -20.2725 1847.47 113.544 ) ( -20.2728 1846.46 112.49 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.0347 1844.8 113.508 ) ( -21.4382 1845.54 113.965 ) ( -21.7115 1844.8 112.948 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4921 1844.8 113.966 ) ( -22.0342 1844.8 113.508 ) ( -23.0526 1844.27 113.235 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7105 1846.46 114.982 ) ( -21.0178 1846.46 114.289 ) ( -22.0341 1845.54 114.561 ) O_METAL1_19AD [ -1 0 0 -0 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -22.0365 1845.54 114.562 ) ( -21.4377 1845.54 113.964 ) ( -22.4934 1844.8 113.967 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0525 1844.27 113.235 ) ( -22.7657 1844.27 112.949 ) ( -23.6769 1844 112.422 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7119 1846.46 114.983 ) ( -21.5441 1847.47 115.201 ) ( -21.0172 1846.46 114.288 ) O_METAL1_19AD [ 0 -0 -1 -0 ] [ 0 -1 0 16 ] 90 1 -1\n"
                              "( -23.0525 1844.8 114.29 ) ( -22.4921 1844.8 113.966 ) ( -23.405 1844.27 113.439 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -21.7115 1846.46 114.983 ) ( -22.7644 1845.54 114.982 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.7646 1845.54 114.983 ) ( -22.0349 1845.54 114.561 ) ( -23.0523 1844.8 114.289 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0523 1844.27 113.235 ) ( -23.6767 1844 112.421 ) ( -23.4045 1844.27 113.439 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -22.4567 1847.47 115.728 ) ( -21.7115 1846.46 114.983 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0519 1844.8 114.289 ) ( -23.4042 1844.27 113.438 ) ( -23.6773 1844.8 114.457 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4733 1852 112.069 ) ( -23.4733 1852 112 ) ( -22.4557 1851.73 112.202 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2725 1849.54 112.491 ) ( -20.2725 1849.54 112 ) ( -20 1848.53 112.527 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.5438 1851.2 112.324 ) ( -21.5438 1851.2 112 ) ( -20.7997 1850.46 112.422 ) O_METAL1_19AD [ 0 -1 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4557 1851.73 112.202 ) ( -22.4557 1851.73 112 ) ( -21.5433 1851.2 112.322 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.7995 1850.46 112.421 ) ( -20.7995 1850.46 112 ) ( -20.2725 1849.54 112.491 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5615 1851.73 112.597 ) ( -23.5097 1852 112.204 ) ( -22.4559 1851.73 112.203 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5273 1849.54 113.439 ) ( -21.0177 1849.54 114.289 ) ( -21.0178 1850.46 113.236 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7626 1851.73 112.947 ) ( -22.5616 1851.73 112.599 ) ( -22.0352 1851.2 113.507 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0175 1850.46 113.235 ) ( -21.4388 1850.46 113.965 ) ( -21.7056 1851.19 112.95 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7643 1850.46 114.982 ) ( -23.0516 1851.2 114.289 ) ( -22.0348 1850.46 114.561 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7112 1849.54 114.983 ) ( -21.5439 1848.53 115.201 ) ( -22.562 1849.54 115.474 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.4402 1850.46 113.967 ) ( -22.035 1850.46 114.561 ) ( -22.0353 1851.2 113.51 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7646 1850.46 114.983 ) ( -22.5611 1849.54 115.474 ) ( -23.5787 1850.46 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0504 1851.2 114.289 ) ( -23.6777 1851.2 114.457 ) ( -23.4026 1851.73 113.438 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0511 1851.73 113.236 ) ( -22.7626 1851.73 112.947 ) ( -22.4919 1851.2 113.965 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.456 1848.53 115.727 ) ( -23.4729 1848.53 116 ) ( -22.5611 1849.54 115.474 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.7112 1849.54 114.983 ) ( -22.0347 1850.46 114.561 ) ( -21.0175 1849.54 114.289 ) O_METAL1_19AD [ -1 0 0 -0 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -23 1851.73 113.212 ) ( -23.4023 1851.73 113.439 ) ( -23.6625 1852 112.413 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0177 1849.54 114.289 ) ( -20.7998 1848.53 114.457 ) ( -21.7127 1849.54 114.984 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.5611 1849.54 115.474 ) ( -22.7646 1850.46 114.983 ) ( -21.7113 1849.54 114.983 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.492 1851.2 113.965 ) ( -23.0499 1851.2 114.288 ) ( -23.051 1851.73 113.236 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7616 1851.73 112.946 ) ( -23.0571 1851.73 113.234 ) ( -23.5769 1852 112.32 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2724 1849.54 112.491 ) ( -20 1848.53 112.526 ) ( -20.5263 1849.54 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7101 1851.2 112.947 ) ( -21.543 1851.2 112.323 ) ( -21.0175 1850.46 113.234 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7109 1851.2 112.947 ) ( -22.5613 1851.73 112.596 ) ( -21.5437 1851.2 112.323 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0173 1850.46 113.235 ) ( -20.7994 1850.46 112.421 ) ( -20.5265 1849.54 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7611 1851.73 112.945 ) ( -23.5758 1852 112.32 ) ( -22.5621 1851.73 112.596 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5264 1849.54 113.438 ) ( -20.2725 1848.53 113.544 ) ( -21.0175 1849.54 114.289 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7647 1850.46 114.982 ) ( -23.5781 1850.46 115.2 ) ( -23.0501 1851.2 114.289 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushWithShortEdges", "[BrushNodeTest]") {
            /*
             See https://github.com/TrenchBroom/TrenchBroom/issues/1194
             */

            const std::string data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_TRUE(nodes.empty());
        }

        TEST_CASE("BrushTest.selectedFaceCount", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushNode brush(Brush::create(worldBounds, {
                // left
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0)),
                // right
                createParaxial(
                    vm::vec3(16.0, 0.0, 0.0),
                    vm::vec3(16.0, 0.0, 1.0),
                    vm::vec3(16.0, 1.0, 0.0)),
                // front
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0),
                    vm::vec3(1.0, 0.0, 0.0)),
                // back
                createParaxial(
                    vm::vec3(0.0, 16.0, 0.0),
                    vm::vec3(1.0, 16.0, 0.0),
                    vm::vec3(0.0, 16.0, 1.0)),
                // top
                createParaxial(
                    vm::vec3(0.0, 0.0, 16.0),
                    vm::vec3(0.0, 1.0, 16.0),
                    vm::vec3(1.0, 0.0, 16.0)),
                // bottom
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
            }).value());
            
            CHECK(!brush.hasSelectedFaces());
            
            SECTION("Selecting faces correctly updates the node's face selection count") {
                brush.selectFace(0u);
                CHECK(brush.hasSelectedFaces());
                
                brush.selectFace(1u);
                CHECK(brush.hasSelectedFaces());
                
                brush.deselectFace(0u);
                CHECK(brush.hasSelectedFaces());
                
                brush.deselectFace(1u);
                CHECK(!brush.hasSelectedFaces());
            }
            
            SECTION("Passing a brush with selected faces to constructor correctly updates the node's face selection count") {
                REQUIRE(!brush.hasSelectedFaces());
                
                Brush copy = brush.brush();
                copy.face(0u).select();
                copy.face(1u).select();
                
                BrushNode another(std::move(copy));
                CHECK(another.hasSelectedFaces());

                another.deselectFace(0u);
                CHECK(another.hasSelectedFaces());
                
                another.deselectFace(1u);
                CHECK(!another.hasSelectedFaces());
            }

            SECTION("Setting a brush with selected faces correctly updates the node's face selection count") {
                REQUIRE(!brush.hasSelectedFaces());
                
                Brush copy = brush.brush();
                copy.face(0u).select();
                copy.face(1u).select();
                
                brush.setBrush(std::move(copy));
                CHECK(brush.hasSelectedFaces());

                brush.deselectFace(0u);
                CHECK(brush.hasSelectedFaces());
                
                brush.deselectFace(1u);
                CHECK(!brush.hasSelectedFaces());
            }
            
            SECTION("Cloning a brush node returns a clone with correct face selection count") {
                REQUIRE(!brush.hasSelectedFaces());
                
                brush.selectFace(0u);
                brush.selectFace(1u);
                REQUIRE(brush.hasSelectedFaces());

                auto clone = std::unique_ptr<BrushNode>(brush.clone(worldBounds));
                CHECK(clone->hasSelectedFaces());
                
                clone->deselectFace(0u);
                clone->deselectFace(1u);
                CHECK(!clone->hasSelectedFaces());
            }
        }


        TEST_CASE("BrushNodeTest.pick", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushNode brush(Brush::create(worldBounds, {
                // left
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0)),
                // right
                createParaxial(
                    vm::vec3(16.0, 0.0, 0.0),
                    vm::vec3(16.0, 0.0, 1.0),
                    vm::vec3(16.0, 1.0, 0.0)),
                // front
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0),
                    vm::vec3(1.0, 0.0, 0.0)),
                // back
                createParaxial(
                    vm::vec3(0.0, 16.0, 0.0),
                    vm::vec3(1.0, 16.0, 0.0),
                    vm::vec3(0.0, 16.0, 1.0)),
                // top
                createParaxial(
                    vm::vec3(0.0, 0.0, 16.0),
                    vm::vec3(0.0, 1.0, 16.0),
                    vm::vec3(1.0, 0.0, 16.0)),
                // bottom
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
            }).value());

            PickResult hits1;
            brush.pick(vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::pos_y()), hits1);
            ASSERT_EQ(1u, hits1.size());

            Hit hit1 = hits1.all().front();
            ASSERT_DOUBLE_EQ(8.0, hit1.distance());
            ASSERT_EQ(vm::vec3::neg_y(), hitToFaceHandle(hit1)->face().boundary().normal);

            PickResult hits2;
            brush.pick(vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::neg_y()), hits2);
            ASSERT_TRUE(hits2.empty());
        }

        TEST_CASE("BrushNodeTest.clone", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushNode original(Brush::create(worldBounds, {
                // left
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0)),
                // right
                createParaxial(
                    vm::vec3(16.0, 0.0, 0.0),
                    vm::vec3(16.0, 0.0, 1.0),
                    vm::vec3(16.0, 1.0, 0.0)),
                // front
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0),
                    vm::vec3(1.0, 0.0, 0.0)),
                // back
                createParaxial(
                    vm::vec3(0.0, 16.0, 0.0),
                    vm::vec3(1.0, 16.0, 0.0),
                    vm::vec3(0.0, 16.0, 1.0)),
                // top
                createParaxial(
                    vm::vec3(0.0, 0.0, 16.0),
                    vm::vec3(0.0, 1.0, 16.0),
                    vm::vec3(1.0, 0.0, 16.0)),
                // bottom
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
            }).value());

            BrushNode* clone = original.clone(worldBounds);
            
            ASSERT_EQ(original.brush().faceCount(), clone->brush().faceCount());
            for (const auto& originalFace : original.brush().faces()) {
                const auto cloneFaceIndex = clone->brush().findFace(originalFace.boundary());
                ASSERT_TRUE(cloneFaceIndex.has_value());
                
                const auto& cloneFace = clone->brush().face(*cloneFaceIndex);
                ASSERT_EQ(originalFace, cloneFace);
            }
            
            delete clone;
        }

        TEST_CASE("BrushNodeTest.testAlmostDegenerateBrush", "[BrushNodeTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1194
            const std::string data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}");

            // This brush is almost degenerate. It should be rejected by the map loader.

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(0u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.invalidBrush1332", "[BrushNodeTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1332
            const std::string data("{\n"
                              "( 91.428573608  0  4.57144165 ) ( 96 16  0 ) ( 82.285690308  0  0          ) rock5_2 0 0 0 1 1\n"
                              "( 95.238098145  0 16          ) ( 96  2 16 ) ( 91.428573608  0  4.57144165 ) rock5_2 0 0 0 1 1\n"
                              "( 96           16 16          ) ( 96 16  0 ) ( 96            2 16          ) rock5_2 0 0 0 1 1\n"
                              "(  0           16 16          ) (  0  0  0 ) ( 96           16  0          ) rock5_2 0 0 90 1 1\n"
                              "(  0            0 16          ) (  0  0  0 ) (  0           16 16          ) rock5_2 0 0 0 1 1\n"

                              // The next face causes an assertion failure. It's the back face, the normal is +Y.
                              "( 96           16 16          ) (  0 16 16 ) ( 96           16  0          ) rock5_2 0 0 90 1 1\n"

                              // Normal -Y (roughly)
                              "(  0            0  0          ) (  0  0 16 ) ( 82.285690308  0  0          ) rock5_2 0 0 0 1 1\n"

                              // Normal +Z (roughly)
                              "(  0            0 16          ) (  0 16 16 ) ( 95.238098145  0 16          ) rock5_2 0 0 0 1 1\n"

                              // Normal -Z (roughly)
                              "( 82.285690308  0  0          ) ( 96 16  0 ) (  0            0  0          ) rock5_2 0 0 0 1 1\n"
                              "}");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }


        TEST_CASE("BrushNodeTest.invalidBrush1395", "[BrushNodeTest]") {
            // Brush causes assertion to fail after having had its vertices snapped
            const std::string data("{\n"
                              "( -72 497 878 ) ( -77 465 878 ) ( -77 396 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -72 497 878 ) ( -59 536 878 ) ( -65 536 905 ) rock4_2 -30 33 0 1 1\n"
                              "( -50 269 878 ) ( -59 279 878 ) ( -40 269 898 ) rock4_2 -1 33 0 1 1\n"
                              "( -67 328 878 ) ( -35 269 904 ) ( -59 279 878 ) rock4_2 -1 33 0 1 1\n"
                              "( -59 279 878 ) ( -35 269 904 ) ( -40 269 898 ) rock4_2 -1 33 0 1 1\n"
                              "( -40 269 898 ) ( -35 269 904 ) ( 28 269 911 ) rock4_2 -30 33 0 1 1\n"
                              "( 171 269 878 ) ( 169 269 884 ) ( 212 340 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 169 269 884 ) ( 192 315 893 ) rock4_2 -30 1 0 1 1\n"
                              "( 192 315 893 ) ( 169 269 884 ) ( 106 269 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 28 269 911 ) ( -53 431 911 ) ( -67 524 911 ) rock4_2 -30 1 0 1 1\n"
                              "( -67 524 911 ) ( -53 431 911 ) ( -69 515 908 ) rock4_2 -30 1 0 1 1\n"
                              "( -69 515 908 ) ( -53 431 911 ) ( -35 269 904 ) rock4_2 -30 1 0 1 1\n"
                              "( -35 269 904 ) ( -53 431 911 ) ( 28 269 911 ) rock4_2 -30 33 0 1 1\n"
                              "( -65 536 911 ) ( -67 524 911 ) ( -69 515 908 ) rock4_2 -30 1 0 1 1\n"
                              "( 205 536 911 ) ( -65 536 911 ) ( -65 536 905 ) rock4_2 -30 33 0 1 1\n"
                              "( -65 536 905 ) ( -65 536 911 ) ( -69 515 908 ) rock4_2 -30 33 0 1 1\n"
                              "( 231 504 911 ) ( 205 536 911 ) ( 246 507 884 ) rock4_2 -30 1 0 1 1\n"
                              "( 246 507 884 ) ( 205 536 911 ) ( 226 536 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 136 301 911 ) ( 231 504 911 ) ( 209 344 892 ) rock4_2 -30 1 0 1 1\n"
                              "( 209 344 892 ) ( 231 504 911 ) ( 237 499 908 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 192 315 893 ) ( 209 344 892 ) rock4_2 -30 1 0 1 1\n"
                              "( 209 344 892 ) ( 192 315 893 ) ( 136 301 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 136 301 911 ) ( 192 315 893 ) ( 106 269 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 209 344 892 ) ( 246 498 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 246 498 878 ) ( 209 344 892 ) ( 237 499 908 ) rock4_2 -1 33 0 1 1\n"
                              "( 246 511 878 ) ( 246 507 884 ) ( 226 536 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 237 499 908 ) ( 246 507 884 ) ( 246 498 878 ) rock4_2 -1 33 0 1 1\n"
                              "( 246 498 878 ) ( 246 507 884 ) ( 246 511 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -65 536 905 ) ( -69 515 908 ) ( -72 497 878 ) rock4_2 -30 33 0 1 1\n"
                              "( -67 328 878 ) ( -69 515 908 ) ( -35 269 904 ) rock4_2 -1 33 0 1 1\n"
                              "( -69 515 908 ) ( -77 465 890 ) ( -72 497 878 ) rock4_2 -30 33 0 1 1\n"
                              "( -72 497 878 ) ( -77 465 890 ) ( -77 465 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -77 465 878 ) ( -77 465 890 ) ( -77 396 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -77 396 878 ) ( -77 465 890 ) ( -67 328 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -67 328 878 ) ( -77 465 890 ) ( -69 515 908 ) rock4_2 -1 33 0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }

        TEST_CASE("BrushNodeTest.invalidBrush1801", "[BrushNodeTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/1801
            // see PolyhedronTest::clipWithInvalidSeam

            const std::string data("{\n"
                              "( -484 1513 395 ) ( -483 1371 131 ) ( -483 1777 253 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -459 1579 -115 ) ( -483 1777 253 ) *water1 -0 -0 -0 1 1\n"
                              "( -459 1579 -115 ) ( -483 1371 131 ) ( -184 1428 237 ) *water1 -0 -0 -0 1 1\n"
                              "( -459 1579 -115 ) ( -184 1428 237 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1428 237 ) ( -184 1513 396 ) ( -184 1777 254 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1513 396 ) ( -484 1513 395 ) ( -184 1777 254 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -484 1513 395 ) ( -184 1513 396 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -184 1513 396 ) ( -184 1428 237 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1777 254 ) ( -483 1777 253 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1777 253 ) ( -459 1579 -115 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/1893
        TEST_CASE("BrushNodeTest.intersectsIssue1893", "[BrushNodeTest]") {
            const std::string data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( 2368 173.07179676972467 525.07179676972441 ) ( 2368 194.92820323027539 530.92820323027559 ) ( 2368 186.92820323027561 517.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 0 -1 0 32 ] 0 1 1\n"
                              "( 2048 173.07179676972467 525.07179676972441 ) ( 2048 194.92820323027539 530.92820323027559 ) ( 2048 181.07179676972444 538.92820323027536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 0 -1 0 32 ] 0 1 1\n"
                              "( 1680 181.07179676972444 538.92820323027536 ) ( 1664 194.92820323027539 530.92820323027559 ) ( 1680 194.92820323027539 530.92820323027559 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 184 539.31370849898462 ) ( 1664 195.31370849898465 528 ) ( 1680 195.31370849898465 528 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 186.92820323027561 538.92820323027536 ) ( 1664 194.92820323027539 525.07179676972441 ) ( 1680 194.92820323027539 525.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 189.65685424949245 537.79795897113263 ) ( 1664 193.79795897113243 522.34314575050757 ) ( 1680 193.79795897113243 522.34314575050757 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 178.3431457505076 537.79795897113263 ) ( 1664 193.79795897113266 533.65685424949243 ) ( 1680 193.79795897113266 533.65685424949243 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 186.92820323027561 517.07179676972441 ) ( 1664 194.92820323027539 530.92820323027559 ) ( 1664 186.92820323027561 517.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 184 516.68629150101515 ) ( 1664 195.31370849898465 528 ) ( 1664 184 516.68629150101515 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 181.07179676972444 517.07179676972441 ) ( 1664 194.92820323027539 525.07179676972441 ) ( 1664 181.07179676972444 517.07179676972441 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 178.34314575050738 518.20204102886714 ) ( 1664 193.79795897113243 522.34314575050757 ) ( 1664 178.34314575050738 518.20204102886714 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 189.65685424949245 518.20204102886737 ) ( 1664 193.79795897113266 533.65685424949243 ) ( 1664 189.65685424949245 518.20204102886737 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 192 520 ) ( 1664 192 536 ) ( 1664 192 520 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972467 525.07179676972441 ) ( 1664 181.07179676972444 538.92820323027536 ) ( 1680 181.07179676972444 538.92820323027536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 172.68629150101518 528 ) ( 1664 184 539.31370849898462 ) ( 1680 184 539.31370849898462 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972444 530.92820323027559 ) ( 1664 186.92820323027561 538.92820323027536 ) ( 1680 186.92820323027561 538.92820323027536 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.20204102886717 533.65685424949243 ) ( 1664 189.65685424949245 537.79795897113263 ) ( 1680 189.65685424949245 537.79795897113263 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.2020410288674 522.34314575050757 ) ( 1664 178.3431457505076 537.79795897113263 ) ( 1680 178.3431457505076 537.79795897113263 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972467 525.07179676972441 ) ( 1664 186.92820323027561 517.07179676972441 ) ( 1664 173.07179676972467 525.07179676972441 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 172.68629150101518 528 ) ( 1664 184 516.68629150101515 ) ( 1664 172.68629150101518 528 ) mt_sr_v1x [ 0 0 -1 28.6864 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972444 530.92820323027559 ) ( 1664 181.07179676972444 517.07179676972441 ) ( 1664 173.07179676972444 530.92820323027559 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.20204102886717 533.65685424949243 ) ( 1664 178.34314575050738 518.20204102886714 ) ( 1664 174.20204102886717 533.65685424949243 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.2020410288674 522.34314575050757 ) ( 1664 189.65685424949245 518.20204102886737 ) ( 1664 174.2020410288674 522.34314575050757 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 520 ) ( 1664 176 536 ) ( 1680 176 536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 536 ) ( 1664 192 536 ) ( 1680 192 536 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 520 ) ( 1664 192 520 ) ( 1664 176 520 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "}\n"
                              "{\n"
                              "( 784 -624 656 ) ( 5536 -624 672 ) ( 5536 -624 656 ) __TB_empty [ 1 0 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 784 -208 656 ) ( 784 4672 672 ) ( 784 -208 672 ) __TB_empty [ 0 -1 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 784 -208 -1792 ) ( 5536 4672 -1792 ) ( 784 4672 -1792 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                              "( 784 -208 1200 ) ( 5536 4672 1200 ) ( 5536 -208 1200 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                              "( 784 4672 656 ) ( 5536 4672 672 ) ( 784 4672 672 ) __TB_empty [ -1 0 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 5536 -208 656 ) ( 5536 4672 672 ) ( 5536 4672 656 ) __TB_empty [ 0 1 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "}\n"
                              "}\n");

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
            ASSERT_TRUE(nodes.at(0)->hasChildren());
            ASSERT_EQ(2u, nodes.at(0)->children().size());

            BrushNode* pipe = static_cast<BrushNode*>(nodes.at(0)->children().at(0));
            BrushNode* cube = static_cast<BrushNode*>(nodes.at(0)->children().at(1));

            ASSERT_TRUE(pipe->intersects(cube));
            ASSERT_TRUE(cube->intersects(pipe));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2361", "[BrushNodeTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/pull/2372#issuecomment-432893836

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::string data = R"(
{
( -5706.7302805113286 648 1090 ) ( -5730.730280567378 730 1100 ) ( -5706.7302804991377 722 1076 ) so_b4b -0 -0 -0 1 1
( -5730.7302804970022 574 1112 ) ( -5706.7302805113286 648 1090 ) ( -5706.7302621135759 484 1090 ) so_b4b -41.2695 6 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5712.7302804989204 720 1048 ) ( -5730.7302803650427 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5768.7302088192082 732 1104 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805003419 728 1112 ) ( -5834.7302796344165 726 1090 ) ( -5844.7302796989916 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302091123302 646 1066 ) ( -5844.7302079667779 726 1066 ) ( -5844.7302796989916 726 1070 ) so_b4b 56 12 270 1 1
( -5844.7302796989916 726 1070 ) ( -5844.7302079667779 726 1066 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5794.7302805078398 710 1026 ) ( -5816.7302804990422 724 1038 ) ( -5808.7302832535743 624 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5838.7302804991104 726 1060 ) ( -5768.7302088192082 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5768.7302088192082 732 1104 ) ( -5838.7302804991104 726 1060 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5838.7302804991104 726 1060 ) ( -5816.7302804990422 724 1038 ) so_b4b -1.26953 126.71 -0 1 1
( -5816.7302804990422 724 1038 ) ( -5832.7301282194012 642 1048 ) ( -5808.7302832535743 624 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5808.7302832535743 624 1026 ) ( -5832.7301282194012 642 1048 ) ( -5832.7304881233285 490 1048 ) so_b4b 67.2695 -120 180 1 1
( -5832.7304881233285 490 1048 ) ( -5832.7301282194012 642 1048 ) ( -5844.7302079667779 726 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5832.7301282194012 642 1048 ) ( -5838.7302804991104 726 1060 ) so_b4b -97.377 98.5979 -0 1 1
( -5838.7302804991104 726 1060 ) ( -5832.7301282194012 642 1048 ) ( -5816.7302804990422 724 1038 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5816.7302804990422 724 1038 ) ( -5794.7302805078398 710 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7304881233285 490 1048 ) ( -5844.7302079667779 726 1066 ) ( -5844.7302091123302 646 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5808.7302832535743 624 1026 ) ( -5832.7304881233285 490 1048 ) ( -5808.7302837141997 492 1026 ) so_b4b 67.2695 -120 180 1 1
( -5808.7302837141997 492 1026 ) ( -5832.7304881233285 490 1048 ) ( -5706.7302802080176 484 1086 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7304881233285 490 1048 ) ( -5832.7302554422868 490 1052 ) ( -5706.7302621135759 484 1090 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5832.7302554422868 490 1052 ) ( -5774.730280496974 488 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5774.730280496974 488 1112 ) ( -5832.7302554422868 490 1052 ) ( -5814.7302804944029 490 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5832.7302554422868 490 1052 ) ( -5840.7302829597875 494 1072 ) so_b4b -1.26953 -0 -0 1 1
( -5840.7302829597875 494 1072 ) ( -5832.7302554422868 490 1052 ) ( -5832.7304881233285 490 1048 ) so_b4b 87.2695 34 180 1 -1
( -5840.7302829597875 494 1072 ) ( -5832.7304881233285 490 1048 ) ( -5840.7302074378586 494 1068 ) so_b4b 87.2695 34 180 1 -1
( -5840.7302074378586 494 1068 ) ( -5832.7304881233285 490 1048 ) ( -5844.7302091123302 646 1066 ) so_b4b -128 -0 -0 1 1
( -5764.7302804806905 494 1026 ) ( -5736.7302804958917 496 1030 ) ( -5734.7302802830618 638 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5702.7302793858989 490 1062 ) ( -5724.7302804988412 496 1038 ) ( -5736.7302804958917 496 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302804958917 496 1030 ) ( -5724.7302804988412 496 1038 ) ( -5734.7302802830618 638 1026 ) so_b4b -1.27002 128 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5698.7302805883301 488 1068 ) ( -5706.7302802080176 484 1086 ) so_b4b -21.27 -56 -0 1 -1
( -5706.7302802080176 484 1086 ) ( -5698.7302805883301 488 1068 ) ( -5808.7302837141997 492 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5808.7302837141997 492 1026 ) ( -5698.7302805883301 488 1068 ) ( -5764.7302804806905 494 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5764.7302804806905 494 1026 ) ( -5698.7302805883301 488 1068 ) ( -5736.7302804958917 496 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302804958917 496 1030 ) ( -5698.7302805883301 488 1068 ) ( -5702.7302793858989 490 1062 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302091123302 646 1066 ) ( -5844.7302808445411 646 1070 ) ( -5840.7302829597875 494 1072 ) so_b4b -0 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5836.7302805003565 642 1090 ) ( -5812.7302805004229 644 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805004229 644 1112 ) ( -5836.7302805003565 642 1090 ) ( -5812.7302805003419 728 1112 ) so_b4b 63.2695 12 180 1 -1
( -5812.7302805003419 728 1112 ) ( -5836.7302805003565 642 1090 ) ( -5834.7302796344165 726 1090 ) so_b4b -15.7554 51.0244 -0 1 1
( -5834.7302796344165 726 1090 ) ( -5836.7302805003565 642 1090 ) ( -5844.7302796989916 726 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302796989916 726 1070 ) ( -5836.7302805003565 642 1090 ) ( -5844.7302808445411 646 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302808445411 646 1070 ) ( -5836.7302805003565 642 1090 ) ( -5840.7302829597875 494 1072 ) so_b4b -0 -0 -0 1 1
( -5840.7302829597875 494 1072 ) ( -5836.7302805003565 642 1090 ) ( -5814.7302804944029 490 1100 ) so_b4b -0 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5812.7302805004229 644 1112 ) ( -5802.7302804990823 490 1108 ) so_b4b -1.27002 128 -0 1 1
( -5802.7302804990823 490 1108 ) ( -5774.730280496974 488 1112 ) ( -5814.7302804944029 490 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5774.730280496974 488 1112 ) ( -5730.73028055137 486 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805004229 644 1112 ) ( -5774.730280496974 488 1112 ) ( -5802.7302804990823 490 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5774.73028677006 726 1026 ) ( -5794.7302805078398 710 1026 ) so_b4b -33.27 6 -0 1 1
( -5844.7302796989916 726 1070 ) ( -5772.7302795053893 732 1112 ) ( -5812.7302805003419 728 1112 ) so_b4b -1.27002 -0 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5730.7302803650427 730 1096 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5748.7302733109655 732 1104 ) ( -5748.7302735130534 732 1108 ) ( -5772.7302795053893 732 1112 ) so_b4b 95.2695 -56 180 1 1
( -5730.7302803650427 730 1096 ) ( -5748.7302733109655 732 1104 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5748.7302733109655 732 1104 ) ( -5768.7302088192082 732 1104 ) so_b4b -1.26953 -0 -0 1 1
( -5712.7302804989204 720 1048 ) ( -5706.7302804991377 722 1076 ) ( -5730.7302803650427 730 1096 ) so_b4b -1.26953 -0 -0 1 1
( -5702.7302804990277 720 1070 ) ( -5706.7302804991377 722 1076 ) ( -5712.7302804989204 720 1048 ) so_b4b -1.26953 -0 -0 1 1
( -5712.7302804989204 720 1048 ) ( -5710.7302804925857 636 1048 ) ( -5698.7302805012459 644 1068 ) so_b4b -0 -0 -0 1 1
( -5698.7302805012459 644 1068 ) ( -5710.7302804925857 636 1048 ) ( -5698.7302805883301 488 1068 ) so_b4b -128 -0 -0 1 1
( -5698.7302805883301 488 1068 ) ( -5710.7302804925857 636 1048 ) ( -5702.7302793858989 490 1062 ) so_b4b -0 -0 -0 1 1
( -5702.7302793858989 490 1062 ) ( -5710.7302804925857 636 1048 ) ( -5724.7302804988412 496 1038 ) so_b4b -0 -0 -0 1 1
( -5724.7302804988412 496 1038 ) ( -5710.7302804925857 636 1048 ) ( -5734.7302802830618 638 1026 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302802830618 638 1026 ) ( -5710.7302804925857 636 1048 ) ( -5734.7302801651795 722 1026 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5710.7302804925857 636 1048 ) ( -5712.7302804989204 720 1048 ) so_b4b -9.75537 -38.9756 -0 1 -1
( -5712.7302804989204 720 1048 ) ( -5698.7302805012459 644 1068 ) ( -5702.7302804990277 720 1070 ) so_b4b -0 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5706.7302805113286 648 1090 ) ( -5698.7302805012459 644 1068 ) so_b4b 88 102 180 1 -1
( -5698.7302805012459 644 1068 ) ( -5706.7302805113286 648 1090 ) ( -5702.7302804990277 720 1070 ) so_b4b -0 -0 -0 1 1
( -5702.7302804990277 720 1070 ) ( -5706.7302805113286 648 1090 ) ( -5706.7302804991377 722 1076 ) so_b4b -0 -0 -0 1 1
( -5706.7302804991377 722 1076 ) ( -5730.730280567378 730 1100 ) ( -5730.7302803650427 730 1096 ) so_b4b 103.27 -56 180 1 1
( -5730.7302803650427 730 1096 ) ( -5730.730280567378 730 1100 ) ( -5748.7302735130534 732 1108 ) so_b4b 99.2695 -56 180 1 1
( -5730.7302804970022 574 1112 ) ( -5737.730280499567 649 1112 ) ( -5706.7302805113286 648 1090 ) so_b4b -41.27 -126 -0 1 -1
( -5706.7302805113286 648 1090 ) ( -5737.730280499567 649 1112 ) ( -5730.730280567378 730 1100 ) so_b4b -1.27002 -0 -0 1 1
( -5730.730280567378 730 1100 ) ( -5737.730280499567 649 1112 ) ( -5748.7302735130534 732 1108 ) so_b4b -1.27002 -0 -0 1 1
( -5748.7302735130534 732 1108 ) ( -5737.730280499567 649 1112 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.27002 -0 -0 1 1
( -5772.7302795053893 732 1112 ) ( -5737.730280499567 649 1112 ) ( -5730.7302804970022 574 1112 ) so_b4b -37.27 6 -0 1 1
}
)";

            IO::TestParserStatus status;

            ASSERT_NO_THROW(IO::NodeReader::read(data, world, worldBounds, status));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2491", "[BrushNodeTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2491

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::string data = R"(
            {
                ( -179 -179 -63 ) ( -158 -158 -69 ) ( 1.055125500745701e+154 1.0551255007456758e+154 -5.2756275037285048e+153 ) _core/tangerine -2.82843 -0 -0 0.0625 0.0625
                ( -132 -126.3431457505086 -60 ) ( -132 188 -60 ) ( -132 -126.34314575050865 -64 ) _core/tangerine 0 0 0 0.0625 0.0625
                ( -188 188 -60 ) ( -188 -182.34314575050769 -60 ) ( -188 188 -64 ) _core/tangerine 0 0 0 0.0625 0.0625
                ( -132 192 -60 ) ( -188 192 -60 ) ( -132 192 -64 ) _core/tangerine -0 -0 -0 0.0625 0.0625
                ( -188 188 -60 ) ( -132 188 -60 ) ( -188 -182.34314575050769 -60 ) _core/tangerine 32 -112 -0 0.0625 0.0625
                ( -132 188 -64 ) ( -188 188 -64 ) ( -132 -126.34314575050865 -64 ) _core/tangerine 32 -112 -0 0.0625 0.0625
            }
            )";

            IO::TestParserStatus status;

            ASSERT_NO_THROW(IO::NodeReader::read(data, world, worldBounds, status));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2686", "[BrushNodeTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2686

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Valve);

            const std::string data = R"(
{
( -751 -623.07933525052886 159.27097151882481 ) ( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752.04220703008457 -655.64774857459861 45.762340887734425 ) wood1_1 [ -0.499995 -0.836519 0.224145 8.59912 ] [ -0.0094517 -0.253533 -0.967281 -43.4648 ] 346.992 1 1
( -746.54446646023075 -654.12665614912589 45.762340832676934 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -746.90192378883967 -622.0185651831514 158.98673884436587 ) wood1_1 [ -0.866028 0.482959 -0.129408 -4.96463 ] [ 0.00536862 -0.249822 -0.968277 -43.5033 ] 7.53177 1 1
( -746.90192378883967 -622.0185651831514 158.98673884436587 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -745 -623.0792133033973 159.27093866053934 ) wood1_1 [ -0.866028 0.482959 -0.129408 -4.96463 ] [ 0.00536862 -0.249822 -0.968277 -43.5033 ] 7.53177 1 1
( -745 -623.0792133033973 159.27093866053934 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -0.499995 0.836519 -0.224145 -8.59909 ] [ 0.00925779 -0.253641 -0.967254 -43.4641 ] 13.0082 1 1
( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752 -627.20176933038158 160.37557439373654 ) ( -753.40030222000973 -661.5816915717096 45.76234097597262 ) wood1_1 [ 0 -0.965926 0.258819 9.92938 ] [ -0.0106727 -0.258804 -0.965871 -43.4111 ] 345 1 1
( -753.40030222000973 -661.5816915717096 45.76234097597262 ) ( -752 -627.20176933038158 160.37557439373654 ) ( -751 -628.8747682432919 160.82385299770002 ) wood1_1 [ 0.500008 -0.836512 0.224143 8.59901 ] [ -0.0094517 -0.264075 -0.964456 -43.3565 ] 346.992 1 1
( -743.90192378794575 -624.91635477344664 159.76319924922808 ) ( -745 -623.0792133033973 159.27093866053934 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -0.499995 0.836519 -0.224145 -8.59909 ] [ 0.00925779 -0.253641 -0.967254 -43.4641 ] 13.0082 1 1
( -752.04220703008457 -655.64774857459861 45.762340887734425 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -751 -623.07933525052886 159.27097151882481 ) wood1_1 [ -0.866028 -0.482959 0.129408 4.96466 ] [ -0.00543319 -0.249714 -0.968304 -43.5042 ] 352.468 1 1
( -751 -623.07933525052886 159.27097151882481 ) ( -752 -624.75226938818867 159.71923270135312 ) ( -753.39055027600557 -658.05150554216561 45.762341114124865 ) wood1_1 [ -0.499995 -0.836519 0.224145 8.59912 ] [ -0.0094517 -0.253533 -0.967281 -43.4648 ] 346.992 1 1
( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752 -624.75226938818867 159.71923270135312 ) ( -752 -627.20176933038158 160.37557439373654 ) wood1_1 [ 0 -0.965926 0.258819 9.92938 ] [ -0.0106727 -0.258804 -0.965871 -43.4111 ] 345 1 1
( -746.90207063287346 -629.93555546737525 161.10809006388723 ) ( -745 -628.87474788883753 160.82384752100626 ) ( -743.97456390268746 -664 45.762340974536315 ) wood1_1 [ 0.866016 0.482978 -0.129414 -4.96484 ] [ 0.00536892 -0.267786 -0.963463 -43.3186 ] 7.53207 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -745 -628.87474788883753 160.82384752100626 ) ( -746.90207063287346 -629.93555546737525 161.10809006388723 ) wood1_1 [ 1 0 0 -0 ] [ 0 -0.965926 0.258819 9.92938 ] -0 1 1
( -751 -628.8747682432919 160.82385299770002 ) ( -749.09792934966106 -629.93555547773678 161.10809006665528 ) ( -752.05952711953228 -664 45.762340944544121 ) wood1_1 [ 0.866016 -0.482978 0.129414 4.96484 ] [ -0.00543343 -0.267894 -0.963433 -43.3173 ] 352.468 1 1
( -752.05952711953228 -664 45.762340944544121 ) ( -749.09792934966106 -629.93555547773678 161.10809006665528 ) ( -749.49773869956948 -665.53645570829394 45.762340998099269 ) wood1_1 [ 0.866016 -0.482978 0.129414 4.96484 ] [ -0.00543343 -0.267894 -0.963433 -43.3173 ] 352.468 1 1
( -746.90192378883967 -622.0185651831514 158.98673884436587 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -749.4887863191035 -654.12665614891398 45.762340833436674 ) wood1_1 [ -1 0 0 -0 ] [ 0 -0.24837 -0.968665 -43.5181 ] -0 1 1
( -749.4887863191035 -654.12665614891398 45.762340833436674 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -752.04220703008457 -655.64774857459861 45.762340887734425 ) wood1_1 [ -0.866028 -0.482959 0.129408 4.96466 ] [ -0.00543319 -0.249714 -0.968304 -43.5042 ] 352.468 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -743.90192378794575 -624.91635477344664 159.76319924922808 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ 0 0.965926 -0.258819 -9.92938 ] [ 0.0106727 -0.258804 -0.965871 -43.4111 ] 15 1 1
( -751 -628.8747682432919 160.82385299770002 ) ( -752.05952711953228 -664 45.762340944544121 ) ( -753.40030222000973 -661.5816915717096 45.76234097597262 ) wood1_1 [ 0.500008 -0.836512 0.224143 8.59901 ] [ -0.0094517 -0.264075 -0.964456 -43.3565 ] 346.992 1 1
( -743.97456390268746 -664 45.762340974536315 ) ( -746.53638375403534 -665.53645569340722 45.762340997376214 ) ( -746.90207063287346 -629.93555546737525 161.10809006388723 ) wood1_1 [ 0.866016 0.482978 -0.129414 -4.96484 ] [ 0.00536892 -0.267786 -0.963463 -43.3186 ] 7.53207 1 1
( -746.90207063287346 -629.93555546737525 161.10809006388723 ) ( -746.53638375403534 -665.53645569340722 45.762340997376214 ) ( -749.49773869956948 -665.53645570829394 45.762340998099269 ) wood1_1 [ 1 0 0 -0 ] [ 0 -0.269238 -0.963074 -43.3036 ] -0 1 1
( -742.51072427503652 -658.28759504188008 45.762340891699573 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -743.90192378813185 -627.03768398273758 160.33160771552403 ) wood1_1 [ 0 0.965926 -0.258819 -9.92938 ] [ 0.0106727 -0.258804 -0.965871 -43.4111 ] 15 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -745 -628.87474788883753 160.82384752100626 ) wood1_1 [ 0.499998 0.836517 -0.224144 -8.59906 ] [ 0.00925781 -0.263967 -0.964487 -43.358 ] 13.0082 1 1
( -745 -628.87474788883753 160.82384752100626 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -743.97456390268746 -664 45.762340974536315 ) wood1_1 [ 0.499998 0.836517 -0.224144 -8.59906 ] [ 0.00925781 -0.263967 -0.964487 -43.358 ] 13.0082 1 1
( -743.97456390268746 -664 45.762340974536315 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -1 0 0 -0 ] [ 0 -1 0 9.92938 ] -0 1 1
}
            )";

            IO::TestParserStatus status;

            ASSERT_NO_THROW(IO::NodeReader::read(data, world, worldBounds, status));
        }
    }
}

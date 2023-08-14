/*
 Copyright (C) 2021 Kristian Duske

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

#include "Assets/Texture.h"
#include "Error.h"
#include "IO/ExportOptions.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "Logger.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <fmt/format.h>

#include <memory>
#include <optional>
#include <sstream>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("ObjSerializer.writeBrush")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Quake3};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode =
    new Model::BrushNode{builder.createCube(64.0, "some_texture").value()};
  map.defaultLayer()->addChild(brushNode);

  auto objStream = std::ostringstream{};
  auto mtlStream = std::ostringstream{};
  const auto mtlFilename = "some_file_name.mtl";
  const auto objOptions =
    ObjExportOptions{"/some/export/path.obj", ObjMtlPathMode::RelativeToGamePath};

  auto writer = NodeWriter{
    map, std::make_unique<ObjSerializer>(objStream, mtlStream, mtlFilename, objOptions)};
  writer.writeMap();

  CHECK(objStream.str() == R"(mtllib some_file_name.mtl
# vertices
v -32 -32 -32
v -32 -32 32
v -32 32 32
v -32 32 -32
v 32 32 32
v 32 -32 32
v 32 -32 -32
v 32 32 -32

# texture coordinates
vt 32 -32
vt -32 -32
vt -32 32
vt 32 32

# normals
vn -1 0 -0
vn 0 0 1
vn 0 -1 -0
vn 0 1 -0
vn 0 0 -1
vn 1 0 -0

o entity0_brush0
usemtl some_texture
f  1/1/1  2/2/1  3/3/1  4/4/1
usemtl some_texture
f  5/4/2  3/3/2  2/2/2  6/1/2
usemtl some_texture
f  6/1/3  2/2/3  1/3/3  7/4/3
usemtl some_texture
f  8/4/4  4/3/4  3/2/4  5/1/4
usemtl some_texture
f  7/1/5  1/2/5  4/3/5  8/4/5
usemtl some_texture
f  8/4/6  5/3/6  6/2/6  7/1/6

)");

  CHECK(mtlStream.str() == R"(newmtl some_texture

)");
}

TEST_CASE("ObjSerializer.writePatch")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Quake3};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* patchNode = new Model::PatchNode{Model::BezierPatch{
    3,
    3,
    {{0, 0, 0},
     {1, 0, 1},
     {2, 0, 0},
     {0, 1, 1},
     {1, 1, 2},
     {2, 1, 1},
     {0, 2, 0},
     {1, 2, 1},
     {2, 2, 0}},
    "some_texture"}};
  map.defaultLayer()->addChild(patchNode);

  auto objStream = std::ostringstream{};
  auto mtlStream = std::ostringstream{};
  const auto mtlFilename = "some_file_name.mtl";
  const auto objOptions =
    ObjExportOptions{"/some/export/path.obj", ObjMtlPathMode::RelativeToGamePath};

  auto writer = NodeWriter{
    map, std::make_unique<ObjSerializer>(objStream, mtlStream, mtlFilename, objOptions)};
  writer.writeMap();

  CHECK(objStream.str() == R"(mtllib some_file_name.mtl
# vertices
v 0 0 -0
v 0 0.21875 -0.25
v 0.25 0.4375 -0.25
v 0.25 0.21875 -0
v 0.5 0.59375 -0.25
v 0.5 0.375 -0
v 0.75 0.6875 -0.25
v 0.75 0.46875 -0
v 1 0.71875 -0.25
v 1 0.5 -0
v 1.25 0.6875 -0.25
v 1.25 0.46875 -0
v 1.5 0.59375 -0.25
v 1.5 0.375 -0
v 1.75 0.4375 -0.25
v 1.75 0.21875 -0
v 2 0.21875 -0.25
v 2 0 -0
v 0 0.375 -0.5
v 0.25 0.59375 -0.5
v 0.5 0.75 -0.5
v 0.75 0.84375 -0.5
v 1 0.875 -0.5
v 1.25 0.84375 -0.5
v 1.5 0.75 -0.5
v 1.75 0.59375 -0.5
v 2 0.375 -0.5
v 0 0.46875 -0.75
v 0.25 0.6875 -0.75
v 0.5 0.84375 -0.75
v 0.75 0.9375 -0.75
v 1 0.96875 -0.75
v 1.25 0.9375 -0.75
v 1.5 0.84375 -0.75
v 1.75 0.6875 -0.75
v 2 0.46875 -0.75
v 0 0.5 -1
v 0.25 0.71875 -1
v 0.5 0.875 -1
v 0.75 0.96875 -1
v 1 1 -1
v 1.25 0.96875 -1
v 1.5 0.875 -1
v 1.75 0.71875 -1
v 2 0.5 -1
v 0 0.46875 -1.25
v 0.25 0.6875 -1.25
v 0.5 0.84375 -1.25
v 0.75 0.9375 -1.25
v 1 0.96875 -1.25
v 1.25 0.9375 -1.25
v 1.5 0.84375 -1.25
v 1.75 0.6875 -1.25
v 2 0.46875 -1.25
v 0 0.375 -1.5
v 0.25 0.59375 -1.5
v 0.5 0.75 -1.5
v 0.75 0.84375 -1.5
v 1 0.875 -1.5
v 1.25 0.84375 -1.5
v 1.5 0.75 -1.5
v 1.75 0.59375 -1.5
v 2 0.375 -1.5
v 0 0.21875 -1.75
v 0.25 0.4375 -1.75
v 0.5 0.59375 -1.75
v 0.75 0.6875 -1.75
v 1 0.71875 -1.75
v 1.25 0.6875 -1.75
v 1.5 0.59375 -1.75
v 1.75 0.4375 -1.75
v 2 0.21875 -1.75
v 0 0 -2
v 0.25 0.21875 -2
v 0.5 0.375 -2
v 0.75 0.46875 -2
v 1 0.5 -2
v 1.25 0.46875 -2
v 1.5 0.375 -2
v 1.75 0.21875 -2
v 2 0 -2

# texture coordinates
vt 0 -0

# normals
vn 0.5499719409228703 -0.6285393610547089 -0.5499719409228703
vn 0.5734623443633283 -0.6553855364152325 -0.4915391523114243
vn 0.5144957554275265 -0.6859943405700353 -0.5144957554275265
vn 0.4915391523114243 -0.6553855364152325 -0.5734623443633283
vn 0.3713906763541037 -0.7427813527082074 -0.5570860145311556
vn 0.35218036253024954 -0.7043607250604991 -0.6163156344279367
vn 0.19611613513818404 -0.7844645405527362 -0.5883484054145521
vn 0.1849000654084097 -0.7396002616336388 -0.647150228929434
vn 0 -0.8 -0.6
vn 0 -0.7525766947068778 -0.658504607868518
vn -0.19611613513818404 -0.7844645405527362 -0.5883484054145521
vn -0.1849000654084097 -0.7396002616336388 -0.647150228929434
vn -0.3713906763541037 -0.7427813527082074 -0.5570860145311556
vn -0.35218036253024954 -0.7043607250604991 -0.6163156344279367
vn -0.5144957554275265 -0.6859943405700353 -0.5144957554275265
vn -0.4915391523114243 -0.6553855364152325 -0.5734623443633283
vn -0.5734623443633283 -0.6553855364152325 -0.4915391523114243
vn -0.5499719409228703 -0.6285393610547089 -0.5499719409228703
vn 0.6163156344279367 -0.7043607250604991 -0.35218036253024954
vn 0.5570860145311556 -0.7427813527082074 -0.3713906763541037
vn 0.4082482904638631 -0.8164965809277261 -0.4082482904638631
vn 0.2182178902359924 -0.8728715609439696 -0.4364357804719848
vn 0 -0.8944271909999159 -0.4472135954999579
vn -0.2182178902359924 -0.8728715609439696 -0.4364357804719848
vn -0.4082482904638631 -0.8164965809277261 -0.4082482904638631
vn -0.5570860145311556 -0.7427813527082074 -0.3713906763541037
vn -0.6163156344279367 -0.7043607250604991 -0.35218036253024954
vn 0.647150228929434 -0.7396002616336388 -0.1849000654084097
vn 0.5883484054145521 -0.7844645405527362 -0.19611613513818404
vn 0.4364357804719848 -0.8728715609439696 -0.2182178902359924
vn 0.23570226039551587 -0.9428090415820635 -0.23570226039551587
vn 0 -0.9701425001453319 -0.24253562503633297
vn -0.23570226039551587 -0.9428090415820635 -0.23570226039551587
vn -0.4364357804719848 -0.8728715609439696 -0.2182178902359924
vn -0.5883484054145521 -0.7844645405527362 -0.19611613513818404
vn -0.647150228929434 -0.7396002616336388 -0.1849000654084097
vn 0.658504607868518 -0.7525766947068778 -0
vn 0.6 -0.8 -0
vn 0.4472135954999579 -0.8944271909999159 -0
vn 0.24253562503633297 -0.9701425001453319 -0
vn 0 -1 -0
vn -0.24253562503633297 -0.9701425001453319 -0
vn -0.4472135954999579 -0.8944271909999159 -0
vn -0.6 -0.8 -0
vn -0.658504607868518 -0.7525766947068778 -0
vn 0.647150228929434 -0.7396002616336388 0.1849000654084097
vn 0.5883484054145521 -0.7844645405527362 0.19611613513818404
vn 0.4364357804719848 -0.8728715609439696 0.2182178902359924
vn 0.23570226039551587 -0.9428090415820635 0.23570226039551587
vn 0 -0.9701425001453319 0.24253562503633297
vn -0.23570226039551587 -0.9428090415820635 0.23570226039551587
vn -0.4364357804719848 -0.8728715609439696 0.2182178902359924
vn -0.5883484054145521 -0.7844645405527362 0.19611613513818404
vn -0.647150228929434 -0.7396002616336388 0.1849000654084097
vn 0.6163156344279367 -0.7043607250604991 0.35218036253024954
vn 0.5570860145311556 -0.7427813527082074 0.3713906763541037
vn 0.4082482904638631 -0.8164965809277261 0.4082482904638631
vn 0.2182178902359924 -0.8728715609439696 0.4364357804719848
vn 0 -0.8944271909999159 0.4472135954999579
vn -0.2182178902359924 -0.8728715609439696 0.4364357804719848
vn -0.4082482904638631 -0.8164965809277261 0.4082482904638631
vn -0.5570860145311556 -0.7427813527082074 0.3713906763541037
vn -0.6163156344279367 -0.7043607250604991 0.35218036253024954
vn 0.5734623443633283 -0.6553855364152325 0.4915391523114243
vn 0.5144957554275265 -0.6859943405700353 0.5144957554275265
vn 0.3713906763541037 -0.7427813527082074 0.5570860145311556
vn 0.19611613513818404 -0.7844645405527362 0.5883484054145521
vn 0 -0.8 0.6
vn -0.19611613513818404 -0.7844645405527362 0.5883484054145521
vn -0.3713906763541037 -0.7427813527082074 0.5570860145311556
vn -0.5144957554275265 -0.6859943405700353 0.5144957554275265
vn -0.5734623443633283 -0.6553855364152325 0.4915391523114243
vn 0.5499719409228703 -0.6285393610547089 0.5499719409228703
vn 0.4915391523114243 -0.6553855364152325 0.5734623443633283
vn 0.35218036253024954 -0.7043607250604991 0.6163156344279367
vn 0.1849000654084097 -0.7396002616336388 0.647150228929434
vn 0 -0.7525766947068778 0.658504607868518
vn -0.1849000654084097 -0.7396002616336388 0.647150228929434
vn -0.35218036253024954 -0.7043607250604991 0.6163156344279367
vn -0.4915391523114243 -0.6553855364152325 0.5734623443633283
vn -0.5499719409228703 -0.6285393610547089 0.5499719409228703

o entity0_patch0
usemtl some_texture
f  1/1/1  2/1/2  3/1/3  4/1/4
f  4/1/4  3/1/3  5/1/5  6/1/6
f  6/1/6  5/1/5  7/1/7  8/1/8
f  8/1/8  7/1/7  9/1/9  10/1/10
f  10/1/10  9/1/9  11/1/11  12/1/12
f  12/1/12  11/1/11  13/1/13  14/1/14
f  14/1/14  13/1/13  15/1/15  16/1/16
f  16/1/16  15/1/15  17/1/17  18/1/18
f  2/1/2  19/1/19  20/1/20  3/1/3
f  3/1/3  20/1/20  21/1/21  5/1/5
f  5/1/5  21/1/21  22/1/22  7/1/7
f  7/1/7  22/1/22  23/1/23  9/1/9
f  9/1/9  23/1/23  24/1/24  11/1/11
f  11/1/11  24/1/24  25/1/25  13/1/13
f  13/1/13  25/1/25  26/1/26  15/1/15
f  15/1/15  26/1/26  27/1/27  17/1/17
f  19/1/19  28/1/28  29/1/29  20/1/20
f  20/1/20  29/1/29  30/1/30  21/1/21
f  21/1/21  30/1/30  31/1/31  22/1/22
f  22/1/22  31/1/31  32/1/32  23/1/23
f  23/1/23  32/1/32  33/1/33  24/1/24
f  24/1/24  33/1/33  34/1/34  25/1/25
f  25/1/25  34/1/34  35/1/35  26/1/26
f  26/1/26  35/1/35  36/1/36  27/1/27
f  28/1/28  37/1/37  38/1/38  29/1/29
f  29/1/29  38/1/38  39/1/39  30/1/30
f  30/1/30  39/1/39  40/1/40  31/1/31
f  31/1/31  40/1/40  41/1/41  32/1/32
f  32/1/32  41/1/41  42/1/42  33/1/33
f  33/1/33  42/1/42  43/1/43  34/1/34
f  34/1/34  43/1/43  44/1/44  35/1/35
f  35/1/35  44/1/44  45/1/45  36/1/36
f  37/1/37  46/1/46  47/1/47  38/1/38
f  38/1/38  47/1/47  48/1/48  39/1/39
f  39/1/39  48/1/48  49/1/49  40/1/40
f  40/1/40  49/1/49  50/1/50  41/1/41
f  41/1/41  50/1/50  51/1/51  42/1/42
f  42/1/42  51/1/51  52/1/52  43/1/43
f  43/1/43  52/1/52  53/1/53  44/1/44
f  44/1/44  53/1/53  54/1/54  45/1/45
f  46/1/46  55/1/55  56/1/56  47/1/47
f  47/1/47  56/1/56  57/1/57  48/1/48
f  48/1/48  57/1/57  58/1/58  49/1/49
f  49/1/49  58/1/58  59/1/59  50/1/50
f  50/1/50  59/1/59  60/1/60  51/1/51
f  51/1/51  60/1/60  61/1/61  52/1/52
f  52/1/52  61/1/61  62/1/62  53/1/53
f  53/1/53  62/1/62  63/1/63  54/1/54
f  55/1/55  64/1/64  65/1/65  56/1/56
f  56/1/56  65/1/65  66/1/66  57/1/57
f  57/1/57  66/1/66  67/1/67  58/1/58
f  58/1/58  67/1/67  68/1/68  59/1/59
f  59/1/59  68/1/68  69/1/69  60/1/60
f  60/1/60  69/1/69  70/1/70  61/1/61
f  61/1/61  70/1/70  71/1/71  62/1/62
f  62/1/62  71/1/71  72/1/72  63/1/63
f  64/1/64  73/1/73  74/1/74  65/1/65
f  65/1/65  74/1/74  75/1/75  66/1/66
f  66/1/66  75/1/75  76/1/76  67/1/67
f  67/1/67  76/1/76  77/1/77  68/1/68
f  68/1/68  77/1/77  78/1/78  69/1/69
f  69/1/69  78/1/78  79/1/79  70/1/70
f  70/1/70  79/1/79  80/1/80  71/1/71
f  71/1/71  80/1/80  81/1/81  72/1/72

)");

  CHECK(mtlStream.str() == R"(newmtl some_texture

)");
}

TEST_CASE("ObjSerializer.writeRelativeMaterialPath")
{
  const auto worldBounds = vm::bbox3{8192.0};

  // must outlive map
  auto texture = Assets::Texture{"some_texture", 16, 16};
  texture.setRelativePath("textures/some_texture.png");

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Quake3};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode =
    new Model::BrushNode{builder.createCube(64.0, "some_texture").value()};
  map.defaultLayer()->addChild(brushNode);

  for (size_t i = 0; i < brushNode->brush().faceCount(); ++i)
  {
    brushNode->setFaceTexture(i, &texture);
  }

  auto objStream = std::ostringstream{};
  auto mtlStream = std::ostringstream{};
  const auto mtlName = "some_mtl_file.mtl";

  using T = std::tuple<ObjExportOptions, std::string, std::optional<std::string>>;

  const auto [options, textureAbsolutePath, expectedPath] = GENERATE(values<T>({
    {{"/home/that_guy/quake/export/file.obj", ObjMtlPathMode::RelativeToExportPath},
     "/home/that_guy/quake/textures/some_texture.png",
     "../textures/some_texture.png"},
    {{"/home/that_guy/quake/export/file.obj", ObjMtlPathMode::RelativeToExportPath},
     "",
     std::nullopt},
    {{"/home/that_guy/quake/export/file.obj", ObjMtlPathMode::RelativeToGamePath},
     "/home/that_guy/quake/textures/some_texture.png",
     "textures/some_texture.png"},
  }));

  CAPTURE(options, textureAbsolutePath);

  texture.setAbsolutePath(textureAbsolutePath);

  auto writer = NodeWriter{
    map, std::make_unique<ObjSerializer>(objStream, mtlStream, mtlName, options)};
  writer.writeMap();

  const auto expectedMtl = expectedPath ? fmt::format(
                             R"(newmtl some_texture
map_Kd {}

)",
                             *expectedPath)
                                        : R"(newmtl some_texture

)";

  CHECK(mtlStream.str() == expectedMtl);
}
} // namespace IO
} // namespace TrenchBroom

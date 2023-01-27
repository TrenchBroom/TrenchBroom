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

#include "IO/DiskIO.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"

#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("WadFileSystemTest.loadEntries")
{
  const Path wadPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Wad/cr8_czg.wad");
  NullLogger logger;
  WadFileSystem fs(wadPath, logger);

  CHECK_THAT(
    fs.findItems(Path("")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("blowjob_machine.D"), Path("bongs2.D"),        Path("can-o-jam.D"),
      Path("cap4can-o-jam.D"),   Path("coffin1.D"),       Path("coffin2.D"),
      Path("cr8_czg_1.D"),       Path("cr8_czg_2.D"),     Path("cr8_czg_3.D"),
      Path("cr8_czg_4.D"),       Path("cr8_czg_5.D"),     Path("crackpipes.D"),
      Path("czg_backhole.D"),    Path("czg_fronthole.D"), Path("dex_5.D"),
      Path("eat_me.D"),          Path("for_sux-m-ass.D"), Path("lasthopeofhuman.D"),
      Path("polished_turd.D"),   Path("speedM_1.D"),      Path("u_get_this.D"),
    }));
}
} // namespace IO
} // namespace TrenchBroom

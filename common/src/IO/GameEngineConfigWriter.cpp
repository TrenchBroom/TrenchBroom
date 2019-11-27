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

#include "GameEngineConfigWriter.h"

#include "EL/Value.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

#include <ostream>

namespace TrenchBroom {
    namespace IO {
        GameEngineConfigWriter::GameEngineConfigWriter(const Model::GameEngineConfig& config, std::ostream& stream) :
        m_config(config),
        m_stream(stream) {
            assert(!m_stream.bad());
        }

        void GameEngineConfigWriter::writeConfig() {
            EL::MapType map;
            map["version"] = EL::Value(1.0);
            map["profiles"] = writeProfiles(m_config);
            m_stream << EL::Value(map) << "\n";
        }

        EL::Value GameEngineConfigWriter::writeProfiles(const Model::GameEngineConfig& config) const {
            EL::ArrayType array;
            for (size_t i = 0; i < config.profileCount(); ++i) {
                const Model::GameEngineProfile* profile = config.profile(i);
                array.push_back(writeProfile(profile));
            }

            return EL::Value(array);
        }

        EL::Value GameEngineConfigWriter::writeProfile(const Model::GameEngineProfile* profile) const {
            EL::MapType map;
            map["name"] = EL::Value(profile->name());
            map["path"] = EL::Value(profile->path().asString());
            map["parameters"] = EL::Value(profile->parameterSpec());
            return EL::Value(map);
        }
    }
}

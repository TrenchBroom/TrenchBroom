/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"

namespace TrenchBroom {
    namespace IO {
        GameEngineConfigWriter::GameEngineConfigWriter(const Model::GameEngineConfig& config, std::ostream& stream) :
        m_config(config),
        m_stream(stream) {
            assert(!m_stream.bad());
        }
        
        void GameEngineConfigWriter::writeConfig() {
            ConfigTable table;
            table.addEntry("version", new ConfigValue("1"));
            table.addEntry("profiles", writeProfiles(m_config));
            table.appendToStream(m_stream);
        }

        ConfigList* GameEngineConfigWriter::writeProfiles(const Model::GameEngineConfig& config) const {
            ConfigList* result = new ConfigList();
            for (size_t i = 0; i < config.profileCount(); ++i) {
                const Model::GameEngineProfile* profile = config.profile(i);
                result->addEntry(writeProfile(profile));
            }
            
            return result;
        }
        
        ConfigTable* GameEngineConfigWriter::writeProfile(const Model::GameEngineProfile* profile) const {
            ConfigTable* result = new ConfigTable();
            result->addEntry("name", new ConfigValue(profile->name()));
            result->addEntry("path", new ConfigValue(profile->path().asString()));
            return result;
        }
    }
}

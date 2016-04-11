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

#include "CompilationConfigWriter.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        CompilationConfigWriter::CompilationConfigWriter(const Model::CompilationConfig& config, std::ostream& stream) :
        m_config(config),
        m_stream(stream) {
            assert(!m_stream.bad());
        }
        
        void CompilationConfigWriter::writeConfig() {
            ConfigTable table;
            table.addEntry("version", new ConfigValue("1"));
            table.addEntry("profiles", writeProfiles(m_config));
            table.appendToStream(m_stream);
        }

        ConfigList* CompilationConfigWriter::writeProfiles(const Model::CompilationConfig& config) const {
            ConfigList* result = new ConfigList();
            for (size_t i = 0; i < config.profileCount(); ++i) {
                const Model::CompilationProfile* profile = config.profile(i);
                result->addEntry(writeProfile(profile));
            }
            
            return result;
        }

        ConfigTable* CompilationConfigWriter::writeProfile(const Model::CompilationProfile* profile) const {
            ConfigTable* result = new ConfigTable();
            result->addEntry("name", new ConfigValue(profile->name()));
            result->addEntry("tasks", writeTasks(profile));
            return result;
        }

        class CompilationConfigWriter::WriteCompilationTaskVisitor : public Model::ConstCompilationTaskVisitor {
        private:
            ConfigList* m_result;
        public:
            WriteCompilationTaskVisitor(ConfigList* result) : m_result(result) {}
        public:
            void visit(const Model::CompilationCopyFiles* task) {
                ConfigTable* table = new ConfigTable();
                table->addEntry("type", new ConfigValue("copy"));
                table->addEntry("source", new ConfigValue(task->sourceSpec()));
                table->addEntry("target", new ConfigValue(task->targetSpec()));
                m_result->addEntry(table);
            }
            
            void visit(const Model::CompilationRunTool* task) {
                ConfigTable* table = new ConfigTable();
                table->addEntry("type", new ConfigValue("tool"));
                table->addEntry("tool", new ConfigValue(task->toolSpec()));
                table->addEntry("parameters", new ConfigValue(task->parameterSpec()));
                m_result->addEntry(table);
            }
        };

        ConfigList* CompilationConfigWriter::writeTasks(const Model::CompilationProfile* profile) const {
            ConfigList* result = new ConfigList();
            WriteCompilationTaskVisitor visitor(result);
            profile->accept(visitor);
            return result;
        }
    }
}

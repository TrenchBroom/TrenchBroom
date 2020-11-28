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

#include "MapFileSerializer.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "Macros.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityAttributes.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <fmt/format.h>

#include <iterator> // for std::ostreambuf_iterator
#include <memory>
#include <functional>
#include <utility> // for std::pair
#include <vector>
#include <sstream>
#include <algorithm>

#include <thread>
#include <future>
#include <atomic>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QThread>

namespace TrenchBroom {
    namespace IO {
        class QuakeFileSerializer : public MapFileSerializer {
        public:
            explicit QuakeFileSerializer(std::ostream& stream) :
            MapFileSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);
                fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
            }
        protected:
            void writeFacePoints(std::ostream& stream, const Model::BrushFace& face) const {
                const Model::BrushFace::Points& points = face.points();

                fmt::format_to(std::ostreambuf_iterator<char>(stream), "( {} {} {} ) ( {} {} {} ) ( {} {} {} )",
                               points[0].x(),
                               points[0].y(),
                               points[0].z(),
                               points[1].x(),
                               points[1].y(),
                               points[1].z(),
                               points[2].x(),
                               points[2].y(),
                               points[2].z());
            }

            void writeTextureInfo(std::ostream& stream, const Model::BrushFace& face) const {
                const std::string& textureName = face.attributes().textureName().empty() ? Model::BrushFaceAttributes::NoTextureName : face.attributes().textureName();

                fmt::format_to(std::ostreambuf_iterator<char>(stream), " {} {} {} {} {} {}",
                               textureName,
                               face.attributes().xOffset(),
                               face.attributes().yOffset(),
                               face.attributes().rotation(),
                               face.attributes().xScale(),
                               face.attributes().yScale());
            }

            void writeValveTextureInfo(std::ostream& stream, const Model::BrushFace& face) const {
                const std::string& textureName = face.attributes().textureName().empty() ? Model::BrushFaceAttributes::NoTextureName : face.attributes().textureName();
                const vm::vec3 xAxis = face.textureXAxis();
                const vm::vec3 yAxis = face.textureYAxis();

                fmt::format_to(std::ostreambuf_iterator<char>(stream), " {} [ {} {} {} {} ] [ {} {} {} {} ] {} {} {}",
                               textureName,

                               xAxis.x(),
                               xAxis.y(),
                               xAxis.z(),
                               face.attributes().xOffset(),

                               yAxis.x(),
                               yAxis.y(),
                               yAxis.z(),
                               face.attributes().yOffset(),

                               face.attributes().rotation(),
                               face.attributes().xScale(),
                               face.attributes().yScale());
            }
        };

        class Quake2FileSerializer : public QuakeFileSerializer {
        public:
            explicit Quake2FileSerializer(std::ostream& stream) :
            QuakeFileSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);

                // Neverball's "mapc" doesn't like it if surface attributes aren't present.
                // This suggests the Radiants always output these, so it's probably a compatibility danger.
                writeSurfaceAttributes(stream, face);

                fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
            }
        protected:
            void writeSurfaceAttributes(std::ostream& stream, const Model::BrushFace& face) const {
                fmt::format_to(std::ostreambuf_iterator<char>(stream), " {} {} {}",
                               face.attributes().surfaceContents(),
                               face.attributes().surfaceFlags(),
                               face.attributes().surfaceValue());
            }
        };

        class Quake2ValveFileSerializer : public Quake2FileSerializer {
        public:
            explicit Quake2ValveFileSerializer(std::ostream& stream) :
            Quake2FileSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeValveTextureInfo(stream, face);
                writeSurfaceAttributes(stream, face);

                fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
            }
        };

        class DaikatanaFileSerializer : public Quake2FileSerializer {
        private:
            std::string SurfaceColorFormat;
        public:
            explicit DaikatanaFileSerializer(std::ostream& stream) :
            Quake2FileSerializer(stream),
            SurfaceColorFormat(" %d %d %d") {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);

                if (face.attributes().hasSurfaceAttributes() || face.attributes().hasColor()) {
                    writeSurfaceAttributes(stream, face);
                }
                if (face.attributes().hasColor()) {
                    writeSurfaceColor(stream, face);
                }

                fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
            }
        protected:
            void writeSurfaceColor(std::ostream& stream, const Model::BrushFace& face) const {
                fmt::format_to(std::ostreambuf_iterator<char>(stream), " {} {} {}",
                               static_cast<int>(face.attributes().color().r()),
                               static_cast<int>(face.attributes().color().g()),
                               static_cast<int>(face.attributes().color().b()));
            }
        };

        class Hexen2FileSerializer : public QuakeFileSerializer {
        public:
            explicit Hexen2FileSerializer(std::ostream& stream):
            QuakeFileSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);
                fmt::format_to(std::ostreambuf_iterator<char>(stream), " 0\n"); // extra value written here
            }
        };

        class ValveFileSerializer : public QuakeFileSerializer {
        public:
            explicit ValveFileSerializer(std::ostream& stream) :
            QuakeFileSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, const Model::BrushFace& face) const override {
                writeFacePoints(stream, face);
                writeValveTextureInfo(stream, face);
                fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
            }
        };

        std::unique_ptr<NodeSerializer> MapFileSerializer::create(const Model::MapFormat format, std::ostream& stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return std::make_unique<QuakeFileSerializer>(stream);
                case Model::MapFormat::Quake2:
                    // TODO 2427: Implement Quake3 serializers and use them
                case Model::MapFormat::Quake3:
                case Model::MapFormat::Quake3_Legacy:
                    return std::make_unique<Quake2FileSerializer>(stream);
                case Model::MapFormat::Quake2_Valve:
                case Model::MapFormat::Quake3_Valve:
                    return std::make_unique<Quake2ValveFileSerializer>(stream);
                case Model::MapFormat::Daikatana:
                    return std::make_unique<DaikatanaFileSerializer>(stream);
                case Model::MapFormat::Valve:
                    return std::make_unique<ValveFileSerializer>(stream);
                case Model::MapFormat::Hexen2:
                    return std::make_unique<Hexen2FileSerializer>(stream);
                case Model::MapFormat::Unknown:
                    throw FileFormatException("Unknown map file format");
                switchDefault()
            }
        }

        MapFileSerializer::MapFileSerializer(std::ostream& stream) :
        m_line(1),
        m_stream(stream) {}

        template<class T, class L>
        auto parallelTransform(const std::vector<T>& input, L&& transform) {
            using ResultType = decltype(transform(std::declval<const T&>()));

            std::vector<ResultType> result;
            result.resize(input.size());

            unsigned int numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) {
                numThreads = 1;
            }

            std::atomic<size_t> nextIndex(0);

            std::vector<std::future<void>> threads;
            threads.resize(numThreads);

            for (unsigned int i = 0; i < numThreads; ++i) {
                threads[i] = std::async(std::launch::async, [&]() {
                    while (true) {
                        const size_t ourIndex = std::atomic_fetch_add(&nextIndex, static_cast<size_t>(1));
                        if (ourIndex >= input.size()) {
                            break;
                        }
                        result[ourIndex] = transform(input[ourIndex]);
                    }
                });
            }

            for (unsigned int i = 0; i < numThreads; ++i) {
                threads[i].wait();
            }

            return result;
        }


        void MapFileSerializer::precomputeNodes(const std::vector<const Model::Node*>& nodes) {
            qDebug() << "precomputing serialization for" << nodes.size() << "nodes";


            int worlds=0;
            int layers=0;
            int groups=0;
            int ents=0;
            int brushes=0;

            std::vector<const Model::BrushNode*> brushNodes;
            brushNodes.reserve(nodes.size());

            for (auto* n : nodes) {
                n->accept(kdl::overload(
                         [&](auto&& thisLambda, const Model::WorldNode* world)   {worlds++;
                                world->visitChildren(thisLambda);
                            },
                         [&](auto&& thisLambda, const Model::LayerNode* layer)   {layers++;
                                layer->visitChildren(thisLambda);
                            },
                         [&](auto&& thisLambda, const Model::GroupNode* group)   {groups++;
                            group->visitChildren(thisLambda);
                            },
                         [&](auto&& thisLambda, const Model::EntityNode* entity) {ents++;
                            entity->visitChildren(thisLambda);
                            },
                         [&](const Model::BrushNode* brush) {
                             brushes++;
                             brushNodes.push_back(brush);
                         }
                     ));
            }

            qDebug() << "precomputing serialization for"
                     << worlds << "worlds"
                    << layers << "layers"
                    << groups << "groups"
                    << ents << "ents"
                    << brushes << "brushes";

            QElapsedTimer timer;
            timer.start();

            ensure(m_nodeToPrecomputedString.empty(), "reused serializer");

            using NodeString = std::pair<const Model::Node*, std::string>;

            std::function<NodeString(const Model::BrushNode*)> transform =
                [&](const Model::BrushNode* node) -> NodeString {
                    std::string string = writeBrushFaces(node->brush());
                    return NodeString(node, std::move(string));
                };

            //std::vector<NodeString> result = QtConcurrent::blockingMapped<std::vector<NodeString>>(brushNodes, transform);
            //std::vector<NodeString> result = kdl::vec_transform(nodes, transform);
            std::vector<NodeString> result = parallelTransform(brushNodes, transform);

            //std::vector<NodeString> result;
            //result.resize(nodes.size());
            //std::transform(std::execution::par_unseq,
            //   std::cbegin(nodes), std::cend(nodes),
            //   std::begin(result),
            //   transform);

            qDebug() << "end of blocking map " << timer.elapsed();
            

            // move strings into a map
            for (auto& [node, string]: result) {
                m_nodeToPrecomputedString[node] = std::move(string);
            }

            qDebug() << "precomputeNodes took " << timer.elapsed() << "ms with threads:" << QThread::idealThreadCount();

            //
            // for (const auto& [node, name] : m_nodeToPrecomputedString) {
            //     qDebug() << "node:" << node << "name:" << QString::fromStdString(name);
            // }
        }

        void MapFileSerializer::doBeginFile() {}
        void MapFileSerializer::doEndFile() {}

        void MapFileSerializer::doBeginEntity(const Model::Node* /* node */) {
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "// entity {}\n", entityNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "{{\n");
            ++m_line;
        }

        void MapFileSerializer::doEndEntity(const Model::Node* node) {
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "}}\n");
            ++m_line;
            setFilePosition(node);
        }

        void MapFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "\"{}\" \"{}\"\n",
                           escapeEntityAttribute( attribute.name()),
                           escapeEntityAttribute(attribute.value()));
            ++m_line;
        }

        void MapFileSerializer::doBrush(const Model::BrushNode* brush) {
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "// brush {}\n", brushNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "{{\n");
            ++m_line;

            // write pre-serialized brush faces
            const std::string& preSerialized = m_nodeToPrecomputedString.at(brush);
            m_stream << preSerialized;

            fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "}}\n");
            ++m_line;
            setFilePosition(brush);
        }

        void MapFileSerializer::doBrushFace(const Model::BrushFace& face) {
            const size_t lines = 1u;
            doWriteBrushFace(m_stream, face);
            face.setFilePosition(m_line, lines);
            m_line += lines;
        }

        void MapFileSerializer::setFilePosition(const Model::Node* node) {
            const size_t start = startLine();
            node->setFilePosition(start, m_line - start);
        }

        size_t MapFileSerializer::startLine() {
            assert(!m_startLineStack.empty());
            const size_t result = m_startLineStack.back();
            m_startLineStack.pop_back();
            return result;
        }

        /**
         * Threadsafe
         */
        std::string MapFileSerializer::writeBrushFaces(const Model::Brush& brush) const {
            std::stringstream stream;
            for (const Model::BrushFace& face : brush.faces()) {
                doWriteBrushFace(stream, face);
            }
            return stream.str();
        }
    }
}

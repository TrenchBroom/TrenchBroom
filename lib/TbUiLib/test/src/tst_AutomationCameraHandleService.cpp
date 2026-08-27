/* Copyright (C) 2026 */
#include <QPointer>

#include "ui/AutomationCameraHandleService.h"
#include "ui/CatchConfig.h"

#include <memory>
#include <unordered_map>
#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{
class Documents : public AutomationCameraDocumentResolver
{
public:
  std::unordered_map<std::string, QPointer<QObject>> documents;

  QObject* findDocument(const std::string_view id) const override
  {
    const auto found = documents.find(std::string{id});
    return found == documents.end() ? nullptr : found->second.data();
  }
};

class Capture : public AutomationCameraCapture
{
public:
  std::vector<AutomationCameraHandle> handles;

  Result<AutomationCameraCaptureResult, AutomationCameraHandleError> capture(
    const AutomationCameraHandle& handle) override
  {
    handles.push_back(handle);
    return AutomationCameraCaptureResult{
      {{"capture.png"},
       {320, 200},
       automation::AutomationCaptureMode::Offscreen,
       std::nullopt},
      9u};
  }
};

automation::AutomationRenderRequest request()
{
  return {
    {automation::AutomationProjection::Perspective,
     {1.0, 2.0, 3.0},
     {0.0, 1.0, 0.0},
     {0.0, 0.0, 1.0},
     75.0,
     std::nullopt,
     1.0,
     4096.0},
    {320, 200},
    automation::AutomationRenderMode::Textured,
    {}};
}

} // namespace

TEST_CASE("AutomationCameraHandleService")
{
  auto documents = Documents{};
  auto firstDocument = std::make_unique<QObject>();
  documents.documents.emplace("document-one", firstDocument.get());
  auto tokenIndex = 0u;
  auto registry = AutomationCameraHandleRegistry{
    documents, [&] { return std::string{"handle-"} + std::to_string(++tokenIndex); }};
  auto capture = Capture{};
  auto service = AutomationCameraHandleService{registry, capture};

  SECTION("captures immutable pane-independent values through one explicit document")
  {
    const auto created = service.handle(
      "cameras.create",
      {{"documentId", "document-one"},
       {"request", automation::renderRequestToJson(request())}});
    REQUIRE(created.is_success());
    const auto id = created.value().value("cameraId").toString();
    CHECK(created.value().value("documentId") == "document-one");

    auto changed = request();
    changed.camera.position = {10.0, 20.0, 30.0};
    const auto updated = service.handle(
      "cameras.update",
      {{"cameraId", id}, {"request", automation::renderRequestToJson(changed)}});
    REQUIRE(updated.is_success());
    CHECK(capture.handles.empty());

    const auto captured = service.handle("cameras.capture", {{"cameraId", id}});
    REQUIRE(captured.is_success());
    REQUIRE(capture.handles.size() == 1u);
    CHECK(capture.handles.front().documentId == "document-one");
    CHECK(capture.handles.front().request.camera.position == vm::vec3d{10.0, 20.0, 30.0});
    CHECK(captured.value().value("revision") == 9);

    REQUIRE(service.handle("cameras.delete", {{"cameraId", id}}).is_success());
    CHECK(service.handle("cameras.get", {{"cameraId", id}}).is_error());
  }

  SECTION(
    "invalidates handles when their one document lifetime ends and never reuses ids")
  {
    const auto first = registry.create("document-one", request());
    REQUIRE(first.is_success());
    firstDocument.reset();
    CHECK(registry.get(first.value().id).is_error());

    auto secondDocument = std::make_unique<QObject>();
    documents.documents["document-two"] = secondDocument.get();
    const auto second = registry.create("document-two", request());
    REQUIRE(second.is_success());
    CHECK(second.value().id != first.value().id);
  }
}

} // namespace tb::ui

/* Copyright (C) 2026 */
#include "ui/AutomationCameraHandleService.h"

#include <QUuid>

#include "ui/AppController.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationVirtualRenderService.h"
#include "ui/MapWindow.h"

#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{
AutomationCameraHandleError error(
  const AutomationCameraHandleErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

std::string token()
{
  return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

AutomationCameraHandleServiceResult requestError(std::string message)
{
  return error(AutomationCameraHandleErrorCode::InvalidRequest, std::move(message));
}

std::optional<automation::AutomationRenderRequest> request(const QJsonObject& params)
{
  const auto value = params.value("request");
  return value.isObject() ? automation::renderRequestFromJson(value.toObject())
                          : std::nullopt;
}

} // namespace

AutomationCameraHandleRegistry::AutomationCameraHandleRegistry(
  const AutomationCameraDocumentResolver& documents, TokenGenerator tokenGenerator)
  : m_documents{documents}
  , m_tokenGenerator{tokenGenerator ? std::move(tokenGenerator) : token}
{
}

AutomationCameraHandleResult AutomationCameraHandleRegistry::create(
  std::string documentId, automation::AutomationRenderRequest request)
{
  if (documentId.empty() || m_documents.findDocument(documentId) == nullptr)
    return error(
      AutomationCameraHandleErrorCode::UnknownDocument, "Unknown explicit documentId");
  auto id = std::string{};
  do
  {
    id = "camera-" + m_tokenGenerator();
  } while (id == "camera-" || m_issuedIds.contains(id));
  m_issuedIds.insert(id);
  auto handle =
    AutomationCameraHandle{std::move(id), std::move(documentId), std::move(request)};
  auto* lifetime = m_documents.findDocument(handle.documentId);
  m_handles.emplace(handle.id, handle);
  connect(lifetime, &QObject::destroyed, this, [this, retiredId = handle.id] {
    m_handles.erase(retiredId);
  });
  return handle;
}

AutomationCameraHandleResult AutomationCameraHandleRegistry::update(
  const std::string& handleId, automation::AutomationRenderRequest request)
{
  const auto found = m_handles.find(handleId);
  if (found == m_handles.end())
    return error(AutomationCameraHandleErrorCode::UnknownHandle, "Unknown camera handle");
  if (m_documents.findDocument(found->second.documentId) == nullptr)
  {
    m_handles.erase(found);
    return error(
      AutomationCameraHandleErrorCode::UnknownDocument,
      "Camera document is no longer live");
  }
  found->second.request = std::move(request);
  return found->second;
}

AutomationCameraHandleResult AutomationCameraHandleRegistry::get(
  const std::string& handleId) const
{
  const auto found = m_handles.find(handleId);
  if (
    found == m_handles.end()
    || m_documents.findDocument(found->second.documentId) == nullptr)
  {
    if (found != m_handles.end())
      m_handles.erase(found);
    return error(AutomationCameraHandleErrorCode::UnknownHandle, "Unknown camera handle");
  }
  return found->second;
}

bool AutomationCameraHandleRegistry::erase(const std::string& handleId)
{
  return m_handles.erase(handleId) > 0u;
}

AutomationCameraHandleService::AutomationCameraHandleService(
  AutomationCameraHandleRegistry& handles, AutomationCameraCapture& capture)
  : m_handles{handles}
  , m_capture{capture}
{
}

AutomationCameraHandleServiceResult AutomationCameraHandleService::handle(
  const QString& method, const QJsonObject& params)
{
  if (method == "cameras.create")
  {
    const auto documentId = params.value("documentId");
    const auto renderRequest = request(params);
    if (!documentId.isString() || documentId.toString().isEmpty() || !renderRequest)
      return requestError("cameras.create requires documentId and a valid request");
    const auto created =
      m_handles.create(documentId.toString().toStdString(), *renderRequest);
    return created.is_error()
             ? AutomationCameraHandleServiceResult{std::get<AutomationCameraHandleError>(
                 created.error())}
             : handleToJson(created.value());
  }
  const auto id = params.value("cameraId");
  if (!id.isString() || id.toString().isEmpty())
    return requestError("Camera methods require cameraId");
  const auto handleId = id.toString().toStdString();
  if (method == "cameras.get")
  {
    const auto found = m_handles.get(handleId);
    return found.is_error()
             ? AutomationCameraHandleServiceResult{std::get<AutomationCameraHandleError>(
                 found.error())}
             : handleToJson(found.value());
  }
  if (method == "cameras.update")
  {
    const auto renderRequest = request(params);
    if (!renderRequest)
      return requestError("cameras.update requires a valid request");
    const auto updated = m_handles.update(handleId, *renderRequest);
    return updated.is_error()
             ? AutomationCameraHandleServiceResult{std::get<AutomationCameraHandleError>(
                 updated.error())}
             : handleToJson(updated.value());
  }
  if (method == "cameras.delete")
  {
    if (!m_handles.erase(handleId))
      return error(
        AutomationCameraHandleErrorCode::UnknownHandle, "Unknown camera handle");
    return QJsonObject{{"cameraId", id}, {"deleted", true}};
  }
  if (method == "cameras.capture")
  {
    const auto found = m_handles.get(handleId);
    if (found.is_error())
      return std::get<AutomationCameraHandleError>(found.error());
    const auto captured = m_capture.capture(found.value());
    if (captured.is_error())
      return std::get<AutomationCameraHandleError>(captured.error());
    auto result = handleToJson(found.value()).value();
    result.insert("output", automation::renderOutputToJson(captured.value().output));
    result.insert("revision", static_cast<qint64>(captured.value().revision));
    return result;
  }
  return error(AutomationCameraHandleErrorCode::MethodNotFound, "Unknown camera method");
}

AutomationCameraHandleServiceResult AutomationCameraHandleService::handleToJson(
  const AutomationCameraHandle& handle) const
{
  return QJsonObject{
    {"cameraId", QString::fromStdString(handle.id)},
    {"documentId", QString::fromStdString(handle.documentId)},
    {"request", automation::renderRequestToJson(handle.request)}};
}

AutomationRegistryCameraDocumentResolver::AutomationRegistryCameraDocumentResolver(
  const AutomationDocumentRegistry& documents)
  : m_documents{documents}
{
}

QObject* AutomationRegistryCameraDocumentResolver::findDocument(
  const std::string_view documentId) const
{
  return m_documents.findWindow(QString::fromStdString(std::string{documentId}));
}

AutomationVirtualCameraCapture::AutomationVirtualCameraCapture(
  AppController& appController, const AutomationDocumentRegistry& documents)
  : m_appController{appController}
  , m_documents{documents}
{
}

Result<AutomationCameraCaptureResult, AutomationCameraHandleError>
AutomationVirtualCameraCapture::capture(const AutomationCameraHandle& handle)
{
  auto* window = m_documents.findWindow(QString::fromStdString(handle.documentId));
  if (window == nullptr)
  {
    return error(
      AutomationCameraHandleErrorCode::UnknownDocument,
      "Camera document is no longer live");
  }
  auto service = AutomationVirtualRenderService{m_appController};
  const auto captured = service.capture(window->document(), handle.request);
  if (!captured)
  {
    return error(
      AutomationCameraHandleErrorCode::CaptureFailed, captured.message.toStdString());
  }
  return AutomationCameraCaptureResult{captured.output, captured.revision};
}

} // namespace tb::ui

/* Copyright (C) 2026 */
#pragma once

#include <QJsonObject>
#include <QObject>
#include <QPointer>

#include "base/Result.h"
#include "ui/AutomationRenderRequest.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace tb::ui
{
class AppController;
class AutomationDocumentRegistry;

enum class AutomationCameraHandleErrorCode
{
  InvalidRequest,
  UnknownDocument,
  UnknownHandle,
  CaptureFailed,
  MethodNotFound,
};

struct AutomationCameraHandleError
{
  AutomationCameraHandleErrorCode code = AutomationCameraHandleErrorCode::InvalidRequest;
  std::string message;
};

struct AutomationCameraHandle
{
  std::string id;
  std::string documentId;
  automation::AutomationRenderRequest request;
};

using AutomationCameraHandleResult =
  Result<AutomationCameraHandle, AutomationCameraHandleError>;

/** Maps an explicit non-reusable document identity to its lifetime object. */
class AutomationCameraDocumentResolver
{
public:
  virtual ~AutomationCameraDocumentResolver() = default;
  virtual QObject* findDocument(std::string_view documentId) const = 0;
};

class AutomationCameraHandleRegistry : public QObject
{
public:
  using TokenGenerator = std::function<std::string()>;

  explicit AutomationCameraHandleRegistry(
    const AutomationCameraDocumentResolver& documents,
    TokenGenerator tokenGenerator = {});

  AutomationCameraHandleResult create(
    std::string documentId, automation::AutomationRenderRequest request);
  AutomationCameraHandleResult update(
    const std::string& handleId, automation::AutomationRenderRequest request);
  AutomationCameraHandleResult get(const std::string& handleId) const;
  bool erase(const std::string& handleId);

private:
  const AutomationCameraDocumentResolver& m_documents;
  TokenGenerator m_tokenGenerator;
  mutable std::unordered_map<std::string, AutomationCameraHandle> m_handles;
  std::unordered_set<std::string> m_issuedIds;
};

struct AutomationCameraCaptureResult
{
  automation::AutomationRenderOutput output;
  size_t revision = 0u;
};

class AutomationCameraCapture
{
public:
  virtual ~AutomationCameraCapture() = default;
  virtual Result<AutomationCameraCaptureResult, AutomationCameraHandleError> capture(
    const AutomationCameraHandle& handle) = 0;
};

using AutomationCameraHandleServiceResult =
  Result<QJsonObject, AutomationCameraHandleError>;

/** Direct `cameras.*` handler facade; it owns no GUI pane or foreground selection. */
class AutomationCameraHandleService
{
public:
  AutomationCameraHandleService(
    AutomationCameraHandleRegistry& handles, AutomationCameraCapture& capture);

  AutomationCameraHandleServiceResult handle(
    const QString& method, const QJsonObject& params);

private:
  AutomationCameraHandleServiceResult handleToJson(
    const AutomationCameraHandle& handle) const;

  AutomationCameraHandleRegistry& m_handles;
  AutomationCameraCapture& m_capture;
};

/** Production camera lifetime adapter over the strict document registry. */
class AutomationRegistryCameraDocumentResolver : public AutomationCameraDocumentResolver
{
public:
  explicit AutomationRegistryCameraDocumentResolver(
    const AutomationDocumentRegistry& documents);

  QObject* findDocument(std::string_view documentId) const override;

private:
  const AutomationDocumentRegistry& m_documents;
};

/** Production capture adapter over the focus-neutral virtual render service. */
class AutomationVirtualCameraCapture : public AutomationCameraCapture
{
public:
  AutomationVirtualCameraCapture(
    AppController& appController, const AutomationDocumentRegistry& documents);

  Result<AutomationCameraCaptureResult, AutomationCameraHandleError> capture(
    const AutomationCameraHandle& handle) override;

private:
  AppController& m_appController;
  const AutomationDocumentRegistry& m_documents;
};
} // namespace tb::ui

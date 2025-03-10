/*
 Copyright (C) 2025 Kristian Duske

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

#pragma once

#include "upd/HttpClient.h"

namespace upd
{

struct TestError
{
  QString msg;
};

template <typename Callback>
struct TestHttpOperation : public HttpOperation
{
  Callback successCallback;
  HttpClient::ErrorCallback errorCallback;
  TestHttpOperation*& pendingOperationPtr = nullptr;
  bool cancelled = false;

  TestHttpOperation(
    Callback successCallback_,
    HttpClient::ErrorCallback errorCallback_,
    TestHttpOperation*& pendingOperationPtr_)
    : successCallback{std::move(successCallback_)}
    , errorCallback{std::move(errorCallback_)}
    , pendingOperationPtr{pendingOperationPtr_}
  {
    pendingOperationPtr = this;
  }

  ~TestHttpOperation() override
  {
    if (pendingOperationPtr == this)
    {
      pendingOperationPtr = nullptr;
    }
  }

  void cancel() override { cancelled = true; }
};

struct TestHttpClient : public HttpClient
{
  HttpOperation* get(
    const QUrl& url, GetCallback getCallback, ErrorCallback errorCallback) const override;

  HttpOperation* download(
    const QUrl& url,
    DownloadCallback downloadCallback,
    ErrorCallback errorCallback) const override;

  mutable TestHttpOperation<GetCallback>* pendingGetOperation = nullptr;
  mutable TestHttpOperation<DownloadCallback>* pendingDownloadOperation = nullptr;
};

} // namespace upd

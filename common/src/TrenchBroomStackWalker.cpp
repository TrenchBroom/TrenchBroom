/*
 Copyright (C) 2016 Eric Wasylishen

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

#ifdef _WIN32
#ifdef _MSC_VER
#include "StackWalker.h"
#include <QMutexLocker>
#endif
#else
#include <execinfo.h>
#endif

#include "TrenchBroomStackWalker.h"

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom
{
#ifdef _WIN32
#ifdef _MSC_VER

// use https://stackwalker.codeplex.com/
class TBStackWalker : public StackWalker
{
public:
  std::stringstream m_string;
  TBStackWalker()
    : StackWalker()
  {
  }
  void clear() { m_string.str(""); }
  std::string asString() { return m_string.str(); }

protected:
  virtual void OnOutput(LPCSTR szText) { m_string << szText; }
};

static QMutex s_stackWalkerMutex;
static TBStackWalker* s_stackWalker;

static std::string getStackTraceInternal(CONTEXT* context)
{
  // StackWalker is not threadsafe so acquire a mutex
  QMutexLocker lock(&s_stackWalkerMutex);

  if (s_stackWalker == nullptr)
  {
    // create a shared instance on first use
    s_stackWalker = new TBStackWalker();
  }
  s_stackWalker->clear();
  if (context == nullptr)
  {
    // get the current call stack
    s_stackWalker->ShowCallstack();
  }
  else
  {
    // get the call stack of the exception.
    // see: http://www.codeproject.com/Articles/11132/Walking-the-callstack
    s_stackWalker->ShowCallstack(GetCurrentThread(), context);
  }
  return s_stackWalker->asString();
}

std::string TrenchBroomStackWalker::getStackTrace()
{
  return getStackTraceInternal(nullptr);
}

std::string TrenchBroomStackWalker::getStackTraceFromContext(void* context)
{
  return getStackTraceInternal(static_cast<CONTEXT*>(context));
}
#else
// TODO: not sure what to use on mingw
std::string TrenchBroomStackWalker::getStackTrace()
{
  return "";
}
#endif
#else
std::string TrenchBroomStackWalker::getStackTrace()
{
  const int MaxDepth = 256;
  void* callstack[MaxDepth];
  const int frames = backtrace(callstack, MaxDepth);

  // copy into a vector
  std::vector<void*> framesVec(callstack, callstack + frames);
  if (framesVec.empty())
    return "";

  std::stringstream ss;
  char** strs = backtrace_symbols(&framesVec.front(), static_cast<int>(framesVec.size()));
  for (size_t i = 0; i < framesVec.size(); i++)
  {
    ss << strs[i] << std::endl;
  }
  free(strs);
  return ss.str();
}
#endif
} // namespace TrenchBroom

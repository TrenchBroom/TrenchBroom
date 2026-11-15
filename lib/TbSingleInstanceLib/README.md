# Single Instance Library

A library that ensures that only one instance of a Qt6 application runs at a time.

## Introduction

When an application is started while another instance of it is already running, the new
instance forwards its arguments (e.g. the files to open) to the running instance and then
terminates. The library doesn't depend on any other part of TrenchBroom.

All instances that are created with the same key compete for the primary role:

- The **primary** instance holds a lock file in the temporary directory and listens on a
  local socket (a named pipe on Windows). It receives the messages sent by other
  instances.
- A **secondary** instance found the lock held by another instance and connected to it.
  It should send its message to the primary instance and terminate.
- A **standalone** instance could not establish either role within the given timeout,
  e.g. because the instance holding the lock doesn't accept connections. It should run on
  its own so that a broken connection never prevents the application from starting.

The lock is only considered stale if the process that holds it isn't running anymore,
so a crashed primary instance doesn't block later instances.

## Keys

Instances only compete with instances that use the same key. `makeKey` creates a key from
the application name, the current user, and any additional strings that should separate
instances, such as the canonical path of the application's executable. This allows
separate installations of an application to run side by side.

The user must be part of the key because named pipes are not separated per user on
Windows. `makeKey` takes care of that.

## Usage

```cpp
auto app = QApplication{argc, argv};

auto singleInstance = tb::singleinstance::SingleInstance{
  tb::singleinstance::makeKey(u"MyApp", {QCoreApplication::applicationFilePath()}),
  std::chrono::seconds{5}};

if (singleInstance.role() == tb::singleinstance::Role::Secondary)
{
  if (singleInstance.sendMessage(app.arguments(), std::chrono::seconds{10}))
  {
    return 0;
  }
  // otherwise continue as a standalone instance
}

QObject::connect(
  &singleInstance,
  &tb::singleinstance::SingleInstance::messageReceived,
  [](const QStringList& arguments) {
    // open the files, bring a window to the front, ...
  });

return app.exec();
```

## Bringing Windows to the Front

On Windows, a process may only bring its windows to the foreground if the foreground
process allows it. A secondary instance therefore allows any process to do so before it
sends its message, so that the primary instance can activate its window when it receives
the message.

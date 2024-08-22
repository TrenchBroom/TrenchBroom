/*
 Copyright (C) 2021 Kristian Duske

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

#include "NotifierConnection.h"

#include "kdl/set_temp.h"
#include "kdl/tuple_utils.h"

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

namespace TrenchBroom
{
/**
 * Base class for notifier state. This is only necessary so that NotifierConnection is
 * independent of the Notifier type.
 */
class NotifierStateBase
{
public:
  virtual ~NotifierStateBase() = default;

private:
  friend class NotifierConnection;
  virtual void disconnect(size_t id) = 0;
};

/**
 * A notifier that multiple observers can connect to.
 *
 * Observers are notified in the order in which they were connected. The same observer can
 * be connected multiple times.
 *
 * @tparam A the types of the parameters passed to the observer callbacks.
 */
template <typename... A>
class Notifier
{
private:
  using Callback = std::function<void(A...)>;

  struct Observer
  {
    Callback callback;
    size_t id;
    bool pendingRemove;

    Observer(Callback i_callback, const size_t i_id)
      : callback{std::move(i_callback)}
      , id{i_id}
      , pendingRemove{false}
    {
    }
  };

  class NotifierState : public NotifierStateBase
  {
  private:
    size_t m_nextId{0};
    std::vector<Observer> m_observers;
    std::vector<Observer> m_toAdd;
    bool m_notifying{false};

  public:
    ~NotifierState() override = default;

    size_t connect(Callback callback)
    {
      const auto id = m_nextId++;
      if (m_notifying)
      {
        m_toAdd.emplace_back(std::move(callback), id);
      }
      else
      {
        m_observers.emplace_back(std::move(callback), id);
      }
      return id;
    }

    size_t connect(Notifier& notifier)
    {
      const auto id = m_nextId++;
      auto callback = [&](auto&&... a) { notifier(std::forward<decltype(a)>(a)...); };
      if (m_notifying)
      {
        m_toAdd.emplace_back(std::move(callback), id);
      }
      else
      {
        m_observers.emplace_back(std::move(callback), id);
      }
      return id;
    }

    template <typename... NA>
    void notify(NA&&... a)
    {
      processPendingObservers();

      const kdl::set_temp notifying(m_notifying);
      for (const auto& observer : m_observers)
      {
        if (!observer.pendingRemove)
        {
          observer.callback(std::forward<NA>(a)...);
        }
      }
    }

    void disconnect(const size_t id) override
    {
      if (const auto it = findObserver(m_toAdd, id); it != std::end(m_toAdd))
      {
        m_toAdd.erase(it);
        return;
      }

      if (const auto it = findObserver(m_observers, id); it != std::end(m_observers))
      {
        if (m_notifying)
        {
          it->pendingRemove = true;
        }
        else
        {
          m_observers.erase(it);
        }
      }
    }

  private:
    typename std::vector<Observer>::iterator findObserver(
      std::vector<Observer>& observers, const size_t id)
    {
      return std::find_if(
        std::begin(observers), std::end(observers), [&](const auto& observer) {
          return observer.id == id;
        });
    }

    void processPendingObservers()
    {
      assert(!m_notifying);

      for (auto it = std::begin(m_observers); it != std::end(m_observers);)
      {
        if (it->pendingRemove)
        {
          it = m_observers.erase(it);
        }
        else
        {
          ++it;
        }
      }

      while (!m_toAdd.empty())
      {
        auto observer = std::move(m_toAdd.back());
        m_observers.push_back(std::move(observer));
        m_toAdd.pop_back();
      }
    }
  };

  std::shared_ptr<NotifierState> m_state{std::make_shared<NotifierState>()};

public:
  friend class NotifierConnection;

  Notifier() = default;

  Notifier(const Notifier&) = delete;
  Notifier(Notifier&&) noexcept = default;

  Notifier& operator=(const Notifier&) = delete;
  Notifier& operator=(Notifier&&) noexcept = default;

  /**
   * Adds the given observer callback to this notifier.
   *
   * If this notifier is currently notifying, then the callback will be connected, but it
   * will not be notified of the current notification.
   *
   * Returns connection object that disconnects the callback from this notification when
   * it goes out of scope.
   */
  [[nodiscard]] NotifierConnection connect(Callback callback)
  {
    const auto id = m_state->connect(std::move(callback));
    return NotifierConnection{m_state, id};
  }

  /**
   * Adds the given observer callback to this notifier.
   *
   * This overload is useful when the callback is a member function.
   *
   * @param receiver the receiver object, i.e. the owner of the member function
   * @param callback the observer callback to call when the notification happens
   */
  template <typename R, typename MemberCallback>
  [[nodiscard]] NotifierConnection connect(R* receiver, MemberCallback callback)
  {
    return connect([receiver = receiver, callback = std::move(callback)](auto&&... args) {
      std::invoke(callback, receiver, std::forward<decltype(args)>(args)...);
    });
  }

  /**
   * Adds the given observer callback to this notifier.
   *
   * If this notifier is currently notifying, then the callback will be connected, but it
   * will not be notified of the current notification.
   *
   * Returns connection object that disconnects the callback from this notification when
   * it goes out of scope.
   */
  [[nodiscard]] NotifierConnection connect(Notifier& notifier)
  {
    const auto id = m_state->connect(notifier);
    return NotifierConnection{m_state, id};
  }

  /**
   * Notifies all observers of this notifier with the given arguments.
   *
   * @param a the arguments to pass to each notifier
   */
  template <typename... NA>
  void notify(NA&&... a)
  {
    m_state->notify(std::forward<NA>(a)...);
  }

  /**
   * Notifies all observers of this notifier with the given arguments.
   *
   * @param a the arguments to pass to each notifier
   */
  template <typename... NA>
  void operator()(NA&&... a)
  {
    notify(std::forward<NA>(a)...);
  }
};

/**
 * RAII style helper tht notifies the given notifier when it is destroyed, passing the
 * given arguments.
 */
template <typename... A>
class NotifyAfter
{
private:
  Notifier<A...>& m_notifier;

protected:
  std::function<void(Notifier<A...>&)> m_notify;

public:
  /**
   * Creates a new instance to notify the given notifier. The given arguments are passed
   * to the notifier.
   *
   * The given arguments are captured using kdl::tup_capture. Callers must ensure that
   * objects passed to the constructor by (const) lvalue reference outlive this object.
   *
   * @param notify controls whether or not the notifications should be sent
   * @param notifier the notifier to notify
   * @param a the arguments to pass to the notifier
   */
  template <typename... NA>
  explicit NotifyAfter(const bool notify, Notifier<A...>& notifier, NA&&... a)
    : m_notifier{notifier}
    , m_notify{makeNotify(notify, std::forward<NA>(a)...)}
  {
  }

  virtual ~NotifyAfter() { m_notify(m_notifier); }

private:
  template <typename... NA>
  static std::function<void(Notifier<A...>&)> makeNotify(const bool notify, NA&&... a)
  {
    if (notify)
    {
      return [args = kdl::tup_capture(std::forward<NA>(a)...)](Notifier<A...>& notifier) {
        std::apply(notifier, args);
      };
    }
    return [](Notifier<A...>&) {};
  }
};

template <typename... AA, typename... NA>
NotifyAfter(bool, Notifier<AA...>&, NA&&...) -> NotifyAfter<AA...>;

/**
 * RAII style helper tht notifies a given notifier immediately and another notifier when
 * it is destroyed, passing the given arguments to either notifier.
 */
template <typename... A>
class NotifyBeforeAndAfter : public NotifyAfter<A...>
{
public:
  /**
   * Creates a new instance that notifies the given notifiers.
   *
   * @param notify controls whether or not the notifications should be sent
   * @param before the notifier to notify immediately
   * @param after the notifier to notify later when this object is destroyed
   * @param a the arguments to pass to either notifier
   */
  template <typename... NA>
  NotifyBeforeAndAfter(
    const bool notify, Notifier<A...>& before, Notifier<A...>& after, NA&&... a)
    : NotifyAfter<A...>{notify, after, std::forward<NA>(a)...}
  {
    NotifyAfter<A...>::m_notify(before);
  }

  /**
   * Creates a new instance that notifies the given notifiers.
   *
   * @param before the notifier to notify immediately
   * @param after the notifier to notify later when this object is destroyed
   * @param a the arguments to pass to either notifier
   */
  template <typename... NA>
  NotifyBeforeAndAfter(Notifier<A...>& before, Notifier<A...>& after, NA&&... a)
    : NotifyBeforeAndAfter{true, before, after, std::forward<NA>(a)...}
  {
  }
};

template <typename... AA, typename... NA>
NotifyBeforeAndAfter(bool, Notifier<AA...>&, Notifier<AA...>&, NA&&...)
  -> NotifyBeforeAndAfter<AA...>;

template <typename... AA, typename... NA>
NotifyBeforeAndAfter(Notifier<AA...>&, Notifier<AA...>&, NA&&...)
  -> NotifyBeforeAndAfter<AA...>;
} // namespace TrenchBroom

#pragma once

template <typename T>
template <typename... Arg>
bim::axmol::input::key_observer_handle<T>::key_observer_handle(Arg&&... args)
  : m_observer(std::make_shared<T>(std::forward<Arg>(args)...))
{}

template <typename T>
bim::axmol::input::key_observer_handle<T>::~key_observer_handle() = default;

template <typename T>
T* bim::axmol::input::key_observer_handle<T>::operator->() const
{
  return m_observer.get();
}

template <typename T>
T& bim::axmol::input::key_observer_handle<T>::operator*() const
{
  return *m_observer;
}

template <typename T>
bim::axmol::input::key_observer_handle<T>::operator key_observer_pointer()
    const
{
  return m_observer;
}

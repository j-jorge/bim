#pragma once

namespace bim::axmol::input
{
  /**
   * Wraps a data from the backend (i.e. axmol) together with a flag indicating
   * if the data is still available for input processing or if it has already
   * been consumed.
   */
  template <typename T>
  class backend_event
  {
  public:
    explicit backend_event(T data);

    bool is_available() const;
    void consume();

    T get() const;

  private:
    T m_data;
    bool m_is_available;
  };
}

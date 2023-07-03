#include <axmol/math/Vec2.h>

#include <string_view>

namespace bim::axmol::display
{
  class main_view
  {
  public:
    main_view(std::string_view title, const ax::Size& size, float scale);
    ~main_view();

    main_view(const main_view&) = delete;
    main_view& operator=(const main_view&) = delete;
  };
}

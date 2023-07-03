#pragma once

#include <bim/axmol/widget/declare_controls_struct.hpp>

// TODO: remove
#include <axmol/2d/Node.h>

namespace ax
{
  class Scene;
}

namespace bim::axmol::widget
{
  class context;
}

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::app
{
  class main_scene
  {
  public:
    main_scene(ax::Scene& scene, const bim::axmol::widget::context& context,
               const iscool::style::declaration& style);

  private:
    ax::Scene& m_scene;

    bim_declare_controls_struct(controls, m_controls, 2);
  };
}

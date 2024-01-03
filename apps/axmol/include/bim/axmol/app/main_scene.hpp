#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

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

    bim::axmol::input::tree m_inputs;
  };
}

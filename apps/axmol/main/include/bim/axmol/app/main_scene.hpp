// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <unordered_map>

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
    ~main_scene();

    bim::axmol::input::node_reference input_node() const;

    void add_in_main_canvas(ax::Node& node,
                            const bim::axmol::input::node_reference& inputs);
    void add_in_overlays(ax::Node& node,
                         const bim::axmol::input::node_reference& inputs);
    void remove_from_overlays(ax::Node& node);

  private:
    using node_to_inputs_map =
        std::unordered_map<const ax::Node*, bim::axmol::input::node_reference>;

  private:
    ax::Scene& m_scene;

    bim_declare_controls_struct(controls, m_controls, 2);

    bim::axmol::input::tree m_inputs;
    bim::axmol::input::tree m_main_canvas_inputs;
    bim::axmol::input::tree m_overlay_inputs;

    node_to_inputs_map m_overlay_node_inputs;
  };
}

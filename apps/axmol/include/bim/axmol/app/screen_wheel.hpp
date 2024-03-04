#pragma once

#include <bim/axmol/input/tree.hpp>
#include <bim/axmol/widget/declare_controls_struct.hpp>

#include <iscool/context.hpp>

#include <memory>

namespace bim::axmol::widget
{
  class context;
}

namespace bim::net
{
  class session_handler;
}

namespace iscool
{
  namespace preferences
  {
    class local_preferences;
  }
  namespace style
  {
    class declaration;
  }
}

namespace bim::axmol::app
{
  class main_scene;
  class scene_lock;
  class lobby;

  class screen_wheel
  {
    ic_declare_context(
        m_context,
        ic_context_declare_parent_properties(                      //
            ((const bim::axmol::widget::context&)(widget_context)) //
            ((main_scene*)(main_scene))                            //
            ((bim::net::session_handler*)(session_handler))),
        ic_context_no_properties);

  public:
    screen_wheel(const context& context,
                 const iscool::style::declaration& style);
    ~screen_wheel();

  private:
    bim::axmol::ref_ptr<ax::Node> m_main_container;
    bim::axmol::input::tree m_inputs;

    bim_declare_controls_struct(controls, m_controls, 2);

    bim::axmol::ref_ptr<ax::Node> m_lobby_container;
    std::unique_ptr<lobby> m_lobby;
  };
}

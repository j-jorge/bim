#include <bim/axmol/app/root_scene.hpp>

#include <bim/axmol/ref_ptr.impl.hpp>

#include <iscool/signals/implement_signal.hpp>

template class bim::axmol::ref_ptr<bim::axmol::app::root_scene>;

IMPLEMENT_SIGNAL(bim::axmol::app::root_scene, clean_up, m_clean_up)

bim::axmol::app::root_scene* bim::axmol::app::root_scene::create()
{
  root_scene* result(new root_scene());

  if (result->init())
    result->autorelease();
  else
    AX_SAFE_DELETE(result);

  return result;
}

void bim::axmol::app::root_scene::cleanup()
{
  ax::Scene::cleanup();
  m_clean_up();
}

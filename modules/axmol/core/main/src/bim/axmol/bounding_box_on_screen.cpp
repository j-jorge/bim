#include <bim/axmol/bounding_box_on_screen.hpp>

#include <axmol/2d/Node.h>

ax::Rect bim::axmol::bounding_box_on_screen(const ax::Node& node)
{
  const ax::Vec2 bottom_left(node.convertToWorldSpace(ax::Vec2::ZERO));

  ax::Vec2 scale(node.getScaleX(), node.getScaleY());

  for (const ax::Node* n = node.getParent(); n != nullptr; n = n->getParent())
    {
      scale.x *= n->getScaleX();
      scale.y *= n->getScaleY();
    }

  return ax::Rect(bottom_left, scale * node.getContentSize());
}

#include <bim/axmol/displayed.hpp>

#include <axmol/2d/Node.h>

bool bim::axmol::displayed(const ax::Node& node)
{
  if (!node.isRunning())
    return false;

  const ax::Node* n = &node;

  do
    {
      if (!n->isVisible())
        return false;

      n = n->getParent();
    }
  while (n != nullptr);

  return true;
}

#include <bim/axmol/ref_ptr.hpp>
#include <bim/axmol/ref_ptr.impl.hpp>

#include <axmol/2d/ActionInterval.h>
#include <axmol/2d/ClippingNode.h>
#include <axmol/2d/Label.h>
#include <axmol/2d/Layer.h>
#include <axmol/2d/Node.h>
#include <axmol/2d/Sprite.h>

template class bim::axmol::ref_ptr<ax::Action>;
template class bim::axmol::ref_ptr<ax::ActionInterval>;
template class bim::axmol::ref_ptr<ax::ClippingNode>;
template class bim::axmol::ref_ptr<ax::Label>;
template class bim::axmol::ref_ptr<ax::Layer>;
template class bim::axmol::ref_ptr<ax::LayerColor>;
template class bim::axmol::ref_ptr<ax::Node>;
template class bim::axmol::ref_ptr<ax::Sprite>;

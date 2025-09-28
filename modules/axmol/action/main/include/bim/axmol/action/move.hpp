// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <axmol/2d/ActionInterval.h>

namespace iscool::style
{
  class declaration;
}

namespace bim::axmol::action
{
  class move : public ax::MoveBy
  {
  public:
    static move* create(const iscool::style::declaration& style);

    ~move();

    void startWithTarget(ax::Node* target) override;

  private:
    explicit move(const iscool::style::declaration& style);

    bool init();

  private:
    const iscool::style::declaration& m_style;
  };
}

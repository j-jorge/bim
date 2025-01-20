// SPDX-License-Identifier: AGPL-3.0-only
#pragma once

#include <bim/net/message/client_token.hpp>
#include <bim/net/message/encounter_id.hpp>
#include <bim/net/message/message_type.hpp>

#include <bim/game/feature_flags_fwd.hpp>

#include <iscool/net/message/raw_message.hpp>

namespace bim::net
{
  DECLARE_RAW_MESSAGE(accept_named_game, message_type::accept_named_game,
                      ((client_token)(request_token)) //
                      ((encounter_id)(encounter_id))  //
                      ((bim::game::feature_flags)(features)));
}

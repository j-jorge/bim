// SPDX-License-Identifier: AGPL-3.0-only
/**
 * This file uses the X-macro idiom to generate code that must be executed for
 * every component type.
 */

bim_game_x_component(animation_state)                   //
    bim_game_x_component(arena_reduction_state)         //
    bim_game_x_component(bomb)                          //
    bim_game_x_component(bomb_power_up)                 //
    bim_game_x_component(bomb_power_up_spawner)         //
    bim_game_x_component(burning)                       //
    bim_game_x_component(crate)                         //
    bim_game_x_component(crushed)                       //
    bim_game_x_component(dead)                          //
    bim_game_x_component(falling_block)                 //
    bim_game_x_component(flame)                         //
    bim_game_x_component(flame_blocker)                 //
    bim_game_x_component(flame_power_up)                //
    bim_game_x_component(flame_power_up_spawner)        //
    bim_game_x_component(fog_of_war)                    //
    bim_game_x_component(fractional_position_on_grid)   //
    bim_game_x_component(game_timer)                    //
    bim_game_x_component(invincibility_state)           //
    bim_game_x_component(invisibility_power_up)         //
    bim_game_x_component(invisibility_power_up_spawner) //
    bim_game_x_component(invisibility_state)            //
    bim_game_x_component(kicked)                        //
    bim_game_x_component(layer_front)                   //
    bim_game_x_component(layer_zero)                    //
    bim_game_x_component(player)                        //
    bim_game_x_component(player_action)                 //
    bim_game_x_component(player_action_queue)           //
    bim_game_x_component(position_on_grid)              //
    bim_game_x_component(power_up)                      //
    bim_game_x_component(shallow)                       //
    bim_game_x_component(shield)                        //
    bim_game_x_component(shield_power_up)               //
    bim_game_x_component(shield_power_up_spawner)       //
    bim_game_x_component(solid)                         //
    bim_game_x_component(timer)                         //
    bim_game_x_component(wall)                          //

#undef bim_game_x_component

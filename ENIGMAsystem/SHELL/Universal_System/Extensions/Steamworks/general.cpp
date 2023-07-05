#include "general.h"

#include "game_client/c_stats_and_achievements.h"
#include "game_client/game_client.h"

namespace enigma_user {

bool steam_initialised() { return steamworks::c_main::is_initialised(); }

bool steam_stats_ready() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_stats_ready but not initialized, consider calling steam_init first.", M_ERROR);
    return false;
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_stats_ready but not logged in, please log into Steam first.", M_ERROR);
    return false;
  }

  return steamworks::c_main::get_c_game_client()->get_c_stats_and_achievements()->stats_valid();
}

unsigned steam_get_app_id() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_get_app_id but not initialized, consider calling steam_init first.", M_ERROR);
    return -1;
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_get_app_id but not logged in, please log into Steam first.", M_ERROR);
    return -1;
  }

  return steamworks::c_main::get_c_game_client()->get_steam_app_id();
}

unsigned steam_get_user_account_id() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_get_user_account_id but not initialized, consider calling steam_init first.", M_ERROR);
    return -1;
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_get_user_account_id but not logged in, please log into Steam first.", M_ERROR);
    return -1;
  }

  return steamworks::c_main::get_c_game_client()->get_c_steam_id_local_user().GetAccountID();
}

unsigned long long steam_get_user_steam_id() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_get_user_steam_id but not initialized, consider calling steam_init first.", M_ERROR);
    return -1;
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_get_user_steam_id but not logged in, please log into Steam first.", M_ERROR);
    return -1;
  }

  return steamworks::c_main::get_c_game_client()->get_c_steam_id_local_user().ConvertToUint64();
}

std::string steam_get_persona_name() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_get_persona_name but not initialized, consider calling steam_init first.", M_ERROR);
    return std::to_string(-1);
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_get_persona_name but not logged in, please log into Steam first.", M_ERROR);
    return std::to_string(-1);
  }

  return steamworks::c_game_client::get_steam_persona_name();
}

std::string steam_get_user_persona_name(CSteamID user_persona_name) {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_get_user_persona_name but not initialized, consider calling steam_init first.",
                  M_ERROR);
    return std::to_string(-1);
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_get_user_persona_name but not logged in, please log into Steam first.", M_ERROR);
    return std::to_string(-1);
  }

  return steamworks::c_game_client::get_steam_user_persona_name(user_persona_name);
}

bool steam_is_user_logged_on() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_is_user_logged_on but not initialized, consider calling steam_init first.", M_ERROR);
    return false;
  }

  return steamworks::c_game_client::is_user_logged_on();
}

std::string steam_current_game_language() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_current_game_language but not initialized, consider calling steam_init first.",
                  M_ERROR);
    return std::to_string(-1);
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_current_game_language but not logged in, please log into Steam first.", M_ERROR);
    return std::to_string(-1);
  }

  return steamworks::c_main::get_c_game_client()->get_current_game_language();
}

std::string steam_available_languages() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_available_languages but not initialized, consider calling steam_init first.", M_ERROR);
    return std::to_string(-1);
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_available_languages but not logged in, please log into Steam first.", M_ERROR);
    return std::to_string(-1);
  }

  return steamworks::c_main::get_c_game_client()->get_available_game_languages();
}

bool steam_is_subscribed() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_is_subscribed but not initialized, consider calling steam_init first.", M_ERROR);
    return false;
  }

  if (!steamworks::c_game_client::is_user_logged_on()) {
    DEBUG_MESSAGE("Calling steam_is_subscribed but not logged in, please log into Steam first.", M_ERROR);
    return false;
  }

  return steamworks::c_game_client::is_subscribed();
}

void steam_set_warning_message_hook() {
  if (!steamworks::c_main::is_initialised()) {
    DEBUG_MESSAGE("Calling steam_is_subscribed but not initialized, consider calling steam_init first.", M_ERROR);
    return;
  }

  steamworks::c_main::set_warning_message_hook();
}

}  // namespace enigma_user

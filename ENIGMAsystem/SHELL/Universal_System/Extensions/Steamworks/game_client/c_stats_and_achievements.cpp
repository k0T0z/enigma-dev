/** Copyright (C) 2023-2024 Saif Kandil
***
*** This file is a part of the ENIGMA Development Environment.
***
*** ENIGMA is free software: you can redistribute it and/or modify it under the
*** terms of the GNU General Public License as published by the Free Software
*** Foundation, version 3 of the license or any later version.
***
*** This application and its source code is distributed AS-IS, WITHOUT ANY
*** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*** details.
***
*** You should have received a copy of the GNU General Public License along
*** with this code. If not, see <http://www.gnu.org/licenses/>
**/

#include "c_stats_and_achievements.h"

namespace steamworks {

////////////////////////////////////////////////////////
// Public fields & functions
////////////////////////////////////////////////////////

c_stats_and_achievements::c_stats_and_achievements()
    : stats_valid_(false),
      steam_user_(NULL),
      steam_user_stats_(NULL),
      c_game_id_(SteamUtils()->GetAppID()),
      m_CallbackUserStatsReceived(this, &c_stats_and_achievements::on_user_stats_received),
      m_CallbackUserStatsStored(this, &c_stats_and_achievements::on_user_stats_stored),
      m_CallbackAchievementStored(this, &c_stats_and_achievements::on_achievement_stored) {
  c_stats_and_achievements::steam_user_ = SteamUser();
  c_stats_and_achievements::steam_user_stats_ = SteamUserStats();

  c_stats_and_achievements::request_current_stats();
}

bool c_stats_and_achievements::stats_valid() { return c_stats_and_achievements::stats_valid_; }

bool c_stats_and_achievements::set_achievement(const std::string& achievement_name) {
  return c_stats_and_achievements::steam_user_stats_->SetAchievement(achievement_name.c_str());
}

bool c_stats_and_achievements::get_achievement(const std::string& achievement_name, bool* achieved) {
  return c_stats_and_achievements::steam_user_stats_->GetAchievement(achievement_name.c_str(), achieved);
}

bool c_stats_and_achievements::clear_achievement(const std::string& achievement_name) {
  return c_stats_and_achievements::steam_user_stats_->ClearAchievement(achievement_name.c_str());
}

bool c_stats_and_achievements::set_stat_int(const std::string& stat_name, const int32 value) {
  return c_stats_and_achievements::steam_user_stats_->SetStat(stat_name.c_str(), value);
}

bool c_stats_and_achievements::get_stat_int(const std::string& stat_name, int32* value) {
  return c_stats_and_achievements::steam_user_stats_->GetStat(stat_name.c_str(), value);
}

bool c_stats_and_achievements::set_stat_float(const std::string& stat_name, const float value) {
  return c_stats_and_achievements::steam_user_stats_->SetStat(stat_name.c_str(), value);
}

bool c_stats_and_achievements::get_stat_float(const std::string& stat_name, float* value) {
  return c_stats_and_achievements::steam_user_stats_->GetStat(stat_name.c_str(), value);
}

bool c_stats_and_achievements::set_stat_average_rate(const std::string& stat_name, const float count_this_session,
                                                     const double session_length) {
  return c_stats_and_achievements::steam_user_stats_->UpdateAvgRateStat(stat_name.c_str(), count_this_session,
                                                                        session_length);
}

// float c_stats_and_achievements::get_stat_average_rate(const std::string& stat_name) {
//   return c_stats_and_achievements::get_stat_float(stat_name);
// }

void c_stats_and_achievements::on_user_stats_received(UserStatsReceived_t* pCallback) {
  if (c_stats_and_achievements::c_game_id_.ToUint64() != pCallback->m_nGameID) return;

  if (c_stats_and_achievements::stats_valid_) return;

  if (k_EResultOK == pCallback->m_eResult) {
    DEBUG_MESSAGE("Calling RequestCurrentStats succeeded.", M_INFO);

    c_stats_and_achievements::stats_valid_ = true;

  } else {
    DEBUG_MESSAGE(
        "Calling RequestCurrentStats failed. Only returns false if there is no user logged in; otherwise, true." +
            std::to_string(pCallback->m_eResult),
        M_INFO);
  }
}

void c_stats_and_achievements::on_user_stats_stored(UserStatsStored_t* pCallback) {
  if (c_stats_and_achievements::c_game_id_.ToUint64() != pCallback->m_nGameID) return;

  if (k_EResultOK == pCallback->m_eResult) {
    DEBUG_MESSAGE("Calling StoreStats succeeded.", M_INFO);
  } else if (k_EResultInvalidParam == pCallback->m_eResult) {
    DEBUG_MESSAGE("Calling StoreStats with some stats that are failed to validate. Retrying ...", M_INFO);
    UserStatsReceived_t callback;
    callback.m_eResult = k_EResultOK;
    callback.m_nGameID = c_stats_and_achievements::c_game_id_.ToUint64();
    on_user_stats_received(&callback);
    //c_stats_and_achievements::store_stats();
  } else {
    DEBUG_MESSAGE("Calling StoreStats failed. Retrying ..." + std::to_string(pCallback->m_eResult), M_INFO);
    //c_stats_and_achievements::store_stats();
  }
}

void c_stats_and_achievements::on_achievement_stored(UserAchievementStored_t* pCallback) {
  if (c_stats_and_achievements::c_game_id_.ToUint64() != pCallback->m_nGameID) return;

  if (0 == pCallback->m_nMaxProgress) {
    DEBUG_MESSAGE("Achievement '" + std::string(pCallback->m_rgchAchievementName) + "' unlocked!.", M_INFO);
  } else {
    DEBUG_MESSAGE("Achievement '" + std::string(pCallback->m_rgchAchievementName) + "' progress callback, (" +
                      std::to_string(pCallback->m_nCurProgress) + "," + std::to_string(pCallback->m_nMaxProgress) +
                      ").",
                  M_INFO);
  }
}

bool c_stats_and_achievements::store_stats() {
  return c_stats_and_achievements::steam_user_stats_->StoreStats();
}

bool c_stats_and_achievements::reset_all_stats() {
  return c_stats_and_achievements::steam_user_stats_->ResetAllStats(false);
}

bool c_stats_and_achievements::reset_all_stats_achievements() {
  return c_stats_and_achievements::steam_user_stats_->ResetAllStats(true);
}

bool c_stats_and_achievements::request_current_stats() {
  return c_stats_and_achievements::steam_user_stats_->RequestCurrentStats();
}

}  // namespace steamworks

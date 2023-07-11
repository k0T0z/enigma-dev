#ifndef GAMECLIENT_LEADERBOARDS_H
#define GAMECLIENT_LEADERBOARDS_H

#include "game_client.h"

namespace steamworks {

class c_game_client;

class c_leaderboards {
 public:
  c_leaderboards();
  ~c_leaderboards() = default;

  void create_leaderboard(const std::string &leaderboard_name, const ELeaderboardSortMethod leaderboard_sort_method,
                          const ELeaderboardDisplayType leaderboard_display_type);
  void upload_score(
      const std::string &leaderboard_name, const int score,
      const ELeaderboardUploadScoreMethod leaderboard_upload_score_method = k_ELeaderboardUploadScoreMethodNone);

 private:
  bool loading_;

  // Called when SteamUserStats()->FindOrCreateLeaderboard() returns asynchronously
  void on_find_leaderboard(LeaderboardFindResult_t *pFindLearderboardResult, bool bIOFailure);
  CCallResult<c_leaderboards, LeaderboardFindResult_t> m_SteamCallResultCreateLeaderboard;

  // Called when SteamUserStats()->UploadLeaderboardScore() returns asynchronously
  void on_upload_score(LeaderboardScoreUploaded_t *pFindLearderboardResult, bool bIOFailure);
  CCallResult<c_leaderboards, LeaderboardScoreUploaded_t> m_SteamCallResultUploadScore;
};
}  // namespace steamworks

#endif  // GAMECLIENT_LEADERBOARDS_H

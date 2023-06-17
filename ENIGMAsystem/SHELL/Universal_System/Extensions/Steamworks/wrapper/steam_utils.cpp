#include "steam_utils.h"

namespace steamworks {

uint32 steam_utils::get_app_id() {
  if (!steam_client::is_valid()) {
    DEBUG_MESSAGE("Calling steam_utils::get_app_id but not initialized, consider calling steam_client::init first",
                  M_ERROR);
    return -1;
  }

  return (AppId_t)SteamUtils()->GetAppID();
}

}  // namespace steamworks

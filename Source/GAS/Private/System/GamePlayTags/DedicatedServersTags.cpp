#include "System/GamePlayTags/DedicatedServersTags.h"

namespace DedicatedServersTags
{
	namespace GameSessionsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreateGameSession, "DedicatedServersTags.GameSessionsAPI.CreateGameSession", "Creates a new Game Session on the GameSessions API");
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreatePlayerSession, "DedicatedServersTags.GameSessionsAPI.CreatePlayerSession", "Creates a new Player Session on the GameSessions API");
	}
}
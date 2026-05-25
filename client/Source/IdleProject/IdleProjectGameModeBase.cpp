#include "IdleProjectGameModeBase.h"
#include "CharacterSystem/IdleCharacter.h"

AIdleProjectGameModeBase::AIdleProjectGameModeBase()
{
	DefaultPawnClass = AIdleCharacter::StaticClass();
}

#include "screens/EyesState.h"
#include "config/BoardConfig.h"

bool EyesState::finished() const { return elapsedMs_ >= board::timing::SCREEN_DURATION_MS; }

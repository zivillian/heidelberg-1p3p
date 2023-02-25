#include "config.h"

PhaseState::PhaseState()
    :CurrentState(State::WaitingForOff)
    ,DesiredPhases(3)
{}
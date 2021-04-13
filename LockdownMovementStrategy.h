#pragma once
#include "MovementStrategy.h"

class LockdownMovementStrategy : public MovementStrategy {
    public:
    double execute() override;
};
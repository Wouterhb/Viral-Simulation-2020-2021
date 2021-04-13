#pragma once
#include "MovementStrategy.h"

class RegularMovementStrategy : public MovementStrategy {
    public:
    double execute() override;
};
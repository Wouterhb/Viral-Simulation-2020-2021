#pragma once
#include "MovementStrategy.h"

class LockdownMovementStrategy : public MovementStrategy
{
public:
    double movement(double point, double speed, double dt) override;
};
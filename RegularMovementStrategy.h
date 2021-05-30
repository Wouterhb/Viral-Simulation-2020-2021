#pragma once
#include "MovementStrategy.h"

class RegularMovementStrategy : public MovementStrategy
{
public:
    double movement(double point, double speed, double dt) override;
};
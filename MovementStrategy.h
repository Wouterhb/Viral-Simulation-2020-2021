#pragma once

class MovementStrategy
{
public:
    virtual double movement(double point, double speed, double dt) = 0;
};
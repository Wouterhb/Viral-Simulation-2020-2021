#include "RegularMovementStrategy.h"

double RegularMovementStrategy::movement(double point, double speed, double dt)
{
    return point + speed * dt;
}

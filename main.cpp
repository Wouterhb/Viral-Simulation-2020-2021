// Corona Simulation - basic simulation of a human transmissable virus
// Copyright (C) 2020  wbrinksma

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "simulation.h"
#include "html_canvas.h"
#include "ChartJS_handler.h"
#include "LockdownMovementStrategy.h"
#include "RegularMovementStrategy.h"
#include <iostream>
#include <random>
#include <math.h>
#include <memory>

// Constants to control the simulation
const int SUBJECT_COUNT = 100;
const int SIM_WIDTH = 800;
const int SIM_HEIGHT = 500;
const int SUBJECT_RADIUS = 5;
const float SOCIAL_DISTANCE_PERCENTAGE = 0.75; // Between 0 and 1

// Initialize both strategies
LockdownMovementStrategy lockdownMovementStrategy;
RegularMovementStrategy regularMovementStrategy;

int main()
{
    corsim::Simulation simulation(SIM_WIDTH, SIM_HEIGHT, std::make_unique<corsim::HTMLCanvas>(30, 150, SIM_WIDTH, SIM_HEIGHT),
                                  std::make_unique<corsim::ChartJSHandler>());

    // Code to randomly generate certain numbers, which is done by using certain distributions
    std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    std::uniform_real_distribution<double> dist_w(1.0, SIM_WIDTH);
    std::uniform_real_distribution<double> dist_h(1.0, SIM_HEIGHT);
    std::uniform_real_distribution<double> dist_dx(-1.0, 1.0);
    std::uniform_real_distribution<double> dist_dy(-1.0, 1.0);

    for (int i = 0; i < SUBJECT_COUNT; ++i)
    {
        // Randomly generate x position for the subject
        double x = dist_w(mt);
        // Randomly generate y position for the subject
        double y = dist_h(mt);

        // Create the subject
        corsim::Subject subject(x, y, SUBJECT_RADIUS, false);

        // Set subject speed on x axis
        subject.set_dx(dist_dx(mt));
        // Set subject speed on y axis
        subject.set_dy(dist_dy(mt));

        // Set movement strategy of the subject
        if (i < SUBJECT_COUNT * SOCIAL_DISTANCE_PERCENTAGE)
        {
            // SOCIAL_DISTANCE_PERCENTAGE
            subject.setMovementStrategy(&lockdownMovementStrategy);
        }
        else
        {
            // The rest
            subject.setMovementStrategy(&regularMovementStrategy);
        }

        // Infect only the last subject
        if (i == SUBJECT_COUNT - 1)
        {
            subject.infect();
        }

        // Add subject to simulation
        simulation.add_subject(std::move(subject));
    }

    // Run simulation
    simulation.run();
}
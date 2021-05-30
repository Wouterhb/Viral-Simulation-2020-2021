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
#include <emscripten.h>
#include <math.h>

namespace corsim
{

    Simulation::Simulation(int width, int height, std::unique_ptr<Canvas> canvas, std::unique_ptr<StatisticsHandler> statisticsHandler) : _sim_width{width}, _sim_height{height}, _canvas{std::move(canvas)}, _statisticsHandler{std::move(statisticsHandler)} {}

    void Simulation::add_subject(Subject &&subject)
    {
        this->_subjects.emplace_back(std::move(subject));
    }

    void Simulation::run()
    {
        if (running)
        {
            return;
        }

        running = true;

        while (true)
        {
            this->tick();
            emscripten_sleep(tick_speed);
        }
    }

    int counter = 0;

    void Simulation::tick()
    {
        counter++;

        double dt = tick_speed / 10.0;

        std::vector<Subject *> collision_checker;

        for (Subject &subject : _subjects)
        {
            collision_checker.emplace_back(&subject);

            wall_collision(subject);
        }

        for (int i = collision_checker.size() - 1; i < collision_checker.size(); i--)
        {
            Subject *current_checking = collision_checker.at(i);
            collision_checker.erase(collision_checker.end());

            for (Subject *subject : collision_checker)
            {
                subject_collision(*current_checking, *subject);
            }
        }

        int numberInfected = 0;

        for (Subject &subject : _subjects)
        {
            subject.move(dt);

            if (subject.infected())
            {
                numberInfected++;
                subject.infectedDurationCountdown();
            }
            else if (subject.immune())
            {
                subject.immuneDurationCountdown();
            }
        }

        if (counter % 30 == 0)
        {
            _statisticsHandler.get()->communicate_number_infected(counter / 30, numberInfected);
        }

        draw_to_canvas();
    }

    void Simulation::draw_to_canvas()
    {
        _canvas.get()->clear();
        _canvas.get()->draw_rectangle(0, 0, 1, _sim_height, BLACK);
        _canvas.get()->draw_rectangle(0, 0, _sim_width, 1, BLACK);
        _canvas.get()->draw_rectangle(0, _sim_height - 1, _sim_width, 1, BLACK);
        _canvas.get()->draw_rectangle(_sim_width - 1, 0, 1, _sim_height, BLACK);

        for (Subject &subject : _subjects)
        {
            CanvasColor color = BLUE;

            if (subject.infected())
            {
                color = RED;
            }
            else if (subject.immune())
            {
                color = GREEN;
            }

            _canvas.get()->draw_ellipse(subject.x(), subject.y(), subject.radius(), color);
        }
    }

    void Simulation::wall_collision(Subject &subject)
    {
        if (subject.x() - subject.radius() + subject.dx() < 0 ||
            subject.x() + subject.radius() + subject.dx() > _sim_width)
        {
            subject.set_dx(subject.dx() * -1);
        }
        if (subject.y() - subject.radius() + subject.dy() < 0 ||
            subject.y() + subject.radius() + subject.dy() > _sim_height)
        {
            subject.set_dy(subject.dy() * -1);
        }
        if (subject.y() + subject.radius() > _sim_height)
        {
            subject.set_y(_sim_height - subject.radius());
        }
        if (subject.y() - subject.radius() < 0)
        {
            subject.set_y(subject.radius());
        }
        if (subject.x() + subject.radius() > _sim_width)
        {
            subject.set_x(_sim_width - subject.radius());
        }
        if (subject.x() - subject.radius() < 0)
        {
            subject.set_x(subject.radius());
        }
    }

    double distance(Subject &subject1, Subject &subject2)
    {
        return sqrt(pow(subject1.x() - subject2.x(), 2) + pow(subject1.y() - subject2.y(), 2));
    }

    void Simulation::subject_collision(Subject &subject1, Subject &subject2)
    {
        double dist = distance(subject1, subject2);

        if (dist < subject1.radius() + subject2.radius())
        {
            if (subject1.infected() || subject2.infected())
            {
                subject1.infect();
                subject2.infect();
            }

            double theta1 = subject1.angle();
            double theta2 = subject2.angle();
            double phi = atan2(subject1.x() - subject2.x(), subject1.y() - subject2.y());

            double dx1F = ((2.0 * cos(theta2 - phi)) / 2) * cos(phi) + sin(theta1 - phi) * cos(phi + M_PI / 2.0);
            double dy1F = ((2.0 * cos(theta2 - phi)) / 2) * sin(phi) + sin(theta1 - phi) * sin(phi + M_PI / 2.0);

            double dx2F = ((2.0 * cos(theta1 - phi)) / 2) * cos(phi) + sin(theta2 - phi) * cos(phi + M_PI / 2.0);
            double dy2F = ((2.0 * cos(theta1 - phi)) / 2) * sin(phi) + sin(theta2 - phi) * sin(phi + M_PI / 2.0);

            subject1.set_dx(dx1F);
            subject1.set_dy(dy1F);
            subject2.set_dx(dx2F);
            subject2.set_dy(dy2F);

            static_collision(subject1, subject2, false);
        }
    }

    void Simulation::static_collision(Subject &subject1, Subject &subject2, bool emergency)
    {
        double overlap = subject1.radius() + subject2.radius() - distance(subject1, subject2);
        Subject &smallerObject = subject1.radius() < subject2.radius() ? subject1 : subject2;
        Subject &biggerObject = subject1.radius() > subject2.radius() ? subject1 : subject2;

        if (emergency)
        {
            Subject &temp = smallerObject;
            smallerObject = biggerObject;
            biggerObject = temp;
        }

        double theta = atan2((biggerObject.y() - smallerObject.y()), (biggerObject.x() - smallerObject.x()));

        smallerObject.set_x(smallerObject.x() - overlap * cos(theta));
        smallerObject.set_y(smallerObject.y() - overlap * sin(theta));

        if (distance(subject1, subject2) < subject1.radius() + subject2.radius())
        {
            if (!emergency)
            {
                static_collision(subject1, subject2, true);
            }
        }
    }

}
/*
** EPITECH PROJECT, 2024
** ECS tests
** File description:
** ACore
*/

#pragma once

#include "Entities/AEntities.hpp"
#include "Systeme/ASysteme.hpp"
#include <vector>

class ACore {
    public:
        ACore();
        ~ACore();
    private:
        std::vector<AEntities> _entities;
        std::vector<ASysteme> _systems;
};
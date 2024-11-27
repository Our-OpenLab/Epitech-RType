/*
** EPITECH PROJECT, 2024
** ECS tests
** File description:
** ACore
*/

#pragma once

#include "ICore.hpp"
#include "Entities/IEntities.hpp"
#include "Systeme/ISysteme.hpp"
#include <vector>

class ACore : public ICore {
    public:
        ACore();
        ~ACore();
    private:
        std::vector<IEntities> _entities;
        std::vector<ISysteme> _systems;
};
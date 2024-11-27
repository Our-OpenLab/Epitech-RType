/*
** EPITECH PROJECT, 2024
** ECS tests
** File description:
** AEntities
*/

#pragma once

#include "IEntities.hpp"
#include "Component/IComponent.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class AEntities : public IEntities {
    public:
        AEntities(std::string name);
        ~AEntities();
    private:
        std::unordered_map<std::string, IComponent> _components;
};
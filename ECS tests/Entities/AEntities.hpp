/*
** EPITECH PROJECT, 2024
** ECS tests
** File description:
** AEntities
*/

#pragma once

#include "Component/Component.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class AEntities {
    public:
        AEntities(std::string name);
        ~AEntities();
    private:
        std::unordered_map<std::string, Component> _components;
};
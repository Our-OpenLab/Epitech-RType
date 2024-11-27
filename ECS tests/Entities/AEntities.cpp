/*
** EPITECH PROJECT, 2024
** ECS tests
** File description:
** AEntities
*/

#include "AEntities.hpp"

AEntities::AEntities(std::string name){
    this->_components[name] = IComponent();
}

AEntities::~AEntities(){}

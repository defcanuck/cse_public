#pragma once

#include "ClassDef.h"
#include "ecs/ECS_Context.h"

#include <memory>

#define CREATE_COMPONENT(component_type) \
std::static_pointer_cast<Component>(CREATE_CLASS(component_type));

#define ADD_COMPONENT(entity, component_type, ...) \
entity->addComponent<component_type>(CREATE_CLASS(component_type, __VA_ARGS__)); \



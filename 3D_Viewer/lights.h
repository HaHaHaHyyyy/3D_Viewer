//
// Created by progr on 30.05.2026.
//

#ifndef HOSPITALAPP_LIGHTS_H
#define HOSPITALAPP_LIGHTS_H

#include "utils.h"

void init_lights(vec3 color, float ambient, float diffuse);
void set_light_color(vec3 color);
void set_light_intensity(float ambient, float diffuse);

#endif
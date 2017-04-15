#include "SpaceModel.h"
#include <stdlib.h>
#include <stdio.h>

SpaceModel::SpaceModel(RectangleD bounds, std::vector<Object> &objects) {
    this->bounds = bounds;
    // deep copy
    this->objects = objects;
}

void 
SpaceModel::remove_objects_outside_bounds() {
    std::vector<Object>::iterator it = objects.begin();
    while (it != objects.end()) {
        if (!point2d_is_in_rectangled(it->position, this->bounds)) {
            it = objects.erase(it);
        } else {
            ++it;
        }
    }
}

void SpaceModel::update(GS_FLOAT dt) {
}


SpaceModel::~SpaceModel() {
}

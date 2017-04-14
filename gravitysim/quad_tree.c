#include <stdbool.h>
#include <stdlib.h>
#include "quad_tree.h"
#include "build_config.h"


quadtree::quadtree(RectangleD bounds) {
    this->bounds = bounds;
    for (int i = 0; i < 4; i++) {
        this->children[i] = NULL;
    }
    this->object = NULL;
    this->has_children = 0;
    this->com = object_zero();
}

~quadtree::quadtree() {
    for (int i = 0; i < 4; i++) {
        if (this->children[i]) {
            delete this->children[i];
        }
    }
}

void 
quadtree::add_objects(ObjectArray *objects) {
    size_t i;
    for (i = 0; i < objects->len; i++) {
        if (point2d_is_in_rectangled(objects->objects[i].position, this->bounds)) {
            this->add_object(&objects->objects[i]);
        }
    }
}

void 
quadtree::add_object(Object *object) {
    if (this->has_children) {
        q = point2d_nquad_of_rectangled(object->position, this->bounds);
        if (this->children[q]) {
            this->children[q]->add_object(object);
        } else {
            this->children[q] = new quadtree(rectangled_nquad(this->bounds, q));
            this->add_object(this->children[q], object);
        }
        this->com->add(*object);
    } else {
        if (this->object) {
            q = point2d_nquad_of_rectangled(object->position, this->bounds);
            if (!this->children[q]) {
                this->children[q] = new quadtree(rectangled_nquad(this->bounds, q));
            }
            this->children[q]->add_object(object);
            
            q = point2d_nquad_of_rectangled(this->object->position, this->bounds);
            if (!this->children[q]) {
                this->children[q] = new quadtree(rectangled_nquad(this->bounds, q));
            }
            this->children[q]->add_object(this->object);
            
            this->has_children = 1;
            this->object = NULL;
            this->com->add(*object);
        } else {
            this->object = object;
            this->com = *object;
        }
    }
}

Point2D 
quadtree::get_force_on_object(Object *object) {
    GS_FLOAT s, d;
    Point2D dr, result = point2d_zero();    
    if (!this->has_children) {
        if (this->object != object) {
            result = object_calculate_force(*object, this->com);
        }
    } else {
        s = MAX(this->bounds.size.x, this->bounds.size.y);
        dr = point2d_sub(object->position, this->com.position);
        d = point2d_length(dr);
        if ((s/d) < SD_TRESHOLD) {
            result = object_calculate_force(*object, this->com);
        } else {
            result = point2d_zero();
            for (int i = 0; i < 4; i++) {
                if (this->children[i] != NULL) {
                    result = point2d_add(result,
                                         quadtree_get_force_on_object(this->children[i], object));
                }
            }
        }
    }
    return result;
}

void 
quadtree::apply_to_objects(ObjectArray *objects, GS_FLOAT dt) {
    size_t i;
    Object *object;
    for (i = 0; i < objects->len; i++) {
        object = &objects->objects[i];
        Point2D acc = get_force_on_object(object);
        Point2D dv = point2d_multiply(acc, dt);
        object->speed = point2d_add(object->speed, dv);
    }    
}


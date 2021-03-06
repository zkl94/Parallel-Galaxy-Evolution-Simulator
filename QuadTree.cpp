#include <stdbool.h>
#include <stdlib.h>
#include "QuadTree.h"
#include "build_config.h"
#include "log.h"

static int log_level = LOG_INFO;

QuadTree::QuadTree(RectangleD bounds) {
    ENTER();
    this->bounds = bounds;
    for (int i = 0; i < 4; i++) {
        this->children[i] = NULL;
    }
    this->object = NULL;
    this->has_children = 0;
    this->com = Object::make_zero();
    LEAVE();
}

QuadTree::~QuadTree() {
    for (int i = 0; i < 4; i++) {
        if (this->children[i] != NULL ) {
            delete this->children[i];
        }
    }
}

void 
QuadTree::add_objects(std::vector<Object> &objects) {
    ENTER();
    size_t i;
    DBG("size: %ld\n", objects.size());
    for (i = 0; i < objects.size(); i++) {
        if (point2d_is_in_rectangled(objects[i].position, this->bounds)) {
            this->add_object(&objects[i]);
        }
    }
    LEAVE();
}

void 
QuadTree::add_object(Object *object) {
    ENTER();
    int q;
    if (this->has_children) {
        q = point2d_nquad_of_rectangled(object->position, this->bounds);
        if (this->children[q]) {
            this->children[q]->add_object(object);
        } else {
            this->children[q] = new QuadTree(rectangled_nquad(this->bounds, q));
            this->children[q]->add_object(object);
        }
        this->com = Object::add(this->com, *object);
    } else {
        if (this->object) {
            q = point2d_nquad_of_rectangled(object->position, this->bounds);
            if (!this->children[q]) {
                this->children[q] = new QuadTree(rectangled_nquad(this->bounds, q));
            }
            this->children[q]->add_object(object);
            
            q = point2d_nquad_of_rectangled(this->object->position, this->bounds);
            if (!this->children[q]) {
                this->children[q] = new QuadTree(rectangled_nquad(this->bounds, q));
            }
            this->children[q]->add_object(this->object);
            
            this->has_children = 1;
            this->object = NULL;
            this->com = Object::add(this->com, *object);
        } else {
            this->object = object;
            this->com = Object::add(this->com, *object);
        }
    }
    LEAVE();
}

Point2D 
QuadTree::get_force_on_object(Object *object) {
    ENTER();
    GS_FLOAT s, d;
    Point2D dr, result = point2d_zero();    
    if (!this->has_children) {
        if (this->object != object) {
            result = Object::calculate_force(*object, this->com);
        }
    } else {
        s = MAX(this->bounds.size.x, this->bounds.size.y);
        dr = point2d_sub(object->position, this->com.position);
        d = point2d_length(dr);
        if ((s/d) < SD_TRESHOLD) {
            result = Object::calculate_force(*object, this->com);
        } else {
            result = point2d_zero();
            for (int i = 0; i < 4; i++) {
                if (this->children[i] != NULL) {
                    result = point2d_add(result, this->children[i]->get_force_on_object(object));
                }
            }
        }
    }
    LEAVE();
    return result;
}

void 
QuadTree::apply_to_objects(std::vector<Object> &objects, GS_FLOAT dt) {
    ENTER();
    size_t i;
    Object *object;
    for (i = 0; i < objects.size(); i++) {
        object = &objects[i];
        Point2D acc = get_force_on_object(object);
        Point2D dv = point2d_multiply(acc, dt);
        //printf("pos.x:%f, pos.y:%f, acc.x:%f, acc.y:%f, dv.x:%f, dv.y:%f\n", 
        //        object->position.x, object->position.y, acc.x, acc.y, dv.x, dv.y);
        object->speed = point2d_add(object->speed, dv);
    }
    LEAVE();
}


#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include "array.h"
#include "math.h"

#define QUAD_TREE_LEAF_MAX_ENTITIES 1000
template< typename T >
struct Quad_Tree_Leaf {
    T entities[QUAD_TREE_LEAF_MAX_ENTITIES] {};
    int entity_count {};
};

template< typename T >
struct Quad_Tree_Node {
    Vec2 center {};
    Vec2 dimensions {};
    Quad_Tree_Node<T> *children[4] {};
    Quad_Tree_Leaf<T> *leaf = nullptr; // if not null => this node is a leaf
};

template< typename T >
struct Quad_Tree {
    Quad_Tree_Node<T> *root {};
    Vec2 root_origin {};
    Vec2 root_dimensions {};
    Array<Quad_Tree_Node<T>> quad_tree_nodes {};
    Array<Quad_Tree_Leaf<T>> quad_tree_leaves {};
    int levels {};

    Quad_Tree(Vec2 origin, Vec2 dimensions, int levels) : root_origin{origin}, root_dimensions{dimensions}, levels{levels} {
        quad_tree_nodes.reserve( (int) ceil((pow(4.0f,levels)-1.0f)/3.0f) );
        quad_tree_nodes.lock_capacity();
        quad_tree_leaves.reserve( (int) pow(4.0f, levels) );
        quad_tree_leaves.lock_capacity();
        push_new_root_node();
    }

    void push_new_root_node() {
        Quad_Tree_Node<T> root_node {};
        root_node.center = root_origin;
        root_node.dimensions = root_dimensions;
        root = quad_tree_nodes.push(root_node);
    }

    void clear() {
        quad_tree_nodes.clear();
        quad_tree_leaves.clear();
        push_new_root_node();
    }

    void add_entity_quad(const T &entity, Vec2 entity_pos, Vec2 entity_dim) {
        Vec2 half_dim = entity_dim/2.0f;
        add_entity(entity, {entity_pos.x() + half_dim.x(), entity_pos.y() + half_dim.y()});
        add_entity(entity, {entity_pos.x() + half_dim.x(), entity_pos.y() - half_dim.y()});
        add_entity(entity, {entity_pos.x() - half_dim.x(), entity_pos.y() + half_dim.y()});
        add_entity(entity, {entity_pos.x() - half_dim.x(), entity_pos.y() - half_dim.y()});
    }

    void add_entity(const T &entity, Vec2 pos) {
        bool in_bounds = pos_in_bounds(pos);
        if (!in_bounds) { return; }
        add_entity(root, entity, pos, 0);
    }

    void add_entity(Quad_Tree_Node<T> *node, const T &entity, Vec2 pos, int level) {

        // Leaf node base case
        if (level == levels-1) {
            if (!node->leaf) {
                node->leaf = quad_tree_leaves.push(Quad_Tree_Leaf<T>{});
            }
            if (node->leaf->entity_count >= QUAD_TREE_LEAF_MAX_ENTITIES) {
                fprintf(stderr, "Quad_Tree::add_entity: leaf entities array is full");
                exit(1);
            }

            // Search if entity already added in entities list
            // TODO: this linear search is slow, seek alternative
            bool found = false;
            for (int i = 0; i < node->leaf->entity_count; ++i) {
                if (entity == node->leaf->entities[i]) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                node->leaf->entities[node->leaf->entity_count] = entity;
                ++node->leaf->entity_count;
            }

            return;
        }

        // Internal node case
        Child_Info child_info = get_child_info(node, pos);

        if (!node->children[child_info.child_i]) {
            Quad_Tree_Node<T> child {};
            child.dimensions = child_info.child_dim;
            child.center = child_info.child_center;
            node->children[child_info.child_i] = quad_tree_nodes.push(child);
        }

        add_entity(node->children[child_info.child_i], entity, pos, level+1);
    }

    struct Search_Result {
        Quad_Tree_Leaf<T> *leaves[4] {};
    };

    Search_Result search(Vec2 pos, Vec2 dim) const {
        Search_Result result {};
        Vec2 half_dim = dim/2.0f;
        result.leaves[0] = search({pos.x() + half_dim.x(), pos.y() + half_dim.y()});
        result.leaves[1] = search({pos.x() + half_dim.x(), pos.y() - half_dim.y()});
        result.leaves[2] = search({pos.x() - half_dim.x(), pos.y() + half_dim.y()});
        result.leaves[3] = search({pos.x() - half_dim.x(), pos.y() - half_dim.y()});
        return result;
    }

    // Returns null if no enemies in pos's quadrant
    Quad_Tree_Leaf<T> *search(Vec2 pos) const {
        Vec2 half_dim = root_dimensions/2.0f;
        bool in_bounds = pos_in_bounds(pos);
        if (!in_bounds) { return nullptr; }
        return search(root, pos, 0);
    }

    // Returns null if no enemies in pos's quadrant
    Quad_Tree_Leaf<T> *search(Quad_Tree_Node<T> *node, Vec2 pos, int level) const {
        // Leaf node base case
        if (level == levels-1) {
            return node->leaf;
        }

        // Internal node case
        Child_Info child_info = get_child_info(node, pos);

        if (!node->children[child_info.child_i]) { return nullptr; }

        return search(node->children[child_info.child_i], pos, level+1);
    }

    bool pos_in_bounds(Vec2 pos) const {
        Vec2 half_dim = root_dimensions/2.0f;
        float root_x_min = root_origin.x() - half_dim.x();
        float root_x_max = root_origin.x() + half_dim.x();
        float root_y_min = root_origin.y() - half_dim.y();
        float root_y_max = root_origin.y() + half_dim.y();
        return pos.x() > root_x_min && pos.x() < root_x_max && pos.y() > root_y_min && pos.y() < root_y_max;
    }

    //
    // Helpers
    //

    struct Child_Info {
        int child_i {};
        Vec2 child_center {};
        Vec2 child_dim {};
    };

    Child_Info get_child_info(Quad_Tree_Node<T> *node, Vec2 pos) const {
        int child_i {};
        Vec2 child_center {};
        const Vec2 child_dim = node->dimensions / 2.0f;

        if (pos.x() < node->center.x()) {
            if (pos.y() < node->center.y()) {
                child_i = 0;
                child_center.x() = node->center.x() - child_dim.x()/2.0f;
                child_center.y() = node->center.y() - child_dim.y()/2.0f; 
            } else {
                child_i = 1;
                child_center.x() = node->center.x() - child_dim.x()/2.0f;
                child_center.y() = node->center.y() + child_dim.y()/2.0f; 
            }
        }
        else {
            if (pos.y() < node->center.y()) {
                child_i = 2;
                child_center.x() = node->center.x() + child_dim.x()/2.0f;
                child_center.y() = node->center.y() - child_dim.y()/2.0f; 
            } else {
                child_i = 3;
                child_center.x() = node->center.x() + child_dim.x()/2.0f;
                child_center.y() = node->center.y() + child_dim.y()/2.0f; 
            }
        }

        return {child_i, child_center, child_dim};
    }
};

#endif
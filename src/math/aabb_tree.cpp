#include "math/aabb_tree.hpp"

void AABBTree::init(int start, int end) {
    tree.resize(end);
    for (int i = start; i < end; i++) {
        AABBTreeNode* node = (AABBTreeNode*) malloc(sizeof(AABBTreeNode));
        node->next = i+1;
        tree[i] = node;
    }
    tree[end-1]->next = -1;
}

void AABBTree::clear() {
    root = -1;
    free_list = -1;
    insertionCount = 0;
    for(size_t i = 0; i < tree.size(); i++) {
        tree[i]->height = -1;
        tree[i]->item = NULL;
        tree[i]->next = i+1;
    }
    tree[tree.size()-1]->next = tree[tree.size()-1]->height = -1;
}

void AABBTree::insert(const Rectf &aabb, MovingObject *obj) {
    int index = allocateNode();
    tree[index]->originalAABB = aabb;
    tree[index]->aabb = aabb.grown(fattenFactor);
    tree[index]->item = obj;
    tree[index]->height = 0;
    insertLeaf(index);
    // TODO Store index in map
}

bool AABBTree::remove(MovingObject* obj) {
    size_t index = 0; // TODO Get index from map
    assert(0 <= index && index < tree.size());
    removeLeaf(index);
    freeNode(index);
}

void AABBTree::freeNode(int index) {
    assert(0 <= index && index < tree.size());
    tree[index]->next = free_list;
    tree[index]->height = -1;
    tree[index]->item = NULL;
    free_list = index;
}

int AABBTree::allocateNode() {
    if(free_list != -1) {
        int id = free_list;
        free_list = tree[id]->next;
        // Do some stuff TODO

        return id;
    }
    // We need to allocate new nodes
}

void AABBTree::insertLeaf(int leaf) {
    insertionCount++;
    // If the tree is empty, we become the root
    if (root == -1) {
        root = leaf;
        tree[leaf]->parent = -1;
        return; // That was simple
    }

    // Find the best insertion position using heuristics
    // We basically have 3 options :
    //  1. Insert somewhere left
    //  2. Insert somewhere right
    //  3. Insert HERE
    //  => use heuristic to figure it out
    auto leafAABB = tree[leaf]->aabb;
    auto index = root;
    while (tree[index]->child1 != -1) { // Iterate until we have found a leaf

        auto child1 = tree[index]->child1;
        auto child2 = tree[index]->child2;

        auto area   = tree[index]->aabb.perimeter();

        auto combinedAABB = tree[index]->aabb.unify(leafAABB);
        auto combinedArea = combinedAABB.perimeter();

        auto cost = 2*combinedArea;
        auto inheritanceCost = 2 * (combinedArea - area);

        // TODO Compute cost for child 1
        // Compute cost for child 2

        // Compare costs and decide

    }
    // index will be the sibling of leaf
    int sibling = index;
    int oldParent = tree[sibling]->parent;
    int newParent = allocateNode();
    tree[newParent]->parent = oldParent;
    tree[newParent]->item = NULL;
    tree[newParent]->aabb = leafAABB.unify(tree[sibling]->aabb);
    tree[newParent]->height = tree[sibling]->height+1;

    if(oldParent != -1) {
        if (tree[oldParent]->child1 == sibling)
            tree[oldParent]->child1 = newParent;
        else
            tree[oldParent]->child2 = newParent;

    } else {
        // sibling was the root
        root = newParent;
    }
    tree[newParent]->child1 = sibling;
    tree[sibling]->parent = newParent;
    tree[newParent]->child2 = leaf;
    tree[leaf]->parent = newParent;

    // Last step: Fix heights and rebalance
    index = tree[leaf]->parent;
    while(index != -1) { // Iterate to root
        index = balance(index);
        auto child1 = tree[leaf]->child1;
        auto child2 = tree[leaf]->child2;

        tree[index]->height = 1+std::max(tree[child1]->height, tree[child2]->height);
        tree[index]->aabb = tree[child1]->aabb.unify(tree[child2]->aabb);

        index = tree[index]->parent;
    }
}

void AABBTree::removeLeaf(int leaf) {
    if (leaf == root) {
        root = -1;
        return;
    }

    auto parent = tree[leaf]->parent;
    auto grandparent = tree[parent]->parent;

    int sibling;
    if (tree[parent]->child1 == leaf)
    {
        sibling = tree[parent]->child2;
    } else {
        sibling = tree[parent]->child1;
    }

    if (grandparent != -1) {
        // Destroy parent and connect sibling to grandparent
        if(tree[grandparent]->child2 == parent) {
            tree[grandparent]->child2 = sibling;
        } else {
            tree[grandparent]->child1 = sibling;
        }
        tree[sibling]->parent = grandparent;
        freeNode(parent);

        int index = grandparent;
        while(index != -1) {
            index = balance(index);
            auto child1 = tree[leaf]->child1;
            auto child2 = tree[leaf]->child2;

            tree[index]->height = 1+std::max(tree[child1]->height, tree[child2]->height);
            tree[index]->aabb = tree[child1]->aabb.unify(tree[child2]->aabb);

            index = tree[index]->parent;
        }
    } else {
        root = sibling;
        tree[sibling]->parent = -1;
        freeNode(parent);
    }
}

// Balances the tree
int AABBTree::balance(int a) {
    if(tree[a]->child1 == -1 || tree[a]->height < 2) {
        return a;
    }
    int b = tree[a]->child1;
    int c = tree[a]->child2;

    int balance = tree[b]->height - tree[c]->height;
    if(balance > 1) {
        auto f = tree[c]->child1;
        auto g = tree[c]->child2;

        // swap a and c
        tree[c]->child1 = a;
        tree[c]->parent = tree[a]->parent;
        tree[a]->parent = c;
        // a's old parent should point to c
        auto cParent = tree[c]->parent;
        if (cParent != -1) {
            if (tree[cParent]->child1 == a) {
                    tree[cParent]->child1 = c;
            }
            else {
                assert(tree[cParent]->child2 == a);
                tree[cParent]->child2 = c;
            }
        }
        else {
            root = c;
        }
        // rotate
        if (tree[f]->height > tree[g]->height) {
            tree[c]->child2 = f;
            tree[a]->child2 = g;
            tree[g]->parent = a;

            tree[a]->aabb = tree[b]->aabb.unify(tree[g]->aabb);
            tree[c]->aabb = tree[a]->aabb.unify(tree[f]->aabb);

            tree[a]->height = 1 + std::max(tree[b]->height, tree[g]->height);
            tree[c]->height = 1 + std::max(tree[a]->height, tree[f]->height);
        }
        else {
            tree[c]->child2 = g;
            tree[a]->child2 = f;
            tree[f]->parent = a;

            tree[a]->aabb = tree[b]->aabb.unify(tree[f]->aabb);
            tree[c]->aabb = tree[a]->aabb.unify(tree[g]->aabb);

            tree[a]->height = 1 + std::max(tree[b]->height, tree[f]->height);
            tree[c]->height = 1 + std::max(tree[a]->height, tree[g]->height);
        }
        return c;
    }

    if(balance < -1) {
        auto d = tree[b]->child1;
        auto e = tree[b]->child2;

        // swap a and b
        tree[b]->child1 = a;
        tree[b]->parent = tree[a]->parent;
        tree[a]->parent = b;

        // a's old parent should point to b
        auto bParent = tree[b]->parent;
        if (bParent != -1) {
            if (tree[bParent]->child1 == a) {
                tree[bParent]->child1 = b;
            }
            else {
                assert(tree[bParent]->child2 == a);
                tree[bParent]->child2 = b;
            }
        }
        else {
            root = b;
        }

        // rotate
        if (tree[d]->height > tree[e]->height) {
            tree[b]->child2 = d;
            tree[a]->child1 = e;
            tree[e]->parent = a;

            tree[a]->aabb = tree[c]->aabb.unify(tree[e]->aabb);
            tree[b]->aabb = tree[a]->aabb.unify(tree[d]->aabb);

            tree[a]->height = 1 + std::max(tree[c]->height, tree[e]->height);
            tree[b]->height = 1 + std::max(tree[a]->height, tree[d]->height);
        }
        else {
            tree[b]->child2 = e;
            tree[a]->child1 = d;
            tree[d]->parent = a;

            tree[a]->aabb = tree[c]->aabb.unify(tree[d]->aabb);
            tree[b]->aabb = tree[a]->aabb.unify(tree[e]->aabb);

            tree[a]->height = 1 + std::max(tree[c]->height, tree[d]->height);
            tree[b]->height = 1 + std::max(tree[a]->height, tree[e]->height);
        }

        return b;
    }
    return a;
}

#include "math/aabb_tree.hpp"
#include <stack>

/// This is a growable LIFO stack with an initial capacity of N.
/// If the stack size exceeds the initial capacity, the heap is used
/// to increase the size of the stack.
template <typename T, int N>
class b2GrowableStack
{
public:
    b2GrowableStack()
    {
        m_stack = m_array;
        m_count = 0;
        m_capacity = N;
    }

    ~b2GrowableStack()
    {
        if (m_stack != m_array)
        {
            free(m_stack);
            m_stack = NULL;
        }
    }

    void Push(const T& element)
    {
        if (m_count == m_capacity)
        {
            T* old = m_stack;
            m_capacity *= 2;
            m_stack = (T*)malloc(m_capacity * sizeof(T));
            memcpy(m_stack, old, m_count * sizeof(T));
            if (old != m_array)
            {
                free(old);
            }
        }

        m_stack[m_count] = element;
        ++m_count;
    }

    T Pop()
    {
        --m_count;
        return m_stack[m_count];
    }

    int GetCount()
    {
        return m_count;
    }

private:
    T* m_stack;
    T m_array[N];
    int m_count;
    int m_capacity;
};


AABBTree::AABBTree(int start, int end ) {
    tree.resize(end);
    root = -1;
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

int AABBTree::insert(const Rectf &aabb, MovingObject *obj) {
    int index = allocateNode();
    tree[index]->originalAABB = aabb;
    tree[index]->aabb = aabb.grown(fattenFactor);
    tree[index]->item = obj;
    tree[index]->height = 0;
    insertLeaf(index);
    // TODO Store index in map
    return index;
}

bool AABBTree::remove(MovingObject* obj) {
    size_t index = 0; // TODO Get index from map
    assert(0 <= index && index < tree.size());
    removeLeaf(index);
    freeNode(index);
    return true;
}

void AABBTree::freeNode(int index) {
    assert(0 <= index && index < (int) tree.size());
    tree[index]->next = free_list;
    tree[index]->height = -1;
    tree[index]->item = NULL;
    free_list = index;
}

int AABBTree::allocateNode() {
    if(free_list == -1) {
        auto count = tree.size();
        tree.resize(2*tree.size());
        for(size_t i = count; i < 2*count; i++) {
            tree[i] = (AABBTreeNode*) malloc(sizeof(AABBTreeNode));
            tree[i]->next = i+1;
            tree[i]->height = -1;
        }
        tree[2*count-1]->next = -1;
        free_list = count;
    }
    int id = free_list;
    free_list = tree[id]->next;
    tree[id]->parent = -1;
    tree[id]->child1 = -1;
    tree[id]->child2 = -1;
    tree[id]->height = 0;
    tree[id]->item = NULL;
    return id;

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
        double cost1 = 0.0, cost2 = 0.0;
        if(tree[child1]->child1 == -1) {
            auto aabb_perim = leafAABB.unify(tree[child1]->aabb).perimeter();
            cost1 = aabb_perim + inheritanceCost;
        }else{
            cost1 = tree[child1]->aabb.perimeter() - leafAABB.unify(tree[child1]->aabb).perimeter() + inheritanceCost;
        }
        // Compute cost for child 2
        if(tree[child2]->child1 == -1) {
            auto aabb_perim = leafAABB.unify(tree[child2]->aabb).perimeter();
            cost2 = aabb_perim + inheritanceCost;
        }else{
            cost2 = tree[child2]->aabb.perimeter() - leafAABB.unify(tree[child2]->aabb).perimeter() + inheritanceCost;
        }
        // Compare costs and decide
        if(cost < cost1 && cost < cost2)
            break; // This is the optimal space to insert
        else if(cost1 < cost2)
            index = child1;
        else
            index = child2;
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
        tree[newParent]->child1 = sibling;
        tree[sibling]->parent = newParent;
        tree[newParent]->child2 = leaf;
        tree[leaf]->parent = newParent;
    } else {
        // sibling was the root
        root = newParent;
        tree[newParent]->child1 = sibling;
        tree[sibling]->parent = newParent;
        tree[newParent]->child2 = leaf;
        tree[leaf]->parent = newParent;
    }


    // Last step: Fix heights and rebalance
    index = tree[leaf]->parent;
    while(index != -1) { // Iterate to root
        index = balance(index);
        auto child1 = tree[index]->child1;
        auto child2 = tree[index]->child2;

        tree[index]->height = 1+std::max(tree[child1]->height, tree[child2]->height);
        tree[index]->aabb = tree[child1]->aabb.unify(tree[child2]->aabb);

        index = tree[index]->parent;
    }
}

void AABBTree::moveProxy(int index, const Rectf &aabb, const Vector &displacement) {
    assert(0 <= index && index < tree.size());
    assert((tree[index]->child1) == -1);

    tree[index]->originalAABB = aabb;
    if (tree[index]->aabb.contains(aabb)) {
        return;
    }
    removeLeaf(index);
    // Extend AABB
    auto b = aabb.grown(fattenFactor);
    // Predict AABB displacement
    auto d = displacement*displacementFactor;
    if (d.x < 0) {
        b.p1.x += d.x;
        b.p2.x -= d.x;
    }
    else {
        b.p2.x += d.x;
    }
    if(d.y < 0) {
        b.p1.y += d.y;
        b.p2.y -= d.y;
    }
    else {
        b.p2.y += d.y;
    }

    tree[index]->aabb = b;
    insertLeaf(index);
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
            auto child1 = tree[index]->child1;
            auto child2 = tree[index]->child2;

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

    int balance = tree[c]->height - tree[b]->height;
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

void AABBTree::search(const Rectf &r, std::function<void()> collision_ok, std::list<MovingObject *> &fill) {
    b2GrowableStack<int, 256> stack;

    stack.Push(root);
    while (stack.GetCount() > 0) {
        int index = stack.Pop();
        if(index == -1)
            continue;
        auto node = tree[index];
        if(node->aabb.contains(r)) {
            if (node->child1 == -1) {
                fill.push_back(node->item);
            } else {
                if (node->child1 != -1)
                    stack.Push(node->child1);
                if (node->child2 != -1)
                    stack.Push(node->child2);
            }
        }
    }
}
#ifndef SUPERTUX_HEADER_MATH_AABB_TREE_HPP_INCLUDED 
#define SUPERTUX_HEADER_MATH_AABB_TREE_HPP_INCLUDED

#include "broadphase.hpp"

/**
 * Implementation of an AABB-Tree inspired / copied from Box2D
 *
 * Maintains a balanced tree of AABBs
 */
 struct AABBTreeNode {
     // (Possibly) fattened AABB
     Rectf aabb;
     // This nodes "real" AABB
     Rectf originalAABB;

     // This tree nodes family. Are ints because the tree in an array
     int parent = -1;
     int child1 = -1;
     int child2 = -1;
     int next;
     int height = -1; // -1 for an unused variable, 0 for a child

     MovingObject* item; // The moving object stored at this node
 };
 class AABBTree  {
 public:
     AABBTree(int start = 0, int end = 16);
     int free_list = 0; /** Pointer to first element in free list */
     std::vector< AABBTreeNode* > tree;
     const float fattenFactor = 4.0f; // Amount in percent by which AABBs are fattened
     const float displacementFactor = 8.0f;

     int insertionCount = 0;
     int size = 16;
     int root;

     void init(int start, int end); /** Initialises the tree */
     void clear(); /** Clears the tree */
     int allocateNode(); /** Returns index of next free node (and initialises it) */
     int insert(const Rectf& aabb, MovingObject* obj);
     void moveProxy(int index, const Rectf& aabb, const Vector& displacement);
     void search(const Rectf& r, std::function<void()> collision_ok,  std::list< MovingObject* >& fill);
     void insertLeaf(int leaf);
     void removeLeaf(int leaf);
     bool remove(MovingObject* obj);
     int balance(int index);
     void freeNode(int index);
 };
#endif
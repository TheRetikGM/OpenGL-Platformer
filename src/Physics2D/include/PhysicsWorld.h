#pragma once
#include <vector>
#include "AABB.hpp"
#include "RigidBody.h"
#include "MeterUnitConverter.hpp"
#include "helper.hpp"
#include <list>
#include <cstdint>
#include <map>
#include <iterator>
#include <cstddef>
#include <stack>

/*
* This is my implementation of 2D physics engine.
* You can have multiple physics worlds which each handle its own bodies.
* Everything is in meters. That includes size of bodies etc.
* * For conversion you can define your units-per-meter ratio and then use functions ToUnits() and ToMeters() for conversions.
*/

namespace Physics2D
{
	class CollisionQuadTree;

	class PhysicsWorld : public MeterUnitConverter
	{
	public:
		// Definition of gravity.
		glm::vec2 Gravity;

		/* Events */
		std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> OnCollisionEnter = [](RigidBody* A, RigidBody* B, const CollisionInfo& info) {};
		std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> OnCollisionExit = [](RigidBody* A, RigidBody* B, const CollisionInfo& info) {};

		CollisionQuadTree* CollisionTree;

		PhysicsWorld(glm::vec2 world_start, glm::vec2 world_end, float units_per_meter = 1.0f);
		~PhysicsWorld();

		static float GetMinBodySize() { return minBodySize; }
		static float GetMaxBodySize() { return maxBodySize; }
		static float GetMinDensity() { return minDensity; }
		static float GetMaxDensity() { return maxDensity; }
		static int GetMinPrecision() { return minPrecision; }
		static int GetMaxPrecision() { return maxPrecision; }

		void		AddBody(std::shared_ptr<Physics2D::RigidBody> body);
		RigidBody*  AddCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*  AddRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*  AddPolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*  AddCapsuleBody(CapsuleOrientation o, glm::vec2 position, glm::vec2 size, float density, float isStatic, float restitution, bool inUnits = false);

		bool		RemoveBody(int index);
		int			GetBodyIndex(RigidBody* body);
		void		RemoveAllBodies();
		RigidBody*  GetBody(int index);
		std::string GetBodyName(int index);
		RigidBody*  GetNamedBody(std::string name);
		int			BodyCount() { return (int)bodies.size(); }
		void		Update(float dt, int precision = 1);

		const AABB& GetBodyAABB(int index) { return GetBody(index)->GetAABB(); }
		glm::vec2	GetBodyPosition(int index, bool inUnits = true) { return GetBody(index)->GetCenter(inUnits); }
		const AABB& GeNamedBodyAABB(std::string name) { return GetNamedBody(name)->GetAABB(); }
		glm::vec2	GeNamedBodyPosition(std::string name, bool inUnits = true) { return GetNamedBody(name)->GetCenter(inUnits); }


	protected:
		static float minBodySize;
		static float maxBodySize;
		static float minDensity;
		static float maxDensity;
		static int minPrecision;
		static int maxPrecision;

		// Array of bodies.
		std::vector<std::shared_ptr<RigidBody>> bodies;
		
		void checkAndResolveCollision(RigidBody* bodyA, RigidBody* bodyB, float& dt);
		void responseToCollision(RigidBody* b1, RigidBody* b2, const CollisionInfo& info, float& dt);
		void callCollisionEnter_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
		void callCollisionExit_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
	};

	// Collision QuadTree for broad-phase collision check.
	class CollisionQuadTree
	{
	public:
		struct Node
		{
			glm::vec2 center;		// Center point of octree node.
			glm::vec2 halfSize;		// Half the size of the node volume.
			Node* pChild[4];		// Pointers to the eight children nodes.
			RigidBody* pObjList;	// Linked list of objects contained at this node.
			Node* pParent;
		};
		// Forward iterator for quadtree.
		// NOTE: If, during iteration, the object moves in the tree,
		// iterator can either enter infinite loop or it may get to the same
		// object more times, thus resulting in incorrect results.
		// To avoid undefined behaviour, do not use any functions, that would
		// change the tree structure. Including update, insert, remove, delete, 
		// build, rebuild, etc.
		class iterator
		{
		public:
			using iterator_cagetory = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = RigidBody;
			using pointer = RigidBody*;
			using reference = RigidBody&;

			iterator(pointer o, std::stack<Node*> s, Node* c);

			pointer operator*() const;
			reference operator->();
			// Prefix incerement.
			iterator& operator++();
			// Postfix increment
			iterator operator++(int) {
				iterator tmp(*this);
				++(*this);
				return tmp;
			}
			friend bool operator==(const iterator& a, const iterator& b) { return a.pObject == b.pObject; }
			friend bool operator!=(const iterator& a, const iterator& b) { return a.pObject != b.pObject; }

			// Static functions to get end and begin
			// iterators. Just to stay organised.
			static iterator end();
			// Recursively finds the first valid object in quadtree.
			static iterator begin(Node* pTree);
		private:
			pointer pObject;
			std::stack<Node*> stack;
			Node* pCurrentNode;

			void getNext();
			void pushChildren(Node* pTree);
		};

		Node*		root;		// Pointer to the root node.
		glm::vec2	WorldMin;	// The top-left coordinate of world.
		glm::vec2	WorldMax;	// The bottom-right coordinate of world.
		glm::vec2	WorldCenter;
		glm::vec2	WorldHalfSize;
		std::function<void(RigidBody*, RigidBody*, float&)> OnCollisionTest;
		// NOTE: can cause 1-10% performance decrease.
		bool SortOnTests = false;	// Perform ALL collision tests in order based on distance between given objects.
		
		CollisionQuadTree(glm::vec2 w_min, glm::vec2 w_max);
		// Note: Objects pointed to in quadtree should
		// be deleted AFTER the quadtree is deleted.
		// Otherwise quadtree may try to dereference NULL pointer
		// and it can lead to memory leaks.
		~CollisionQuadTree();

		// Pre-allocates the tree up to stopDepth levels and returns pointer to the tree.
		void Build(int stopDepth);
		// Recursively deletes the whole tree.
		void Delete();
		// Inserts object into quadtree. If object is not in the boundaries of 
		// the world, then it isn't inserted.
		void InsertObject(RigidBody* pObject);
		void RemoveObject(RigidBody* pObject);
		// If the object moved, this function should be called.
		// It updates the node in which this object is.
		void UpdateObject(RigidBody* pObject);
		void TestAllCollisions(float& dt);
		// Deletes the tree and pre-allocates stopDepth levels.
		void Rebuild(int stopDepth);
		// Checks if object is fully in the world boundaries.
		bool ObjectIsInWorld(RigidBody* pObject);

		// Iterator functions.
		iterator begin() const { return iterator::begin(root); }
		iterator end()	 const { return iterator::end(); }

	protected:
		static const int MAX_DEPTH = 40;
		Node* ancestorStack[MAX_DEPTH];			// Keeps tract of all ancestor object lists in a stack.
		int depth = 0;	// Traversal of ancestorStack.

		// Recursively inserts object into most suitable subtree.
		void insertObject(Node* pTree, RigidBody* pObject);
		void testAllCollisions(Node* pTree, float& dt);
		void testAllCollisions_sort(Node* pTree, float& dt);
		// Tests collision between pA and all objects in
		// linked list in sorted order.
		void testCollisions_sort(RigidBody* pA, RigidBody* pObjList, float& dt);
		// Does the same as ObjectIsInWorld(), but it does so 
		// on specific tree or subtree.
		bool objectIsInTree(Node* pTree, RigidBody* pObject);
		// Recursively deletes specific tree or subtree.
		void deleteTree(Node*& pTree);
		void reverseInsertObject(Node* pTree, RigidBody* pObject);
		// Cleans emtpy subtrees in upwards recursion from given subtree.
		void cleanUpTree(Node* pTree);

		// Allow access to protected members.
		friend class PhysicsWorld;
	};
}
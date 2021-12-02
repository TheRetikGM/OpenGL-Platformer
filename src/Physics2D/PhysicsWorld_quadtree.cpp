#include "PhysicsWorld.h"
#include <cassert>

using namespace Physics2D;

CollisionQuadTree::CollisionQuadTree(glm::vec2 w_min, glm::vec2 w_max)
	: root(nullptr)
	, WorldMin(w_min)
	, WorldMax(w_max)
	, OnCollisionTest([](RigidBody* A, RigidBody* B, float& dt) {})
{
	WorldHalfSize = (w_max - w_min) / 2.0f;
	WorldCenter = w_min + WorldHalfSize;
	this->Build(1);
	int a = 0;
}
CollisionQuadTree::~CollisionQuadTree()
{
	Delete();
}
void CollisionQuadTree::Build(int stopDepth) 
{
	static std::function<Node* (Node*, glm::vec2, glm::vec2, int)> buildTree = [&](Node* pParent, glm::vec2 center, glm::vec2 halfSize, int StopDepth) {
		if (StopDepth < 0)
			return (Node*)0;
		// Construct and fill in 'root' of this subtree.
		Node* pNode = new Node;
		pNode->center = center;
		pNode->halfSize = halfSize;
		pNode->pObjList = nullptr;
		pNode->pParent = pParent;
		memset(pNode->pChild, 0, 4 * sizeof(Node*));

		// Recursively create the 4 children of the subtree.
		glm::vec2 offset;
		glm::vec2 step = halfSize * 0.5f;
		for (int i = 0; i < 4; i++)
		{
			offset.x = ((i & 1) ? step.x : -step.x);
			offset.y = ((i & 2) ? step.y : -step.y);
			pNode->pChild[i] = buildTree(pNode, center + offset, step, StopDepth - 1);
		}

		return pNode;
	};
	root = buildTree(nullptr, WorldCenter, WorldHalfSize, stopDepth);
	int a = 0;
}
void CollisionQuadTree::deleteTree(Node*& pTree)
{
	// Clear next pointers of objects, so that we don't get 
	// incorrect linked lists.
	static std::function<void(RigidBody*)> clearObjList = [&](RigidBody* pObject) {
		if (!pObject)
			return;
		clearObjList(pObject->pNextObject);
		pObject->pNextObject = nullptr;
		pObject->pParentNode = nullptr;
	};
	if (pTree == nullptr)
		return;

	for (int i = 0; i < 4; i++)
		deleteTree(pTree->pChild[i]);

	clearObjList(pTree->pObjList);
	delete pTree;
	pTree = nullptr;
}
void CollisionQuadTree::Delete() 
{
	deleteTree(root);
}
void CollisionQuadTree::insertObject(Node* pTree, RigidBody* pObject)
{
	int index = 0, straddle = 0;

	glm::vec2 aabb_hsize = pObject->GetAABB().size / 2.0f; // Half AABB size
	glm::vec2 aabb_center = pObject->GetAABB().position + aabb_hsize;

	// Compute quadrant number [0..3]. If straddling any
	// of the x, y axes exit directly.
	for (int i = 0; i < 2; i++)
	{
		float delta = aabb_center[i] - pTree->center[i];
		float adelta = abs(delta);
		if (adelta - aabb_hsize[i] < 0.0f || adelta + aabb_hsize[i] > pTree->halfSize[i]) {
			straddle = 1;
			break;
		}

		if (delta > 0.0f)
			index |= (1 << i);	// YX
	}

	if (!straddle)
	{
		// If child is not created, create one.
		if (!pTree->pChild[index]) {
			glm::vec2 offset;
			glm::vec2 step = pTree->halfSize * 0.5f;
			offset.x = ((index & 1) ? step.x : -step.x);
			offset.y = ((index & 2) ? step.y : -step.y);

			pTree->pChild[index] = new Node;
			pTree->pChild[index]->center = pTree->center + offset;
			pTree->pChild[index]->halfSize = step;
			pTree->pChild[index]->pObjList = nullptr;
			pTree->pChild[index]->pParent = pTree;
			memset(&pTree->pChild[index]->pChild[0], '\0', 4 * sizeof(Node*));
		}
		// Insert object into child node.
		insertObject(pTree->pChild[index], pObject);
	}
	else
	{
		// Object is straddling with one of the axes, so
		// link object into linked list at this node.
		pObject->pNextObject = pTree->pObjList;
		pTree->pObjList = pObject;

		// Save the parent node pointer to the object, so
		// that we can save performance when searching for it.
		pObject->pParentNode = pTree;
	}
}
void CollisionQuadTree::InsertObject(RigidBody* pObject) 
{
	// If object isn't in world boundaries, don't insert it.
	if (!objectIsInTree(root, pObject))
		return;
	insertObject(this->root, pObject);
}
// Removes the object from linked list it belonged to.
void CollisionQuadTree::RemoveObject(RigidBody* pObject) 
{
	Node* pTree = (Node*)pObject->pParentNode;
	if (!pTree)
		return;
	
	// If object is the last element in linked list.
	if (pTree->pObjList == pObject)
		pTree->pObjList = pObject->pNextObject;
	else
	{
		// Find the previous object in tree's linked list.
		RigidBody* prev = pTree->pObjList;
		while (prev && prev->pNextObject != pObject)
			prev = prev->pNextObject;

		// If previous object was found, link it to the objects next object.
		if (prev)
			prev->pNextObject = pObject->pNextObject;
	}

	// Clear the parent node and linked list pointer.
	pObject->pParentNode = nullptr;
	pObject->pNextObject = nullptr;

	// Delte the subtree, if the linked list and all childs are emtpy.
	// However, don't delete root subtree.
	cleanUpTree(pTree);
}
// All of the empty subtrees up from this pTree will be deleted.
void CollisionQuadTree::cleanUpTree(Node* pTree)
{
	if (pTree != root)
	{
		Node* pParent = pTree->pParent;
		// Get the parent pointer, so that when we delete the
		// subtree, it does not point to invalid memory.
		int i;
		for (i = 0; i < 4; i++)
			if (pTree->pParent->pChild[i] == pTree)
				break;
		Node*& _pTree = pTree->pParent->pChild[i];

		// Only delete subtree if it contains no objects
		// and all of its children are empty.
		if (!_pTree->pObjList) {
			for (i = 0; i < 4; i++)
				if (_pTree->pChild[i]) break;
			if (i == 4) {
				delete _pTree;
				_pTree = nullptr;
			}
		}

		// Recursively go up the tree.
		cleanUpTree(pParent);
	}
}
void CollisionQuadTree::UpdateObject(RigidBody* pObject) 
{
	Node*& pNode = (Node*&)pObject->pParentNode;
	// If object has no parent, it cannot be updated.
	if (!pNode)
		return;

	// Traverse the tree upwards and try to insert
	// at each parent node. If the linked list of
	// objects, from which it was removed, is empty, delete it.
	RemoveObject(pObject);
	if (!pNode)
		InsertObject(pObject);
	else
		reverseInsertObject(pNode, pObject);
}
void CollisionQuadTree::reverseInsertObject(Node* pTree, RigidBody* pObject)
{
	if (objectIsInTree(pTree, pObject))
		insertObject(pTree, pObject);
	else if (pTree->pParent)
		reverseInsertObject(pTree->pParent, pObject);
}
void CollisionQuadTree::TestAllCollisions(float& dt) 
{
	if (SortOnTests)
		testAllCollisions_sort(root, dt);
	else
		testAllCollisions(root, dt);
}
void CollisionQuadTree::testCollisions_sort(RigidBody* pA, RigidBody* pObjList, float& dt)
{
	static RigidBody* pB;
	RigidBody* pRoot = nullptr;
	for (pB = pObjList; pB; pB = pB->pNextObject) {
		if (pA == pB)
			continue;

		// Distence between centers. No need to compute sqrt,
		// as we don't need precise value to compare them.
		const glm::vec2& bp = pB->position;
		const glm::vec2& ba = pA->position;
		glm::vec2 ab;
		ab.x = bp.x - ba.x;
		ab.y = bp.y - ba.y;
		pB->dist = ab.x * ab.x + ab.y * ab.y;

		// Direct insert of pB into the sorting list. //
		pB->pNextSortNode = nullptr;
		// Handle missing root.
		if (!pRoot)
			pRoot = pB;
		// Handle insert as first element.
		else if (pB->dist < pRoot->dist)
		{
			pB->pNextSortNode = pRoot;
			pRoot = pB;
		}
		// Actual direct insert into linked list.
		else
		{
			RigidBody* pPrevNode = pRoot;
			RigidBody* pN = pRoot->pNextSortNode;
			for (pN; pN; pN = pN->pNextSortNode) {
				if (pB->dist < pN->dist) {
					pPrevNode->pNextSortNode = pB;
					pB->pNextSortNode = pN;
					break;
				}
				pPrevNode = pN;
			}
			// Handle insert as last element.
			if (pN == nullptr)
				pPrevNode->pNextSortNode = pB;
		}
	}
	// Test againts all objects in linked list 
	// and clear linked list pointers.
	RigidBody* pNode = pRoot;
	while (pNode)
	{
		RigidBody* pNext = pNode->pNextSortNode;
		OnCollisionTest(pA, pNode, dt);
		pNode->pNextSortNode = nullptr;
		pNode = pNext;
	}
}
void CollisionQuadTree::testAllCollisions(Node* pTree, float& dt)
{
	if (depth >= MAX_DEPTH)
		return;

	// Check collision between all objects on this level and all
	// ancestor objects. The current level is included as its own
	// ancestor so all necessary pairwise test are done.
	ancestorStack[depth++] = pTree;
	for (int n = 0; n < depth; n++) {
		RigidBody* pA, * pB;
		for (pA = ancestorStack[n]->pObjList; pA; pA = pA->pNextObject)
		{
			// Test vs kinematic body should be in sorted order.
			//if (pA->IsKinematic)
			//	testCollisions_sort(pA, pTree->pObjList, dt);
			//else
			//{
				for (pB = pTree->pObjList; pB; pB = pB->pNextObject) {
					// Avoid testing both A->B and B->A
					if (pA == pB)
						break;
					// Perform the collision test between A and B.
					OnCollisionTest(pA, pB, dt);
				}
			//}
		}
	}

	// Recursively visit all existing children.
	for (int i = 0; i < 4; i++)
		if (pTree->pChild[i])
			testAllCollisions(pTree->pChild[i], dt);

	// Remove current node from ancestor stack before returning.
	depth--;
}
void CollisionQuadTree::testAllCollisions_sort(Node* pTree, float& dt)
{
	if (depth >= MAX_DEPTH)
		return;

	ancestorStack[depth++] = pTree;
	for (int n = 0; n < depth; n++) {
		RigidBody* pA, * pB;
		for (pA = ancestorStack[n]->pObjList; pA; pA = pA->pNextObject)
		{
			testCollisions_sort(pA, pTree->pObjList, dt);
		}
	}

	// Recursively visit all existing children.
	for (int i = 0; i < 4; i++)
		if (pTree->pChild[i])
			testAllCollisions_sort(pTree->pChild[i], dt);

	// Remove current node from ancestor stack before returning.
	depth--;
}
void CollisionQuadTree::Rebuild(int stopDepth)
{
	Delete();
	Build(stopDepth);
}
bool CollisionQuadTree::ObjectIsInWorld(RigidBody* pObject)
{
	return objectIsInTree(root, pObject);
}
bool CollisionQuadTree::objectIsInTree(Node* pTree, RigidBody* pObject)
{
	glm::vec2 o_min = pObject->GetAABB().GetMin();
	glm::vec2 o_max = pObject->GetAABB().GetMax();
	glm::vec2 w_min = pTree->center - pTree->halfSize;
	glm::vec2 w_max = pTree->center + pTree->halfSize;

	return o_min.x >= w_min.x && o_min.y >= w_min.y
		&& o_max.x <= w_max.x && o_max.y <= w_max.y;
}

/* - - - - QuadTree::iterator definitions - - - - */
CollisionQuadTree::iterator::iterator(pointer o, std::stack<Node*> s, Node* c) 
	: pObject(o), stack(s), pCurrentNode(c) 
{
}
CollisionQuadTree::iterator::pointer CollisionQuadTree::iterator::operator*() const 
{ 
	return pObject; 
}
CollisionQuadTree::iterator::reference CollisionQuadTree::iterator::operator->() 
{ 
	return *pObject; 
}
CollisionQuadTree::iterator& CollisionQuadTree::iterator::operator++() 
{
	getNext();
	return *this;
}
void CollisionQuadTree::iterator::getNext()
{
	// If there is another object in linked list,
	// get to it.
	if (pObject->pNextObject) {
		pObject = pObject->pNextObject;
		return;
	}
	// If not, then find the first object in other subnodes.
	while (!stack.empty())
	{
		pCurrentNode = stack.top();
		stack.pop();

		if (!pCurrentNode)
			continue;

		pushChildren(pCurrentNode);

		if (pCurrentNode->pObjList) {
			pObject = pCurrentNode->pObjList;
			return;
		}
	}
	// If no next object was found, set pObject
	// to nullptr. Then this iterator will equal to end.
	pObject = nullptr;
}

void CollisionQuadTree::iterator::pushChildren(Node* pTree)
{
	// Push children in reverse order, so that
	// we access them from left to right.
	for (int i = 3; i >= 0; i--)
		if (pTree->pChild[i])
			stack.push(pTree->pChild[i]);
}
CollisionQuadTree::iterator CollisionQuadTree::iterator::end() 
{ 
	return CollisionQuadTree::iterator(nullptr, std::stack<CollisionQuadTree::Node*>(), nullptr); 
}
// Recursively finds the first valid object in quadtree.
CollisionQuadTree::iterator CollisionQuadTree::iterator::begin(CollisionQuadTree::Node* pTree) 
{
	std::stack<CollisionQuadTree::Node*> stack;
	if (pTree)
		stack.push(pTree);

	while (!stack.empty()) {
		CollisionQuadTree::Node* pNode = stack.top();
		stack.pop();

		for (int i = 3; i >= 0; i--)
			if (pNode->pChild[i])
				stack.push(pNode->pChild[i]);

		if (pNode->pObjList)
			return CollisionQuadTree::iterator(pNode->pObjList, stack, pNode);
	}
	return CollisionQuadTree::iterator::end();
}
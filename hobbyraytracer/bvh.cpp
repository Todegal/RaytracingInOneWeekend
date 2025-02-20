#include "hobbyraytracer.h"
#include "bvh.h"

#include <execution>

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>>& srcObjects,
	size_t start, size_t end)
{
	// Pick a random axis to compare the objects on
	int a = glm::linearRand(0, 2);
	auto comparator = (a == 0) ? BVHNode::boxXCompare :
		(a == 1) ? BVHNode::boxYCompare
		: BVHNode::boxZCompare;

	size_t noObjects = end - start;

	/*std::cout << "Building BVHNetwork with " << noObjects << " objects around the " 
		<< std::vector<std::string>({ "X", "Y", "Z" })[axis] << " axis..." << std::endl;*/

	if (noObjects == 1) // If there's only one object set both the right and left pointer to it
	{
		left = right = srcObjects[start];
	}
	else if (noObjects == 2) // If there's two objects assign the pointers to whichever one is lesser on the chosen axis
	{
		if (comparator(srcObjects[start], srcObjects[start + 1]))
		{
			left = srcObjects[start];
			right = srcObjects[start + 1];
		}
		else
		{
			left = srcObjects[start + 1];
			right = srcObjects[start];
		}
	}
	else if (noObjects != 0) // Otherwise sort the objects on the chosen axis and put them into two nodes further down the tree
	{
		std::sort(std::execution::par, srcObjects.begin() + start, srcObjects.begin() + end, comparator);

		size_t mid = start + noObjects / 2;
		left = std::make_shared<BVHNode>(srcObjects, start, mid);
		right = std::make_shared<BVHNode>(srcObjects, mid, end);
	}
	else
	{
		return;
	}

	// Find the bounding boxes for the two pointers and combine them to get the bounding box for this node

	AABB boxLeft, boxRight;

	if (!left->boundingBox(boxLeft)
		|| !right->boundingBox(boxRight))
	{
		std::cout << "No bounding box in BVHNode constructor" << std::endl;
	}

	box = AABB::surroundingBox(boxLeft, boxRight);
}

bool BVHNode::boundingBox(AABB& outputBox)
{
	outputBox = box;
	return true;
}

bool BVHNode::hit(const ray& r, float t_min, float t_max, hitRecord& rec) const
{
	if (!box.hit(r, t_min, t_max))
		return false;

	bool hitLeft = left->hit(r, t_min, t_max, rec);
	bool hitRight = right->hit(r, t_min, hitLeft ? rec.t : t_max, rec);

	return hitLeft || hitRight;
}

bool BVHNode::boxCompare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis)
{
	AABB boxA, boxB;

	if (!a->boundingBox(boxA) || !b->boundingBox(boxB))
	{
		std::cout << "No bounding box in BVHNode constructor" << std::endl;
	}

	return boxA.getMin()[axis] < boxB.getMin()[axis];
}
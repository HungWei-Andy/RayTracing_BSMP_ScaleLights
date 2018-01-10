#include "BSP.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "Object.h"
#include "Intersection.h"

using namespace std;

void BSP::build() {
   // Make sure all objects are properly wrapped
   for (vector<Object*>::iterator itr = objects.begin(); itr < objects.end(); itr++) {
     Boundaries curr = (*itr)->getBounds();
     bounds.min = Vector(min(bounds.min.x, curr.min.x),
                         min(bounds.min.y, curr.min.y),
                         min(bounds.min.z, curr.min.z));
     bounds.max = Vector(max(bounds.max.x, curr.max.x),
                         max(bounds.max.y, curr.max.y),
                         max(bounds.max.z, curr.max.z));
   }

   // For debugging.
   if (true) {
      for (int i = 0; i < depth; i++) {
        cout << "\t";
      }
      cout << objects.size() << " | " <<
         bounds.min.x << ", " << bounds.min.y << ", " << bounds.min.z << " || " <<
         bounds.max.x << ", " << bounds.max.y << ", " << bounds.max.z <<
         " % " << axis << " X " << axisRetries << endl;
   }

   // We've hit our limit so this is a leaf node. No need to split again.
   if (objects.size() <= MIN_OBJECT_COUNT) {
      return;
   }

   // Where to split the bounds
   //double splitValue = bounds.splitValue(axis);
   /* A better split value is the median of all values*/
   vector<double> cntr_values;
   for (vector<Object*>::iterator itr = objects.begin(); itr < objects.end(); itr++) {
      Object* obj = *itr;
      Boundaries curr = obj->getBounds();
      double min, max;

      switch(axis) {
         case 'x':
            min = curr.min.x;
            max = curr.max.x;
            break;
         case 'y':
            min = curr.min.y;
            max = curr.max.y;
            break;
         case 'z':
            min = curr.min.z;
            max = curr.max.z;
            break;
      }

      cntr_values.push_back(min);
      cntr_values.push_back(max);
   }
   sort(cntr_values.begin(), cntr_values.end());
   int splitValueIndex = (int) floor(cntr_values.size() / 2.0);
   double splitValue = cntr_values[splitValueIndex];
   
   vector<Object*> leftObjects;
   vector<Object*> rightObjects;

   for (vector<Object*>::iterator itr = objects.begin(); itr < objects.end(); itr++) {
      Object* obj = *itr;
      Boundaries curr = obj->getBounds();
      double min, max;

      switch(axis) {
         case 'x':
            min = curr.min.x;
            max = curr.max.x;
            break;
         case 'y':
            min = curr.min.y;
            max = curr.max.y;
            break;
         case 'z':
            min = curr.min.z;
            max = curr.max.z;
            break;
      }

      if (min < splitValue) {
         leftObjects.push_back(obj);
      }

      if (max > splitValue) {
         rightObjects.push_back(obj);
      }
   }

   int newAxis = toggleAxis();

   if (leftObjects.size() != objects.size() &&
       rightObjects.size() != objects.size()) {
      // Since this split separated geometry a little bit, make children to
      // split up geometry further.
      left = new BSP(depth + 1, newAxis, leftObjects);
      right = new BSP(depth + 1, newAxis, rightObjects);
   } else if (axisRetries < 2) {
      axis = toggleAxis();
      axisRetries++;
      build();
   } else {
      // Do nothing since we're out of axis retries.
   }
}

char BSP::toggleAxis() {
   return axis == 'x' ? 'y' : (axis == 'y' ? 'z' : 'x');
}

void BSP::clearFlag() {
   if (left != NULL) left->clearFlag();
   if (right != NULL) right->clearFlag();
   searched = false;
}

Intersection BSP::getClosestIntersection(const Ray& ray, const Vector& maxv, const Vector& minv) {
   double distance;
   if (!bounds.intersect(ray, maxv, minv) || searched) {
      return Intersection();
   }

   if (left != NULL && right != NULL) {
      Intersection leftIntersection = left->getClosestIntersection(ray, maxv, minv);
      Intersection rightIntersection = right->getClosestIntersection(ray, maxv, minv);
      
      if (left->searched && right->searched)
	 searched = true;

      return leftIntersection.distance < rightIntersection.distance ?
       leftIntersection : rightIntersection;
   } else {
      searched = true;
      return getClosestObjectIntersection(ray);
   }
}

Intersection BSP::getClosestIntersection(const Ray& ray) {
   double distance;
   if (!bounds.intersect(ray, &distance) || searched) {
      return Intersection();
   }

   if (left != NULL && right != NULL) {
      Intersection leftIntersection = left->getClosestIntersection(ray);
      Intersection rightIntersection = right->getClosestIntersection(ray);

      return leftIntersection.distance < rightIntersection.distance ?
       leftIntersection : rightIntersection;
   } else {
      return getClosestObjectIntersection(ray);
   }
}

Intersection BSP::getClosestObjectIntersection(const Ray& ray) {
   // No children so just go through current objects like normal.
   Intersection closestIntersection;

   for (vector<Object*>::iterator itr = objects.begin(); itr < objects.end(); itr++) {
      Intersection intersection = (*itr)->intersect(ray);

      if (intersection.didIntersect && intersection.distance <
       closestIntersection.distance) {
         closestIntersection = intersection;
      }
   }

   return closestIntersection;
}
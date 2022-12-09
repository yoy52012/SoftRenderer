#pragma once

#include <vector>
#include <array>
#include "MathUtils.h"

namespace SoftRenderer
{
    class Sphere
    {
    public:
        /** The sphere's center point. */
        glm::vec3 mCenter = glm::vec3(0);

        /** The sphere's radius. */
        float mRadius = 0.0f;

        /**
         * Creates and initializes a new sphere.
         */
        Sphere()
            : mCenter(0.0f, 0.0f, 0.0f)
            , mRadius(0)
        { }

        /**
         * Creates and initializes a new sphere with the specified parameters.
         *
         * @param center Center of sphere.
         * @param radius Radius of sphere.
         */
        Sphere(const glm::vec3& center, float radius)
            : mCenter(center)
            , mRadius(radius)
        { }

        /**
         * Constructor.
         *
         * @param points Pointer to list of points this sphere must contain.
         */
        Sphere(const std::vector<glm::vec3>& points)
        {
            int32_t count = points.size();

            // Min/max points of AABB
            int32_t minIndex[3] = { 0, 0, 0 };
            int32_t maxIndex[3] = { 0, 0, 0 };

            for (int32_t i = 0; i < count; i++)
            {
                for (int32_t k = 0; k < 3; k++)
                {
                    minIndex[k] = points[i][k] < points[minIndex[k]][k] ? i : minIndex[k];
                    maxIndex[k] = points[i][k] > points[maxIndex[k]][k] ? i : maxIndex[k];
                }
            }

            float largestDistSqr = 0.0f;
            int32_t largestAxis = 0;
            for (int k = 0; k < 3; k++)
            {
                glm::vec3 pointMin = points[minIndex[k]];
                glm::vec3 pointMax = points[maxIndex[k]];

                float distSqr = glm::length2(pointMax - pointMin);
                if (distSqr > largestDistSqr)
                {
                    largestDistSqr = distSqr;
                    largestAxis = k;
                }
            }

            glm::vec3 pointMin = points[minIndex[largestAxis]];
            glm::vec3 pointMax = points[maxIndex[largestAxis]];

            this->mCenter = 0.5f * (pointMin + pointMax);
            this->mRadius = 0.5f * std::sqrt(largestDistSqr);
            float radiusSqr = this->mRadius * this->mRadius;

            // Adjust to fit all points
            for (int i = 0; i < count; i++)
            {
                float distSqr = glm::length2(points[i] - this->mCenter);

                if (distSqr > radiusSqr)
                {
                    float dist = std::sqrt(distSqr);
                    float t = 0.5f + 0.5f * (this->mRadius / dist);

                    this->mCenter = glm::lerp(points[i], this->mCenter, t);
                    this->mRadius = 0.5f * (this->mRadius + dist);
                }
            }
        }


        /**
         * Constructor.
         *
         * @param spheres Pointer to list of spheres this sphere must contain.
         */
        Sphere(const std::vector<Sphere>& spheres)
        {
            int32_t count = spheres.size();

            // Min/max points of AABB
            int32_t minIndex[3] = { 0, 0, 0 };
            int32_t maxIndex[3] = { 0, 0, 0 };

            for (int32_t i = 0; i < count; i++)
            {
                for (int32_t k = 0; k < 3; k++)
                {
                    minIndex[k] = spheres[i].mCenter[k] - spheres[i].mRadius < spheres[minIndex[k]].mCenter[k] - spheres[minIndex[k]].mRadius ? i : minIndex[k];
                    maxIndex[k] = spheres[i].mCenter[k] + spheres[i].mRadius > spheres[maxIndex[k]].mCenter[k] + spheres[maxIndex[k]].mRadius ? i : maxIndex[k];
                }
            }

            float largestDist = 0.0f;
            int32_t largestAxis = 0;
            for (int32_t k = 0; k < 3; k++)
            {
                Sphere sphereMin = spheres[minIndex[k]];
                Sphere sphereMax = spheres[maxIndex[k]];

                float dist = glm::length(sphereMax.mCenter - sphereMin.mCenter) + sphereMin.mRadius + sphereMax.mRadius;
                if (dist > largestDist)
                {
                    largestDist = dist;
                    largestAxis = k;
                }
            }

            *this = spheres[minIndex[largestAxis]];
            *this += spheres[maxIndex[largestAxis]];

            // Adjust to fit all spheres
            for (int i = 0; i < count; i++)
            {
                *this += spheres[i];
            }
        }

        // Conversion from other variant type.
        Sphere(const Sphere& sphere)
            : mCenter(sphere.mCenter)
            , mRadius(sphere.mRadius)
        { }


        /**
         * Check whether two spheres are the same within specified tolerance.
         *
         * @param sphere The other sphere.
         * @param tolerance Error Tolerance.
         * @return true if spheres are equal within specified tolerance, otherwise false.
         */
        bool equals(const Sphere& sphere, float tolerance = glm::epsilon<float>()) const
        {
            return  glm::all(glm::equal(mCenter, sphere.mCenter, tolerance)) && std::abs(mRadius - sphere.mRadius) <= tolerance;
        }

        /**
         * Check whether sphere is inside of another.
         *
         * @param other The other sphere.
         * @param tolerance Error Tolerance.
         * @return true if sphere is inside another, otherwise false.
         */
        bool isInside(const Sphere& other, float tolerance = glm::epsilon<float>()) const
        {
            if (mRadius > other.mRadius + tolerance)
            {
                return false;
            }

            return glm::length2(mCenter - other.mCenter) <= glm::length2(other.mRadius + tolerance - mRadius);
        }

        /**
        * Checks whether the given location is inside this sphere.
        *
        * @param p The location to test for inside the bounding volume.
        * @param tolerance Error Tolerance.
        * @return true if location is inside this volume.
        */
        bool isInside(const glm::vec3& p, float tolerance = glm::epsilon<float>()) const
        {
            return glm::length2(mCenter - p) <= glm::length2(mRadius + tolerance);
        }

        /**
         * Test whether this sphere intersects another.
         * 
         * @param  other The other sphere.
         * @param  tolerance Error tolerance.
         * @return true if spheres intersect, false otherwise.
         */
        bool intersects(const Sphere& other, float tolerance = glm::epsilon<float>()) const
        {
            return glm::length2(mCenter - other.mCenter) <= glm::length2(std::max(0.f, other.mRadius + mRadius + tolerance));
        }

        /**
         * Get result of Transforming sphere by Matrix.
         *
         * @param M Matrix to transform by.
         * @return Result of transformation.
         */
        Sphere transformBy(const glm::mat4& M) const
        {
            Sphere	result;

            glm::vec4 transformedCenter = M * glm::vec4(mCenter, 1.0f);
            result.mCenter = glm::vec3(transformedCenter.x, transformedCenter.y, transformedCenter.z);

            const glm::vec3 XAxis(M[0][0], M[0][1], M[0][2]);
            const glm::vec3 YAxis(M[1][0], M[1][1], M[1][2]);
            const glm::vec3 ZAxis(M[2][0], M[2][1], M[2][2]);

            result.mRadius = glm::sqrt(std::max(glm::dot(XAxis, XAxis), std::max(glm::dot(YAxis, YAxis), glm::dot(ZAxis, ZAxis)))) * mRadius;

            return result;
        }

        /**
         * Adds to this bounding box to include a new bounding volume.
         *
         * @param other the bounding volume to increase the bounding volume to.
         * @return Reference to this bounding volume after resizing to include the other bounding volume.
         */
        Sphere& operator+=(const Sphere& other)
        {
            if (mRadius == 0.f)
            {
                *this = other;
                return *this;
            }

            glm::vec3 toOther = other.mCenter - mCenter;
            float distSqr = glm::length2(toOther);

            if (glm::length2(mRadius - other.mRadius) + 1.e-4f >= distSqr)
            {
                // Pick the smaller
                if (mRadius < other.mRadius)
                {
                    *this = other;
                }
            }
            else
            {
                float dist = std::sqrt(distSqr);

                Sphere newSphere;
                newSphere.mRadius = (dist + other.mRadius + mRadius) * 0.5f;
                newSphere.mCenter = mCenter;

                if (dist > 1.e-4f)
                {
                    newSphere.mCenter += toOther * ((newSphere.mRadius - mRadius) / dist);
                }

                // make sure both are inside afterwards

                *this = newSphere;
            }

            return *this;
        }

        /**
         * Gets the result of addition to this bounding volume.
         *
         * @param other The other volume to add to this.
         * @return A new bounding volume.
         */
        Sphere operator+(const Sphere& other) const
        {
            return Sphere(*this) += other;
        }
    };

    class Box
    {
    public:
        /** Holds the box's minimum point. */
        glm::vec3 mMin = glm::vec3(0);

        /** Holds the box's maximum point. */
        glm::vec3 mMax = glm::vec3(0);

        /** Holds a flag indicating whether this box is valid. */
        uint8_t mIsValid = 0;

    public:
        /**
         * Creates and initializes a new box with zero extent and marks it as invalid.
         */
        Box()
        {}

        /**
         * Creates and initializes a new box from the specified extents.
         *
         * @param inMin The box's minimum point.
         * @param inMax The box's maximum point.
         */
        Box( const glm::vec3& inMin, const glm::vec3& inMax )
            : mMin(inMin)
            , mMax(inMax)
            , mIsValid(1)
        { }

        /**
         * Creates and initializes a new box from the given set of points.
         *
         * @param Points Array of Points to create for the bounding volume.
         * @param Count The number of points.
         */
        Box(const std::vector<glm::vec3>& points) 
            : mMin(0, 0, 0)
            , mMax(0, 0, 0)
            , mIsValid(0)
        {
            for (int32_t i = 0; i < points.size(); i++)
            {
                *this += points[i];
            }
        }


        // Conversion from other type.
        Box(const Box& other) 
            : mMin(other.mMin)
            , mMax(other.mMax)
        {}

    public:

        /**
         * Compares two boxes for equality.
         *
         * @return true if the boxes are equal, false otherwise.
         */
        bool operator==( const Box& other ) const
        {
            return (mMin == other.mMin) && (mMax == other.mMax);
        }

        /**
         * Compares two boxes for inequality.
         *
         * @return false if the boxes are equal, true otherwise.
         */
        bool operator!=( const Box& other) const
        {
            return (mMin != other.mMin) || (mMax != other.mMax);
        }

        /**
         * Check against another box for equality, within specified error limits.
         *
         * @param other The box to check against.
         * @param tolerance Error tolerance.
         * @return true if the boxes are equal within tolerance limits, false otherwise.
         */
        bool equals(const Box& other, float tolerance = glm::epsilon<float>()) const
        {
            return glm::all(glm::equal(mMin, other.mMin, tolerance)) && glm::all(glm::equal(mMax, other.mMax, tolerance));
        }

        /**
         * Adds to this bounding box to include a given point.
         *
         * @param Other the point to increase the bounding volume to.
         * @return Reference to this bounding box after resizing to include the other point.
         */
        Box& operator+=( const glm::vec3 &other )
        {
            if (mIsValid)
            {
                mMin.x = std::min(mMin.x, other.x);
                mMin.y = std::min(mMin.y, other.y);
                mMin.z = std::min(mMin.z, other.z);

                mMax.x = std::max(mMax.x, other.x);
                mMax.y = std::max(mMax.y, other.y);
                mMax.z = std::max(mMax.z, other.z);
            }
            else
            {
                mMin = mMax = other;
                mIsValid = 1;
            }

            return *this;
        }

        /**
         * Gets the result of addition to this bounding volume.
         *
         * @param other The other point to add to this.
         * @return A new bounding volume.
         */
        Box operator+( const glm::vec3& other ) const
        {
            return Box(*this) += other;
        }

        /**
         * Adds to this bounding box to include a new bounding volume.
         *
         * @param Other the bounding volume to increase the bounding volume to.
         * @return Reference to this bounding volume after resizing to include the other bounding volume.
         */
        Box& operator+=( const Box& other )
        {
            if (mIsValid && other.mIsValid)
            {
                mMin.x = std::min(mMin.x, other.mMin.x);
                mMin.y = std::min(mMin.y, other.mMin.y);
                mMin.z = std::min(mMin.z, other.mMin.z);

                mMax.x = std::max(mMax.x, other.mMax.x);
                mMax.y = std::max(mMax.y, other.mMax.y);
                mMax.z = std::max(mMax.z, other.mMax.z);
            }
            else if (other.mIsValid)
            {
                *this = other;
            }

            return *this;
        }

        /**
         * Gets the result of addition to this bounding volume.
         *
         * @param other The other volume to add to this.
         * @return A new bounding volume.
         */
        Box operator+( const Box& other ) const
        {
            return Box(*this) += other;
        }

        /**
         * Gets reference to the min or max of this bounding volume.
         *
         * @param Index the index into points of the bounding volume.
         * @return a reference to a point of the bounding volume.
         */
        glm::vec3& operator[]( int32_t index )
        {
            assert((index >= 0) && (index < 2));

            if (index == 0)
            {
                return mMin;
            }

            return mMax;
        }

    public:

        /** 
         * Calculates the distance of a point to this box.
         *
         * @param point The point.
         * @return The distance.
         */
        float computeSquaredDistanceToPoint( const glm::vec3& point ) const
        {
            // Accumulates the distance as we iterate axis
            float distSquared = 0;

            // Check each axis for min/max and add the distance accordingly
            // NOTE: Loop manually unrolled for > 2x speed up
            if (point.x < mMin.x)
            {
                distSquared += glm::length2<float>(point.x - mMin.x);
            }
            else if (point.x > mMax.x)
            {
                distSquared += glm::length2<float>(point.x - mMax.x);
            }

            if (point.y < mMin.y)
            {
                distSquared += glm::length2<float>(point.y - mMin.y);
            }
            else if (point.y > mMax.y)
            {
                distSquared += glm::length2<float>(point.y - mMax.y);
            }

            if (point.z < mMin.z)
            {
                distSquared += glm::length2<float>(point.z - mMin.z);
            }
            else if (point.z > mMax.z)
            {
                distSquared += glm::length2<float>(point.z - mMax.z);
            }

            return distSquared;
        }

        /**
         * Calculates squared distance between two boxes.
         */
        float computeSquaredDistanceToBox(const Box& box) const
        {
            glm::vec3 axisDistances = glm::abs(getCenter() - box.getCenter()) - (getExtent() + box.getExtent());
            axisDistances.x = std::max(axisDistances.x, 0.0f);
            axisDistances.y = std::max(axisDistances.y, 0.0f);
            axisDistances.z = std::max(axisDistances.z, 0.0f);
            return glm::dot(axisDistances, axisDistances);
        }

        /** 
         * Returns a box of increased size.
         *
         * @param w The size to increase the volume by.
         * @return A new bounding box.
         */
        Box expandBy(float w) const
        {
            return Box(mMin - glm::vec3(w, w, w), mMax + glm::vec3(w, w, w));
        }

        /**
        * Returns a box of increased size.
        *
        * @param v The size to increase the volume by.
        * @return A new bounding box.
        */
        Box expandBy(const glm::vec3& v) const
        {
            return Box(mMin - v, mMax + v);
        }

        /**
        * Returns a box of increased size.
        *
        * @param neg The size to increase the volume by in the negative direction (positive values move the bounds outwards)
        * @param pos The size to increase the volume by in the positive direction (positive values move the bounds outwards)
        * @return A new bounding box.
        */
        Box expandBy(const glm::vec3& neg, const glm::vec3& pos) const
        {
            return Box(mMin - neg, mMax + pos);
        }

        /** 
         * Returns a box with its position shifted.
         *
         * @param offset The vector to shift the box by.
         * @return A new bounding box.
         */
        Box shiftBy( const glm::vec3& offset ) const
        {
            return Box(mMin + offset, mMax + offset);
        }

        /**
         * Gets the center point of this box.
         *
         * @return The center point.
         */
        glm::vec3 getCenter() const
        {
            return glm::vec3((mMin + mMax) * 0.5f);
        }

        /**
         * Gets the center and extents of this box.
         *
         * @param center [out] Will contain the box center point.
         * @param extents [out] Will contain the extent around the center.
         */
        void getCenterAndExtents( glm::vec3& center, glm::vec3& extents ) const
        {
            extents = getExtent();
            center = mMin + extents;
        }

        /**
         * Calculates the closest point on or inside the box to a given point in space.
         *
         * @param Point The point in space.
         * @return The closest point on or inside the box.
         */
        glm::vec3 getClosestPointTo( const glm::vec3& Point ) const;

        /**
         * Gets the extents of this box.
         *
         * @return The box extents.
         */
        glm::vec3 getExtent() const
        {
            return 0.5f * (mMax - mMin);
        }

        /**
         * Gets the size of this box.
         *
         * @return The box size.
         */
        glm::vec3 getSize() const
        {
            return (mMax - mMin);
        }

        /**
         * Gets the volume of this box.
         *
         * @return The box volume.
         */
        float getVolume() const
        {
            return (mMax.x - mMin.x) * (mMax.y - mMin.y) * (mMax.z - mMin.z);
        }

        /**
         * Set the initial values of the bounding box to Zero.
         */
        void init()
        {
            mMin = mMax = glm::vec3(0);
            mIsValid = 0;
        }

        /**
         * Checks whether the given bounding box intersects this bounding box.
         *
         * @param other The bounding box to intersect with.
         * @return true if the boxes intersect, false otherwise.
         */
        bool intersect( const Box& other ) const
        {
            if ((mMin.x > other.mMax.x) || (other.mMin.x > mMax.x))
            {
                return false;
            }

            if ((mMin.y > other.mMax.y) || (other.mMin.y > mMax.y))
            {
                return false;
            }

            if ((mMin.z > other.mMax.z) || (other.mMin.z > mMax.z))
            {
                return false;
            }

            return true;
        }

        /**
         * Checks whether the given bounding box intersects this bounding box in the XY plane.
         *
         * @param other The bounding box to test intersection.
         * @return true if the boxes intersect in the XY Plane, false otherwise.
         */
        bool intersectXY( const Box& other ) const
        {
            if ((mMin.x > other.mMax.x) || (other.mMin.x > mMax.x))
            {
                return false;
            }

            if ((mMin.y > other.mMax.y) || (other.mMin.y > mMax.y))
            {
                return false;
            }

            return true;
        }

        /**
         * Returns the overlap Box of two box
         *
         * @param other The bounding box to test overlap
         * @return the overlap box. It can be 0 if they don't overlap
         */
        Box overlap( const Box& other ) const
        {
            if (intersect(other) == false)
            {
                static Box emptyBox;
                return emptyBox;
            }

            // otherwise they overlap
            // so find overlapping box
            glm::vec3 minVector, maxVector;

            minVector.x = std::max(mMin.x, other.mMin.x);
            maxVector.x = std::min(mMax.x, other.mMax.x);

            minVector.y = std::max(mMin.y, other.mMin.y);
            maxVector.y = std::min(mMax.y, other.mMax.y);

            minVector.z = std::max(mMin.z, other.mMin.z);
            maxVector.z = std::min(mMax.z, other.mMax.z);

            return Box(minVector, maxVector);
        }

        /** 
         * Checks whether the given location is inside this box.
         * 
         * @param in The location to test for inside the bounding volume.
         * @return true if location is inside this volume.
         * @see IsInsideXY
         */
        bool isInside( const glm::vec3& in ) const
        {
            return ((in.x > mMin.x) && (in.x < mMax.x) && (in.y > mMin.y) && (in.y < mMax.y) && (in.z > mMin.z) && (in.z < mMax.z));
        }

        /** 
         * Checks whether the given location is inside or on this box.
         * 
         * @param In The location to test for inside the bounding volume.
         * @return true if location is inside this volume.
         * @see IsInsideXY
         */
        bool isInsideOrOn( const glm::vec3& in ) const
        {
            return ((in.x >= mMin.x) && (in.x <= mMax.x) && (in.y >= mMin.y) && (in.y <= mMax.y) && (in.z >= mMin.z) && (in.z <= mMax.z));
        }

        /** 
         * Checks whether a given box is fully encapsulated by this box.
         * 
         * @param other The box to test for encapsulation within the bounding volume.
         * @return true if box is inside this volume.
         */
        bool isInside( const Box& other ) const
        {
            return (isInside(other.mMin) && isInside(other.mMax));
        }

        /**
         * Gets a bounding volume transformed by a matrix.
         *
         * @param M The matrix to transform by.
         * @return The transformed box.
         */
        Box transformBy( const glm::mat4& M ) const
        {
            // if we are not valid, return another invalid box.
            if (!mIsValid)
            {
                return Box();
            }

            Box newBox;

            auto origin = (mMin + mMax) * 0.5f;
            auto extend = (mMax - mMin) * 0.5f;

            auto newOrigin = M * glm::vec4(origin, 1.0f);
            auto newExtend = M * glm::vec4(extend, 1.0f);

            auto newMin = newOrigin - newExtend;
            auto newMax = newOrigin + newExtend;

            newBox.mMin = glm::vec3(newMin.x, newMin.y, newMin.z);
            newBox.mMax = glm::vec3(newMax.x, newMax.y, newMax.z);
            newBox.mIsValid = 1;

            return newBox;
        }

        /** 
         * Returns the current world bounding box transformed and projected to screen space
         *
         * @param projM The projection matrix.
         * @return The transformed box.
         */
        Box transformProjectBy( const glm::mat4& projM ) const
        {
            auto vertices = getVertices();

            Box newBox;

            for (int32_t vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex++)
            {
                auto projectedVertex = projM * glm::vec4(vertices[vertexIndex], 1.0f);
                newBox += projectedVertex / projectedVertex.w;
            }

            return newBox;
        }

        /**
         * Get the vertices that make up this box.
         * 
         * 
         */
        std::array<glm::vec3, 8> getVertices() const
        {
            std::array<glm::vec3, 8> vertices;

            vertices[0] = glm::vec3(mMin);
            vertices[1] = glm::vec3(mMin.x, mMin.y, mMax.z);
            vertices[2] = glm::vec3(mMin.x, mMax.y, mMin.z);
            vertices[3] = glm::vec3(mMax.x, mMin.y, mMin.z);
            vertices[4] = glm::vec3(mMax.x, mMax.y, mMin.z);
            vertices[5] = glm::vec3(mMax.x, mMin.y, mMax.z);
            vertices[6] = glm::vec3(mMin.x, mMax.y, mMax.z);
            vertices[7] = glm::vec3(mMax);
        }

    public:
        /** 
         * Utility function to build an AABB from origin and extent 
         *
         * @param origin The location of the bounding box.
         * @param extent Half size of the bounding box.
         * @return A new axis-aligned bounding box.
         */
        static Box buildAABB( const glm::vec3& origin, const glm::vec3& extent )
        {
            Box newBox(origin - extent, origin + extent);

            return newBox;
        }
    };


    class BoxSphereBounds
    {
    public:
        /** Holds the origin of the bounding box and sphere. */
        glm::vec3 mOrigin;

        /** Holds the extent of the bounding box. */
        glm::vec3 mBoxExtent;

        /** Holds the radius of the bounding sphere. */
        float mSphereRadius;

    public:
        /** Default constructor. */
        BoxSphereBounds() = default;

        /**
         * Creates and initializes a new instance from the specified parameters.
         *
         * @param InOrigin origin of the bounding box and sphere.
         * @param InBoxExtent half size of box.
         * @param InSphereRadius radius of the sphere.
         */
        BoxSphereBounds( const glm::vec3& origin, const glm::vec3& boxExtent, float sphereRadius )
            : mOrigin(origin)
            , mBoxExtent(boxExtent)
            , mSphereRadius(sphereRadius)
        {}

        /**
         * Creates and initializes a new instance from the given Box and Sphere.
         *
         * @param box The bounding box.
         * @param sphere The bounding sphere.
         */
        BoxSphereBounds( const Box& box, const Sphere& sphere )
        {
            box.getCenterAndExtents(mOrigin, mBoxExtent);

            mSphereRadius = std::min(glm::length(mBoxExtent), glm::length(sphere.mCenter - mOrigin) + sphere.mRadius);
        }

        /**
         * Creates and initializes a new instance the given Box.
         *
         * The sphere radius is taken from the extent of the box.
         *
         * @param box The bounding box.
         */
        BoxSphereBounds( const Box& box )
        {
            box.getCenterAndExtents(mOrigin, mBoxExtent);

            mSphereRadius = glm::length(mBoxExtent);
        }

        /**
         * Creates and initializes a new instance for the given sphere.
         */
        BoxSphereBounds( const Sphere& sphere )
        {
            mOrigin = sphere.mCenter;
            mSphereRadius = sphere.mRadius;
            mBoxExtent = glm::vec3(mSphereRadius);
        }
    
        /**
         * Creates and initializes a new instance from the given set of points.
         *
         * The sphere radius is taken from the extent of the box.
         *
         * @param Points The points to be considered for the bounding box.
         */
        BoxSphereBounds( const std::vector<glm::vec3>& points)
        {
            Box boundingBox;

            // find an axis aligned bounding box for the points.
            for (uint32_t pointIndex = 0; pointIndex < points.size(); pointIndex++)
            {
                boundingBox += points[pointIndex];
            }

            boundingBox.getCenterAndExtents(mOrigin, mBoxExtent);

            // using the center of the bounding box as the origin of the sphere, find the radius of the bounding sphere.
            float squaredSphereRadius = 0;

            for (uint32_t pointIndex = 0; pointIndex < points.size(); pointIndex++)
            {
                squaredSphereRadius = std::max<float>(squaredSphereRadius, glm::length2(points[pointIndex] - mOrigin));	// LWC_TODO: Precision loss
            }

            mSphereRadius = std::sqrt(squaredSphereRadius);
        }

        /**
         * Constructs a bounding volume containing both this and B.
         *
         * @param other The other bounding volume.
         * @return The combined bounding volume.
         */
        BoxSphereBounds operator+( const BoxSphereBounds& other ) const
        {
            Box boundingBox;

            boundingBox += (this->mOrigin - this->mBoxExtent);
            boundingBox += (this->mOrigin + this->mBoxExtent);
            boundingBox += (other.mOrigin - other.mBoxExtent);
            boundingBox += (other.mOrigin + other.mBoxExtent);

            // build a bounding sphere from the bounding box's origin and the radii of A and B.
            BoxSphereBounds result(boundingBox);

            result.mSphereRadius = std::min(result.mSphereRadius, std::max(glm::length(mOrigin - result.mOrigin) + mSphereRadius, glm::length(other.mOrigin - result.mOrigin) + other.mSphereRadius));

            return result;
        }

        /**
         * Constructs a bounding volume containing both this and B.
         *
         * @param other The other bounding volume.
         * @return The combined bounding volume.
         */
        BoxSphereBounds& operator+=( const BoxSphereBounds& other )
        {
            BoxSphereBounds boxSphereBounds(other);
            this->mOrigin = boxSphereBounds.mOrigin;
            this->mBoxExtent = boxSphereBounds.mBoxExtent;
            this->mSphereRadius = boxSphereBounds.mSphereRadius;

            return *this;
        }

        /**
         * Compare bounding volume this and other.
         *
         * @param other The other bounding volume.
         * @return true of they match.
         */
        bool operator==(const BoxSphereBounds& other) const
        {
            return mOrigin == other.mOrigin && mBoxExtent == other.mBoxExtent &&  mSphereRadius == other.mSphereRadius;
        }
        
        /**
         * Compare bounding volume this and other.
         *
         * @param other The other bounding volume.
         * @return true of they do not match.
         */	
        bool operator!=(const BoxSphereBounds& other) const
        {
            return !(*this == other);
        }

    public:

        /**
         * Calculates the squared distance from a point to a bounding box
         *
         * @param Point The point.
         * @return The distance.
         */
        float computeSquaredDistanceFromBoxToPoint(const glm::vec3& point) const
        {
            auto mins = mOrigin - mBoxExtent;
            auto maxs = mOrigin + mBoxExtent;

            // Accumulates the distance as we iterate axis
            float distSquared = 0;

            // Check each axis for min/max and add the distance accordingly
            // NOTE: Loop manually unrolled for > 2x speed up
            if (point.x < mins.x)
            {
                distSquared += glm::length2(point.x - mins.x);
            }
            else if (point.x > maxs.x)
            {
                distSquared += glm::length2(point.x - maxs.x);
            }

            if (point.y < mins.y)
            {
                distSquared += glm::length2(point.y - mins.y);
            }
            else if (point.y > maxs.y)
            {
                distSquared += glm::length2(point.y - maxs.y);
            }

            if (point.z < mins.z)
            {
                distSquared += glm::length2(point.z - mins.z);
            }
            else if (point.z > maxs.z)
            {
                distSquared += glm::length2(point.z - maxs.z);
            }

            return distSquared;
        }

        /**
         * Test whether the spheres from two BoxSphereBounds intersect/overlap.
         * 
         * @param  A First BoxSphereBounds to test.
         * @param  B Second BoxSphereBounds to test.
         * @param  Tolerance Error tolerance added to test distance.
         * @return true if spheres intersect, false otherwise.
         */
        static bool spheresIntersect(const BoxSphereBounds& a, const BoxSphereBounds& b, float tolerance = 1.e-4f)
        {  
            return  glm::length2(a.mOrigin - b.mOrigin) <= glm::length2(std::max<float>(0, a.mSphereRadius + b.mSphereRadius + tolerance));
        }

        /**
         * Test whether the boxes from two BoxSphereBounds intersect/overlap.
         * 
         * @param  A First BoxSphereBounds to test.
         * @param  B Second BoxSphereBounds to test.
         * @return true if boxes intersect, false otherwise.
         */
        static bool boxesIntersect(const BoxSphereBounds& a, const BoxSphereBounds& b)
        {
            return a.getBox().intersect(b.getBox());
        }

        /**
         * Gets the bounding box.
         *
         * @return The bounding box.
         */
        Box getBox() const
        {
            return Box(mOrigin - mBoxExtent, mOrigin + mBoxExtent);
        }

        /**
         * Gets the bounding sphere.
         *
         * @return The bounding sphere.
         */
        Sphere getSphere() const
        {
            return Sphere(mOrigin, mSphereRadius);
        }

        /**
         * Gets a bounding volume transformed by a matrix.
         *
         * @param M The matrix.
         * @return The transformed volume.
         */
        BoxSphereBounds transformBy( const glm::mat4& M ) const
        {
            BoxSphereBounds result;

            const auto vecOrigin = mOrigin;
            const auto vecExtent = mBoxExtent;

            auto newOrigin = M * glm::vec4(vecOrigin, 1.0f);
            auto newExtent = M * glm::vec4(vecExtent, 1.0f);

            result.mBoxExtent = glm::vec3(newExtent.x, newExtent.y, newExtent.z);
            result.mOrigin = glm::vec3(newOrigin.x, newOrigin.y, newOrigin.z);

            const glm::vec4 m0 = glm::vec4(M[0][0], M[1][0], M[2][0], M[3][0]); //glm::column(M, 0);
            const glm::vec4 m1 = glm::vec4(M[0][1], M[1][1], M[2][1], M[3][1]); //glm::column(M, 1);
            const glm::vec4 m2 = glm::vec4(M[0][2], M[1][2], M[2][2], M[3][2]); //glm::column(M, 2);
            const glm::vec4 m3 = glm::vec4(M[0][3], M[1][3], M[2][3], M[3][3]); //glm::column(M, 3);

            auto MaxRadius = m0 * m0;
            MaxRadius = m1 * m1 + MaxRadius;
            MaxRadius = m2 * m2 + MaxRadius;
            float radius = std::max(std::max(MaxRadius[0], MaxRadius[1]), MaxRadius[2]);
            result.mSphereRadius = std::sqrt(radius) * mSphereRadius;

            // For non-uniform scaling, computing sphere radius from a box results in a smaller sphere.
            float const boxExtentMagnitude = std::sqrt(glm::dot(newExtent, newExtent));
            result.mSphereRadius = std::min(result.mSphereRadius, boxExtentMagnitude);

            return BoxSphereBounds(result);
        }

    };

} // namespace SoftRenderer

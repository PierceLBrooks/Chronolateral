#ifndef NFE_MATH_UTILITIES_HPP
#define NFE_MATH_UTILITIES_HPP

#include <SFML3D/System/Vector2.hpp>
#include <SFML3D/System/Vector3.hpp>
#include <vector>
#include <complex>
#include <cmath>

namespace NFE
{
    namespace util
    {
        extern const float SQRT_TWO;
        extern const float SQRT_THREE;
        extern const float EULER;
        extern const float PI;
        extern const float HALF_PI;
        extern const float QUARTER_PI;
        extern const float TWO_PI;
        extern const float RAD_TO_DEG;
        extern const float DEG_TO_RAD;
        extern const float INFIN;

        float sqr(float num);
        std::complex<float> sqr(const std::complex<float>& num);
        float sqrt(float num);
        float getDistanceSquared(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2);
        float getDistance(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2);
        float getDistanceSquared(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2);
        float getDistance(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2);
        float getManhattanDistance(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2);
        float getManhattanDistance(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2);
        float getDirection(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2);
        float getDotProduct(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2);
        float getDotProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2);
        sf3d::Vector3f getCrossProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2);
        int getGCD(int x, int y);
        sf3d::Vector2i getAspectRatio(const sf3d::Vector2i& v);
        bool checkWithinBounds(int x, int y, int bounds);
        bool checkWithinBounds(const sf3d::Vector2i& p, int bounds);
        unsigned int hashRotate(unsigned int x, unsigned int y);
        void hashMix(unsigned int& a, unsigned int& b, unsigned int& c);
        void hashFinalMix(unsigned int& a, unsigned int& b, unsigned int& c);
        unsigned int hashFinalMixAlt(unsigned int a, unsigned int b, unsigned int c);
        unsigned int hashString(const std::string& str, unsigned int seed = 0);
        unsigned int hashInts(const std::vector<unsigned int>& ints, unsigned int seed = 0);
        float clamp(float front, float back, float value);
        float mix(float val1, float val2, float key);
        sf3d::Vector2f mix(const sf3d::Vector2f& vec1, const sf3d::Vector2f& vec2, float key);
        sf3d::Vector3f mix(const sf3d::Vector3f& vec1, const sf3d::Vector3f& vec2, float key);
        float getSquareRadius(float degrees);
        float getBoundedAngle(float angle, bool degrees);
        float getPolygonArea(const std::vector<sf3d::Vector2f>& vertices);
        void sortPolygonVertices(std::vector<sf3d::Vector2f>& vertices);
        sf3d::Vector3f getTriangleAngles(const sf3d::Vector2f& a, const sf3d::Vector2f& b, const sf3d::Vector2f& c);
        sf3d::Vector3f getTriangleAngles(const sf3d::Vector3f& sides);
        sf3d::Vector3f getTriangleAngles(float ab, float bc, float ca);
        sf3d::Vector3f getTriangleSides(const sf3d::Vector2f& a, const sf3d::Vector2f& b, const sf3d::Vector2f& c);
        bool checkCoplanarity(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2, const sf3d::Vector3f& p3, const sf3d::Vector3f& p4);
        //bool checkColinearity(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2, const sf3d::Vector2f& p3, const sf3d::Vector2f& p4);
        bool checkColinearity(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2, const sf3d::Vector2f& p3);
        sf3d::Vector3f getNormal(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2, const sf3d::Vector3f& p3);
        sf3d::Vector3f getProjection(const sf3d::Vector3f& point, const sf3d::Vector3f& origin, const sf3d::Vector3f& normal);
        float getMagnitude(const sf3d::Vector2f& vec);
        float getMagnitude(const sf3d::Vector3f& vec);
        sf3d::Vector2f getNormalized(const sf3d::Vector2f& vec);
        sf3d::Vector3f getNormalized(const sf3d::Vector3f& vec);
        float getAngleDifference(float first, float second, bool degreesFirst = false, bool degreesSecond = false, bool degreesDifference = false);
    }
}

#endif // NFE_MATH_UTILITIES_HPP

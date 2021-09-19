#include <NFE/MathUtilities.hpp>
#include <algorithm>

const float NFE::util::SQRT_TWO = sqrtf(2.0f);

const float NFE::util::SQRT_THREE = sqrtf(3.0f);

const float NFE::util::EULER = 2.718281828f;

const float NFE::util::PI = 3.14159256359f;

const float NFE::util::HALF_PI = PI*0.5f;

const float NFE::util::QUARTER_PI = HALF_PI*0.5f;

const float NFE::util::TWO_PI = PI*2.0f;

const float NFE::util::RAD_TO_DEG = 180.0f/PI;

const float NFE::util::DEG_TO_RAD = PI/180.0f;

const float NFE::util::INFIN = std::numeric_limits<float>::infinity();

float NFE::util::sqr(float num)
{
    return num*num;
}

std::complex<float> NFE::util::sqr(const std::complex<float>& num)
{
    return num*num;
}

float NFE::util::sqrt(float num)
{
    if (num != fabsf(num))
    {
        return -sqrtf(fabsf(num));
    }
    return sqrtf(num);
}

float NFE::util::getDistanceSquared(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return sqr(p1.x-p2.x)+sqr(p1.y-p2.y);
}

float NFE::util::getDistance(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return sqrtf(getDistanceSquared(p1,p2));
}

float NFE::util::getDistanceSquared(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return sqr(p1.x-p2.x)+sqr(p1.y-p2.y)+sqr(p1.z-p1.z);
}

float NFE::util::getDistance(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return sqrtf(getDistanceSquared(p1,p2));
}

float NFE::util::getManhattanDistance(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return fabsf(p1.x-p2.x)+fabsf(p1.y-p2.y);
}

float NFE::util::getManhattanDistance(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return fabsf(p1.x-p2.x)+fabsf(p1.y-p2.y)+fabsf(p1.z-p2.z);
}

float NFE::util::getDirection(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return atan2f(p2.y-p1.y,p2.x-p1.x);
}

float NFE::util::getDotProduct(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return (p1.x*p2.x)+(p1.y*p2.y);
}

float NFE::util::getDotProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return (p1.x*p2.x)+(p1.y*p2.y)+(p1.z*p2.z);
}

sf3d::Vector3f NFE::util::getCrossProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return sf3d::Vector3f((p1.y*p2.z)-(p1.z*p2.y),(p1.z*p2.x)-(p1.x*p2.z),(p1.x*p2.y)-(p1.y*p2.x));
}

int NFE::util::getGCD(int x, int y)
{
    int temp;
    while (y != 0)
    {
        temp = y;
        y = x%y;
        x = temp;
    }
    return x;
}

sf3d::Vector2i NFE::util::getAspectRatio(const sf3d::Vector2i& v)
{
    sf3d::Vector2i temp;
    temp.x = getGCD(v.x,v.y);
    temp.y = v.y/temp.x;
    temp.x = v.x/temp.x;
    return temp;
}

bool NFE::util::checkWithinBounds(int x, int y, int bounds)
{
    if ((x < 0) || (y < 0) || (x >= bounds) || (y >= bounds))
    {
        return false;
    }
    return true;
}

bool NFE::util::checkWithinBounds(const sf3d::Vector2i& p, int bounds)
{
    return checkWithinBounds(p.x,p.y,bounds);
}

unsigned int NFE::util::hashRotate(unsigned int x, unsigned int y)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    return ((x<<y)|(x>>(32-y)));
}

void NFE::util::hashMix(unsigned int& a, unsigned int& b, unsigned int& c)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    a -= c;
    a ^= hashRotate(c,4);
    c += b;
    b -= a;
    b ^= hashRotate(a,6);
    a += c;
    c -= b;
    c ^= hashRotate(b,8);
    b += a;
    a -= c;
    a ^= hashRotate(c,16);
    c += b;
    b -= a;
    b ^= hashRotate(a,19);
    a += c;
    c -= b;
    c ^= hashRotate(b,4);
    b += a;
}

void NFE::util::hashFinalMix(unsigned int& a, unsigned int& b, unsigned int& c)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    c ^= b;
    c -= hashRotate(b,14);
    a ^= c;
    a -= hashRotate(c,11);
    b ^= a;
    b -= hashRotate(a,25);
    c ^= b;
    c -= hashRotate(b,16);
    a ^= c;
    a -= hashRotate(c,4);
    b ^= a;
    b -= hashRotate(a,14);
    c ^= b;
    c -= hashRotate(b,24);
}

unsigned int NFE::util::hashFinalMixAlt(unsigned int a, unsigned int b, unsigned int c)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    c ^= b;
    c -= hashRotate(b,14);
    a ^= c;
    a -= hashRotate(c,11);
    b ^= a;
    b -= hashRotate(a,25);
    c ^= b;
    c -= hashRotate(b,16);
    a ^= c;
    a -= hashRotate(c,4);
    b ^= a;
    b -= hashRotate(a,14);
    c ^= b;
    c -= hashRotate(b,24);
    return c;
}

unsigned int NFE::util::hashString(const std::string& str, unsigned int seed)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    unsigned int a, b, c, i = 0, length = str.size();
    a = 0xdeadbeef+(length<<2)+seed;
    b = a;
    c = a;
    while (length > 3)
    {
        a += static_cast<unsigned int>(str[(i*3)+0]);
        b += static_cast<unsigned int>(str[(i*3)+1]);
        c += static_cast<unsigned int>(str[(i*3)+2]);
        hashMix(a,b,c);
        length -= 3;
        ++i;
    }
    switch (length)
    {
    case 3:
        c += static_cast<unsigned int>(str[2]);
    case 2:
        b += static_cast<unsigned int>(str[1]);
    case 1:
        a += static_cast<unsigned int>(str[0]);
        hashFinalMix(a,b,c);
    case 0:
        break;
    }
    return c;
}

unsigned int NFE::util::hashInts(const std::vector<unsigned int>& ints, unsigned int seed)
{
    /*
    Source: http://burtleburtle.net/bob/c/lookup3.c
    Public domain, no warranty
    Original source code has been altered.
    */
    unsigned int a, b, c, i = 0, length = ints.size();
    a = 0xdeadbeef+(length<<2)+seed;
    b = a;
    c = a;
    while (length > 3)
    {
        a += ints[(i*3)+0];
        b += ints[(i*3)+1];
        c += ints[(i*3)+2];
        hashMix(a,b,c);
        length -= 3;
        ++i;
    }
    switch (length)
    {
    case 3:
        c += ints[2];
    case 2:
        b += ints[1];
    case 1:
        a += ints[0];
        hashFinalMix(a,b,c);
    case 0:
        break;
    }
    return c;
}

float NFE::util::clamp(float front, float back, float value)
{
    return std::min(std::max(value,front),back);
}

float NFE::util::mix(float val1, float val2, float key)
{
    return (val2*key)+(val1*(1.0f-key));
}

sf3d::Vector2f NFE::util::mix(const sf3d::Vector2f& vec1, const sf3d::Vector2f& vec2, float key)
{
    return sf3d::Vector2f(mix(vec1.x,vec2.x,key),mix(vec1.y,vec2.y,key));
}

sf3d::Vector3f NFE::util::mix(const sf3d::Vector3f& vec1, const sf3d::Vector3f& vec2, float key)
{
    return sf3d::Vector3f(mix(vec1.x,vec2.x,key),mix(vec1.y,vec2.y,key),mix(vec1.z,vec2.z,key));
}

float NFE::util::getSquareRadius(float degrees)
{
    degrees = getBoundedAngle(degrees,true);
    degrees = fmodf(degrees+45.0f,90.0f)-45.0f;
    return 1.0f/cosf(degrees*DEG_TO_RAD);
}

float NFE::util::getBoundedAngle(float angle, bool degrees)
{
    float temp = degrees?360.0f:TWO_PI;
    while (angle >= temp)
    {
        angle -= temp;
    }
    while (angle <= 0.0f)
    {
        angle += temp;
    }
    return angle;
}

float NFE::util::getPolygonArea(const std::vector<sf3d::Vector2f>& vertices)
{
    /*
    Source: http://code.activestate.com/recipes/578047-area-of-polygon-using-shoelace-formula/
    Licensed under the MIT License
    Original source code has been altered.
    */
    unsigned int temp;
    float area = 0.0f;
    for (unsigned int i = 0; i != vertices.size(); ++i)
    {
        temp = (i+1)%vertices.size();
        area += vertices[i].x*vertices[temp].y;
        area -= vertices[temp].x*vertices[i].y;
    }
    return fabsf(area)*0.5f;
}

void NFE::util::sortPolygonVertices(std::vector<sf3d::Vector2f>& vertices)
{
    sf3d::Vector2f center;
    std::vector<sf3d::Vector3f> sorting;
    for (unsigned int i = 0; i != vertices.size(); ++i)
    {
        center += vertices[i];
    }
    center *= 1.0f/static_cast<float>(vertices.size());
    for (unsigned int i = 0; i != vertices.size(); ++i)
    {
        sorting.push_back(sf3d::Vector3f(vertices[i].x,vertices[i].y,getBoundedAngle(getDirection(center,vertices[i]),false)));
    }
    std::sort(sorting.begin(),sorting.end(),[](const sf3d::Vector3f& first, const sf3d::Vector3f& second){return (first.z<second.z);});
    for (unsigned int i = 0; i != vertices.size(); ++i)
    {
        vertices[i].x = sorting[i].x;
        vertices[i].y = sorting[i].y;
    }
    sorting.clear();
}

sf3d::Vector3f NFE::util::getTriangleAngles(const sf3d::Vector2f& a, const sf3d::Vector2f& b, const sf3d::Vector2f& c)
{
    return getTriangleAngles(getTriangleSides(a,b,c));
}

sf3d::Vector3f NFE::util::getTriangleAngles(const sf3d::Vector3f& sides)
{
    return getTriangleAngles(sides.x,sides.y,sides.z);
}

sf3d::Vector3f NFE::util::getTriangleAngles(float ab, float bc, float ca)
{
    sf3d::Vector3f result;
    unsigned int index;
    bool check;
    float temp;
    float total = 180.0f;
    float& king = ab;
    float& major = bc;
    float& minor = ca;
    if (ab > bc)
    {
        if (ab > ca)
        {
            index = 0;
            king = ab;
            if (bc > ca)
            {
                check = true;
                major = bc;
                minor = ca;
            }
            else
            {
                check = false;
                major = ca;
                minor = bc;
            }
        }
        else
        {
            index = 2;
            king = ca;
            check = true;
            major = ab;
            minor = bc;
        }
    }
    else
    {
        if (bc > ca)
        {
            index = 1;
            king = ab;
            if (ab > ca)
            {
                check = true;
                major = ab;
                minor = ca;
            }
            else
            {
                check = false;
                major = ca;
                minor = ab;
            }
        }
        else
        {
            index = 2;
            king = ca;
            check = false;
            major = bc;
            minor = ab;
        }
    }
    temp = sqr(major)+sqr(minor);
    temp -= sqr(king);
    temp /= 2.0f*major*minor;
    temp = acosf(temp);
    switch (index)
    {
    case 0:
        result.x = temp;
        break;
    case 1:
        result.y = temp;
        break;
    case 2:
        result.z = temp;
        break;
    default:
        return sf3d::Vector3f();
        break;
    }
    total -= temp;
    temp = major*sinf(temp);
    temp /= king;
    temp = asinf(temp);
    checkpoint:
    switch (index%3)
    {
    case 0:
        if (check)
        {
            result.y = temp;
        }
        else
        {
            result.z = temp;
        }
        break;
    case 1:
        if (check)
        {
            result.x = temp;
        }
        else
        {
            result.z = temp;
        }
        break;
    case 2:
        if (check)
        {
            result.x = temp;
        }
        else
        {
            result.y = temp;
        }
        break;
    default:
        return sf3d::Vector3f();
        break;
    }
    if (index < 3)
    {
        index += 3;
        total -= temp;
        temp = total;
        check = !check;
        goto checkpoint;
    }
    return result;
}

sf3d::Vector3f NFE::util::getTriangleSides(const sf3d::Vector2f& a, const sf3d::Vector2f& b, const sf3d::Vector2f& c)
{
    return sf3d::Vector3f(getDistance(b,c),getDistance(c,a),getDistance(a,b));
}

bool NFE::util::checkCoplanarity(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2, const sf3d::Vector3f& p3, const sf3d::Vector3f& p4)
{
    return (getDotProduct((p3-p1),getCrossProduct(p2-p1,p4-p1))!=0.0f);
}

/*bool NFE::util::checkColinearity(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2, const sf3d::Vector2f& p3, const sf3d::Vector2f& p4)
{
    return false;
}*/

bool NFE::util::checkColinearity(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2, const sf3d::Vector2f& p3)
{
    return ((getDistanceSquared(p1,p2)+getDistanceSquared(p1,p3))==getDistanceSquared(p2,p3));
}

sf3d::Vector3f NFE::util::getNormal(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2, const sf3d::Vector3f& p3)
{
    return getCrossProduct(p1-p2,p1-p3);
}

sf3d::Vector3f NFE::util::getProjection(const sf3d::Vector3f& point, const sf3d::Vector3f& origin, const sf3d::Vector3f& pivot)
{
    sf3d::Vector3f normal = getNormalized(getNormal(point,origin,pivot));
    sf3d::Vector3f axisFirst = getNormalized(pivot-origin);
    sf3d::Vector3f axisSecond = getNormalized(getCrossProduct(normal,axisFirst));
    sf3d::Vector3f displacement = point-origin;
    return sf3d::Vector3f(getDotProduct(axisFirst,displacement),getDotProduct(axisSecond,displacement),getDotProduct(normal,displacement)); // x = distance along first axis, y = distance along second axis, z = distance along normal
}

float NFE::util::getMagnitude(const sf3d::Vector2f& vec)
{
    return sqrtf(sqr(vec.x)+sqr(vec.y));
}

float NFE::util::getMagnitude(const sf3d::Vector3f& vec)
{
    return sqrtf(sqr(vec.x)+sqr(vec.y)+sqr(vec.z));
}


sf3d::Vector2f NFE::util::getNormalized(const sf3d::Vector2f& vec)
{
    return vec*(1.0f/getMagnitude(vec));
}

sf3d::Vector3f NFE::util::getNormalized(const sf3d::Vector3f& vec)
{
    return vec*(1.0f/getMagnitude(vec));
}

float NFE::util::getAngleDifference(float first, float second, bool degreesFirst, bool degreesSecond, bool degreesDifference)
{
    float difference;
    if (degreesFirst)
    {
        first *= DEG_TO_RAD;
    }
    if (degreesSecond)
    {
        second *= DEG_TO_RAD;
    }
    difference = second-first;
    difference = atan2f(sinf(difference),cosf(difference));
    if (degreesDifference)
    {
        difference *= RAD_TO_DEG;
    }
    return difference;
}

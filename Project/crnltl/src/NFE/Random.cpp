
// Author: Pierce Brooks

#include <NFE/Random.hpp>

NFE::Random::Random(unsigned int seed)
{
    initialize(seed);
}

NFE::Random::~Random()
{

}

void NFE::Random::initialize(unsigned int seed)
{
    rng.seed(seed);
}

bool NFE::Random::getBool()
{
    return (getInt(0,1)==0);
}

bool NFE::Random::getBool(float chance)
{
    if ((chance < 0.0f) || (chance > 1.0f))
    {
        return false;
    }
    return (getFloat(0.0f,1.0f)<chance);
}

int NFE::Random::getInt(int minimum, int maximum)
{
    distributorInt = std::uniform_int_distribution<int>(minimum, maximum);
    return distributorInt(rng);
}

float NFE::Random::getFloat(float minimum, float maximum)
{
    distributorFloat = std::uniform_real_distribution<float>(minimum, maximum);
    return distributorFloat(rng);
}

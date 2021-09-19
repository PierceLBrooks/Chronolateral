#ifndef NFE_GRID_HPP
#define NFE_GRID_HPP

#include <NFE/MathUtilities.hpp>
#include <SFML3D/Graphics/Image.hpp>
#include <map>
#include <set>

namespace NFE
{
    template <class T>
    class Grid
    {
        public:
            enum Topology
            {
                TORUS,
                SPHERE,
                PLANE,
                QUINCUNCIAL
            };
            class Unit
            {
                public:
                    Unit(Grid* owner, const sf3d::Vector2u& index, T payload) : owner(owner), index(index), payload(payload) {}
                    virtual ~Unit()
                    {
                        if (owner->getIsResponsible())
                        {
                            delete payload;
                        }
                    }
                    Grid* getOwner() const
                    {
                        return owner;
                    }
                    const sf3d::Vector2u& getIndex() const
                    {
                        return index;
                    }
                    T getPayload() const
                    {
                        return payload;
                    }
                    void setPayload(T payload)
                    {
                        this->payload = payload;
                    }
                    float getDirection(Unit* other) const
                    {
                        return getDirection(sf3d::Vector2f(index),sf3d::Vector2f(getRelativeIndex(other)));
                    }
                    float getDistance(Unit* other) const
                    {
                        return sqrtf(getDistanceSquared(other));
                    }
                    float getDistanceSquared(Unit* other) const
                    {
                        return getRelativePosition(other).z;
                    }
                    sf3d::Vector2i getRelativeIndex(Unit* other) const
                    {
                        sf3d::Vector3f temp = getRelativePosition(other);
                        return sf3d::Vector2i(sf3d::Vector2f(temp.x,temp.y));
                    }
                    sf3d::Vector3f getRelativePosition(Unit* other) const
                    {
                        if (other == nullptr)
                        {
                            return sf3d::Vector3f(sf3d::Vector2f(index),util::INFIN);
                        }
                        sf3d::Vector3f result = sf3d::Vector3f(sf3d::Vector2f(other->getIndex()),util::INFIN);
                        if (owner != other->getOwner())
                        {
                            return result;
                        }
                        switch (owner->getTopology())
                        {
                        case Topology::PLANE:
                            result.z = util::getDistanceSquared(sf3d::Vector2f(index),sf3d::Vector2f(result.x,result.y));
                            break;
                        case Topology::SPHERE:
                            {
                                sf3d::Vector2f size = sf3d::Vector2f(owner->getSize());
                                sf3d::Vector2f position = sf3d::Vector2f(index);
                                sf3d::Vector2f positionOther;
                                sf3d::Vector2f distances;
                                distances.y = util::INFIN;
                                for (int i = -1; i != 2; ++i)
                                {
                                    /*for (float j = 0.0f; j < size.y; j += size.y-1.0f)
                                    {
                                        positionOther = sf3d::Vector2f(other->getIndex());
                                        positionOther.x += static_cast<float>(i)*size.x;
                                        if (positionOther.y == j)
                                        {
                                            positionOther.x = fmodf(positionOther.x+(size.x*0.5f),size.x);
                                            if (j == 0.0f)
                                            {
                                                positionOther.y -= 1.0f;
                                            }
                                            else
                                            {
                                                positionOther.y += 1.0f;
                                            }
                                        }
                                        distances.x = util::getDistanceSquared(position,positionOther);
                                        if (distances.x < distances.y)
                                        {
                                            distances.y = distances.x;
                                            result.x = positionOther.x;
                                            result.y = positionOther.y;
                                        }
                                    }*/
                                    for (int j = -1; j != 2; ++j)
                                    {
                                        positionOther = sf3d::Vector2f(other->getIndex());
                                        if (j != 0)
                                        {
                                            positionOther.x = fmodf(positionOther.x+(size.x*0.5f),size.x);
                                            positionOther.y = size.y-positionOther.y;
                                        }
                                        positionOther += sf3d::Vector2f(static_cast<float>(i)*size.x,static_cast<float>(j)*size.y);
                                        distances.x = util::getDistanceSquared(position,positionOther);
                                        if (distances.x < distances.y)
                                        {
                                            distances.y = distances.x;
                                            result.x = positionOther.x;
                                            result.y = positionOther.y;
                                        }
                                    }
                                }
                                result.z = distances.y;
                            }
                            break;
                        case Topology::TORUS:
                            {
                                sf3d::Vector2f size = sf3d::Vector2f(owner->getSize());
                                sf3d::Vector2f position = sf3d::Vector2f(index);
                                sf3d::Vector2f positionOther;
                                sf3d::Vector2f distances;
                                distances.y = util::INFIN;
                                for (int i = -1; i != 2; ++i)
                                {
                                    for (int j = -1; j != 2; ++j)
                                    {
                                        positionOther = sf3d::Vector2f(other->getIndex());
                                        positionOther += sf3d::Vector2f(static_cast<float>(i)*size.x,static_cast<float>(j)*size.y);
                                        distances.x = util::getDistanceSquared(position,positionOther);
                                        if (distances.x < distances.y)
                                        {
                                            distances.y = distances.x;
                                            result.x = positionOther.x;
                                            result.y = positionOther.y;
                                        }
                                    }
                                }
                                result.z = distances.y;
                            }
                            break;
                        case QUINCUNCIAL:
                            {
                                sf3d::Vector2f size = sf3d::Vector2f(owner->getSize());
                                sf3d::Vector2f position = sf3d::Vector2f(index);
                                sf3d::Vector2f positionOther;
                                sf3d::Vector2f distances;
                                distances.y = util::INFIN;
                                for (int i = -1; i != 2; ++i)
                                {
                                    for (int j = -1; j != 2; ++j)
                                    {
                                        positionOther = sf3d::Vector2f(other->getIndex());
                                        if (abs(i%2) != abs(j%2))
                                        {
                                            positionOther.x = (static_cast<float>(size.x)-1.0f)-positionOther.x;
                                            positionOther.y = (static_cast<float>(size.y)-1.0f)-positionOther.y;
                                        }
                                        positionOther += sf3d::Vector2f(static_cast<float>(i)*size.x,static_cast<float>(j)*size.y);
                                        distances.x = util::getDistance(position,positionOther);
                                        if (distances.x < distances.y)
                                        {
                                            distances.y = distances.x;
                                            result.x = positionOther.x;
                                            result.y = positionOther.y;
                                        }
                                    }
                                }
                                result.z = distances.y;
                            }
                            break;
                        }
                        return result;
                    }
                private:
                    sf3d::Vector2u index;
                    Grid* owner;
                    T payload;
            };
            class Neighborhood
            {
                public:
                    enum Style
                    {
                        MOORE,
                        VON_NEUMANN,
                        EUCLID,
                        MENAECHMUS,
                        SQUARE = MOORE,
                        DIAMOND = VON_NEUMANN,
                        CIRCLE = EUCLID,
                        ELLIPSE = MENAECHMUS
                    };
                    typedef std::vector<sf3d::Vector2i> Contents;
                    typedef std::map<float,Contents> RadiusRightMap;
                    typedef std::map<float,RadiusRightMap> RadiusLeftMap;
                    typedef std::map<unsigned int,RadiusLeftMap> IndexRightMap;
                    typedef std::map<unsigned int,IndexRightMap> IndexLeftMap;
                    typedef std::map<Style,IndexLeftMap> StyleMap;
                    typedef std::map<Topology,StyleMap> TopologyMap;
                    typedef std::map<const Grid*,TopologyMap> Bank;
                    Neighborhood(Style style = MOORE, bool cache = false) :
                        style(style),
                        bank(nullptr),
                        relativity(false),
                        origin()
                    {
                        setCache(cache);
                    }
                    virtual ~Neighborhood()
                    {
                        contents.clear();
                        setCache(false);
                    }
                    bool updateFromCache(const Grid* grid, const sf3d::Vector2u& index, const sf3d::Vector2f& radius)
                    {
                        if (!cache)
                        {
                            return false;
                        }
                        typename Bank::iterator iter0 = bank->find(grid);
                        if (iter0 != bank->end())
                        {
                            typename TopologyMap::iterator iter1 = iter0->second.find(grid->getTopology());
                            if (iter1 != iter0->second.end())
                            {
                                typename StyleMap::iterator iter2 = iter1->second.find(style);
                                if (iter2 != iter1->second.end())
                                {
                                    typename IndexLeftMap::iterator iter3 = iter2->second.find(index.x);
                                    if (iter3 != iter2->second.end())
                                    {
                                        typename IndexRightMap::iterator iter4 = iter3->second.find(index.y);
                                        if (iter4 != iter3->second.end())
                                        {
                                            typename RadiusLeftMap::iterator iter5 = iter4->second.find(radius.x);
                                            if (iter5 != iter4->second.end())
                                            {
                                                typename RadiusRightMap::iterator iter6 = iter5->second.find(radius.y);
                                                if (iter6 != iter5->second.end())
                                                {
                                                    contents = iter6->second;
                                                    return true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        return false;
                    }
                    bool update(const Grid* grid, const sf3d::Vector2u& index, float radius)
                    {
                        return update(grid,index,sf3d::Vector2f(radius,radius));
                    }
                    virtual bool update(const Grid* grid, const sf3d::Vector2u& index, const sf3d::Vector2f& radius)
                    {
                        contents.clear();
                        origin = index;
                        sf3d::Vector2i bounds = sf3d::Vector2i(sf3d::Vector2f(std::round(fabsf(radius.x)),std::round(fabsf(radius.y))));
                        if ((bounds.x == 0) || (bounds.y == 0))
                        {
                            return false;
                        }
                        if ((radius.x != radius.y) && (style != MENAECHMUS))
                        {
                            return false;
                        }
                        if (updateFromCache(grid,index,radius))
                        {
                            return true;
                        }
                        sf3d::Vector2i position;
                        sf3d::Vector2i temp;
                        for (int x = -bounds.x; x != bounds.x+1; ++x)
                        {
                            for (int y = -bounds.y; y != bounds.y+1; ++y)
                            {
                                if ((x != 0) || (y != 0))
                                {
                                    position = sf3d::Vector2i(index);
                                    position.x += x;
                                    position.y += y;
                                    temp.x = position.x;
                                    temp.y = position.y;
                                    if (!relativity)
                                    {
                                        if ((position.x < 0) || (position.x >= grid->getSize().x) || (position.y < 0) || (position.y >= grid->getSize().y))
                                        {
                                            if ((bounds.x >= static_cast<int>(grid->getSize().x)) || (bounds.y >= static_cast<int>(grid->getSize().y)))
                                            {
                                                position = sf3d::Vector2i(grid->getAbsoluteIndex(position));
                                            }
                                            else
                                            {
                                                switch (grid->getTopology())
                                                {
                                                case TORUS:
                                                    if (position.x < 0)
                                                    {
                                                        position.x += static_cast<int>(grid->getSize().x);
                                                    }
                                                    if (position.x >= static_cast<int>(grid->getSize().x))
                                                    {
                                                        position.x -= static_cast<int>(grid->getSize().x);
                                                    }
                                                    if (position.y < 0)
                                                    {
                                                        position.y += static_cast<int>(grid->getSize().y);
                                                    }
                                                    if (position.y >= static_cast<int>(grid->getSize().y))
                                                    {
                                                        position.y -= static_cast<int>(grid->getSize().y);
                                                    }
                                                    break;
                                                case SPHERE:
                                                    if (position.x < 0)
                                                    {
                                                        position.x += static_cast<int>(grid->getSize().x);
                                                    }
                                                    if (position.x >= static_cast<int>(grid->getSize().x))
                                                    {
                                                        position.x -= static_cast<int>(grid->getSize().x);
                                                    }
                                                    if (position.y < 0)
                                                    {
                                                        position.x = (position.x+(static_cast<int>(grid->getSize().x)/2))%static_cast<int>(grid->getSize().x);
                                                        position.y = 0-(position.y-0);
                                                    }
                                                    if (position.y >= static_cast<int>(grid->getSize().y))
                                                    {
                                                        position.x = (position.x+(static_cast<int>(grid->getSize().x)/2))%static_cast<int>(grid->getSize().x);
                                                        position.y = (static_cast<int>(grid->getSize().y)-1)-(position.y-static_cast<int>(grid->getSize().y));
                                                    }
                                                    break;
                                                case PLANE:
                                                    break;
                                                case QUINCUNCIAL:
                                                    {
                                                        bool horizontal = false;
                                                        bool vertical = false;
                                                        if (position.x < 0)
                                                        {
                                                            position.x = 0-(position.x-0);
                                                            horizontal = true;
                                                        }
                                                        if (position.x >= static_cast<int>(grid->getSize().x))
                                                        {
                                                            position.x = (static_cast<int>(grid->getSize().x)-1)-(position.x-static_cast<int>(grid->getSize().x));
                                                            horizontal = true;
                                                        }
                                                        if (position.y < 0)
                                                        {
                                                            position.y = 0-(position.y-0);
                                                            vertical = true;
                                                        }
                                                        if (position.y >= static_cast<int>(grid->getSize().y))
                                                        {
                                                            position.y = (static_cast<int>(grid->getSize().y)-1)-(position.y-static_cast<int>(grid->getSize().y));
                                                            vertical = true;
                                                        }
                                                        if (horizontal)
                                                        {
                                                            position.x = (static_cast<int>(grid->getSize().x)-1)-position.x;
                                                        }
                                                        if (vertical)
                                                        {
                                                            position.y = (static_cast<int>(grid->getSize().y)-1)-position.y;
                                                        }
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if ((relativity) || ((position.x >= 0) && (position.x < grid->getSize().x) && (position.y >= 0) && (position.y < grid->getSize().y)))
                                    {
                                        switch (style)
                                        {
                                        case MOORE:
                                            contents.push_back(position);
                                            break;
                                        case VON_NEUMANN:
                                            if (util::getManhattanDistance(sf3d::Vector2f(temp),sf3d::Vector2f(index)) < radius.x+0.5f)
                                            {
                                                contents.push_back(position);
                                            }
                                            break;
                                        case EUCLID:
                                            if (util::getDistance(sf3d::Vector2f(temp),sf3d::Vector2f(index)) < radius.x+0.5f)
                                            {
                                                contents.push_back(position);
                                            }
                                            break;
                                        case MENAECHMUS:
                                            if ((util::sqr(static_cast<float>(index.x)-static_cast<float>(temp.x))/util::sqr(radius.x))+
                                                (util::sqr(static_cast<float>(index.y)-static_cast<float>(temp.y))/util::sqr(radius.y)) < 1.0f)
                                            {
                                                contents.push_back(position);
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        if (cache)
                        {
                            typename Bank::iterator iter0 = bank->find(grid);
                            if (iter0 != bank->end())
                            {
                                typename TopologyMap::iterator iter1 = iter0->second.find(grid->getTopology());
                                if (iter1 != iter0->second.end())
                                {
                                    typename StyleMap::iterator iter2 = iter1->second.find(style);
                                    if (iter2 != iter1->second.end())
                                    {
                                        typename IndexLeftMap::iterator iter3 = iter2->second.find(index.x);
                                        if (iter3 != iter2->second.end())
                                        {
                                            typename IndexRightMap::iterator iter4 = iter3->second.find(index.y);
                                            if (iter4 != iter3->second.end())
                                            {
                                                typename RadiusLeftMap::iterator iter5 = iter4->second.find(radius.x);
                                                if (iter5 != iter4->second.end())
                                                {
                                                    iter5->second[radius.y] = contents;
                                                }
                                                else
                                                {
                                                    iter4->second[radius.x] = {{radius.y,contents}};
                                                }
                                            }
                                            else
                                            {
                                                iter3->second[index.y] = {{radius.x,{{radius.y,contents}}}};
                                            }
                                        }
                                        else
                                        {
                                            iter2->second[index.x] = {{index.y,{{radius.x,{{radius.y,contents}}}}}};
                                        }
                                    }
                                    else
                                    {
                                        iter1->second[style] = {{index.x,{{index.y,{{radius.x,{{radius.y,contents}}}}}}}};
                                    }
                                }
                                else
                                {
                                    iter0->second[grid->getTopology()] = {{style,{{index.x,{{index.y,{{radius.x,{{radius.y,contents}}}}}}}}}};
                                }
                            }
                            else
                            {
                                (*bank)[grid] = {{grid->getTopology(),{{style,{{index.x,{{index.y,{{radius.x,{{radius.y,contents}}}}}}}}}}}};
                            }
                        }
                        return true;
                    }
                    void setStyle(Style style)
                    {
                        this->style = style;
                    }
                    Style getStyle() const
                    {
                        return style;
                    }
                    void setCache(bool cache)
                    {
                        if (this->cache != cache)
                        {
                            if (cache)
                            {
                                bank = new Bank();
                            }
                            else
                            {
                                delete bank;
                                bank = nullptr;
                            }
                            this->cache = cache;
                        }
                    }
                    bool getCache() const
                    {
                        return cache;
                    }
                    void setRelativity(bool relativity)
                    {
                        if (this->relativity != relativity)
                        {
                            if (cache)
                            {
                                delete bank;
                                bank = new Bank();
                            }
                            this->relativity = relativity;
                        }
                    }
                    bool getRelativity() const
                    {
                        return relativity;
                    }
                    unsigned int getSize() const
                    {
                        return contents.size();
                    }
                    const Contents& getContents() const
                    {
                        return contents;
                    }
                    const sf3d::Vector2u& getOrigin() const
                    {
                        return origin;
                    }
                    void findOrthogonalComplement(const Grid* grid, const sf3d::Vector2u& index, float radius, bool reorder = false)
                    {
                        findOrthogonalComplement(grid,index,sf3d::Vector2f(radius,radius),reorder);
                    }
                    void findOrthogonalComplement(const Grid* grid, const sf3d::Vector2u& index, const sf3d::Vector2f& radius, bool reorder = false)
                    {
                        Style temp = style;
                        style = MOORE;
                        if (update(grid,index,radius))
                        {
                            std::set<Unit*> neighborhood;
                            for (unsigned int i = 0; i != contents.size(); ++i)
                            {
                                neighborhood.insert(grid->getUnit(this,i));
                            }
                            style = VON_NEUMANN;
                            if (update(grid,index,radius))
                            {
                                for (unsigned int i = 0; i != contents.size(); ++i)
                                {
                                    typename std::set<Unit*>::iterator iter = neighborhood.find(grid->getUnit(this,i));
                                    if (iter != neighborhood.end())
                                    {
                                        neighborhood.erase(iter);
                                    }
                                }
                                contents.clear();
                                for (typename std::set<Unit*>::iterator iter = neighborhood.begin(); iter != neighborhood.end(); ++iter)
                                {
                                    contents.push_back(sf3d::Vector2i((*iter)->getIndex()));
                                }
                                neighborhood.clear();
                                if (reorder)
                                {
                                    Unit* center = grid->getUnit(index);
                                    std::vector<sf3d::Vector3f> sorting;
                                    for (unsigned int i = 0; i != contents.size(); ++i)
                                    {
                                        sorting.push_back(sf3d::Vector3f(sf3d::Vector2f(contents[i]),util::getBoundedAngle(center->getDirection(grid->getUnit(this,i)),false)));
                                    }
                                    std::sort(sorting.begin(),sorting.end(),[](const sf3d::Vector3f& first, const sf3d::Vector3f& second){return (first.z<second.z);});
                                    for (unsigned int i = 0; i != contents.size(); ++i)
                                    {
                                        contents[i].x = static_cast<unsigned int>(sorting[i].x);
                                        contents[i].y = static_cast<unsigned int>(sorting[i].y);
                                    }
                                    sorting.clear();
                                }
                            }
                        }
                        style = temp;
                    }
                    void report(const Grid* grid, bool log = false)
                    {
                        Unit* unit = grid->getUnit(origin);
                        Unit* unitOther;
                        std::string info;
                        sf3d::Vector2u index;
                        info = "\nNeighborhood Size = ";
                        info += std::to_string(contents.size());
                        info += "\n";
                        for (unsigned int i = 0; i != contents.size(); ++i)
                        {
                            unitOther = grid->getUnit(this,i);
                            index = unitOther->getIndex();
                            info = "\t\n(";
                            info += std::to_string(origin.x);
                            info += ",";
                            info += std::to_string(origin.y);
                            info += ") -> (";
                            info += std::to_string(index.x);
                            info += ",";
                            info += std::to_string(index.y);
                            info += "):\n\t\tDistance = ";
                            info += std::to_string(unit->getDistance(unitOther));
                            info += "\n\t\tDirection = ";
                            info += std::to_string(util::getBoundedAngle(unit->getDirection(unitOther),false)*util::RAD_TO_DEG);
                            info += "\n";
                        }
                    }
                private:
                    bool relativity;
                    bool cache;
                    Bank* bank;
                    Style style;
                    Contents contents;
                    sf3d::Vector2u origin;
            };
            typedef typename Neighborhood::Contents Neighbors;
            Grid(const sf3d::Vector2u& size = sf3d::Vector2u(), bool isResponsible = true, Topology topology = TORUS) :
                isResponsible(isResponsible),
                topology(topology),
                units(nullptr)
            {
                initialize(size);
            }
            virtual ~Grid()
            {
                destroy();
            }
            void destroy()
            {
                if (units == nullptr)
                {
                    return;
                }
                for (unsigned int x = 0; x != size.x; ++x)
                {
                    for (unsigned int y = 0; y != size.y; ++y)
                    {
                        delete units[x][y];
                    }
                    delete[] units[x];
                }
                delete[] units;
                units = nullptr;
                size = sf3d::Vector2u();
            }
            void initialize(const sf3d::Vector2u& size)
            {
                destroy();
                if ((size.x == 0) || (size.y == 0))
                {
                    units = nullptr;
                    return;
                }
                units = new Unit**[size.x];
                for (unsigned int x = 0; x != size.x; ++x)
                {
                    units[x] = new Unit*[size.y];
                    for (unsigned int y = 0; y != size.y; ++y)
                    {
                        units[x][y] = nullptr;
                    }
                }
                this->size = size;
            }
            void setUnit(T unit, const sf3d::Vector2u& index)
            {
                Unit* temp = units[index.x][index.y];
                if (temp == nullptr)
                {
                    units[index.x][index.y] = new Unit(this,index,unit);
                }
                else
                {
                    temp->setPayload(unit);
                }
            }
            Unit* getUnit(Neighborhood* neighborhood, unsigned int index) const
            {
                if (neighborhood->getRelativity())
                {
                    return getUnit(getAbsoluteIndex(sf3d::Vector2i(neighborhood->getOrigin())+neighborhood->getContents()[index]));
                }
                return getUnit(sf3d::Vector2u(neighborhood->getContents()[index]));
            }
            Unit* getUnit(const sf3d::Vector2u& index) const
            {
                return units[index.x][index.y];
            }
            const sf3d::Vector2u& getSize() const
            {
                return size;
            }
            sf3d::Image* getImage(std::function<sf3d::Color(const Unit*)> conversion) const
            {
                if (units == nullptr)
                {
                    return new sf3d::Image();
                }
                sf3d::Image* image = new sf3d::Image();
                image->create(size.x,size.y);
                for (unsigned int x = 0; x != size.x; ++x)
                {
                    for (unsigned int y = 0; y != size.y; ++y)
                    {
                        image->setPixel(x,y,conversion(units[x][y]));
                    }
                }
                return image;
            }
            bool getIsResponsible() const
            {
                return isResponsible;
            }
            void setIsResponsible(bool isResponsible)
            {
                this->isResponsible = isResponsible;
            }
            Topology getTopology() const
            {
                return topology;
            }
            void setTopology(Topology topology)
            {
                this->topology = topology;
            }
            sf3d::Vector2u getAbsoluteIndex(const sf3d::Vector2i& index) const
            {
                sf3d::Vector2u mapping;
                switch (topology)
                {
                case PLANE:
                    mapping = sf3d::Vector2u(index);
                    break;
                case SPHERE:
                    {
                        sf3d::Vector2i temp;
                        temp.x = index.x/static_cast<int>(size.x);
                        temp.y = index.y/static_cast<int>(size.y);
                        mapping.x = static_cast<unsigned int>(abs(index.x));
                        mapping.y = static_cast<unsigned int>(abs(index.y));
                        if (index.x < 0)
                        {
                            --mapping.x;
                        }
                        mapping.x %= size.x;
                        mapping.y %= size.y;
                        if (index.x < 0)
                        {
                            mapping.x = (size.x-1)-mapping.x;
                        }
                        if (index.y < 0)
                        {
                            //mapping.y = abs(index.y%static_cast<int>(size.y));
                            --temp.y;
                            if (abs(temp.y)%2 == 0)
                            {
                                mapping.y = (size.y-1)-(mapping.y-1);
                            }
                        }
                        if (index.y >= static_cast<int>(size.y))
                        {
                            //mapping.y = size.y-abs(index.y%static_cast<int>(size.y));
                            if (abs(temp.y)%2 != 0)
                            {
                                mapping.y = (size.y-1)-mapping.y;
                            }
                        }
                        if (abs(temp.x)%2 != 0)
                        {
                            mapping.x = (size.x-1)-mapping.x;
                        }
                        if (abs(temp.y)%2 != 0)
                        {
                            mapping.x = (mapping.x+(size.x/2))%size.x;
                        }
                        if ((index.y < 0) && (mapping.y < size.y))
                        {
                            ++mapping.y;
                        }
                        if ((index.y >= static_cast<int>(size.y)) && (mapping.y > 0))
                        {
                            --mapping.y;
                        }
                    }
                    /*mapping.x = static_cast<unsigned int>(abs(index.x)%static_cast<int>(size.x));
                    mapping.y = static_cast<unsigned int>(abs(index.y)%static_cast<int>(size.y));
                    if (index.x < 0)
                    {
                        mapping.x = (size.x-1)-mapping.x;
                    }
                    if (index.y < 0)
                    {
                        mapping.y = abs(index.y%static_cast<int>(size.y));
                    }
                    if (index.y >= static_cast<int>(size.y))
                    {
                        mapping.y = size.y-abs(index.y%static_cast<int>(size.y));
                    }
                    if (abs(index.x/static_cast<int>(size.x))%2 != 0)
                    {
                        mapping.x = (size.x-1)-mapping.x;
                    }
                    if (abs(index.y/static_cast<int>(size.y))%2 != 0)
                    {
                        mapping.x = (size.x-1)-mapping.x;
                    }*/
                    break;
                case TORUS:
                    mapping.x = static_cast<unsigned int>(abs(index.x)%static_cast<int>(size.x));
                    mapping.y = static_cast<unsigned int>(abs(index.y)%static_cast<int>(size.y));
                    if (index.x < 0)
                    {
                        mapping.x = (size.x-1)-(mapping.x-1);
                    }
                    if (index.y < 0)
                    {
                        mapping.y = (size.y-1)-(mapping.y-1);
                    }
                    if (abs(index.x/static_cast<int>(size.x))%2 != 0)
                    {
                        mapping.x = (size.x-1)-mapping.x;
                    }
                    if (abs(index.y/static_cast<int>(size.y))%2 != 0)
                    {
                        mapping.y = (size.y-1)-mapping.y;
                    }
                    break;
                default:
                    mapping = size;
                    break;
                }
                return mapping;
            }
            typedef Unit Container;
        private:
            bool isResponsible;
            sf3d::Vector2u size;
            Topology topology;
            Unit*** units;
    };
}

#endif // NFE_GRID_HPP

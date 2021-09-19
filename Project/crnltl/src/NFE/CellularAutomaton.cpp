#include <NFE/CellularAutomaton.hpp>
#include <set>

NFE::CellularAutomaton::Cell::Cell(unsigned int state, unsigned int life) :
    state(state),
    life(life)
{

}

NFE::CellularAutomaton::Cell::~Cell()
{

}

unsigned int NFE::CellularAutomaton::Cell::getState() const
{
    return state;
}

void NFE::CellularAutomaton::Cell::setState(unsigned int state)
{
    this->state = state;
}

unsigned int NFE::CellularAutomaton::Cell::getLife() const
{
    return life;
}

void NFE::CellularAutomaton::Cell::setLife(unsigned int life)
{
    this->life = life;
}

NFE::CellularAutomaton::CellularAutomaton() :
    cells(nullptr),
    cascadeTarget(nullptr),
    generationCount(0),
    generationLoop(1)
{
    create(sf3d::Vector2u());
}

NFE::CellularAutomaton::CellularAutomaton(const sf3d::Vector2u& size) :
    cells(nullptr),
    cascadeTarget(nullptr),
    generationCount(0),
    generationLoop(1)
{
    create(size);
}

NFE::CellularAutomaton::CellularAutomaton(const sf3d::Vector2u& size, unsigned int states, Random* random) :
    cells(nullptr),
    cascadeTarget(nullptr),
    cascadeStateMapPolicy(false),
    cellReusabilityPolicy(false),
    generationCount(0),
    generationLoop(1)
{
    create(size,states,random);
}

NFE::CellularAutomaton::~CellularAutomaton()
{
    delete rules;
    delete cells;
}

void NFE::CellularAutomaton::initialize(const sf3d::Vector2u& size)
{
    delete cells;
    cells = new Cells(size);
    rules = new Rules();
    //rules->emplace<bool,LIFE_RESET_RULE>(true);
    rules->emplace<StateLife,STATE_LIFE_RULE>([](const Cell& cell, unsigned int state){return (state==cell.getState());});
    rules->emplace<Transition,TRANSITION_RULE>([](const Cell&,const Neighbors&){return 0;});
    rules->emplace<Neighborhoods,NEIGHBORHOODS_RULE>(Neighborhoods());
    rules->emplace<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>(NeighborhoodRadius());
    rules->emplace<CascadeStateMap,OUTBOUND_CASCADE_STATE_MAP_RULE>(CascadeStateMap());
    rules->emplace<CascadeStateMap,INBOUND_CASCADE_STATE_MAP_RULE>(CascadeStateMap());
}

void NFE::CellularAutomaton::create(const sf3d::Vector2u& size)
{
    initialize(size);
    for (unsigned int x = 0; x != size.x; ++x)
    {
        for (unsigned int y = 0; y != size.y; ++y)
        {
            cells->setUnit(new Cell(0),sf3d::Vector2u(x,y));
        }
    }
}

void NFE::CellularAutomaton::create(const sf3d::Vector2u& size, unsigned int states, Random* random)
{
    if (states <= 1)
    {
        create(size);
        return;
    }
    initialize(size);
    for (unsigned int x = 0; x != size.x; ++x)
    {
        for (unsigned int y = 0; y != size.y; ++y)
        {
            cells->setUnit(new Cell(static_cast<unsigned int>(random->getInt(0,states-1))),sf3d::Vector2u(x,y));
        }
    }
}

void NFE::CellularAutomaton::accomodateNewTransitionRule(Transition transition)
{
    Transition temp = *rules->get<Transition,TRANSITION_RULE>();
    rules->emplace<Transition,TRANSITION_RULE>([=](const Cell& cell, const Neighbors& neighbors){
                                               unsigned int newState = transition(cell.getState(),neighbors);
                                               if (newState == cell.getState())
                                               {
                                                   return temp(cell,neighbors);
                                               }
                                               return newState;
                                               });
}

void NFE::CellularAutomaton::accomodateNewState(unsigned int newState, Transition transition)
{
    Transition temp = *rules->get<Transition,TRANSITION_RULE>();
    rules->emplace<Transition,TRANSITION_RULE>([=](const Cell& cell, const Neighbors& neighbors){
                                               if ((cell.getState() != newState) && (getNeighborsOfState(neighbors,newState).empty()))
                                               {
                                                   return temp(cell,neighbors);
                                               }
                                               return transition(cell,neighbors);
                                               });
}

void NFE::CellularAutomaton::goToNextGeneration()
{
    bool cascade = false;
    ++generationCount;
    if (generationLoop != 0)
    {
        if (generationCount%generationLoop == 0)
        {
            generationCount = 0;
            if (cascadeTarget != nullptr)
            {
                cascade = true;
                cascadeTarget->update(cells,rules->get<Rule,OUTBOUND_CASCADE_STATE_MAP_RULE>());
                cascadeTarget->goToNextGeneration();
                update(cascadeTarget->getCells(),rules->get<Rule,INBOUND_CASCADE_STATE_MAP_RULE>());
            }
        }
    }
    if ((generationCount != 0) || (cascadeTarget == nullptr))
    {
        Cells* generation = getNextGeneration();
        update(generation,cascade);
        if (!cellReusabilityPolicy)
        {
            delete generation;
        }
    }
}

void NFE::CellularAutomaton::setNeighborhoodStyle(Neighborhood::Style style)
{
    Neighborhood* neighborhood;
    std::shared_ptr<Neighborhoods> neighborhoods = rules->get<Neighborhoods,NEIGHBORHOODS_RULE>();
    for (unsigned int i = 0; i != neighborhoods->size(); ++i)
    {
        neighborhood = neighborhoods->at(i);
        if (neighborhood != nullptr)
        {
            neighborhood->setStyle(style);
        }
    }
}

void NFE::CellularAutomaton::setNeighborhoodCache(bool cache)
{
    Neighborhood* neighborhood;
    std::shared_ptr<Neighborhoods> neighborhoods = rules->get<Neighborhoods,NEIGHBORHOODS_RULE>();
    for (unsigned int i = 0; i != neighborhoods->size(); ++i)
    {
        neighborhood = neighborhoods->at(i);
        if (neighborhood != nullptr)
        {
            neighborhood->setCache(cache);
        }
    }
}

void NFE::CellularAutomaton::setTopology(Topology topology)
{
    cells->setTopology(topology);
}

NFE::CellularAutomaton::Cells::Topology NFE::CellularAutomaton::getTopology() const
{
    return cells->getTopology();
}

void NFE::CellularAutomaton::setCellReusabilityPolicy(bool policy)
{
    cellReusabilityPolicy = policy;
}

bool NFE::CellularAutomaton::getCellReusabilityPolicy() const
{
    return cellReusabilityPolicy;
}

void NFE::CellularAutomaton::setCascadeStateMapPolicy(bool policy)
{
    cascadeStateMapPolicy = policy;
}

bool NFE::CellularAutomaton::getCascadeStateMapPolicy() const
{
    return cascadeStateMapPolicy;
}

void NFE::CellularAutomaton::setCascadeTarget(CellularAutomaton* cascadeTarget)
{
    this->cascadeTarget = cascadeTarget;
}

NFE::CellularAutomaton* NFE::CellularAutomaton::getCascadeTarget() const
{
    return cascadeTarget;
}

void NFE::CellularAutomaton::setGenerationLoop(unsigned int generationLoop)
{
    this->generationLoop = generationLoop;
}

unsigned int NFE::CellularAutomaton::getGenerationLoop() const
{
    return generationLoop;
}

unsigned int NFE::CellularAutomaton::getGenerationCount() const
{
    return generationCount;
}

unsigned int NFE::CellularAutomaton::getStateCount() const
{
    unsigned int stateCount;
    std::set<unsigned int> states;
    for (unsigned int x = 0; x != cells->getSize().x; ++x)
    {
        for (unsigned int y = 0; y != cells->getSize().y; ++y)
        {
            states.insert(cells->getUnit(sf3d::Vector2u(x,y))->getPayload()->getState());
        }
    }
    stateCount = states.size();
    states.clear();
    return stateCount;
}

unsigned int NFE::CellularAutomaton::getCellsOfStateCount(unsigned int state) const
{
    unsigned int result = 0;
    for (unsigned int x = 0; x != cells->getSize().x; ++x)
    {
        for (unsigned int y = 0; y != cells->getSize().y; ++y)
        {
            if (cells->getUnit(sf3d::Vector2u(x,y))->getPayload()->getState() == state)
            {
                ++result;
            }
        }
    }
    return result;
}

const NFE::CellularAutomaton::Cells* NFE::CellularAutomaton::getCells() const
{
    return cells;
}

sf3d::Image* NFE::CellularAutomaton::getLifeImage(const sf3d::Color& old, const sf3d::Color& young) const
{
    return cells->getImage([&](const Cells::Unit* element){return mixColors(old,young,1.0f/static_cast<float>(element->getPayload()->getLife()+1),true);});
}

sf3d::Image* NFE::CellularAutomaton::getImage(bool life) const
{
    if (life)
    {
        return cells->getImage([](const Cells::Unit* element){return mixColors(sf3d::Color::Black,getColorFromKey(static_cast<int>(element->getPayload()->getState()+1)),1.0f/static_cast<float>(element->getPayload()->getLife()+1),true);});
    }
    return cells->getImage([](const Cells::Unit* element){return getColorFromKey(static_cast<int>(element->getPayload()->getState()+1));});
}

std::string NFE::CellularAutomaton::getRulesString()
{
    return getRulesString(*rules);
}

NFE::CellularAutomaton::Rules* NFE::CellularAutomaton::getRules() const
{
    return rules;
}

NFE::CellularAutomaton::Cells* NFE::CellularAutomaton::getNextGeneration()
{
    Neighbors neighbors = Neighbors();
    std::shared_ptr<Transition> transition = rules->get<Transition,TRANSITION_RULE>();
    Cells* generation;
    if (cellReusabilityPolicy)
    {
        generation = cells;
    }
    else
    {
        generation = new Cells(cells->getSize());
        generation->setTopology(cells->getTopology());
    }
    Cell* cell;
    sf3d::Vector2u index;
    unsigned int state;
    for (unsigned int x = 0; x != cells->getSize().x; ++x)
    {
        for (unsigned int y = 0; y != cells->getSize().y; ++y)
        {
            index.x = x;
            index.y = y;
            cell = cells->getUnit(index)->getPayload();
            if (getNeighbors(cell,index,neighbors))
            {
                state = (*transition)(*cell,neighbors);
                if (cellReusabilityPolicy)
                {
                    update(generation->getUnit(index)->getPayload(),state);
                }
                else
                {
                    generation->setUnit(new Cell(state),index);
                }
            }
            neighbors.clear();
        }
    }
    return generation;
}

bool NFE::CellularAutomaton::getNeighbors(const Cell* cell, const sf3d::Vector2u& index, Neighbors& neighbors)
{
    Rule rule;
    Cells::Unit* unit;
    Neighborhood* neighborhood;
    NeighborhoodRadius::iterator iter1;
    StateRadius::iterator iter2;
    Rule::iterator iter3;
    unsigned int stateOther;
    unsigned int state = cell->getState();
    std::shared_ptr<NeighborhoodRadius> radius = rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>();
    std::shared_ptr<Neighborhoods> neighborhoods = rules->get<Neighborhoods,NEIGHBORHOODS_RULE>();
    for (unsigned int i = 0; i != neighborhoods->size(); ++i)
    {
        iter1 = radius->find(i);
        if (iter1 != radius->end())
        {
            iter2 = iter1->second.find(state);
            if (iter2 != iter1->second.end())
            {
                neighborhood = neighborhoods->at(i);
                if (neighborhood != nullptr)
                {
                    if (neighborhood->update(cells,index,iter2->second))
                    {
                        for (unsigned int j = 0; j != neighborhood->getSize(); ++j)
                        {
                            unit = cells->getUnit(neighborhood,j);
                            if (unit != nullptr)
                            {
                                stateOther = unit->getPayload()->getState();
                                iter3 = rule.find(stateOther);
                                if (iter3 == rule.end())
                                {
                                    rule[stateOther] = 1;
                                }
                                else
                                {
                                    //std::cout << "poo  " << unit->getIndex().x << "  " << unit->getIndex().y << std::endl;
                                    ++iter3->second;
                                }
                            }
                        }
                        if (!rule.empty())
                        {
                            neighbors[i] = rule;
                            rule.clear();
                        }
                    }
                }
            }
        }
    }
    return true;
}

sf3d::Color NFE::CellularAutomaton::getColorFromKey(int key)
{
    int block = powf(2,3);
    int temp = key/block;
    temp *= block;
    temp = key-temp;
    sf3d::Color color = sf3d::Color::Black;
    sf3d::Uint8* channels[3] = {&color.r,&color.g,&color.b};
    for (int i = 0; i != 3; ++i)
    {
        if (temp&((int)powf(2,i)))
        {
            *channels[i] = 255/(1+(abs(key)/block));
        }
    }
    return color;
}

sf3d::Color NFE::CellularAutomaton::mixColors(const sf3d::Color& color1, const sf3d::Color& color2, float key, bool alpha)
{
    sf3d::Color color = sf3d::Color::White;
    sf3d::Vector3f temp(key*static_cast<float>(color2.r),key*static_cast<float>(color2.g),key*static_cast<float>(color2.b));
    key = 1.0f-key;
    temp.x += key*static_cast<float>(color1.r);
    temp.y += key*static_cast<float>(color1.g);
    temp.z += key*static_cast<float>(color1.b);
    color.r = static_cast<sf3d::Uint8>(util::clamp(0.0f,255.0f,temp.x));
    color.g = static_cast<sf3d::Uint8>(util::clamp(0.0f,255.0f,temp.y));
    color.b = static_cast<sf3d::Uint8>(util::clamp(0.0f,255.0f,temp.z));
    if (alpha)
    {
        color.a = static_cast<sf3d::Uint8>(util::clamp(0.0f,255.0f,util::mix(static_cast<float>(color2.a),static_cast<float>(color1.a),key)));
    }
    return color;
}

NFE::CellularAutomaton* NFE::CellularAutomaton::getMorphogenesis(const sf3d::Vector2u& size, Random* random, float inhibition, const sf3d::Vector2f& inhibitionRange, const sf3d::Vector2f& activationRange)
{
    CellularAutomaton* morphogenesis;
    if (random == nullptr)
    {
        morphogenesis = new CellularAutomaton(size);
    }
    else
    {
        morphogenesis = new CellularAutomaton(size,2,random);
    }
    Rules* rules = morphogenesis->getRules();
    morphogenesis->setTopology(Cells::Topology::TORUS);
    rules->get<Neighborhoods,NEIGHBORHOODS_RULE>()->push_back(new Neighborhood(Cells::Neighborhood::Style::MENAECHMUS));
    rules->get<Neighborhoods,NEIGHBORHOODS_RULE>()->push_back(new Neighborhood(Cells::Neighborhood::Style::MENAECHMUS));
    rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>()->insert(NeighborhoodRadiusPair(0,{StateRadiusPair(0,activationRange),StateRadiusPair(1,activationRange)}));
    rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>()->insert(NeighborhoodRadiusPair(1,{StateRadiusPair(0,inhibitionRange),StateRadiusPair(1,inhibitionRange)}));
    rules->emplace<Transition,TRANSITION_RULE>([=](const Cell& cell, const Neighbors& neighbors){
                                               unsigned int activators = 0;
                                               unsigned int inhibitors = 0;
                                               unsigned int newState = 0;
                                               //unsigned int newState = cell.getState();
                                               Neighbors::const_iterator iter1;
                                               Rule::const_iterator iter2;
                                               for (unsigned int i = 0; i != 2; ++i)
                                               {
                                                   iter1 = neighbors.find(i);
                                                   if (iter1 != neighbors.end())
                                                   {
                                                       iter2 = iter1->second.find(1);
                                                       if (iter2 != iter1->second.end())
                                                       {
                                                           if (i == 0)
                                                           {
                                                               activators = iter2->second;
                                                           }
                                                           else
                                                           {
                                                               inhibitors = iter2->second;
                                                           }
                                                       }
                                                   }
                                               }
                                               if (cell.getState() == 0)
                                               {
                                                   if (static_cast<float>(activators)-(inhibition*static_cast<float>(inhibitors)) > 0.0f)
                                                   {
                                                       newState = 1;
                                                   }
                                               }
                                               else if (cell.getState() == 1)
                                               {
                                                   if (static_cast<float>(activators)-(inhibition*static_cast<float>(inhibitors)) < 0.0f)
                                                   {
                                                       newState = 0;
                                                   }
                                               }
                                               return newState;
                                               });
    return morphogenesis;
}

NFE::CellularAutomaton* NFE::CellularAutomaton::getFingerprintGame(const sf3d::Vector2u& size, Random* random)
{
    CellularAutomaton* fingerprint = getConwayGameOfLife(size,random);
    fingerprint->getRules()->emplace<StateLife,STATE_LIFE_RULE>([](const Cell& cell, unsigned int state){return (state==cell.getLife());});
    return fingerprint;
}

NFE::CellularAutomaton* NFE::CellularAutomaton::getConwayGameOfLife(const sf3d::Vector2u& size, Random* random)
{
    CellularAutomaton* life;
    if (random == nullptr)
    {
        life = new CellularAutomaton(size);
    }
    else
    {
        life = new CellularAutomaton(size,2,random);
    }
    Rules* rules = life->getRules();
    life->setTopology(Cells::Topology::TORUS);
    rules->get<Neighborhoods,NEIGHBORHOODS_RULE>()->push_back(new Neighborhood(Cells::Neighborhood::Style::MOORE));
    rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>()->insert(NeighborhoodRadiusPair(0,{StateRadiusPair(0,sf3d::Vector2f(1.0f,1.0f)),StateRadiusPair(1,sf3d::Vector2f(1.0f,1.0f))}));
    rules->emplace<Transition,TRANSITION_RULE>([](const Cell& cell, const Neighbors& neighbors){
                                               unsigned int newState = 0;
                                               Neighbors::const_iterator iter1 = neighbors.find(0);
                                               if (iter1 != neighbors.end())
                                               {
                                                   Rule::const_iterator iter2 = iter1->second.find(1);
                                                   if (iter2 != iter1->second.end())
                                                   {
                                                       //newState = ((iter->second==((cell.getState()==1)?2:3))?1:0); // ryan suggested this; it doesn't work
                                                       if (cell.getState() == 0)
                                                       {
                                                           if (iter2->second == 3)
                                                           {
                                                               newState = 1;
                                                           }
                                                       }
                                                       else if (cell.getState() == 1)
                                                       {
                                                           if ((iter2->second == 2) || (iter2->second == 3))
                                                           {
                                                               newState = 1;
                                                           }
                                                       }
                                                   }
                                               }
                                               return newState;
                                               });
    return life;
}

NFE::CellularAutomaton::CascadePair NFE::CellularAutomaton::getPoisonedConwayGameOfLife(const sf3d::Vector2u& size, Random* random, float poisonChance)
{
    CellularAutomaton* poison = getConwayGameOfLife(size);
    CellularAutomaton* conway = getConwayGameOfLife(size,random);
    Rules* rules = conway->getRules();
    std::shared_ptr<Rule> rule = rules->get<CascadeStateMap,OUTBOUND_CASCADE_STATE_MAP_RULE>();
    rule->insert(RulePair(0,0));
    rule->insert(RulePair(1,1));
    rule->insert(RulePair(2,2));
    rule = rules->get<CascadeStateMap,INBOUND_CASCADE_STATE_MAP_RULE>();
    rule->insert(RulePair(0,0));
    rule->insert(RulePair(1,1));
    rule->insert(RulePair(2,2));
    rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>()->at(0).insert(StateRadiusPair(2,sf3d::Vector2f(1.0f,1.0f)));
    rules = poison->getRules();
    rule = rules->get<CascadeStateMap,OUTBOUND_CASCADE_STATE_MAP_RULE>();
    rule->insert(RulePair(0,0));
    rule->insert(RulePair(1,1));
    rule->insert(RulePair(2,2));
    rule = rules->get<CascadeStateMap,INBOUND_CASCADE_STATE_MAP_RULE>();
    rule->insert(RulePair(0,0));
    rule->insert(RulePair(1,1));
    rule->insert(RulePair(2,2));
    rules->get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>()->at(0).insert(StateRadiusPair(2,sf3d::Vector2f(1.0f,1.0f)));
    conway->accomodateNewState(2,[](const Cell&,const Neighbors&){return 2u;});
    poison->accomodateNewState(2,[](const Cell&,const Neighbors&){return 2u;});
    poison->accomodateNewTransitionRule([=](const Cell& cell, const Neighbors& neighbors){
                                        if (random->getFloat(0.0f,1.0f) < poisonChance)
                                        {
                                            return 2u;
                                        }
                                        return cell.getState();
                                        });
    conway->setCascadeTarget(poison);
    return CascadePair(conway,poison);
}

std::string NFE::CellularAutomaton::getRuleString(const std::shared_ptr<Rule> rule)
{
    std::string result = "{";
    for (Rule::const_iterator iter = rule->begin(); iter != rule->end(); ++iter)
    {
        if (iter != rule->begin())
        {
            result += ", ";
        }
        result += "{"+std::to_string(iter->first)+": "+std::to_string(iter->second)+"}";
    }
    result.push_back('}');
    return result;
}

std::string NFE::CellularAutomaton::getRulesString(Rules& rules)
{
    std::string result = "{\n";
    std::shared_ptr<Rule> rule = rules.get<CascadeStateMap,OUTBOUND_CASCADE_STATE_MAP_RULE>();
    std::shared_ptr<NeighborhoodRadius> radius = rules.get<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE>();
    //result += "{LIFE_RESET_RULE: "+std::to_string(*rules.get<bool,LIFE_RESET_RULE>())+"},\n";
    /*result += "{NEIGHBORHOOD_RULE_STYLE: ";
    switch (rules.get<Neighborhood,NEIGHBORHOOD_RULE>()->getStyle())
    {
    case Cells::Neighborhood::Style::MOORE:
        result += "MOORE";
        break;
    case Cells::Neighborhood::Style::VON_NEUMANN:
        result += "VON_NEUMANN";
        break;
    case Cells::Neighborhood::Style::EUCLID:
        result += "EUCLID";
        break;
    }
    result += "},\n";*/
    result += "{NEIGHBORHOOD_RADIUS_RULE: {";
    for (NeighborhoodRadius::iterator iter1 = radius->begin(); iter1 != radius->end(); ++iter1)
    {
        if (iter1 != radius->begin())
        {
            result += ", ";
        }
        result += "{"+std::to_string(iter1->first)+": ";
        for (StateRadius::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
        {
            if (iter2 != iter1->second.begin())
            {
                result += ", ";
            }
            result += "{"+std::to_string(iter2->first)+": <"+std::to_string(iter2->second.x)+"/"+std::to_string(iter2->second.y)+">}";
        }
        result.push_back('}');
    }
    result += "}},\n";
    result += "{OUTBOUND_CASCADE_STATE_MAP_RULE: "+getRuleString(rule)+"},\n";
    rule = rules.get<CascadeStateMap,INBOUND_CASCADE_STATE_MAP_RULE>();
    result += "{INBOUND_CASCADE_STATE_MAP_RULE: "+getRuleString(rule)+"}\n";
    result.push_back('}');
    return result;
}

NFE::CellularAutomaton::Rule NFE::CellularAutomaton::getNeighborsOfState(const Neighbors& neighbors, unsigned int state)
{
    Rule rule = Rule();
    Rule::const_iterator iter2;
    for (Neighbors::const_iterator iter1 = neighbors.begin(); iter1 != neighbors.end(); ++iter1)
    {
        iter2 = iter1->second.find(state);
        if (iter2 != iter1->second.end())
        {
            rule[iter1->first] = iter2->second;
        }
    }
    return rule;
}

void NFE::CellularAutomaton::freeCascadePairs(const std::vector<CascadePair>& cascadePairs)
{
    std::set<CellularAutomaton*> uniques;
    for (unsigned int i = 0; i != cascadePairs.size(); ++i)
    {
        uniques.insert(std::get<0>(cascadePairs[i]));
        uniques.insert(std::get<1>(cascadePairs[i]));
    }
    for (std::set<CellularAutomaton*>::iterator iter = uniques.begin(); iter != uniques.end(); ++iter)
    {
        delete *iter;
    }
    uniques.clear();
}

void NFE::CellularAutomaton::update(const Cells* cells, std::shared_ptr<CascadeStateMap> cascadeStateMap)
{
    if ((cells->getSize().x != this->cells->getSize().x) || (cells->getSize().y != this->cells->getSize().y))
    {
        return;
    }
    CascadeStateMap::const_iterator iter;
    sf3d::Vector2u index;
    Cell* cell;
    Cell* cellOther;
    for (unsigned int x = 0; x != cells->getSize().x; ++x)
    {
        for (unsigned int y = 0; y != cells->getSize().y; ++y)
        {
            index.x = x;
            index.y = y;
            cell = this->cells->getUnit(index)->getPayload();
            cellOther = cells->getUnit(index)->getPayload();
            iter = cascadeStateMap->find(cellOther->getState());
            if (iter != cascadeStateMap->end())
            {
                cell->setState(iter->second);
                cell->setLife(cellOther->getLife());
            }
            else
            {
                if (cascadeStateMapPolicy)
                {
                    cell->setState(cellOther->getState());
                    cell->setLife(cellOther->getLife());
                }
            }
        }
    }
}

void NFE::CellularAutomaton::update(const Cells* cells, bool cascade)
{
    if ((cellReusabilityPolicy) && (!cascade))
    {
        return;
    }
    if ((cells->getSize().x != this->cells->getSize().x) || (cells->getSize().y != this->cells->getSize().y))
    {
        return;
    }
    sf3d::Vector2u index;
    unsigned int state;
    Cell* cell;
    for (unsigned int x = 0; x != cells->getSize().x; ++x)
    {
        for (unsigned int y = 0; y != cells->getSize().y; ++y)
        {
            index.x = x;
            index.y = y;
            cell = this->cells->getUnit(index)->getPayload();
            state = cells->getUnit(index)->getPayload()->getState();
            update(cell,state);
        }
    }
}

void NFE::CellularAutomaton::update(Cell* cell,  unsigned int state)
{
    if ((*rules->get<StateLife,STATE_LIFE_RULE>())(*cell,state))
    {
        cell->setLife(cell->getLife()+1);
    }
    else
    {
        //if (*rules->get<bool,LIFE_RESET_RULE>())
        {
            cell->setLife(0);
        }
        cell->setState(state);
    }
}

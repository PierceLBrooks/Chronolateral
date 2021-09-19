#ifndef NFE_CELLULAR_AUTOMATON_HPP
#define NFE_CELLULAR_AUTOMATON_HPP

#include <NFE/Repository.hpp>
#include <NFE/Random.hpp>
#include <NFE/Grid.hpp>
#include <functional>
#include <map>

namespace NFE
{
    class CellularAutomaton
    {
        public:
            class Cell
            {
                public:
                    Cell(unsigned int state = 0, unsigned int life = 0);
                    ~Cell();
                    unsigned int getState() const;
                    void setState(unsigned int state);
                    unsigned int getLife() const;
                    void setLife(unsigned int life);
                private:
                    unsigned int state;
                    unsigned int life;
            };
            struct STATE_LIFE_RULE {};
            struct TRANSITION_RULE {};
            struct NEIGHBORHOODS_RULE {};
            struct NEIGHBORHOOD_RADIUS_RULE {};
            struct INBOUND_CASCADE_STATE_MAP_RULE {};
            struct OUTBOUND_CASCADE_STATE_MAP_RULE {};
            typedef Grid<Cell*> Cells;
            typedef Cells::Topology Topology;
            typedef Cells::Neighborhood Neighborhood;
            typedef std::vector<Neighborhood*> Neighborhoods;
            typedef std::map<unsigned int,unsigned int> Rule;
            typedef std::map<unsigned int,Rule> Neighbors;
            typedef std::map<unsigned int,sf3d::Vector2f> StateRadius;
            typedef std::map<unsigned int,StateRadius> NeighborhoodRadius;
            typedef std::pair<unsigned int,sf3d::Vector2f> StateRadiusPair;
            typedef std::pair<unsigned int,StateRadius> NeighborhoodRadiusPair;
            typedef std::pair<unsigned int,unsigned int> RulePair;
            typedef std::function<bool(const Cell&,unsigned int)> StateLife;
            typedef std::function<unsigned int(const Cell&,const Neighbors&)> Transition;
            typedef Rule CascadeStateMap;
            typedef RepositorySlot<StateLife,STATE_LIFE_RULE> StateLifeRuleSlot;
            typedef RepositorySlot<Transition,TRANSITION_RULE> TransitionRuleSlot;
            typedef RepositorySlot<Neighborhoods,NEIGHBORHOODS_RULE> NeighborhoodsRuleSlot;
            typedef RepositorySlot<NeighborhoodRadius,NEIGHBORHOOD_RADIUS_RULE> NeighborhoodRadiusRuleSlot;
            typedef RepositorySlot<CascadeStateMap,INBOUND_CASCADE_STATE_MAP_RULE> InboundCascadeStateMapRuleSlot;
            typedef RepositorySlot<CascadeStateMap,OUTBOUND_CASCADE_STATE_MAP_RULE> OutboundCascadeStateMapRuleSlot;
            typedef Repository<StateLifeRuleSlot,TransitionRuleSlot,NeighborhoodsRuleSlot,NeighborhoodRadiusRuleSlot,InboundCascadeStateMapRuleSlot,OutboundCascadeStateMapRuleSlot> Rules;
            typedef std::pair<CellularAutomaton*,CellularAutomaton*> CascadePair;
            CellularAutomaton();
            CellularAutomaton(const sf3d::Vector2u& size);
            CellularAutomaton(const sf3d::Vector2u& size, unsigned int states, Random* random);
            virtual ~CellularAutomaton();
            void create(const sf3d::Vector2u& size);
            void create(const sf3d::Vector2u& size, unsigned int states, Random* random);
            void accomodateNewTransitionRule(Transition transition);
            void accomodateNewState(unsigned int newState, Transition transition);
            void goToNextGeneration();
            void setNeighborhoodStyle(Neighborhood::Style style);
            void setNeighborhoodCache(bool cache);
            void setTopology(Topology topology);
            Cells::Topology getTopology() const;
            void setCascadeTarget(CellularAutomaton* cascadeTarget);
            CellularAutomaton* getCascadeTarget() const;
            void setCellReusabilityPolicy(bool policy);
            bool getCellReusabilityPolicy() const;
            void setCascadeStateMapPolicy(bool policy);
            bool getCascadeStateMapPolicy() const;
            void setGenerationLoop(unsigned int generationLoop);
            unsigned int getGenerationLoop() const;
            unsigned int getGenerationCount() const;
            unsigned int getStateCount() const;
            unsigned int getCellsOfStateCount(unsigned int state) const;
            const Cells* getCells() const;
            sf3d::Image* getLifeImage(const sf3d::Color& old, const sf3d::Color& young) const;
            sf3d::Image* getImage(bool life = true) const;
            std::string getRulesString();
            Rules* getRules() const;
            Cells* getNextGeneration();
            virtual bool getNeighbors(const Cell* cell, const sf3d::Vector2u& index, Neighbors& neighbors);
            static sf3d::Color getColorFromKey(int key);
            static sf3d::Color mixColors(const sf3d::Color& color1, const sf3d::Color& color2, float key, bool alpha);
            static CellularAutomaton* getMorphogenesis(const sf3d::Vector2u& size, Random* random = nullptr, float inhibition = 0.2f, const sf3d::Vector2f& inhibitionRange = sf3d::Vector2f(6.1f,6.1f), const sf3d::Vector2f& activationRange = sf3d::Vector2f(2.3f,2.3f));
            static CellularAutomaton* getFingerprintGame(const sf3d::Vector2u& size, Random* random = nullptr);
            static CellularAutomaton* getConwayGameOfLife(const sf3d::Vector2u& size, Random* random = nullptr);
            static CascadePair getPoisonedConwayGameOfLife(const sf3d::Vector2u& size, Random* random, float poisonChance = 0.01f);
            static std::string getRuleString(const std::shared_ptr<Rule> rule);
            static std::string getRulesString(Rules& rules);
            static Rule getNeighborsOfState(const Neighbors& neighbors, unsigned int state);
            static void freeCascadePairs(const std::vector<CascadePair>& cascadePairs);
        protected:
            void update(const Cells* cells, std::shared_ptr<CascadeStateMap> cascadeStateMap);
            void update(const Cells* cells, bool cascade = false);
            void update(Cell* cell, unsigned int state);
        private:
            void initialize(const sf3d::Vector2u& size);
            unsigned int generationLoop;
            unsigned int generationCount;
            bool cellReusabilityPolicy;
            bool cascadeStateMapPolicy;
            CellularAutomaton* cascadeTarget;
            Rules* rules;
            Cells* cells;
    };
}

#endif // NFE_CELLULAR_AUTOMATON_HPP

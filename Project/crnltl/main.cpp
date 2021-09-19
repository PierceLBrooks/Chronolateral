
// Author: Pierce Brooks

#include <regex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <SFML3D/Audio.hpp>
#include <SFML3D/Network.hpp>
#include <SFML3D/Graphics.hpp>
#include <SFML3D/Window/Event.hpp>
#include <SFML3D/Window/Mouse.hpp>
#include <SFML3D/Window/Keyboard.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <TupleSpace/TupleSpace.hpp>
#include <TupleSpace/TcpConnectionHandlerAgent.hpp>

class Player
{
public:
    Player(float height, bool team);
    virtual ~Player();
    sf3d::Vector3f min;
    sf3d::Vector3f max;
    sf3d::Vector3f direction;
    sf3d::Vector3f position;
    sf3d::Vector3f motion;
    sf3d::Cuboid* appearance;
    float aim;
    float angle;
    float height;
    bool team;
    bool hit;
    bool self;
};

class Bullet
{
public:
    Bullet(float life, bool team, const sf3d::Vector3f& direction);
    virtual ~Bullet();
    sf3d::Cuboid* appearance;
    float life;
    bool team;
    bool peer;
    sf3d::Vector3f direction;
};

class ObjModel : public sf3d::Model
{
private:
    struct FaceData
    {
    public:
        unsigned int position0, position1, position2;
        unsigned int textureCoordinate0, textureCoordinate1, textureCoordinate2;
        unsigned int normal0, normal1, normal2;
    };

    sf3d::Vertex makeVertex(const std::string& indices)
    {
        sf3d::Vertex vertex;
        std::istringstream indiceStream(indices);

        unsigned int positionIndex = 0;
        unsigned int textureCoordinateIndex = 0;
        unsigned int normalIndex = 0;
        char separator = 0;

        if (!(indiceStream >> positionIndex >> separator).good())
        {
            return vertex;
        }

        if (!(indiceStream >> textureCoordinateIndex >> separator).good())
        {
            return vertex;
        }

        if (!(indiceStream >> normalIndex).eof())
        {
            return vertex;
        }

        // .obj indices start at 1
        positionIndex -= 1;
        textureCoordinateIndex -= 1;
        normalIndex -= 1;

        if ((textureCoordinateIndex >= m_vertexTextureCoordinates.size()) && (m_vertexTextureCoordinates.empty()))
        {
            m_vertexTextureCoordinates.push_back(sf3d::Vector2f());
        }

        if ((positionIndex >= m_vertexPositions.size()) ||
            (textureCoordinateIndex >= m_vertexTextureCoordinates.size()) ||
            (normalIndex >= m_vertexNormals.size()))
        {
            return vertex;
        }

        vertex.position = m_vertexPositions[positionIndex];
        vertex.texCoords = m_vertexTextureCoordinates[textureCoordinateIndex];
        vertex.normal = m_vertexNormals[normalIndex];

        // Not needed, but just for demonstration
        vertex.color = getColor();

        return vertex;
    }

public:
    bool loadFromFile(const std::string& filename)
    {
        // Open our model file
        std::ifstream modelFile(filename.c_str());
        if (!modelFile.is_open())
        {
            return false;
        }

        // Parse the model file line by line
        std::string line;
        std::istringstream lineStream;
        std::string token;
        std::regex pattern;

        pattern = std::regex("//");

        while (true)
        {
            std::getline(modelFile, line);

            // Break on error or failure to read (end of file)
            if ((modelFile.bad()) || (modelFile.fail()))
            {
                break;
            }

            line = std::regex_replace(line, pattern, "/1/");

            lineStream.clear();
            lineStream.str(line);
            lineStream >> token;

            if (token == "v")
            {
                // Handle vertex positions
                sf3d::Vector3f position;
                lineStream >> position.x >> position.y >> position.z;
                m_vertexPositions.push_back(position);
            }
            else if (token == "vt")
            {
                // Handle vertex texture coordinates
                sf3d::Vector2f coordinate;
                lineStream >> coordinate.x >> coordinate.y;
                m_vertexTextureCoordinates.push_back(coordinate);
            }
            else if (token == "vn")
            {
                // Handle vertex normals
                sf3d::Vector3f normal;
                lineStream >> normal.x >> normal.y >> normal.z;
                m_vertexNormals.push_back(normal);
            }
            else if (token == "f")
            {
                // Handle faces
                std::string vertexString0, vertexString1, vertexString2;

                lineStream >> vertexString0 >> vertexString1 >> vertexString2;

                sf3d::Vertex vertex0(makeVertex(vertexString0));
                sf3d::Vertex vertex1(makeVertex(vertexString1));
                sf3d::Vertex vertex2(makeVertex(vertexString2));

                addVertex(vertex0);
                addVertex(vertex1);
                addVertex(vertex2);

                unsigned int index = getVertexCount();

                addFace(index - 3, index - 2, index - 1);
            }
        }

        // Update the underlying polyhedron geometry
        update();

        return true;
    }

private:
    std::vector<sf3d::Vector3f> m_vertexPositions;
    std::vector<sf3d::Vector2f> m_vertexTextureCoordinates;
    std::vector<sf3d::Vector3f> m_vertexNormals;
    std::vector<FaceData> m_faceData;
};

Player::Player(float height, bool team) :
    height(height),
    team(team),
    hit(false),
    self(false)
{
    appearance = new sf3d::Cuboid(sf3d::Vector3f(10.0f, height*2.0f, 10.0f));
    appearance->setColor(team?sf3d::Color::Cyan:sf3d::Color::Magenta);
    appearance->setOrigin(sf3d::Vector3f(0.0f, -height, 0.0f));
}

Player::~Player()
{
    delete appearance;
}

Bullet::Bullet(float life, bool team, const sf3d::Vector3f& direction) :
    life(life),
    team(team),
    direction(direction)
{
    appearance = new sf3d::Cuboid();
    appearance->setColor(sf3d::Color::Black);
    peer = false;
}

Bullet::~Bullet()
{
    delete appearance;
}

void swap(float& left, float& right)
{
    float temp = left;
    left = right;
    right = temp;
}

bool checkIntersection(const sf3d::Vector3f& orig, const sf3d::Vector3f& dir, const sf3d::Vector3f& min, const sf3d::Vector3f& max)
{
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection

    float tmin = (min.x - orig.x) / dir.x;
    float tmax = (max.x - orig.x) / dir.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (min.y - orig.y) / dir.y;
    float tymax = (max.y - orig.y) / dir.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (min.z - orig.z) / dir.z;
    float tzmax = (max.z - orig.z) / dir.z;

    if (tzmin > tzmax) swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    return true;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    std::string temp;
    unsigned int counter = 0;
    for (unsigned int i = 0; i != str.size(); ++i)
    {
        temp.push_back(str[i]);
        if (str[i] == delimiter[counter])
        {
            ++counter;
            if (counter == delimiter.size())
            {
                result.push_back(temp.substr(0,temp.size()-counter));
                temp.clear();
                counter = 0;
            }
        }
    }
    if (!temp.empty())
    {
        result.push_back(temp);
    }
    return result;
}

template <typename T>
sf3d::Vector2<T> operator *(const sf3d::Vector2<T>& left, const sf3d::Vector2<T>& right)
{
    return sf3d::Vector2<T>(left.x * right.x, left.y * right.y);
}

template <typename T>
sf3d::Vector3<T> operator *(const sf3d::Vector3<T>& left, const sf3d::Vector3<T>& right)
{
    return sf3d::Vector3<T>(left.x * right.x, left.y * right.y, left.z * right.z);
}

float getDirection(const sf3d::Vector2f& left, const sf3d::Vector2f& right)
{
    return atan2f(right.y-left.y, right.x-left.x);
}

float getDistance(const sf3d::Vector2f& left, const sf3d::Vector2f& right)
{
    return sqrtf(powf(right.y-left.y, 2.0f)+powf(right.x-left.x, 2.0f));
}

float getDistance(const sf3d::Vector3f& left, const sf3d::Vector3f& right)
{
    return sqrtf(powf(right.z-left.z, 2.0f)+powf(right.y-left.y, 2.0f)+powf(right.x-left.x, 2.0f));
}

float getDotProduct(const sf3d::Vector2f& p1, const sf3d::Vector2f& p2)
{
    return (p1.x*p2.x)+(p1.y*p2.y);
}

float getDotProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    return (p1.x*p2.x)+(p1.y*p2.y)+(p1.z*p2.z);
}

sf3d::Vector3f getCrossProduct(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2)
{
    glm::vec3 cross = glm::cross(glm::vec3(p1.x, p1.y, p1.z), glm::vec3(p2.x, p2.y, p2.z));
    return sf3d::Vector3f(cross.x, cross.y, cross.z);
}

sf3d::Vector3f getNormal(const sf3d::Vector3f& p1, const sf3d::Vector3f& p2, const sf3d::Vector3f& p3)
{
    return getCrossProduct(p1-p2,p1-p3);
}

float getMagnitude(const sf3d::Vector2f& vec)
{
    return sqrtf((vec.x*vec.x)+(vec.y*vec.y));
}

float getMagnitude(const sf3d::Vector3f& vec)
{
    return sqrtf((vec.x*vec.x)+(vec.y*vec.y)+(vec.z*vec.z));
}

sf3d::Vector2f getNormalized(const sf3d::Vector2f& vec)
{
    return vec*(1.0f/getMagnitude(vec));
}

sf3d::Vector3f getNormalized(const sf3d::Vector3f& vec)
{
    return vec*(1.0f/getMagnitude(vec));
}

sf3d::Vector3f getProjection(const sf3d::Vector3f& point, const sf3d::Vector3f& origin, const sf3d::Vector3f& pivot)
{
    sf3d::Vector3f normal = getNormalized(getNormal(point,origin,pivot));
    sf3d::Vector3f axisFirst = getNormalized(pivot-origin);
    sf3d::Vector3f axisSecond = getNormalized(getCrossProduct(normal,axisFirst));
    sf3d::Vector3f displacement = point-origin;
    return sf3d::Vector3f(getDotProduct(axisFirst,displacement),getDotProduct(axisSecond,displacement),getDotProduct(normal,displacement)); // x = distance along first axis, y = distance along second axis, z = distance along normal
}

int run(TupleSpace* tupleSpace, sf3d::Font& font, sf3d::RenderWindow& window, sf3d::RenderTexture& frameTexture, const std::vector<std::string>& arguments)
{
    std::string tail = "\n\r";
    std::string victim;
    std::string host;
    std::string name;
    std::map<std::string, Player*> players;
    std::vector<Bullet*> bullets;
    std::vector<std::string> peers;
    std::vector<sf3d::Packet*> packets;
    std::vector<std::pair<sf3d::Text*, float>> texts;
    std::vector<std::pair<sf3d::Sound*, sf3d::SoundBuffer*>> hurtSounds;
    sf3d::Sound* landSound = nullptr;
    sf3d::Sound* shotSound = nullptr;
    sf3d::Sound* jumpSound = nullptr;
    sf3d::SoundBuffer* landBuffer = nullptr;
    sf3d::SoundBuffer* shotBuffer = nullptr;
    sf3d::SoundBuffer* jumpBuffer = nullptr;
    TcpConnectionHandlerAgent* agent = nullptr;
    bool role = false;
    bool jump = false;
    bool team = true;
    bool minigame = false;
    bool focus = true;
    bool announced = false;
    bool announcement = false;
    bool list = false;
    int hurt = 0;
    int hurts = 4;
    int result = 0;
    int lettering = 24;
    float scale = 50.0f;
    float animation = 0.0f;
    float speed = 1.0f;
    float velocity = 250.0f;
    float length = 1.0f;
    float magnitude = 0.0f;
    float pi = 3.141592654f;
    float yaw = pi / 2.0f;
    float pitch = 0.0f;
    float zoom = -100.0f;
    float delta = 0.0f;
    float time = 0.0f;
    float height = 15.0f;
    float gravity = 50.0f;
    float fall = 0.0f;
    float shoot = 0.0f;
    float angle = 0.0f;
    float range = 5000.0f;
    float gap = 25.0f;
    float radius = 5.0f;
    float life = 0.25f;
    sf3d::Clock clock;
    sf3d::Color color;
    sf3d::Light lightCamera;
    sf3d::Light lightNortheast;
    sf3d::Light lightNorthwest;
    sf3d::Light lightSoutheast;
    sf3d::Light lightSouthwest;
    sf3d::SphericalPolyhedron lampNortheast(radius);
    sf3d::SphericalPolyhedron lampNorthwest(radius);
    sf3d::SphericalPolyhedron lampSoutheast(radius);
    sf3d::SphericalPolyhedron lampSouthwest(radius);
    sf3d::Cuboid wallNorth(sf3d::Vector3f(1000.0f, velocity, 1.0f));
    sf3d::Cuboid wallSouth(sf3d::Vector3f(1000.0f, velocity, 1.0f));
    sf3d::Cuboid wallEast(sf3d::Vector3f(1.0f, velocity, 1000.0f));
    sf3d::Cuboid wallWest(sf3d::Vector3f(1.0f, velocity, 1000.0f));
    sf3d::Cuboid floor(sf3d::Vector3f(1000.0f, 1.0f, 1000.0f));
    sf3d::Cuboid ceiling(sf3d::Vector3f(1000.0f, 1.0f, 1000.0f));
    sf3d::Cuboid axisX(sf3d::Vector3f(500.0f, 2.0f, 2.0f));
    sf3d::Cuboid axisY(sf3d::Vector3f(2.0f, 500.0f, 2.0f));
    sf3d::Cuboid axisZ(sf3d::Vector3f(2.0f, 2.0f, 500.0f));
    sf3d::Camera camera(90.0f, 0.001f, 2500.0f);
    sf3d::Sprite frame(frameTexture.getTexture());
    sf3d::RectangleShape reticleVertical;
    sf3d::RectangleShape reticleHorizontal;
    sf3d::Vector2f coordinate;
    sf3d::Vector3f previous;
    sf3d::Vector3f direction;
    sf3d::Vector3f movement;
    sf3d::Vector3f rightVector;
    sf3d::Vector3f offset;
    sf3d::Vector3f step;
    sf3d::Vector3f origin = camera.getPosition();
    float aspectRatio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
    sf3d::Vector3f downscaleFactor(1.0f / static_cast<float>(window.getSize().x) * aspectRatio, -1.0f / static_cast<float>(window.getSize().y), 1.0f);
    camera.scale(1.0f / aspectRatio, 1.0f, 1.0f);
    camera.setPosition(sf3d::Vector3f());
    window.setView(camera);
    lightCamera.setColor(sf3d::Color::White);
    lightCamera.setAmbientIntensity(0.5f);
    lightCamera.setDiffuseIntensity(1.0f);
    lightCamera.setLinearAttenuation(0.002f);
    lightCamera.setQuadraticAttenuation(0.0005f);
    lightCamera.enable();
    lightNortheast.setColor(sf3d::Color::Red);
    lightNortheast.setAmbientIntensity(0.0f);
    lightNortheast.setDiffuseIntensity(radius*0.5f);
    lightNortheast.setLinearAttenuation(0.0002f);
    lightNortheast.setQuadraticAttenuation(0.00005f);
    lightNortheast.enable();
    lightNortheast.setPosition(sf3d::Vector3f(500.0f-gap, velocity*0.5f, -500.0f+gap));
    lightNorthwest.setColor(sf3d::Color::Blue);
    lightNorthwest.setAmbientIntensity(0.0f);
    lightNorthwest.setDiffuseIntensity(radius*0.5f);
    lightNorthwest.setLinearAttenuation(0.0002f);
    lightNorthwest.setQuadraticAttenuation(0.00005f);
    lightNorthwest.enable();
    lightNorthwest.setPosition(sf3d::Vector3f(-500.0f+gap, velocity*0.5f, -500.0f+gap));
    lightSoutheast.setColor(sf3d::Color::Green);
    lightSoutheast.setAmbientIntensity(0.0f);
    lightSoutheast.setDiffuseIntensity(radius*0.5f);
    lightSoutheast.setLinearAttenuation(0.0002f);
    lightSoutheast.setQuadraticAttenuation(0.00005f);
    lightSoutheast.enable();
    lightSoutheast.setPosition(sf3d::Vector3f(500.0f-gap, velocity*0.5f, 500.0f-gap));
    lightSouthwest.setColor(sf3d::Color::Yellow);
    lightSouthwest.setAmbientIntensity(0.0f);
    lightSouthwest.setDiffuseIntensity(radius*0.5f);
    lightSouthwest.setLinearAttenuation(0.0002f);
    lightSouthwest.setQuadraticAttenuation(0.00005f);
    lightSouthwest.enable();
    lightSouthwest.setPosition(sf3d::Vector3f(-500.0f+gap, velocity*0.5f, 500.0f-gap));
    lampNortheast.setColor(lightNortheast.getColor());
    lampNortheast.setPosition(lightNortheast.getPosition());
    lampNorthwest.setColor(lightNorthwest.getColor());
    lampNorthwest.setPosition(lightNorthwest.getPosition());
    lampSoutheast.setColor(lightSoutheast.getColor());
    lampSoutheast.setPosition(lightSoutheast.getPosition());
    lampSouthwest.setColor(lightSouthwest.getColor());
    lampSouthwest.setPosition(lightSouthwest.getPosition());
    sf3d::Light::enableLighting();
    //frame.setScale(0.75f, 0.75f);
    floor.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    ceiling.setPosition(sf3d::Vector3f(0.0f, velocity, 0.0f));
    wallNorth.setPosition(sf3d::Vector3f(0.0f, velocity*0.5f, -500.0f));
    wallSouth.setPosition(sf3d::Vector3f(0.0f, velocity*0.5f, 500.0f));
    wallEast.setPosition(sf3d::Vector3f(500.0f, velocity*0.5f, 0.0f));
    wallWest.setPosition(sf3d::Vector3f(-500.0f, velocity*0.5f, 0.0f));
    axisX.setColor(sf3d::Color::Red);
    axisX.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisX.setOrigin(sf3d::Vector3f(-250.0f, 1.0f, 1.0f));
    axisY.setColor(sf3d::Color::Green);
    axisY.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisY.setOrigin(sf3d::Vector3f(1.0f, -250.0f, 1.0f));
    axisZ.setColor(sf3d::Color::Blue);
    axisZ.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisZ.setOrigin(sf3d::Vector3f(1.0f, 1.0f, -250.0f));
    reticleHorizontal.setFillColor(sf3d::Color::White);
    reticleHorizontal.setSize(sf3d::Vector2f(15.0f, 2.5f));
    reticleHorizontal.setOrigin(reticleHorizontal.getSize()*0.5f);
    reticleHorizontal.setPosition(sf3d::Vector2f(frameTexture.getSize())*0.5f);
    reticleVertical.setFillColor(sf3d::Color::White);
    reticleVertical.setSize(sf3d::Vector2f(2.5f, 15.0f));
    reticleVertical.setOrigin(reticleVertical.getSize()*0.5f);
    reticleVertical.setPosition(sf3d::Vector2f(frameTexture.getSize())*0.5f);

    for (int i = 0; i != hurts; ++i)
    {
        sf3d::SoundBuffer* buffer = new sf3d::SoundBuffer();
        if (buffer->loadFromFile("hurt"+std::to_string(i)+".wav"))
        {
            hurtSounds.push_back(std::pair<sf3d::Sound*, sf3d::SoundBuffer*>(new sf3d::Sound(*buffer), buffer));
        }
        else
        {
            delete buffer;
            hurtSounds.push_back(std::pair<sf3d::Sound*, sf3d::SoundBuffer*>(nullptr, nullptr));
        }
    }
    landBuffer = new sf3d::SoundBuffer();
    if (landBuffer->loadFromFile("land.wav"))
    {
        landSound = new sf3d::Sound(*landBuffer);
    }
    else
    {
        delete landBuffer;
        landBuffer = nullptr;
    }
    shotBuffer = new sf3d::SoundBuffer();
    if (shotBuffer->loadFromFile("shot.wav"))
    {
        shotSound = new sf3d::Sound(*shotBuffer);
    }
    else
    {
        delete shotBuffer;
        shotBuffer = nullptr;
    }
    jumpBuffer = new sf3d::SoundBuffer();
    if (jumpBuffer->loadFromFile("jump.wav"))
    {
        jumpSound = new sf3d::Sound(*jumpBuffer);
    }
    else
    {
        delete jumpBuffer;
        jumpBuffer = nullptr;
    }

    color = sf3d::Color::White;

    // Enable depth testing so we can draw 3D objects in any order
    frameTexture.enableDepthTest(true);

    // Keep the mouse cursor within the window
    sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);

    if ((!arguments.empty()) && (tupleSpace != nullptr))
    {
        if (arguments.size() > 1)
        {
            name = arguments[1];
            if (arguments.size() > 2)
            {
                if (arguments.size() > 3)
                {
                    agent = new TcpConnectionHandlerAgent(sf3d::IpAddress(arguments[2]), 9001);
                    window.setTitle("Client "+name+" Team "+std::to_string(team));
                }
                else
                {
                    role = true;
                    team = false;
                    host = name;
                    agent = new TcpConnectionHandlerAgent(9001, 255);
                    window.setTitle("Server "+name+" Team "+std::to_string(team));
                }
                team = static_cast<bool>(atoi(arguments.back().c_str()));
                players[name] = new Player(height, team);
                players[name]->self = true;
            }
        }
        else
        {
            return -2;
        }
        camera.setPosition(sf3d::Vector3f(125.0f, 0.0f, 125.0f)*(team?-1.0f:1.0f));
        origin = camera.getPosition();
        yaw = getDirection(sf3d::Vector2f(origin.x, origin.z), sf3d::Vector2f());
        direction.x = cosf(yaw);
        direction.z = -sinf(yaw);
        if (team)
        {
            yaw *= -1.0f;
            direction *= -1.0f;
        }
        camera.setDirection(direction);
    }

    gap /= radius;

    clock.restart();
    while (window.isOpen())
    {
        int deltaX = 0;
        int deltaY = 0;
        glm::vec3 upVector;
        sf3d::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf3d::Event::Closed:
                    window.close();
                    break;
                case sf3d::Event::KeyPressed:
                    switch (event.key.code)
                    {
                        case sf3d::Keyboard::Key::Escape:
                            window.close();
                            break;
                    }
                    break;
                case sf3d::Event::MouseMoved:
                    deltaX = event.mouseMove.x - (window.getSize().x / 2);
                    deltaY = event.mouseMove.y - (window.getSize().y / 2);
                    break;
                case sf3d::Event::GainedFocus:
                    focus = true;
                    break;
                case sf3d::Event::LostFocus:
                    focus = false;
                    break;
            }
        }
        if (!window.isOpen())
        {
            continue;
        }
        if (result != 0)
        {
            window.close();
            continue;
        }
        delta = clock.restart().asSeconds() * 0.5f;
        time += delta;
        /*if (!focus)
        {
            window.clear(sf3d::Color::Black);
            window.display();
            continue;
        }*/

        // Keep the mouse cursor within the window
        if (focus)
        {
            sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);
        }

        if ((!minigame) && (focus))
        {
            yaw -= (static_cast<float>(deltaX) / 10.0f) * delta;
            pitch -= (static_cast<float>(deltaY) / 10.0f) * delta;
            if (pitch < -1.5f)
            {
                pitch = -1.5f;
            }
            if (pitch > 1.5f)
            {
                pitch = 1.5f;
            }
            direction.x = cosf(yaw) * cosf(pitch);
            direction.y = sinf(pitch);
            direction.z = -sinf(yaw) * cosf(pitch);
            //direction = getNormalized(direction);
            angle = atan2f(direction.z, direction.x);
            //direction += previous;
            //magnitude = sqrtf((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z));
            //direction /= magnitude;
            rightVector.x = -direction.z;
            rightVector.y = 0.0f;
            rightVector.z = direction.x;
            magnitude = sqrtf((rightVector.x * rightVector.x) + (rightVector.y * rightVector.y) + (rightVector.z * rightVector.z));
            rightVector /= magnitude;

            movement = getNormalized(direction*sf3d::Vector3f(1.0f, 0.0f, 1.0f));

            step = offset;

            // W key pressed : move forward
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::W))
            {
                offset += sf3d::Vector3f(movement * velocity * delta);
            }

            // A key pressed : strafe left
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::A))
            {
                offset += sf3d::Vector3f(rightVector * -velocity * delta);
            }

            // S key pressed : move backward
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::S))
            {
                offset += sf3d::Vector3f(movement * -velocity * delta);
            }

            // D key pressed : strafe right
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::D))
            {
                offset += sf3d::Vector3f(rightVector * velocity * delta);
            }

            /*
            // Q key pressed : move upwards
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::Q))
            {
                offset += sf3d::Vector3f(0.0f, velocity * delta, 0.0f);
            }

            // E key pressed : move downwards
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::E))
            {
                offset += sf3d::Vector3f(0.0f, -velocity * delta, 0.0f);
            }
            */

            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::Space))
            {
                if (!jump)
                {
                    if (jumpSound != nullptr)
                    {
                        jumpSound->play();
                    }
                    jump = true;
                    fall = velocity / gravity;
                }
            }

            if (jump)
            {
                fall -= gravity * delta * 0.5f;
                offset += sf3d::Vector3f(0.0f, fall, 0.0f);
                if (offset.y < 0.0f)
                {
                    if (landSound != nullptr)
                    {
                        landSound->play();
                    }
                    offset.y = 0.0f;
                    fall = 0.0f;
                    jump = false;
                }
            }

            if (origin.x+offset.x < wallWest.getPosition().x+gap)
            {
                offset.x = (wallWest.getPosition().x+gap)-origin.x;
            }
            if (origin.x+offset.x > wallEast.getPosition().x-gap)
            {
                offset.x = (wallEast.getPosition().x-gap)-origin.x;
            }
            if (origin.z+offset.z < wallNorth.getPosition().z+gap)
            {
                offset.z = (wallNorth.getPosition().z+gap)-origin.z;
            }
            if (origin.z+offset.z > wallSouth.getPosition().z-gap)
            {
                offset.z = (wallSouth.getPosition().z-gap)-origin.z;
            }
            //std::cout << offset.x << " " << offset.z << std::endl;

            camera.setPosition(origin + offset);
            camera.move(sf3d::Vector3f(0.0f, height, 0.0f));

            if ((agent != nullptr) && (!peers.empty()))
            {
                sf3d::Vector3f position = origin+offset;
                sf3d::Vector3f motion = offset-step;
                if ((getDistance(motion, sf3d::Vector3f()) > 1.0f) || (deltaX != 0) || (deltaY != 0))
                {
                    std::string response = "\tM"+name+"\t"+std::to_string(position.x)+" "+std::to_string(position.y)+" "+std::to_string(position.z)+" "+std::to_string(motion.x)+" "+std::to_string(motion.y)+" "+std::to_string(motion.z)+" "+std::to_string(angle)+tail;
                    sf3d::Packet* packet = new sf3d::Packet();
                    packet->append(response.c_str(), response.size());
                    packets.push_back(packet);
                }
            }

            if (shoot <= 0.0f)
            {
                if (sf3d::Mouse::isButtonPressed(sf3d::Mouse::Button::Left))
                {
                    Bullet* bullet = new Bullet(life, team, direction);
                    shoot = 0.05f;
                    bullets.push_back(bullet);
                    bullet->appearance->setSize(sf3d::Vector3f(range, 1.0f, 1.0f));
                    bullet->appearance->setOrigin(-bullet->appearance->getSize()*0.5f);
                    bullet->appearance->setRotation((-angle)*(180.0f/pi), sf3d::Vector3f(0.0f, 1.0f, 0.0f));
                    bullet->appearance->rotate(pitch*(180.0f/pi), rightVector);
                    bullet->appearance->setPosition(camera.getPosition());
                    bullet->appearance->move(-sf3d::Vector3f(0.0f, height, 0.0f)*0.5f);
                    if (agent != nullptr)
                    {
                        std::string response = "\tS"+name+"\t"+std::to_string(direction.x)+" "+std::to_string(direction.y)+" "+std::to_string(direction.z)+" "+std::to_string(pitch)+tail;
                        sf3d::Packet* packet = new sf3d::Packet();
                        packet->append(response.c_str(), response.size());
                        packets.push_back(packet);
                    }
                    if (shotSound != nullptr)
                    {
                        shotSound->play();
                    }
                }
            }
            else
            {
                shoot -= delta;
            }
        }

        upVector = glm::cross(glm::vec3(direction.x, direction.y, direction.z), glm::vec3(rightVector.x, rightVector.y, rightVector.z));
        camera.setDirection(direction);
        camera.setUpVector(-sf3d::Vector3f(upVector.x, upVector.y, upVector.z));

        // Inform the window to update its view with the new camera data
        frameTexture.setView(camera);

        // Clear the window
        frameTexture.clear(sf3d::Color::Black);

        lightCamera.setPosition(camera.getPosition());

        // Draw the background
        //frameTexture.draw(sf3d::Color::Black);

        // Disable lighting for the text and the light sphere
        sf3d::Light::disableLighting();

        // Disable depth testing for sf3d::Text because it requires blending
        frameTexture.enableDepthTest(false);
        for (int i = 0; i != texts.size(); ++i)
        {
            sf3d::Text* text = std::get<0>(texts[i]);
            if (std::get<1>(texts[i]) < 0.0f)
            {
                text->setColor(sf3d::Color::Black);
                texts[i] = std::pair<sf3d::Text*, float>(text, 7.5);
            }
            texts[i] = std::pair<sf3d::Text*, float>(text, std::get<1>(texts[i])-delta);
            if (std::get<1>(texts[i]) < 0.0f)
            {
                std::cout << text->getString().toAnsiString() << std::endl;
                delete text;
                texts.erase(texts.begin()+i);
                --i;
                continue;
            }
            text->setPosition(sf3d::Vector3f(gap, gap*gap, 0.0f));
            frameTexture.draw(*text);
        }
        frameTexture.enableDepthTest(true);

        // Enable lighting again
        sf3d::Light::enableLighting();

        // Draw everything
        frameTexture.draw(floor);
        frameTexture.draw(ceiling);
        frameTexture.draw(wallNorth);
        frameTexture.draw(wallSouth);
        frameTexture.draw(wallEast);
        frameTexture.draw(wallWest);
        frameTexture.draw(lampNortheast);
        frameTexture.draw(lampNorthwest);
        frameTexture.draw(lampSoutheast);
        frameTexture.draw(lampSouthwest);
        //frameTexture.draw(axisX);
        //frameTexture.draw(axisY);
        //frameTexture.draw(axisZ);
        for (std::map<std::string, Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
        {
            Player* player = iter->second;
            if (!player->self)
            {
                sf3d::FloatBox bounds = iter->second->appearance->getGlobalBounds();
                sf3d::Vector3f min = sf3d::Vector3f(std::min(bounds.left, bounds.left+bounds.width), std::min(bounds.top, bounds.top+bounds.height), std::min(bounds.front, bounds.front+bounds.depth));
                sf3d::Vector3f max = sf3d::Vector3f(std::max(bounds.left, bounds.left+bounds.width), std::max(bounds.top, bounds.top+bounds.height), std::max(bounds.front, bounds.front+bounds.depth));
                player->min = min;
                player->max = max;
                player->position += player->motion*delta;
                player->motion *= 0.5f;
                player->appearance->setPosition(player->position);
                player->appearance->setRotation(player->angle*(180.0f/pi), sf3d::Vector3f(0.0f, 1.0f, 0.0f));
                frameTexture.draw(*player->appearance);
            }
        }
        for (int i = 0; i != bullets.size(); ++i)
        {
            Bullet* bullet = bullets[i];
            bullet->life -= delta;
            if (bullet->life <= 0.0f)
            {
                delete bullet;
                bullets.erase(bullets.begin()+i);
                --i;
                continue;
            }
            frameTexture.draw(*bullet->appearance);
            if (bullet->peer)
            {
                continue;
            }
            for (std::map<std::string, Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
            {
                if ((iter->second->hit) || (iter->second->self) || (iter->second->team == bullet->team))
                {
                    continue;
                }
                if (checkIntersection(bullet->appearance->getPosition(), bullet->direction, iter->second->min, iter->second->max))
                {
                    std::string response = "\tH"+name+"\t"+iter->first+tail;
                    sf3d::Packet* packet = new sf3d::Packet();
                    packet->append(response.c_str(), response.size());
                    packets.push_back(packet);
                    if (role)
                    {
                        iter->second->appearance->setColor(sf3d::Color::Black);
                        victim = iter->first;
                        minigame = true;
                        texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                        std::get<0>(texts.back())->setString(sf3d::String(name+" shot "+iter->first));
                        ++hurt;
                        if (hurt == hurts)
                        {
                            hurt = 0;
                        }
                        if (std::get<0>(hurtSounds[hurt]) != nullptr)
                        {
                            std::get<0>(hurtSounds[hurt])->play();
                        }
                    }
                    iter->second->hit = true;
                }
            }
        }

        // Disable lighting and reset to 2D view to draw information
        sf3d::Light::disableLighting();
        frameTexture.setView(window.getDefaultView());

        // Draw informational text
        reticleHorizontal.rotate(delta*36.0f);
        reticleVertical.rotate(delta*36.0f);
        reticleHorizontal.setScale(sf3d::Vector3f(sqrtf(fabsf(fmodf(time, 2.0f)-1.0f)+1.0f), 1.0f, 1.0f));
        reticleVertical.setScale(sf3d::Vector3f(1.0f, reticleHorizontal.getScale().x, 1.0f));
        frameTexture.draw(reticleHorizontal);
        frameTexture.draw(reticleVertical);

        // Reset view to our camera and enable lighting again
        frameTexture.setView(camera);
        sf3d::Light::enableLighting();

        // Finally, display the rendered frame on screen
        frameTexture.display();

        window.clear(sf3d::Color(128, 128, 128));
        window.draw(frame);
        window.display();

        if (agent != nullptr)
        {
            Tuple* reception = tupleSpace->get("RECEIVE_PACKET");
            while (reception != nullptr)
            {
                bool omit = false;
                sf3d::Packet* packet = static_cast<sf3d::Packet*>(reception->getItemAsVoid(0));
                std::string message = std::string(static_cast<const char*>(packet->getData()), packet->getDataSize());
                message = message.substr(message.find_first_of('\t')).substr(1);
                message = message.substr(0, message.find_first_of(tail));
                while (message.front() == '\t')
                {
                    message = message.substr(1);
                }
                if (message.front() != 'M')
                {
                    std::cout << "recv " << message << std::endl;
                }
                delete packet;
                delete reception;
                if (!message.empty())
                {
                    switch (message.front())
                    {
                        case 'S':
                            omit = true;
                            {
                                std::string peer = message.substr(1, message.find_first_of('\t')-1);
                                if (peer != name)
                                {
                                    if (std::find(peers.begin(), peers.end(), peer) != peers.end())
                                    {
                                        std::vector<std::string> splits = split(message.substr(message.find_first_of('\t')).substr(1)," ");
                                        if (splits.size() == 4)
                                        {
                                            sf3d::Vector3f right;
                                            sf3d::Vector3f aim = sf3d::Vector3f(atof(splits[0].c_str()), atof(splits[1].c_str()), atof(splits[2].c_str()));
                                            Player* player = players[peer];
                                            Bullet* bullet = new Bullet(life, player->team, aim);
                                            player->hit = false;
                                            player->aim = atof(splits[3].c_str());
                                            player->direction = aim;
                                            bullet->peer = true;
                                            right.x = -aim.z;
                                            right.y = 0.0f;
                                            right.z = aim.x;
                                            magnitude = sqrtf((right.x * right.x) + (right.y * right.y) + (right.z * right.z));
                                            right /= magnitude;
                                            bullets.push_back(bullet);
                                            bullet->appearance->setSize(sf3d::Vector3f(range, 1.0f, 1.0f));
                                            bullet->appearance->setOrigin(-bullet->appearance->getSize()*0.5f);
                                            bullet->appearance->setRotation((-player->angle)*(180.0f/pi), sf3d::Vector3f(0.0f, 1.0f, 0.0f));
                                            bullet->appearance->rotate(player->aim*(180.0f/pi), right);
                                            bullet->appearance->setPosition(player->position);
                                            bullet->appearance->move(sf3d::Vector3f(0.0f, height, 0.0f));
                                            if ((role) && (!minigame))
                                            {
                                                std::string response = "\t"+message+tail;
                                                packet = new sf3d::Packet();
                                                packet->append(response.c_str(), response.size());
                                                packets.push_back(packet);
                                                for (std::map<std::string, Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
                                                {
                                                    std::string shooter = peer;
                                                    std::string target = iter->first;
                                                    if (shooter == target)
                                                    {
                                                        continue;
                                                    }
                                                    if (players[shooter]->team == players[target]->team)
                                                    {
                                                        continue;
                                                    }
                                                    if (players[target]->hit)
                                                    {
                                                        continue;
                                                    }
                                                    if ((shooter == name) || (target == name))
                                                    {
                                                        Player* player = players[name];
                                                        player->angle = atan2f(direction.z, direction.x);
                                                        player->appearance->setPosition(player->position);
                                                        player->appearance->setRotation(player->angle*(180.0f/pi), sf3d::Vector3f(0.0f, 1.0f, 0.0f));
                                                        sf3d::FloatBox bounds = player->appearance->getGlobalBounds();
                                                        sf3d::Vector3f min = sf3d::Vector3f(std::min(bounds.left, bounds.left+bounds.width), std::min(bounds.top, bounds.top+bounds.height), std::min(bounds.front, bounds.front+bounds.depth));
                                                        sf3d::Vector3f max = sf3d::Vector3f(std::max(bounds.left, bounds.left+bounds.width), std::max(bounds.top, bounds.top+bounds.height), std::max(bounds.front, bounds.front+bounds.depth));
                                                        player->min = min;
                                                        player->max = max;
                                                        player->position = origin+offset;
                                                        player->direction = direction;
                                                    }
                                                    sf3d::Vector3f position = players[shooter]->position;
                                                    position += sf3d::Vector3f(0.0f, height, 0.0f);
                                                    if (checkIntersection(position, players[shooter]->direction, players[target]->min, players[target]->max))
                                                    {
                                                        std::string response = "\tH"+shooter+"\t"+target+tail;
                                                        packet = new sf3d::Packet();
                                                        packet->append(response.c_str(), response.size());
                                                        packets.push_back(packet);
                                                        minigame = true;
                                                        victim = target;
                                                        players[target]->hit = true;
                                                        players[target]->appearance->setColor(sf3d::Color::Black);
                                                        texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                                        std::get<0>(texts.back())->setString(sf3d::String(shooter+" shot "+target));
                                                        ++hurt;
                                                        if (hurt == hurts)
                                                        {
                                                            hurt = 0;
                                                        }
                                                        if (std::get<0>(hurtSounds[hurt]) != nullptr)
                                                        {
                                                            std::get<0>(hurtSounds[hurt])->play();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        std::cout << "no hit" << std::endl;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case 'M':
                            omit = true;
                            {
                                std::string peer = message.substr(1, message.find_first_of('\t')-1);
                                if (peer != name)
                                {
                                    if (std::find(peers.begin(), peers.end(), peer) != peers.end())
                                    {
                                        std::vector<std::string> splits = split(message.substr(message.find_first_of('\t')).substr(1)," ");
                                        if (splits.size() == 7)
                                        {
                                            Player* player = players[peer];
                                            sf3d::Vector3f position = sf3d::Vector3f(atof(splits[0].c_str()), atof(splits[1].c_str()), atof(splits[2].c_str()));
                                            sf3d::Vector3f motion = sf3d::Vector3f(atof(splits[3].c_str()), atof(splits[4].c_str()), atof(splits[5].c_str()));
                                            player->position = position;
                                            player->motion = motion;
                                            player->angle = atof(splits[6].c_str());
                                            player->hit = false;
                                            if (role)
                                            {
                                                std::string response = "\t"+message+tail;
                                                packet = new sf3d::Packet();
                                                packet->append(response.c_str(), response.size());
                                                packets.push_back(packet);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case 'D':
                            {
                                std::string peer = message.substr(1);
                                std::cout << peer << "!" << std::endl;
                                if (peer == name)
                                {
                                    window.close();
                                    result = 2;
                                }
                                else
                                {
                                    if (peer == host)
                                    {
                                        window.close();
                                        result = 3;
                                    }
                                    else
                                    {
                                        auto iter = std::find(peers.begin(), peers.end(), peer);
                                        if (iter != peers.end())
                                        {
                                            texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                            std::get<0>(texts.back())->setString(sf3d::String(peer+" disconnected"));
                                            delete players[peer];
                                            players.erase(players.find(peer));
                                            peers.erase(iter);
                                            if (role)
                                            {
                                                std::string response = "\tD"+peer+tail;
                                                packet = new sf3d::Packet();
                                                packet->append(response.c_str(), response.size());
                                                packets.push_back(packet);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case 'C':
                            {
                                std::string peer = message.substr(1, message.find_first_of('\t')-1);
                                if (peer == name)
                                {
                                    result = -3;
                                }
                                else
                                {
                                    if (std::find(peers.begin(), peers.end(), peer) == peers.end())
                                    {
                                        players[peer] = new Player(height, static_cast<bool>(atoi(message.substr(message.find_first_of('\t')).substr(1).c_str())));
                                        peers.push_back(peer);
                                        if (role)
                                        {
                                            std::string response = "\tA"+name+"\t"+peer+"\t"+message.substr(message.find_first_of('\t')).substr(1)+tail;
                                            packet = new sf3d::Packet();
                                            packet->append(response.c_str(), response.size());
                                            packets.push_back(packet);
                                            if (minigame)
                                            {
                                                response = "\tH"+host+"\t"+victim+tail;
                                                packet = new sf3d::Packet();
                                                packet->append(response.c_str(), response.size());
                                                packets.push_back(packet);
                                            }
                                            texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                            std::get<0>(texts.back())->setString(sf3d::String(peer+" connected"));
                                        }
                                    }
                                }
                            }
                            break;
                        case 'A':
                            {
                                std::string peer = message.substr(1, message.find_first_of('\t')-1);
                                if (peer == name)
                                {
                                    result = -4;
                                }
                                else
                                {
                                    if (std::find(peers.begin(), peers.end(), peer) == peers.end())
                                    {
                                        if (!role)
                                        {
                                            texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                            std::get<0>(texts.back())->setString(sf3d::String(peer+" connected"));
                                            players[peer] = new Player(height, false);
                                            peers.push_back(peer);
                                            host = peer;
                                            peer = message.substr(message.find_first_of('\t')).substr(1, message.find_last_of('\t')-(message.find_first_of('\t')+1));
                                            if (peer == name)
                                            {
                                                announced = true;
                                            }
                                            else
                                            {
                                                players[peer] = new Player(height, static_cast<bool>(atoi(message.substr(message.find_last_of('\t')).substr(1).c_str())));
                                                peers.push_back(peer);
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case 'H':
                            omit = true;
                            {
                                std::string shooter = message.substr(1, message.find_first_of('\t')-1);
                                std::string target = message.substr(message.find_first_of('\t')).substr(1, message.find_last_of('\t')-(message.find_first_of('\t')+1));
                                std::cout << shooter << " " << target << "?" << std::endl;
                                if ((players.find(shooter) != players.end()) && (players.find(target) != players.end()) && (!minigame))
                                {
                                    if ((!players[target]->hit) || (!role))
                                    {
                                        if ((shooter == name) || (target == name))
                                        {
                                            Player* player = players[name];
                                            player->angle = atan2f(direction.z, direction.x);
                                            player->appearance->setPosition(player->position);
                                            player->appearance->setRotation(player->angle*(180.0f/pi), sf3d::Vector3f(0.0f, 1.0f, 0.0f));
                                            sf3d::FloatBox bounds = player->appearance->getGlobalBounds();
                                            sf3d::Vector3f min = sf3d::Vector3f(std::min(bounds.left, bounds.left+bounds.width), std::min(bounds.top, bounds.top+bounds.height), std::min(bounds.front, bounds.front+bounds.depth));
                                            sf3d::Vector3f max = sf3d::Vector3f(std::max(bounds.left, bounds.left+bounds.width), std::max(bounds.top, bounds.top+bounds.height), std::max(bounds.front, bounds.front+bounds.depth));
                                            player->min = min;
                                            player->max = max;
                                            player->position = origin+offset;
                                            player->direction = direction;
                                        }
                                        if (role)
                                        {
                                            sf3d::Vector3f position = players[shooter]->position;
                                            position += sf3d::Vector3f(0.0f, height, 0.0f);
                                            if (checkIntersection(position, players[shooter]->direction, players[target]->min, players[target]->max))
                                            {
                                                std::string response = "\t"+message+tail;
                                                packet = new sf3d::Packet();
                                                packet->append(response.c_str(), response.size());
                                                packets.push_back(packet);
                                                minigame = true;
                                                victim = target;
                                                players[target]->hit = true;
                                                players[target]->appearance->setColor(sf3d::Color::Black);
                                                texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                                std::get<0>(texts.back())->setString(sf3d::String(shooter+" shot "+target));
                                                ++hurt;
                                                if (hurt == hurts)
                                                {
                                                    hurt = 0;
                                                }
                                                if (std::get<0>(hurtSounds[hurt]) != nullptr)
                                                {
                                                    std::get<0>(hurtSounds[hurt])->play();
                                                }
                                            }
                                            else
                                            {
                                                std::cout << "no hit" << std::endl;
                                            }
                                        }
                                        else
                                        {
                                            minigame = true;
                                            victim = target;
                                            players[target]->hit = true;
                                            players[target]->appearance->setColor(sf3d::Color::Black);
                                            texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                                            std::get<0>(texts.back())->setString(sf3d::String(shooter+" shot "+target));
                                            ++hurt;
                                            if (hurt == hurts)
                                            {
                                                hurt = 0;
                                            }
                                            if (std::get<0>(hurtSounds[hurt]) != nullptr)
                                            {
                                                std::get<0>(hurtSounds[hurt])->play();
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    std::cout << "no shoot" << std::endl;
                                }
                            }
                            break;
                        default:
                          std::cout << "no " << message.front() << std::endl;
                          break;
                    }
                    if (!omit)
                    {
                        for (int i = 0; i != peers.size(); ++i)
                        {
                            std::cout << name << " | " << host << " | " << peers[i] << " | " << i << std::endl;
                        }
                    }
                }
                reception = tupleSpace->get("RECEIVE_PACKET");
            }
            if (announcement)
            {
                if ((fmodf(time, 5.0f) < delta) && (!announced))
                {
                    announcement = false;
                }
            }
            else
            {
                announcement = true;
                if (!role)
                {
                    std::string response = "\tC"+name+"\t"+std::to_string(static_cast<int>(team))+tail;
                    sf3d::Packet* packet = new sf3d::Packet();
                    packet->append(response.c_str(), response.size());
                    packets.push_back(packet);
                }
            }
            while (!packets.empty())
            {
                Tuple* tuple = new Tuple("v", packets.front());
                std::string message = std::string(static_cast<const char*>(packets.front()->getData()), packets.front()->getDataSize());
                message = message.substr(message.find_first_of('\t'));
                if (message.substr(1).front() != 'M')
                {
                    std::cout << message << std::endl;
                }
                packets.erase(packets.begin());
                tupleSpace->put("PACKET_READY", tuple);
            }
            packets.clear();
        }

        if (!victim.empty())
        {
            //std::cout << victim << std::endl;
            if (players.find(victim) != players.end())
            {
                players[victim]->hit = true;
            }
        }

        if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::Tab))
        {
            if (!list)
            {
                if ((name.empty()) && (!arguments.empty()))
                {
                    name = arguments.front();
                }
                list = true;
                texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                std::get<0>(texts.back())->setString(sf3d::String(name));
                std::cout << name << std::endl;
                for (int i = 0; i != peers.size(); ++i)
                {
                    texts.push_back(std::pair<sf3d::Text*, float>(new sf3d::Text(sf3d::String(""), font, lettering), -1.0f));
                    std::get<0>(texts.back())->setString(sf3d::String(peers[i]));
                    std::cout << peers[i] << std::endl;
                }
            }
            else
            {
                std::cout << "list" << std::endl;
            }
        }
        else
        {
            list = false;
        }
    }

    if ((agent != nullptr) && (result != 3) && (!peers.empty()))
    {
        std::string response = "\tD"+name+tail;
        sf3d::Packet* packet = new sf3d::Packet();
        packet->append(response.c_str(), response.size());
        Tuple* tuple = new Tuple("v", packet);
        std::string message = std::string(static_cast<const char*>(packet->getData()), packet->getDataSize());
        message = message.substr(message.find_first_of('\t'));
        std::cout << message << std::endl;
        tupleSpace->put("PACKET_READY", tuple);
    }

    for (int i = 0; i != texts.size(); ++i)
    {
        delete std::get<0>(texts[i]);
    }
    texts.clear();
    for (int i = 0; i != hurtSounds.size(); ++i)
    {
        delete std::get<0>(hurtSounds[i]);
        delete std::get<1>(hurtSounds[i]);
    }
    hurtSounds.clear();
    delete jumpSound;
    delete jumpBuffer;
    delete shotSound;
    delete shotBuffer;
    delete landSound;
    delete landBuffer;
    for (int i = 0; i != bullets.size(); ++i)
    {
        delete bullets[i];
    }
    bullets.clear();
    for (std::map<std::string, Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
    {
        delete iter->second;
    }
    players.clear();

    peers.clear();
    packets.clear();

    //delete agent;

    return result;
}

int main(int argc, char** argv)
{
    int result = 0;
    std::vector<std::string> arguments;
    TupleSpace* tupleSpace = nullptr;
    sf3d::Font* font = new sf3d::Font();
    sf3d::RenderWindow* window = new sf3d::RenderWindow();
    sf3d::RenderTexture* frameTexture = new sf3d::RenderTexture();
    window->create(sf3d::VideoMode(1280, 720), "Chronolateral");
    window->setFramerateLimit(60);
    window->setVerticalSyncEnabled(true);
    window->setMouseCursorVisible(false);
    frameTexture->create(window->getSize().x, window->getSize().y, true);
    for (int i = 0; i != argc; ++i)
    {
        arguments.push_back(std::string(argv[i]));
        std::cout << i << '\t' << arguments.back() << std::endl;
    }
    if (arguments.size() > 1)
    {
        tupleSpace = new TupleSpace();
    }
    if (font->loadFromFile("sansation.ttf"))
    {
        result = run(tupleSpace, *font, *window, *frameTexture, arguments);
    }
    else
    {
        std::cout << "font fail" << std::endl;
    }
    delete font;
    delete frameTexture;
    delete window;
    std::cout << result << std::endl;
    if (result == 3)
    {
        std::cout << "host disconnected" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return result;
}


// Author: Pierce Brooks

#include <regex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
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

int run(sf3d::RenderWindow& window, sf3d::RenderTexture& frameTexture, const std::vector<std::string>& arguments)
{
    bool jump = false;
    bool team = true;
    bool minigame = false;
    bool focus = true;
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
    sf3d::Light light;
    sf3d::Clock clock;
    sf3d::Color color;
    sf3d::RectangleShape reticleVertical;
    sf3d::RectangleShape reticleHorizontal;
    sf3d::Sprite frame(frameTexture.getTexture());
    sf3d::Cuboid axisX(sf3d::Vector3f(500.0f, 2.0f, 2.0f));
    sf3d::Cuboid axisY(sf3d::Vector3f(2.0f, 500.0f, 2.0f));
    sf3d::Cuboid axisZ(sf3d::Vector3f(2.0f, 2.0f, 500.0f));
    sf3d::Camera camera(90.0f, 0.001f, 1000.0f);
    sf3d::Vector2f coordinate;
    sf3d::Vector3f previous;
    sf3d::Vector3f direction;
    sf3d::Vector3f movement;
    sf3d::Vector3f rightVector;
    sf3d::Vector3f offset;
    sf3d::Vector3f origin = camera.getPosition();
    float aspectRatio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
    sf3d::Vector3f downscaleFactor(1.0f / static_cast<float>(window.getSize().x) * aspectRatio, -1.0f / static_cast<float>(window.getSize().y), 1.0f);
    camera.scale(1.0f / aspectRatio, 1.0f, 1.0f);
    camera.setPosition(sf3d::Vector3f());
    window.setView(camera);
    light.setColor(sf3d::Color::White);
    light.setAmbientIntensity(0.5f);
    light.setDiffuseIntensity(1.0f);
    light.setLinearAttenuation(0.002f);
    light.setQuadraticAttenuation(0.0005f);
    light.enable();
    sf3d::Light::enableLighting();
    //frame.setScale(0.75f, 0.75f);
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

    color = sf3d::Color::White;

    // Enable depth testing so we can draw 3D objects in any order
    frameTexture.enableDepthTest(true);

    // Keep the mouse cursor within the window
    sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);

    if (!arguments.empty())
    {
        if (arguments.size() > 1)
        {
            team = (bool)atoi(arguments[1].c_str());
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
        delta = clock.restart().asSeconds();
        time += delta;
        if (!focus)
        {
            window.clear(sf3d::Color::Black);
            window.display();
            continue;
        }

        // Keep the mouse cursor within the window
        sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);

        if (!minigame)
        {
            yaw -= (static_cast<float>(deltaX) / 10.0f) * delta;
            pitch -= (static_cast<float>(deltaY) / 10.0f) * delta;
            if (pitch < -1.5f)
            {
                pitch = -1.5f;
            }
            if (pitch > 1.5f)
            {
                pitch > 1.5f;
            }
            direction.x = cosf(yaw) * cosf(pitch);
            direction.y = sinf(pitch);
            direction.z = -sinf(yaw) * cosf(pitch);
            //direction += previous;
            //magnitude = sqrtf((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z));
            //direction /= magnitude;
            rightVector.x = -direction.z;
            rightVector.y = 0.0f;
            rightVector.z = direction.x;
            magnitude = sqrtf((rightVector.x * rightVector.x) + (rightVector.y * rightVector.y) + (rightVector.z * rightVector.z));
            rightVector /= magnitude;

            movement = getNormalized(direction*sf3d::Vector3f(1.0f, 0.0f, 1.0f));

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
                    offset.y = 0.0f;
                    fall = 0.0f;
                    jump = false;
                }
            }

            camera.setPosition(origin + offset);
            camera.move(sf3d::Vector3f(0.0f, height, 0.0f));
        }

        upVector = glm::cross(glm::vec3(direction.x, direction.y, direction.z), glm::vec3(rightVector.x, rightVector.y, rightVector.z));
        camera.setDirection(direction);
        camera.setUpVector(-sf3d::Vector3f(upVector.x, upVector.y, upVector.z));

        // Inform the window to update its view with the new camera data
        frameTexture.setView(camera);

        // Clear the window
        frameTexture.clear(sf3d::Color::Black);

        light.setPosition(camera.getPosition());

        // Draw the background
        //frameTexture.draw(sf3d::Color::Black);

        // Disable lighting for the text and the light sphere
        sf3d::Light::disableLighting();

        // Disable depth testing for sf3d::Text because it requires blending
        frameTexture.enableDepthTest(false);
        //frameTexture.draw(text);
        frameTexture.enableDepthTest(true);

        // Enable lighting again
        sf3d::Light::enableLighting();

        // Draw everything
        frameTexture.draw(axisX);
        frameTexture.draw(axisY);
        frameTexture.draw(axisZ);

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
    }

    return 0;
}

int main(int argc, char** argv)
{
    int result;
    std::vector<std::string> arguments;
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
    result = run(*window, *frameTexture, arguments);
    delete frameTexture;
    delete window;
    return result;
}

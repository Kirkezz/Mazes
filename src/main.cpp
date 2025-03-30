#include "spacegui.h"

#include "Space.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <set>
#include <thread>
using namespace std;

#include "Demo.h"
#include "SpaceRenderer.h"
#include <SFML/Graphics.hpp>
using namespace sf;

int main(int argc, char** argv) {
    Space space(16, 16, Space::SQUARE);

    RenderWindow SFMLWindow;
    SpaceRenderer renderer(space, SFMLWindow, Vector2f(48, 48));

    QApplication app(argc, argv);
    SpaceGUI QtWindow(space, renderer);
    QtWindow.show();

    Demo demo(QtWindow);

    while (SFMLWindow.isOpen()) {
        if (!QtWindow.isVisible()) SFMLWindow.close();

        Vector2f mouse;
        Event event;
        while (SFMLWindow.pollEvent(event)) {
            if (event.type == Event::Closed)
                SFMLWindow.close();
            else if (event.type == Event::KeyPressed) {
                switch (event.key.code) {
                case Keyboard::Tab: renderer.selectNextPoint(); break;
                case Keyboard::Space:
                    space.gridResolution = QtWindow.mazeSets.aisleWidth;
                    renderer.fillStep();
                    renderer.pathStep();
                    space.gridResolution = 1;
                    break;
                case Keyboard::F: space.floodFill(); break;
                case Keyboard::A: renderer.setAllPoints(); break;
                case Keyboard::S:
                    renderer.draw();
                    SFMLWindow.display();
                    renderer.saveScreenshot("screenshots/" + to_string(uniform_int_distribution<>()(space.dre)) + ".png");
                    break;
                case Keyboard::Delete: renderer.deleteSelectedPoint(); break;
                case Keyboard::R: break;
                default: break;
                }
            } else if (event.type == event.MouseButtonPressed) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if (event.mouseButton.button == Mouse::Left) {
                    renderer.LMBPressed(mouse);
                } else if (event.mouseButton.button == Mouse::Right) {
                    renderer.RMBPressed(mouse);
                }
            } else if (event.type == event.MouseButtonReleased) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if (event.mouseButton.button == Mouse::Left) {
                    renderer.LMBReleased(mouse);
                } else if (event.mouseButton.button == Mouse::Right) {
                    renderer.RMBReleased(mouse);
                } else if (event.mouseButton.button == Mouse::Middle) {
                    renderer.MMBReleased(mouse);
                }
            }
            // else if (event.type == Event::Resized) {
            //     FloatRect view(0, 0, event.size.width, event.size.height);
            //     SFMLWindow.setView(View(view));
            // }
        }
        SFMLWindow.clear(renderer.colorScheme[SpaceRenderer::BACKGROUNDCOLOR]);
        space.gridResolution = QtWindow.mazeSets.aisleWidth;
        renderer.update();
        space.gridResolution = 1;
        auto t1 = std::chrono::high_resolution_clock::now();
        renderer.draw();
        SFMLWindow.display();
        QtWindow.update();
        app.processEvents();
        auto t2 = std::chrono::high_resolution_clock::now();
        demo.update();
        std::this_thread::sleep_for(renderer.frameDuration * 1ms - (t2 - t1));
    }
}

#include "Space.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <random>
#include <set>
using namespace std;

#include "SpaceRenderer.h"
#include <SFML/Graphics.hpp>
using namespace sf;

#include "spacegui.h"
#include <QtWidgets/QApplication>
int main(int argc, char** argv) {
    Space space(16, 16, Space::SQUARE);
    RenderWindow SFMLWindow;
    SpaceRenderer renderer(space, SFMLWindow, Vector2f(48, 48));
    QApplication app(argc, argv);
    SpaceGUI QtWindow(space, renderer);
    QtWindow.show();
    // size_t scrcnt = 0;
    while(SFMLWindow.isOpen()) {
        if(!QtWindow.isVisible())
            SFMLWindow.close();
        SFMLWindow.clear(renderer.colorScheme[SpaceRenderer::BACKGROUNDCOLOR]);
        Vector2f mouse;
        Event event;
        while(SFMLWindow.pollEvent(event)) {
            if(event.type == Event::Closed)
                SFMLWindow.close();
            else if(event.type == Event::KeyPressed) {
                switch(event.key.code) {
                case Keyboard::Tab:
                    renderer.selectNextPoint();
                    break;
                case Keyboard::Space:
                    renderer.fillStep();
                    renderer.pathStep();
                    break;
                case Keyboard::F:
                    space.floodFill();
                    break;
                case Keyboard::A:
                    renderer.setAllPoints();
                    break;
                case Keyboard::S:
                    renderer.draw();
                    SFMLWindow.display();
                    renderer.saveScreenshot("screenshots/" + to_string(uniform_int_distribution<>()(space.dre)) + ".png");
                    // renderer.saveScreenshot("screenshots/bfs" + to_string(++scrcnt) + ".png");
                    break;
                    // case Keyboard::R:
                    // scrcnt = 0;
                    // break;
                case Keyboard::Delete:
                    renderer.deleteSelectedPoint();
                    break;
                default:
                    break;
                }
            } else if(event.type == event.MouseButtonPressed) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if(event.mouseButton.button == Mouse::Left) {
                    renderer.LMBPressed(mouse);
                } else if(event.mouseButton.button == Mouse::Right) {
                    renderer.RMBPressed(mouse);
                }
            } else if(event.type == event.MouseButtonReleased) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if(event.mouseButton.button == Mouse::Left) {
                    renderer.LMBReleased(mouse);
                } else if(event.mouseButton.button == Mouse::Right) {
                    renderer.RMBReleased(mouse);
                } else if(event.mouseButton.button == Mouse::Middle) {
                    renderer.MMBReleased(mouse);
                }
            }
        }
        renderer.update();
        renderer.draw();
        SFMLWindow.display();
        QtWindow.update();
        app.processEvents();
    }
}

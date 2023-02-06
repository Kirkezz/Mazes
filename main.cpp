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
    while(SFMLWindow.isOpen()) {
        if(!QtWindow.isVisible())
            SFMLWindow.close();
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
                    renderer.step();
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
        SFMLWindow.clear(renderer.backgroundColor);
        renderer.update();
        renderer.draw();
        SFMLWindow.display();
        app.processEvents();
    }
}

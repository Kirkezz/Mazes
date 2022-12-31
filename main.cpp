#include "Space.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <ctime>
#include <cmath>
#include <set>
using namespace std;

#include <SFML/Graphics.hpp>
#include "SpaceRenderer.h"
using namespace sf;

#include <QtWidgets/QApplication>
#include "spacegui.h"

int main(int argc, char** argv) {
    Space space(8, 8);
    RenderWindow SFMLWindow(VideoMode(768, 768), L"▦");
    SpaceRenderer renderer(space, SFMLWindow, Vector2f(64, 64));
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
            }
            else if(event.type == event.MouseButtonPressed) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if(event.mouseButton.button == Mouse::Left) {
                    renderer.LMBPressed(Point2Df(mouse.x, mouse.y));
                }
                else if(event.mouseButton.button == Mouse::Right) {
                    renderer.RMBPressed(Point2Df(mouse.x, mouse.y));
                }
            }
            else if(event.type == event.MouseButtonReleased) {
                mouse = SFMLWindow.mapPixelToCoords(Mouse::getPosition(SFMLWindow));
                if(event.mouseButton.button == Mouse::Left) {
                    renderer.LMBReleased(Point2Df(mouse.x, mouse.y));
                }
                else if(event.mouseButton.button == Mouse::Right) {
                    renderer.RMBReleased(Point2Df(mouse.x, mouse.y));
                }
                else if(event.mouseButton.button == Mouse::Middle) {
                    renderer.MMBReleased(Point2Df(mouse.x, mouse.y));
                }
            }
        }
        SFMLWindow.clear();
        renderer.update();
        renderer.draw();
        SFMLWindow.display();
        app.processEvents();
    }
}

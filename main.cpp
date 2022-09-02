#include "Space.h"
#include "SpaceRenderer.h"

#include <iostream>
using namespace std;

#include <SFML/Graphics.hpp>
using namespace sf;

int main() {
    RenderWindow window(VideoMode(768, 768), L"▦");
    window.setFramerateLimit(30);
    Space space(16, 16);
    space.zigzag();
    SpaceRenderer renderer(space, window);
    renderer.pointSpeed = 8.f;
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (event.type == Event::KeyPressed) {
                switch (event.key.code) {
                case Keyboard::X:
                    space.mirrorX();
                    renderer.loadSpaceProps();
                    break;
                case Keyboard::Y:
                    space.mirrorY();
                    renderer.loadSpaceProps();
                    break;
                case Keyboard::Tab:
                    renderer.selectNextPoint();
                    break;
                default:
                    break;
                }
            }
            else if (event.type == event.MouseButtonReleased) {
                Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
                if (event.mouseButton.button == Mouse::Left) {
                    renderer.LMBReleased(Point2Df(mouse.x, mouse.y));
                }
                else if (event.mouseButton.button == Mouse::Right) {
                    renderer.RMBReleased(Point2Df(mouse.x, mouse.y));
                }
            }
        }
        window.clear();
        renderer.update();
        renderer.draw();
        window.display();
    }
}

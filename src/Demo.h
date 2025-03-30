#pragma once
#include "./ui_spacegui.h"
#include "spacegui.h"

class Demo {
    SpaceGUI& gui;
    Space& space;
    SpaceRenderer& renderer;
    int currentScene = 0;

  public:
    Demo(SpaceGUI& gui) : gui(gui), space(gui.space), renderer(gui.renderer) {}
    void update() {
        if (!gui.isDemo) return;
        if (currentScene == 42) {
            gui.isDemo = false;
            return;
        }

        static sf::Clock c;
        static bool pause = false;
        if (space.fillStepList.empty() && space.pathStepList.empty()) {
            if (!pause) {
                pause = true;
                c.restart();
                gui.ui->checkBox_5->setChecked(false);
            }
            if (c.getElapsedTime().asSeconds() >= 2.0f) {
                pause = false;
                gui.ui->checkBox_5->setChecked(true);
            } else {
                return;
            }
            ++currentScene;
            switch (currentScene) {
            case 1:
                gui.ui->isVisualization->setChecked(1);
                gui.ui->isSonification->setChecked(1);
                gui.ui->tiling->setCurrentIndex(1);
                gui.ui->mazeAlg->setCurrentIndex(1);
                break;
            case 2:
                gui.ui->SParam1->setValue(24);
                gui.ui->SParam2->setValue(24);
                gui.ui->mazeAlg->setCurrentIndex(2);
                break;
            case 3: gui.ui->mazeAlg->setCurrentIndex(3); break;
            case 4: gui.ui->mazeAlg->setCurrentIndex(4); break;
            case 5:
                gui.ui->checkBox_5->setChecked(false);
                gui.ui->SParam1->setValue(64);
                gui.ui->SParam2->setValue(64);
                gui.ui->mazeAlg->setCurrentIndex(5);
                break;
            case 6:
                gui.ui->SParam1->setValue(12);
                gui.ui->SParam2->setValue(12);
                gui.ui->mazeAlg->setCurrentIndex(6);
                break;
            case 7:
                gui.ui->SParam1->setValue(24);
                gui.ui->SParam2->setValue(24);
                gui.ui->mazeAlg->setCurrentIndex(7);
                break;
            case 8: gui.ui->mazeAlg->setCurrentIndex(8); break;
            case 9: gui.ui->mazeAlg->setCurrentIndex(9); break;
            case 10:
                gui.ui->tiling->setCurrentIndex(2);
                gui.ui->mazeAlg->setCurrentIndex(1);
                break;
            case 11: gui.ui->mazeAlg->setCurrentIndex(3); break;
            case 12: gui.ui->mazeAlg->setCurrentIndex(4); break;
            case 13:
                gui.ui->SParam1->setValue(12);
                gui.ui->SParam2->setValue(12);
                gui.ui->mazeAlg->setCurrentIndex(6);
                break;
            case 14:
                gui.ui->SParam1->setValue(24);
                gui.ui->SParam2->setValue(24);
                gui.ui->mazeAlg->setCurrentIndex(7);
                break;
            case 15: gui.ui->mazeAlg->setCurrentIndex(8); break;
            case 16: gui.ui->mazeAlg->setCurrentIndex(9); break;
            case 17:
                gui.ui->tiling->setCurrentIndex(3);
                gui.ui->SParam2->setValue(24);
                gui.ui->mazeAlg->setCurrentIndex(1);
                break;
            case 18: gui.ui->mazeAlg->setCurrentIndex(3); break;
            case 19: gui.ui->mazeAlg->setCurrentIndex(4); break;
            case 20:
                gui.ui->SParam1->setValue(128);
                gui.ui->mazeAlg->setCurrentIndex(6);
                break;
            case 21:
                gui.ui->SParam1->setValue(256);
                gui.ui->mazeAlg->setCurrentIndex(7);
                break;
            case 22: gui.ui->mazeAlg->setCurrentIndex(8); break;
            case 23: gui.ui->mazeAlg->setCurrentIndex(9); break;
            default: currentScene = 42; return;
            }
            if (gui.ui->tiling->currentIndex() != 3) {
                gui.ui->SRWidth->setValue(768 / gui.ui->SParam1->value());
                gui.ui->SRHeight->setValue(768 / gui.ui->SParam2->value());
            }
            gui.on_apply_clicked();
        }
    }
};

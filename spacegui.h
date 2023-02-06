#pragma once

#include "Space.h"
#include "SpaceRenderer.h"
#include <QMainWindow>
#include <random>

QT_BEGIN_NAMESPACE
namespace Ui {
class SpaceGUI;
}
QT_END_NAMESPACE

class SpaceGUI : public QMainWindow {
    Q_OBJECT
public:
    SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget* parent = nullptr);
    ~SpaceGUI();
private slots:
    void on_tiling_currentIndexChanged(int index);
    void on_apply_clicked();
    void on_isSBSF_stateChanged(int arg1);
    void on_isManualStep_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);
    void on_checkBox_3_stateChanged(int arg1);
    void on_checkBox_4_stateChanged(int arg1);
    void on_checkBox_5_stateChanged(int arg1);
    void on_checkBox_6_stateChanged(int arg1);
    void on_checkBox_7_stateChanged(int arg1);
private:
    Ui::SpaceGUI* ui;
    Space& space;
    SpaceRenderer& renderer;
    int prevTilingIndex = 0;
};

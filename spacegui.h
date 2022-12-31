#pragma once

#include <QMainWindow>
#include "Space.h"
#include "SpaceRenderer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SpaceGUI; }
QT_END_NAMESPACE

class SpaceGUI : public QMainWindow {
    Q_OBJECT

public:
    SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget *parent = nullptr);
    ~SpaceGUI();
private slots:
    void on_maze_clicked(bool checked);
    void on_apply_clicked();
    void on_spaceButtonMirrorX_clicked();
    void on_spaceButtonMirrorY_clicked();
    void on_spaceCheckBoxSBSFilling_stateChanged(int arg1);
    void on_manualStep_stateChanged(int arg1);
    void on_lines_clicked(bool checked);

private:
    Ui::SpaceGUI *ui;
    Space& space;
    SpaceRenderer& renderer;
};

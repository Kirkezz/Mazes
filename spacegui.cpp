#include "spacegui.h"
#include "./ui_spacegui.h"

SpaceGUI::SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget *parent) : QMainWindow(parent), ui(new Ui::SpaceGUI), space(space), renderer(renderer) {
    ui->setupUi(this);
    ui->spaceSpinBoxX->setValue(space.width);
    ui->spaceSpinBoxY->setValue(space.height);
    ui->spaceSpinBoxSizeX->setValue(renderer.rectSize.x);
    ui->spaceSpinBoxSizeY->setValue(renderer.rectSize.y);
    ui->lines->setChecked(renderer.curve);
}
SpaceGUI::~SpaceGUI() {
    delete ui;
}
void SpaceGUI::on_maze_clicked(bool checked) {
    renderer.maze = checked;
}
void SpaceGUI::on_apply_clicked() {
    bool f = (ui->spaceSpinBoxX->value() != space.width || ui->spaceSpinBoxY->value() != space.height) || (ui->spaceSpinBoxSizeX->value() != renderer.rectSize.x || ui->spaceSpinBoxSizeY->value() != renderer.rectSize.y);
    renderer.rectSize.x = ui->spaceSpinBoxSizeX->value(); renderer.rectSize.y = ui->spaceSpinBoxSizeY->value();
    renderer.rectSizeInit();
    space.resize(ui->spaceSpinBoxX->value(), ui->spaceSpinBoxY->value());
    if(f) {
        renderer.loadSpaceProps();
    } else {
        renderer.clear();
    }
    space.stepByStepFilling = ui->spaceCheckBoxSBSFilling->isChecked();
    renderer.spaceStepListDelay = ui->spaceSpinBoxStepListDelay->value();
    (space.*(space.fillArr[ui->spaceFillingComboBox->currentIndex()]))();
    if(space.stepByStepFilling) {
        space.getField().clear();
        space.getField().resize(space.width * space.height);
    }
    renderer.manualStep = ui->manualStep->isChecked();
}
void SpaceGUI::on_spaceButtonMirrorX_clicked() {
    space.mirrorX();
}
void SpaceGUI::on_spaceButtonMirrorY_clicked() {
    space.mirrorY();
}
void SpaceGUI::on_spaceCheckBoxSBSFilling_stateChanged(int arg1) {
    ui->spaceSpinBoxStepListDelay->setEnabled(arg1 && !ui->manualStep->isChecked());
    ui->manualStep->setEnabled(arg1);
}
void SpaceGUI::on_manualStep_stateChanged(int arg1) {
    ui->spaceSpinBoxStepListDelay->setEnabled(!arg1 && ui->spaceCheckBoxSBSFilling->isChecked());
}
void SpaceGUI::on_lines_clicked(bool checked) {
    renderer.curve = checked;
}


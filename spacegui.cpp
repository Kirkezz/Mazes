#include "spacegui.h"
#include "./ui_spacegui.h"

using namespace std;
SpaceGUI::SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget* parent) : QMainWindow(parent), ui(new Ui::SpaceGUI), space(space), renderer(renderer) {
    ui->setupUi(this);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(true);
    ui->checkBox_4->setChecked(true);
    ui->checkBox_5->setChecked(true);
    ui->checkBox_6->setChecked(true);
}
SpaceGUI::~SpaceGUI() { delete ui; }
void SpaceGUI::on_tiling_currentIndexChanged(int index) {
    if(index > 1) { // Space::AMORPHOUS
        ui->label->setText(QString("Ячеек"));
        ui->SParam1->setValue(256);
        ui->label_2->setText(QString("Сглаживание"));
        ui->SParam2->setValue(3);
        ui->label_4->setText(QString("Ширина окна"));
        ui->SRWidth->setValue(768);
        ui->label_5->setText(QString("Высота окна"));
        ui->SRHeight->setValue(768);
    } else if(prevTilingIndex > 1) {
        ui->label->setText(QString("Столбцов"));
        ui->SParam2->setValue(16);
        ui->label_2->setText(QString("Строк"));
        ui->SParam1->setValue(16);
        ui->label_4->setText(QString("Ширина ячейки"));
        ui->SRWidth->setValue(48);
        ui->label_5->setText(QString("Высота ячейки"));
        ui->SRHeight->setValue(48);
    }
    ui->SParam2->setMinimum(index < 2);
    prevTilingIndex = index;
}
void SpaceGUI::on_apply_clicked() {
    switch(ui->tiling->currentIndex()) {
    case 0:
        space.resize(ui->SParam1->value(), ui->SParam2->value(), Space::SQUARE);
        break;
    case 1:
        space.resize(ui->SParam1->value(), ui->SParam2->value(), Space::HEXAGON);
        break;
    case 2: // Space::AMORPHOUS (Voronoi)
        space.VoronoiPoints.clear();
        space.VoronoiPoints.reserve(ui->SParam1->value());
        for(size_t i = 0; i < ui->SParam1->value(); ++i) {
            space.VoronoiPoints.push_back(
                {uniform_int_distribution(0, ui->SRHeight->value())(space.dre), uniform_int_distribution(0, ui->SRWidth->value())(space.dre)});
        }
        space.resize(Point2Df(ui->SRWidth->value(), ui->SRHeight->value()), ui->SParam2->value());
        break;
    }
    renderer.path.clear();
    space.stepList.clear();
    space.stepByStepFilling = ui->isSBSF->isChecked();
    renderer.spaceStepListDelay = ui->mazeAlgDelay->value();
    renderer.manualStep = ui->isManualStep->isChecked();
    (space.*(space.fillArr[ui->mazeAlg->currentIndex()]))();
    if(space.stepByStepFilling) {
        space.stepByStepFilling = false;
        space.clear();
        space.stepByStepFilling = true;
    }
    renderer.loadSpaceProps(Point2Df(ui->SRWidth->value(), ui->SRHeight->value()));
}
void SpaceGUI::on_isSBSF_stateChanged(int arg1) {
    ui->mazeAlgDelay->setEnabled(arg1 && !ui->isManualStep->isChecked());
    ui->isManualStep->setEnabled(arg1);
}
void SpaceGUI::on_isManualStep_stateChanged(int arg1) { ui->mazeAlgDelay->setEnabled(!arg1 && ui->isSBSF->isChecked()); }
void SpaceGUI::on_checkBox_2_stateChanged(int arg1) { renderer.isGridDrawn = arg1; }
void SpaceGUI::on_checkBox_3_stateChanged(int arg1) { renderer.isPathDrawn = arg1; }
void SpaceGUI::on_checkBox_4_stateChanged(int arg1) { renderer.isMazeDrawn = arg1; }
void SpaceGUI::on_checkBox_5_stateChanged(int arg1) { renderer.isCurveDrawn = arg1; }
void SpaceGUI::on_checkBox_6_stateChanged(int arg1) { renderer.isPointsDrawn = arg1; }
void SpaceGUI::on_checkBox_7_stateChanged(int arg1) { renderer.isDebugInfoDrawn = arg1; }

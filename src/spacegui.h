#pragma once

#include "Space.h"
#include "SpaceRenderer.h"
#include <QColorDialog>
#include <QFrame>
#include <QMainWindow>
#include <array>
#include <fstream>
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
    void on_isManualMazeStep_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);
    void on_checkBox_3_stateChanged(int arg1);
    void on_checkBox_4_stateChanged(int arg1);
    void on_checkBox_5_stateChanged(int arg1);
    void on_checkBox_6_stateChanged(int arg1);
    void on_checkBox_7_stateChanged(int arg1);
    void on_pathAlg_currentIndexChanged(int index);
    void on_isManualPathStep_stateChanged(int arg1);
    void on_isPathAlgVisualization_stateChanged(int arg1);
    void on_pathAlgDelay_valueChanged(double arg1);
    void selectColor(QFrame* frame, sf::Color& color, QColor qc = QColorDialog::getColor());
    void on_backgroundColorButton_clicked();
    void on_outlineColorButton_clicked();
    void on_curveColorButton_clicked();
    void on_mazeColorButton_clicked();
    void on_pointsColorButton_clicked();
    void on_pathColorButton_clicked();
    void on_shapeColorButton_clicked();
    void on_firstColorButton_clicked();
    void on_secondColorButton_clicked();
    void on_thirdColorButton_clicked();
    void on_fourthColorButton_clicked();
    void on_saveToFileButton_clicked();
    void on_loadFromFileButton_clicked();
    void on_outlineThickness_valueChanged(double arg1);
    void on_curveThickness_valueChanged(double arg1);
    void on_pathThickness_valueChanged(double arg1);
    void on_mazeThickness_valueChanged(double arg1);
private:
    struct CustomizationData {
        sf::Color colors[SpaceRenderer::NUM_COLORS] = {sf::Color(0, 30, 50),   sf::Color::White,        sf::Color::Blue,         sf::Color(173, 255, 152),
                                                       sf::Color::Magenta,     sf::Color(26, 113, 185), sf::Color(255, 42, 109), sf::Color(223, 101, 148),
                                                       sf::Color(5, 217, 232), sf::Color(1, 25, 15),    sf::Color(255, 42, 109)};
        std::array<float, 4> thicknesses = {2.f, 2.f, 3.f, 2.f};
    };
    void loadCD(const CustomizationData& cd);
    Ui::SpaceGUI* ui;
    Space& space;
    SpaceRenderer& renderer;
    int prevTilingIndex = 0;
};

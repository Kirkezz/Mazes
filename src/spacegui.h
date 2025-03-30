#pragma once

#include "Space.h"
#include "SpaceRenderer.h"
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>

#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/sequence.hpp>

#include <QTranslator>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace bf = boost::fusion;

QT_BEGIN_NAMESPACE
namespace Ui {
class SpaceGUI;
}
QT_END_NAMESPACE

struct MazeGenerationSettings {
    bool* saveIntermediateVPs;
    MetaBool mi_1 = {{.v = saveIntermediateVPs, .name = Tr("Step-by-step Lloyd's algorithm")}};
    double* voronoiVisualizationDelay;
    MetaDouble mi_2 = {{.v = voronoiVisualizationDelay, .name = Tr("Lloyd's algorithm delay")}, {.step = 0.05}};
    unsigned aisleWidth = 1;
    MetaUint mi_3 = {{.v = &aisleWidth, .name = Tr("Aisle width")}, {.min = 1}};
    Space::GrowingTreeMazeSettings* growingTreeMazeSets;
    MetaStruct<Space::GrowingTreeMazeSettings> mi_4 = {{.v = growingTreeMazeSets, .name = Tr("Growing tree maze settings"), .enabled = true}};
};

struct PathfindingSettings {
    SpaceRenderer::PathfindingSettings* pathSets;
    MetaStruct<SpaceRenderer::PathfindingSettings> mi_1 = {{.v = pathSets, .name = Tr("Pathfinding")}};
};

struct VisualizationSettings {
    double* frameDuration;
    MetaDouble mi_1 = {{.v = frameDuration, .name = Tr("Minimum frame delay")}, {}};
    Space::SonificationSettings* sonSets;
    MetaStruct<Space::SonificationSettings> mi_2 = {{.v = sonSets, .name = Tr("Sonification")}};
    struct MGVisualizationSettings {
        bool visualization = true;
        MetaBool mi_1 = {{.v = &visualization, .name = Tr("Visualize/sonify")}};
        double* mazeAlgDelay;
        MetaDouble mi_2 = {{.v = mazeAlgDelay, .name = Tr("Visualization delay")}, {.step = 0.1}};
        bool* manualMazeStep;
        MetaBool mi_3 = {{.v = manualMazeStep, .name = Tr("Step on the space bar")}};
    } mazeSets;
    MetaStruct<MGVisualizationSettings> mi_3 = {{.v = &mazeSets, .name = Tr("Maze generation")}};
    struct PFVisualizationSettings {
        bool visualization = true;
        MetaBool mi_1 = {{.v = &visualization, .name = Tr("Visualize/sonify")}};
        double* pathAlgDelay;
        MetaDouble mi_2 = {{.v = pathAlgDelay, .name = Tr("Visualization delay")}, {.step = 0.1}};
        bool* manualPathStep;
        MetaBool mi_3 = {{.v = manualPathStep, .name = Tr("Step on the space bar")}};
        float* pointSpeed;
        MetaFloat mi_4 = {{.v = pointSpeed, .name = Tr("Speed of points")}, {}};
    } pathSets;
    MetaStruct<PFVisualizationSettings> mi_4 = {{.v = &pathSets, .name = Tr("Pathfinding")}};
};
BOOST_FUSION_ADAPT_STRUCT(Space::SonificationSettings, mi_1, mi_2, mi_3, mi_4, mi_5, mi_6, mi_7, mi_8, mi_9, mi_10)

BOOST_FUSION_ADAPT_STRUCT(Space::SonificationSettings::OscillatorSettings, mi_1, mi_2, mi_3, mi_4, mi_5)

BOOST_FUSION_ADAPT_STRUCT(Space::GrowingTreeMazeSettings, mi_1, mi_2, mi_3, mi_4, mi_5)

BOOST_FUSION_ADAPT_STRUCT(MazeGenerationSettings, mi_1, mi_2, mi_3, mi_4) // todo: move from bindings to substruct in renderer with mi_'s; use inheritance

BOOST_FUSION_ADAPT_STRUCT(SpaceRenderer::PathfindingSettings, mi_1)

BOOST_FUSION_ADAPT_STRUCT(PathfindingSettings, mi_1)

BOOST_FUSION_ADAPT_STRUCT(VisualizationSettings::MGVisualizationSettings, mi_1, mi_2, mi_3)

BOOST_FUSION_ADAPT_STRUCT(VisualizationSettings::PFVisualizationSettings, mi_1, mi_2, mi_3, mi_4)

BOOST_FUSION_ADAPT_STRUCT(VisualizationSettings, mi_1, mi_2, mi_3, mi_4)

struct Demo;

/// A class for rendering a Qt window.
class SpaceGUI : public QMainWindow {
    Q_OBJECT
    Space& space;
    SpaceRenderer& renderer;
  public:
    friend class Demo;
    SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget* parent = nullptr);
    ~SpaceGUI();
    void update();

    struct SettingsHandler {
        QFormLayout* layout;
        QWidget* parent;
        std::function<void(void)>* regenerateDialog;
        int depth;
        SettingsHandler(QWidget* parent, QFormLayout* layout, std::function<void(void)>* regenerateDialog, int depth = 0)
            : parent(parent), layout(layout), regenerateDialog(regenerateDialog), depth(depth) {}
        template<typename T, typename... Ts>
        void operator()(const MetaInfo<T, Ts...>& xx) {
            auto& x = const_cast<MetaInfo<T, Ts...>&>(xx);
            if (!x.enabled) return;
            if constexpr (!std::is_same_v<MetaInfo<T, Ts...>, MetaStruct<T>>) {
                if (!x.setter) x.setter = [&x](auto v) { *x.v = v; };
            }
            x.regenerateDialog = regenerateDialog;

            if constexpr (std::is_same_v<MetaInfo<T, Ts...>, MetaStruct<T>>) {
                QGroupBox* groupBox = new QGroupBox(tr(x.name.c_str()));
                x.widget = (void*) groupBox;

                QFormLayout* subLayout = new QFormLayout;
                boost::fusion::for_each(*x.v, SettingsHandler(parent, subLayout, regenerateDialog));
                groupBox->setLayout(subLayout);

                this->layout->addRow(groupBox);
            } else if constexpr (std::is_same_v<T, bool>) {
                QCheckBox* checkBox = new QCheckBox(parent);
                x.widget = (void*) checkBox;

                checkBox->setChecked(*x.v);
                connect(checkBox, &QCheckBox::checkStateChanged, x.setter);
                layout->addRow(tr(x.name.c_str()), checkBox);
            } else if constexpr (std::is_same_v<MetaInfo<T, Ts...>, MetaFiles>) {
                QPushButton* pushButton = new QPushButton(parent);
                x.widget = (void*) pushButton;
                pushButton->setText(tr(x.name.c_str()));

                connect(pushButton, &QPushButton::clicked, [&x](bool) {
                    QFileDialog dialog(((QPushButton*) x.widget)->parentWidget());
                    dialog.setFileMode(QFileDialog::ExistingFiles);
                    QStringList filenames;
                    if (dialog.exec()) filenames = dialog.selectedFiles();
                    x.v->clear();
                    for (QString& i : filenames) {
                        x.v->push_back(i.toStdString());
                    }
                });

                layout->addRow(pushButton);
            } else if constexpr (std::is_same_v<MetaInfo<T, Ts...>, MetaFolderPath>) {
                QPushButton* pushButton = new QPushButton(parent);
                x.widget = (void*) pushButton;
                pushButton->setText(tr(x.name.c_str()));

                connect(pushButton, &QPushButton::clicked, [&x](bool) {
                    QString dir = QFileDialog::getExistingDirectory(((QPushButton*) x.widget)->parentWidget(), tr("Open output directory"), "/",
                                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

                    *x.v = dir.toStdString();
                });

                layout->addRow(pushButton);
            } else if constexpr (std::is_same_v<MetaInfo<T, Ts...>, MetaSetterButton>) {
                QPushButton* pushButton = new QPushButton(parent);
                x.widget = (void*) pushButton;
                pushButton->setText(tr(x.name.c_str()));

                connect(pushButton, &QPushButton::clicked, x.setter);

                layout->addRow(pushButton);
            } else if constexpr (std::is_same_v<MetaInfo<T, Ts...>, MetaMap>) {
                QComboBox* comboBox = new QComboBox(parent);
                x.widget = (void*) comboBox;

                for (auto& i : x.stringVec) {
                    comboBox->addItem(tr(i.c_str()));
                }
                comboBox->setCurrentIndex(*x.v);
                connect(comboBox, &QComboBox::currentIndexChanged, x.setter);
                layout->addRow(tr(x.name.c_str()), comboBox);
            } else if constexpr (Numeric<T>) {
                using SpinBox = std::conditional_t<std::is_integral_v<T>, QSpinBox, QDoubleSpinBox>;
                SpinBox* spinBox = new SpinBox(parent);
                x.widget = (void*) spinBox;

                spinBox->setMinimum(x.min);
                spinBox->setMaximum(x.max);
                spinBox->setValue(*x.v);
                connect(spinBox, &SpinBox::valueChanged, x.setter);
                layout->addRow(tr(x.name.c_str()), spinBox);
            }

            connect(parent, &QWidget::destroyed, [&](QObject*) { x.widget = nullptr; });
        }
    };

    template<typename T>
    class SettingsBuilder : public QDialog {
      public:
        SettingsBuilder(QWidget* parent, T& s) : QDialog(parent) {
            setWindowFlag(Qt::WindowStaysOnTopHint);
            setAttribute(Qt::WA_DeleteOnClose);
            setWindowTitle("SubWindow");

            QFormLayout* layout = new QFormLayout(this);
            std::function<void(void)>* regenerateDialog = new std::function<void(void)>();
            *regenerateDialog = [&, layout, regenerateDialog] {
                QLayoutItem* item;
                while (layout->count() && (item = layout->takeAt(0)) != nullptr) {
                    if (auto w = item->widget(); w) w->deleteLater();
                    delete item;
                }

                boost::fusion::for_each(s, SettingsHandler(this, layout, regenerateDialog));
                adjustSize();
            };
            (*regenerateDialog)();

            connect(this, &QDialog::destroyed, [regenerateDialog](QObject*) { delete regenerateDialog; });

            // setSizePolicy(QSizePolicy::Minimum);
        }
    };

    MazeGenerationSettings mazeSets = {.saveIntermediateVPs = &space.saveIntermediateVPs,
                                       .voronoiVisualizationDelay = &renderer.voronoiVisualizationDelay,
                                       .growingTreeMazeSets = &space.growingTreeMazeSets};

    PathfindingSettings pathSets = {.pathSets = &renderer.pathSets};

    VisualizationSettings visSets = {.frameDuration = &renderer.frameDuration,
                                     .sonSets = &space.sonSets,
                                     .mazeSets = {.mazeAlgDelay = &renderer.spaceFillStepListDelay, .manualMazeStep = &renderer.manualFillStep},
                                     .pathSets = {.pathAlgDelay = &renderer.spacePathStepListDelay,
                                                  .manualPathStep = &renderer.manualPathStep,
                                                  .pointSpeed = &renderer.pointSpeed}};

    bool isDemo = false;

  private slots:
    void slotLanguageChanged(QAction* action);
    void on_tiling_currentIndexChanged(int index);
    void on_apply_clicked();
    void on_mazeAlg_currentIndexChanged(int index);
    void on_checkBox_2_stateChanged(int arg1);
    void on_checkBox_3_stateChanged(int arg1);
    void on_checkBox_4_stateChanged(int arg1);
    void on_checkBox_5_stateChanged(int arg1);
    void on_checkBox_6_stateChanged(int arg1);
    void on_checkBox_7_stateChanged(int arg1);
    void on_pathAlg_currentIndexChanged(int index);
    void on_isVisualization_stateChanged(int arg1);
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
    void on_fifthColorButton_clicked();
    void on_saveToFileButton_clicked();
    void on_loadFromFileButton_clicked();
    void on_outlineThickness_valueChanged(double arg1);
    void on_curveThickness_valueChanged(double arg1);
    void on_pathThickness_valueChanged(double arg1);
    void on_mazeThickness_valueChanged(double arg1);
    void on_mazeInfo_clicked();
    void on_pathInfo_clicked();
    void on_isSonification_checkStateChanged(const Qt::CheckState& arg1);
    void on_mazeSettings_clicked();
    void on_pathfindingSettings_clicked();
    void on_visualizationSettings_clicked();
    void on_antialiasingLevel_valueChanged(int arg1);
    void on_actionStart_Demo_triggered();

  private:
    struct CustomizationData {
        sf::Color colors[SpaceRenderer::NUM_COLORS] = {sf::Color(0, 30, 50),   sf::Color::White,        sf::Color::Blue,         sf::Color(173, 255, 152),
                                                       sf::Color::Magenta,     sf::Color(26, 113, 185), sf::Color(255, 42, 109), sf::Color(223, 101, 148),
                                                       sf::Color(5, 217, 232), sf::Color(0, 0, 25),     sf::Color(255, 42, 109), sf::Color::Green};
        std::array<float, 4> thicknesses = {2.f, 2.f, 3.f, 4.f};
    };
    void loadCD(const CustomizationData& cd);
    Ui::SpaceGUI* ui;
    int prevTilingIndex = 0;

    QString currentLanguage = "EN";
    void changeEvent(QEvent*);
    void loadLanguage(const QString& rLanguage);
    void createLanguageMenu();

    QTranslator m_translator;   // contains the translations for this application
    QTranslator m_translatorQt; // contains the translations for qt
    QString m_currLang;         // contains the currently loaded language
    QString m_langPath;         // Path of language files. This is always fixed to /languages
};

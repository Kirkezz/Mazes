#include "spacegui.h"
#include "./ui_spacegui.h"
#include <QActionGroup>
#include <QDir>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFileDialog>

using namespace std;

void switchTranslator(QTranslator& translator, const QString& filename) {
    qApp->removeTranslator(&translator);

    bool result = translator.load(filename);

    if (!result) {
        return;
    }
    qApp->installTranslator(&translator);
}

SpaceGUI::SpaceGUI(Space& space, SpaceRenderer& renderer, QWidget* parent) : QMainWindow(parent), ui(new Ui::SpaceGUI), space(space), renderer(renderer) {
    ui->setupUi(this);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(true);
    ui->checkBox_4->setChecked(true);
    ui->checkBox_5->setChecked(true);
    ui->checkBox_6->setChecked(true);
    loadCD(CustomizationData());

    createLanguageMenu();
    ui->menuBar->setNativeMenuBar(false);
    QString resourceFileName = ":/resource/translation/Mazes_EN.qm";
    switchTranslator(m_translator, resourceFileName);

    pathSets.pathSets->mi_1.setter = [&](auto v) { // damn-bitch-you-live-like-this.jpg
        *pathSets.pathSets->mi_1.v = v;
        if (v == 0) { // Manhattan distance
            space.calcWeightFunc = [&](size_t a, size_t b) {
                return (std::abs(int(a / space.width()) - int(b / space.width())) * renderer.shapeSize.x
                        + std::abs(int(a % space.width()) - int(b % space.width())) * renderer.shapeSize.y);
            };
        } else if (v == 1) { // Euclidean distance
            space.calcWeightFunc = [&](size_t a, size_t b) { return Point2Df(renderer.shapes[a].getPosition()).distance(Point2Df(renderer.shapes[b].getPosition())); };
        } // TODO: http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html "Manhattan distance adapted to hexagonal grids."
        if (pathSets.pathSets->mi_1.widget) { ((QComboBox*) pathSets.pathSets->mi_1.widget)->setCurrentIndex(v); }
    };
}
SpaceGUI::~SpaceGUI() { delete ui; }
void SpaceGUI::update() {
    if (!ui->SParam1->isEnabled()) {
        ui->SParam1->setValue(renderer.shapes.size());
    }
}
void SpaceGUI::on_tiling_currentIndexChanged(int index) {
    if (index > 2 || index == 0) { // Space::AMORPHOUS
        if (index == 0) {
            ui->label->setText(tr("Columns"));
            ui->SParam2->setValue(1);
            ui->SParam2->setEnabled(0);
            ui->label_2->setText(tr("Rows"));
            ui->SParam1->setValue(1);
            ui->SParam1->setEnabled(0);
        } else {
            ui->label->setText(tr("Cells"));
            ui->SParam1->setValue(256);
            ui->label_2->setText(tr("Smoothing"));
            ui->SParam2->setValue((index == 3) * 3);
        }
        ui->label_4->setText(tr("Window width"));
        ui->SRWidth->setValue(768);
        ui->label_5->setText(tr("Window height"));
        ui->SRHeight->setValue(768);
    } else if (prevTilingIndex > 2 || prevTilingIndex == 0) {
        ui->label->setText(tr("Columns"));
        ui->SParam2->setValue(16);
        ui->label_2->setText(tr("Rows"));
        ui->SParam1->setValue(16);
        ui->label_4->setText(tr("Cell width"));
        ui->SRWidth->setValue(48);
        ui->label_5->setText(tr("Cell height"));
        ui->SRHeight->setValue(48);
    }
    ui->SParam2->setMinimum(index < 3);

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->mazeAlg->model());
    for (size_t j = 0; j < space.availableFillings[ui->tiling->currentIndex()].size(); ++j) {
        QStandardItem* item = model->item(j);
        item->setFlags(space.availableFillings[ui->tiling->currentIndex()][j] ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);
    }

    if (!space.availableFillings[ui->tiling->currentIndex()][ui->mazeAlg->currentIndex()]) { ui->mazeAlg->setCurrentIndex(0); }

    ui->SParam1->setEnabled(index != 4 && index != 0);
    ui->SParam2->setEnabled(index != 0);

    prevTilingIndex = index;
}
void SpaceGUI::on_mazeAlg_currentIndexChanged(int index) {
    // mazeSets.mi_3.enabled = index == 9;
    if (!isDemo) on_apply_clicked();
}
void SpaceGUI::on_apply_clicked() {
    size_t tiling = ui->tiling->currentIndex();
    if (tiling == 0 || (tiling == 4 && renderer.getPointsSize() == 0)) {
        space.resize(1, 1, Space::SQUARE);
    } else if (tiling == 1) {
        space.resize(ui->SParam1->value(), ui->SParam2->value(), Space::SQUARE);
    } else if (tiling == 2) {
        space.resize(ui->SParam1->value(), ui->SParam2->value(), Space::HEXAGON);
    } else if (tiling == 3 || tiling == 4) {
        space.VoronoiPoints.clear();
        space.VoronoiPoints.reserve(ui->SParam1->value());
        if (ui->tiling->currentIndex() == 3) {
            for (size_t i = 0; i < ui->SParam1->value(); ++i)
                space.VoronoiPoints.push_back(
                    {uniform_int_distribution(0, ui->SRHeight->value())(space.dre), uniform_int_distribution(0, ui->SRWidth->value())(space.dre)});
        } else if (ui->tiling->currentIndex() == 4) {
            auto points = renderer.getPoints();
            for (auto& i : points)
                space.VoronoiPoints.push_back({int(i.x), int(i.y)});
        }
        if (space.VoronoiPoints.size() > 2) space.resize(Point2Df(ui->SRWidth->value(), ui->SRHeight->value()), ui->SParam2->value());
    }

    renderer.path.clear();
    space.fillStepList.clear();
    space.pathStepList.clear();
    {
        lock_guard<std::mutex> guard(space.processedStepsMutex);
        space.processedSteps.clear();
    }

    space.gridResolution = mazeSets.aisleWidth;

    if (space.gridResolution != 1) {
        space.stepByStepFill = false;
        space.clear();
    }

    space.stepByStepFill = ui->isVisualization->isChecked() && visSets.mazeSets.visualization;
    (space.*(space.fillArr[ui->mazeAlg->currentIndex()]))();

    if (space.stepByStepFill) {
        space.stepByStepFill = false;
        space.gridResolution = 1;
        space.clear();
        space.gridResolution = mazeSets.aisleWidth;
        space.clear();
        space.stepByStepFill = true;
    }

    space.gridResolution = 1;

    renderer.loadSpaceProps(Point2Df(ui->SRWidth->value(), ui->SRHeight->value()));
}
void SpaceGUI::on_checkBox_2_stateChanged(int arg1) { renderer.isGridDrawn = arg1; }
void SpaceGUI::on_checkBox_3_stateChanged(int arg1) { renderer.isPathDrawn = arg1; }
void SpaceGUI::on_checkBox_4_stateChanged(int arg1) { renderer.isMazeDrawn = arg1; }
void SpaceGUI::on_checkBox_5_stateChanged(int arg1) { renderer.isCurveDrawn = arg1; }
void SpaceGUI::on_checkBox_6_stateChanged(int arg1) { renderer.isPointsDrawn = arg1; }
void SpaceGUI::on_checkBox_7_stateChanged(int arg1) { renderer.isDebugInfoDrawn = arg1; }
void SpaceGUI::on_pathAlg_currentIndexChanged(int index) { renderer.selectedPathAlg = index; }
void SpaceGUI::on_isVisualization_stateChanged(int arg1) { space.stepByStepPath = arg1 && visSets.pathSets.visualization; }
void SpaceGUI::selectColor(QFrame* frame, sf::Color& color, QColor qc) {
    frame->setStyleSheet(QString("background-color: %1;").arg(qc.name()));
    color = sf::Color(qc.red(), qc.green(), qc.blue(), qc.alpha());
}
void SpaceGUI::on_backgroundColorButton_clicked() { selectColor(ui->backgroundColorFrame, renderer.colorScheme[SpaceRenderer::BACKGROUNDCOLOR]); }
void SpaceGUI::on_outlineColorButton_clicked() { selectColor(ui->outlineColorFrame, renderer.colorScheme[SpaceRenderer::OUTLINECOLOR]); }
void SpaceGUI::on_curveColorButton_clicked() { selectColor(ui->curveColorFrame, renderer.colorScheme[SpaceRenderer::CURVECOLOR]); }
void SpaceGUI::on_mazeColorButton_clicked() { selectColor(ui->mazeColorFrame, renderer.colorScheme[SpaceRenderer::MAZECOLOR]); }
void SpaceGUI::on_pointsColorButton_clicked() { selectColor(ui->pointsColorFrame, renderer.colorScheme[SpaceRenderer::POINTSCOLOR]); }
void SpaceGUI::on_pathColorButton_clicked() { selectColor(ui->pathColorFrame, renderer.colorScheme[SpaceRenderer::PATHCOLOR]); }
void SpaceGUI::on_shapeColorButton_clicked() { selectColor(ui->shapeColorFrame, renderer.colorScheme[SpaceRenderer::DEFAULTCOLOR]); }
void SpaceGUI::on_firstColorButton_clicked() { selectColor(ui->firstColorFrame, renderer.colorScheme[SpaceRenderer::COLOR1]); }
void SpaceGUI::on_secondColorButton_clicked() { selectColor(ui->secondColorFrame, renderer.colorScheme[SpaceRenderer::COLOR2]); }
void SpaceGUI::on_thirdColorButton_clicked() { selectColor(ui->thirdColorFrame, renderer.colorScheme[SpaceRenderer::COLOR3]); }
void SpaceGUI::on_fourthColorButton_clicked() { selectColor(ui->fourthColorFrame, renderer.colorScheme[SpaceRenderer::COLOR4]); }
void SpaceGUI::on_fifthColorButton_clicked() { selectColor(ui->fifthColorFrame, renderer.colorScheme[SpaceRenderer::TEXTCOLOR]); }
void SpaceGUI::on_outlineThickness_valueChanged(double arg1) { renderer.outlineThickness = arg1; }
void SpaceGUI::on_curveThickness_valueChanged(double arg1) { renderer.curveThickness = arg1; }
void SpaceGUI::on_pathThickness_valueChanged(double arg1) { renderer.pathThickness = arg1; }
void SpaceGUI::on_mazeThickness_valueChanged(double arg1) { renderer.mazeThickness = arg1; }
void SpaceGUI::on_saveToFileButton_clicked() {
    string filename = QFileDialog::getSaveFileName(this, tr("Customization filename"), "", tr("Binary files (*.bin)")).toStdString();
    ofstream fs(filename, ios::binary | ios::trunc | ios::out);

    CustomizationData cd;
    for (size_t i = 0; i < SpaceRenderer::NUM_COLORS; ++i) {
        cd.colors[i] = renderer.colorScheme[i];
    }
    cd.thicknesses = {renderer.outlineThickness, renderer.curveThickness, renderer.pathThickness, renderer.mazeThickness};

    fs.write((char*) &cd, sizeof(cd));
    fs.close();
}
void SpaceGUI::on_loadFromFileButton_clicked() {
    string filename = QFileDialog::getOpenFileName(this, tr("Customization filename"), "", tr("Binary files (*.bin)")).toStdString();
    ifstream fs(filename, ios::binary | ios::in);

    CustomizationData cd;
    fs.read((char*) &cd, sizeof(cd));
    loadCD(cd);

    fs.close();
}
void SpaceGUI::loadCD(const CustomizationData& cd) {
    auto temp_cast = [](sf::Color c) { return QColor(c.r, c.g, c.b, c.a); };

    selectColor(ui->shapeColorFrame, renderer.colorScheme[SpaceRenderer::DEFAULTCOLOR], temp_cast(cd.colors[0]));
    selectColor(ui->firstColorFrame, renderer.colorScheme[SpaceRenderer::COLOR1], temp_cast(cd.colors[1]));
    selectColor(ui->secondColorFrame, renderer.colorScheme[SpaceRenderer::COLOR2], temp_cast(cd.colors[2]));
    selectColor(ui->thirdColorFrame, renderer.colorScheme[SpaceRenderer::COLOR3], temp_cast(cd.colors[3]));
    selectColor(ui->fourthColorFrame, renderer.colorScheme[SpaceRenderer::COLOR4], temp_cast(cd.colors[4]));
    selectColor(ui->outlineColorFrame, renderer.colorScheme[SpaceRenderer::OUTLINECOLOR], temp_cast(cd.colors[5]));
    selectColor(ui->curveColorFrame, renderer.colorScheme[SpaceRenderer::CURVECOLOR], temp_cast(cd.colors[6]));
    selectColor(ui->pathColorFrame, renderer.colorScheme[SpaceRenderer::PATHCOLOR], temp_cast(cd.colors[7]));
    selectColor(ui->mazeColorFrame, renderer.colorScheme[SpaceRenderer::MAZECOLOR], temp_cast(cd.colors[8]));
    selectColor(ui->backgroundColorFrame, renderer.colorScheme[SpaceRenderer::BACKGROUNDCOLOR], temp_cast(cd.colors[9]));
    selectColor(ui->pointsColorFrame, renderer.colorScheme[SpaceRenderer::POINTSCOLOR], temp_cast(cd.colors[10]));
    selectColor(ui->fifthColorFrame, renderer.colorScheme[SpaceRenderer::TEXTCOLOR], temp_cast(cd.colors[11]));

    ui->outlineThickness->setValue(cd.thicknesses[0]);
    ui->curveThickness->setValue(cd.thicknesses[1]);
    ui->pathThickness->setValue(cd.thicknesses[2]);
    ui->mazeThickness->setValue(cd.thicknesses[3]);
}
void SpaceGUI::on_mazeInfo_clicked() {
    static const std::array filenames = {"Nothing",           "RecursiveBacktracker", "Ellers",  "Kruskals",     "Prims",
                                         "RecursiveDevision", "Aldous-Broder",        "Wilsons", "Hunt-and-Kill"};
    QDesktopServices::openUrl(QUrl(QString("pdfs/") + QString(currentLanguage + QString("_") + filenames[ui->mazeAlg->currentIndex()]) + QString(".pdf")));
}
void SpaceGUI::on_pathInfo_clicked() {
    static const std::array filenames = {"BFS", "AStar"};
    QDesktopServices::openUrl(QUrl(QString("pdfs/") + QString(currentLanguage + QString("_") + filenames[ui->pathAlg->currentIndex()]) + QString(".pdf")));
}
void SpaceGUI::on_isSonification_checkStateChanged(const Qt::CheckState& arg1) {
    space.isSonification = !space.isSonification;
    (arg1) ? renderer.audioStream.play() : renderer.audioStream.stop();
}
void SpaceGUI::on_mazeSettings_clicked() {
    auto subWindow = new SettingsBuilder(this, mazeSets);
    subWindow->show();
    subWindow->raise();
}
void SpaceGUI::on_pathfindingSettings_clicked() {
    auto subWindow = new SettingsBuilder(this, pathSets);
    subWindow->show();
    subWindow->raise();
}
void SpaceGUI::on_visualizationSettings_clicked() {
    visSets.sonSets->mi_1.setter = [&](auto v) {
        *visSets.sonSets->mi_1.v = v;
        renderer.audioStream.stop();
        renderer.audioStream.play();
    };
    visSets.sonSets->mi_2.setter = [&](auto v) {
        *visSets.sonSets->mi_2.v = v;
        renderer.audioStream.stop();
        renderer.audioStream.play();
    };
    visSets.pathSets.mi_1.setter = [&](auto v) {
        *visSets.pathSets.mi_1.v = v;
        space.stepByStepPath = v && ui->isVisualization->isChecked();
    };
    auto subWindow = new SettingsBuilder(this, visSets);
    subWindow->show();
    subWindow->raise();
}
void SpaceGUI::on_antialiasingLevel_valueChanged(int arg1) {
    renderer.antialiasingLevel = arg1;
    renderer.loadSpaceProps(renderer.shapeSize, true);
}

// from https://github.com/esutton/qt-translation-example
void SpaceGUI::slotLanguageChanged(QAction* action) {
    if (0 == action) return;

    loadLanguage(action->data().toString());
    currentLanguage = action->data().toString();
    setWindowIcon(action->icon());
}
void SpaceGUI::changeEvent(QEvent* event) {
    if (0 != event) {
        switch (event->type()) {
        case QEvent::LanguageChange: ui->retranslateUi(this); break;
        case QEvent::LocaleChange: {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
            currentLanguage = locale;
            break;
        }
        }
    }
    QMainWindow::changeEvent(event);
}
void SpaceGUI::loadLanguage(const QString& rLanguage) {
    if (m_currLang == rLanguage) {
        return;
    }
    m_currLang = rLanguage;

    QLocale locale = QLocale(m_currLang);
    QLocale::setDefault(locale);
    QString languageName = QLocale::languageToString(locale.language());

    QString resourceFileName = QString("%1/Mazes_%2.qm").arg(m_langPath).arg(rLanguage);
    switchTranslator(m_translator, resourceFileName);
}
void SpaceGUI::createLanguageMenu(void) {
    QString defaultLocale = QLocale::system().name();       // e.g. "EN_US"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "en"
    m_langPath = ":/resource";
    m_langPath.append("/translation");
    QDir dir(m_langPath);

    QActionGroup* langGroup = new QActionGroup(ui->menuLanguage);
    langGroup->setExclusive(true);
    connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotLanguageChanged(QAction*)));

    QStringList fileNames = dir.entryList(QStringList("Mazes_*.qm"));
    for (int i = 0; i < fileNames.size(); ++i) {
        QString locale;
        locale = fileNames[i];

        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);

        QString lang = QLocale::languageToString(QLocale(locale).language());
        QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

        QAction* action = new QAction(ico, lang, this);
        action->setCheckable(true);
        action->setData(locale);

        ui->menuLanguage->addAction(action);
        langGroup->addAction(action);

        if (defaultLocale == locale) {
            action->setChecked(true);
        }
    }
}

void SpaceGUI::on_actionStart_Demo_triggered() { isDemo = true; }

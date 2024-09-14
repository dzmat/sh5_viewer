// #include <winnt.h>

#include <QString>
#include <QStringList>


#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mylog.h"

// ----------------------------------------------------------------------

#ifdef MDEBUG
const char *debugSavegameFilename = "C:/Users/dinozaur/Documents/SH5/data/cfg/SaveGames/00000002/Campaign-2020-08-30_1706/CampaignMission.mis";
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , saveGameFileName("")
{
    ui->setupUi(this);
    mylogger::logptr = ui->textEdit_Memo1;// very dangerous exporting, dirty and quick
    mylogger::log("Mainwindow constructor started");
    filter = std::make_shared<TFilter>();
    //    DecimalSeparator='.';

    // world setup
    installNewWorld(std::make_shared<TWorld>());
#ifdef MDEBUG
    load_world(debugSavegameFilename);
#endif
    // interface start setup
    ui->Image1->interceptRadius = 50;
    // ui->eradius_of_intercept->setText("50");
    QObject::connect(ui->Image1, &TmyQFrame::size_changed, this, &MainWindow::update_m);
    QObject::connect(ui->Image1, &TmyQFrame::scale_changed, this, &MainWindow::update_m);
    QObject::connect(ui->Image1, &TmyQFrame::changed_polar_coords, this, &MainWindow::update_polar_coords);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::load_world(const QString &fname)
{
    std::shared_ptr<TWorld>wp = std::make_shared<TWorld>();
    bool ok = wp->load_file(fname);
    if (ok) {
        saveGameFileName = fname;
        installNewWorld(wp);
        ui->Image1->worldChanged();
        mylogger::log("U-Boat params:");
        world->my_boat.dump(ui->textEdit_Memo1);
        ui->Image1->viewpoint.pos = world->my_boat.coord;
        ui->Image1->viewpoint.m = ui->e_coef->text().toDouble();
        update_m();
        ui->e_tmp->setText("File loaded successfully");
    }
    else {
        ui->e_tmp->setText("Failed to load world");
    }
}

// ---------------------------------------------------------------------------

void MainWindow::update_m()
{
    TmyQFrame *img = ui->Image1;
    double sq_size = img->viewpoint.m * img->width() / 1000.0;
    ui->e_sq_size->setText(QString("%1").arg(sq_size));
    ui->e_coef->setText(QString("%1").arg(img->viewpoint.m));
}

void MainWindow::update_polar_coords(double r, double head)
{
    ui->e_dist->setText(QString("%1").arg(r, 0, 'f', 3));
    ui->e_head->setText(QString("%1").arg(head, 5, 'f', 1, '0'));
}

// ---------------------------------------------------------------------------

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this
        , "Open Save Game"
#ifdef MDEBUG
        , debugSavegameFilename
#else
        , ""
#endif
        , "Save Game (*.mis)"
        );
    if (fileName.isEmpty()) return;
    ui->e_coords->setText(fileName);
    load_world(fileName);
}

void MainWindow::on_pbToBoat_clicked()
{
    ui->Image1->setScale(ui->e_coef->text().toDouble());
    ui->Image1->toBoat();
    ui->Image1->update();
}

void MainWindow::installNewWorld(std::shared_ptr<TWorld> iw)
{
    world = iw;
    ui->Image1->setWorld(world);
    filter->installNewWorld(iw);
}

void MainWindow::on_spinMinGroup_valueChanged(int )
{
    filter->set_min_group_size(ui->spinMinGroup->value());
    ui->Image1->update();
}

void MainWindow::on_chZeroSpeeds_stateChanged(int )
{
    filter->set_draw_zero_speed_only(ui->chZeroSpeeds->isChecked());
    ui->Image1->update();
}

void MainWindow::on_eradius_of_intercept_textChanged(const QString &)
{
    bool ok;
    QString txt = ui->eradius_of_intercept->text();
    int radius = txt.toInt(&ok);
    if (!ok) {
        if (txt.isEmpty())
            radius = 0;
        else
            return; // wrong value at input? do nothing.
    }
    ui->Image1->interceptRadius = radius;
    filter->set_radius(radius);
    ui->Image1->update();
}

void MainWindow::on_tbZoomIn_clicked()
{
    ui->Image1->viewpoint.m /= 2;
    update_m();
    ui->Image1->update();
}

void MainWindow::on_tbZoomOut_clicked()
{
    ui->Image1->viewpoint.m *= 2;
    update_m();
    ui->Image1->update();
}

void MainWindow::on_e_flt_type_of_ship_textChanged(const QString &arg1)
{
    bool ok;
    int test = arg1.toInt(&ok);
    if (!ok)
        filter->set_type(0);    // draw all types
    else
        filter->set_type(test);
    ui->Image1->update();
}

void MainWindow::on_e_coef_textChanged(const QString &arg1)
{
    bool ok;
    double val = arg1.toDouble(&ok);
    if (!ok) return;
    ui->Image1->viewpoint.m = val;
    ui->Image1->update();
    update_m();
}

void MainWindow::on_pbReloadFile_clicked()
{
    load_world(saveGameFileName);
}

void MainWindow::on_daysBeforeLineEdit_textChanged(const QString &arg1)
{
    bool ok;
    double test = arg1.toDouble(&ok);
    if (ok) {
        filter->set_days_before(test);    // update filter
        ui->Image1->update();
    }
}

void MainWindow::on_daysAfterLineEdit_textChanged(const QString &arg1)
{
    bool ok;
    double test = arg1.toDouble(&ok);
    if (ok) {
        filter->set_days_after(test);    // update filter
        ui->Image1->update();
    }
}

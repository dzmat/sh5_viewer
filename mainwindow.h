#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QtWidgets/QTextEdit>
#include <QMainWindow>
#include <QGraphicsScene>

#include "coords.h"
#include "support_classes.h"
#include "tworld.h"
#include "tmyqframe.h"

namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pbToBoat_clicked();

    void on_spinMinGroup_valueChanged(int arg1);

    void on_chZeroSpeeds_stateChanged(int arg1);

    void on_eradius_of_intercept_textChanged(const QString &arg1);


    void on_tbZoomIn_clicked();

    void on_tbZoomOut_clicked();

    void on_e_flt_type_of_ship_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<TWorld> world;



    void update_m();
    void redraw();
    void load_world(const QString& fname);

};

#endif // MAINWINDOW_H

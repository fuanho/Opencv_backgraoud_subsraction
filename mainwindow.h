#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStack>
#include <opencv2/opencv.hpp>
#include <Rectangle.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_open_file_clicked();

    void on_play_button_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

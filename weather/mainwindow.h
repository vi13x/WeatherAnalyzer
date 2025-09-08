#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr); // объявлен конструктор
    ~MainWindow();                                  // объявлен деструктор
};

#endif // MAINWINDOW_H

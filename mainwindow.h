#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addRecord();
    void analyzeData();
    void saveToJson();
    void loadFromJson();

private:
    QTableWidget *table;
    QPlainTextEdit *analysisText;

    QDateTimeEdit *dateTimeEdit;
    QDoubleSpinBox *radiationSpin;
    QDoubleSpinBox *temperatureSpin;
    QDoubleSpinBox *humiditySpin;
    QDoubleSpinBox *pressureSpin;
    QDoubleSpinBox *windSpin;
    QDoubleSpinBox *uvSpin;
};

#endif // MAINWINDOW_H

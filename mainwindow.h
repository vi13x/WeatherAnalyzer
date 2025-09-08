#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addRecord();
    void analyzeData();
    void saveToJson();
    void loadFromJson();

private:
    // Элементы интерфейса
    QComboBox *cityComboBox;
    QDateTimeEdit *dateTimeEdit;
    QDoubleSpinBox *radiationSpin;
    QDoubleSpinBox *temperatureSpin;
    QDoubleSpinBox *humiditySpin;
    QDoubleSpinBox *pressureSpin;
    QDoubleSpinBox *windSpin;
    QDoubleSpinBox *uvSpin;

    QTableWidget *table;
    QPlainTextEdit *analysisText;

    QPushButton *btnAdd;
    QPushButton *btnAnalyze;
    QPushButton *btnSave;
    QPushButton *btnLoad;

    // Метод для инициализации городов
    void initializeCities();
};

#endif // MAINWINDOW_H

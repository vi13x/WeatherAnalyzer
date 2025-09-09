#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

// Предварительные объявления для графиков
class QTabWidget;
class QChartView;
class QChart;
class QLineSeries;
class QValueAxis;
class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QTableWidget;
class QPlainTextEdit;
class QPushButton;

struct Coord { double lat; double lon; };

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
    void updateCharts();

private:
    void initializeCities();
    void setupCharts();
    void createTemperatureChart();
    void createRadiationChart();
    void createHumidityChart();
    void createWindChart();
    void createPressureChart();
    void createUVChart();


    // Указатели на UI элементы
    QTabWidget *tabWidget;
    QWidget *dataTab;
    QWidget *chartsTab;

    // Графики
    QChartView *temperatureChartView;
    QChartView *radiationChartView;
    QChartView *humidityChartView;
    QChartView *windChartView;
    QChartView *pressureChartView;
    QChartView *uvChartView;

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
    QPushButton *btnUpdateCharts;

    QMap<QString, Coord> cityCoords;
};

#endif // MAINWINDOW_H

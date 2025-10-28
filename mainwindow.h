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
class QDateTimeAxis;
class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QTableWidget;
class QPlainTextEdit;
class QListWidget;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QFormLayout;

struct Coord { double lat; double lon; };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addRecord();
    void saveToJson();
    void loadFromJson();
    void updateCharts();
    void analyzeData();
    void findMinMax();
    void computeTrend();
    void applySort();

private:
    void initializeCities();
    void setupCharts();
    void createRadiationChart();


    // Указатели на UI элементы
    QTabWidget *tabWidget;
    QWidget *dataTab;
    QWidget *chartsTab;


    // Графики
    QChartView *radiationChartView;

    QComboBox *cityComboBox;
    QDateTimeEdit *dateTimeEdit;
    QDoubleSpinBox *radiationSpin;
    QListWidget *cityOverlayList;
    QComboBox *chartTypeCombo;

    QTableWidget *table;
    QPlainTextEdit *analysisText;

    QPushButton *btnAdd;
    QPushButton *btnAnalyze;
    QPushButton *btnSave;
    QPushButton *btnLoad;
    QPushButton *btnUpdateCharts;
    QPushButton *btnThemeToggle;
    QPushButton *btnSelectAllCities;
    QPushButton *btnClearAllCities;
    QPushButton *btnFindMinMax;
    QPushButton *btnTrend;
    QComboBox *sortCombo;
    QPushButton *btnApplySort;

    QMap<QString, Coord> cityCoords;
    int seqCounter = 0;
};

#endif // MAINWINDOW_H

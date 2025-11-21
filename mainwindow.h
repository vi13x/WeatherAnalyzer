#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStyledItemDelegate>
#include <QMap>
#include <QVector>
#include <QColor>
#include <QDate>
#include <QSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>
// ✅ добавлено

QT_BEGIN_NAMESPACE
class QTabWidget;
class QChartView;
class QChart;
class QLineSeries;
class QSplineSeries;
class QScatterSeries;
class QValueAxis;
class QComboBox;
class QDateTimeEdit;
class QListWidget;
class QPushButton;
class QGroupBox;
class QFormLayout;
class QAbstractSeries;
class QLegendMarker;
QT_END_NAMESPACE

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

    QTabWidget *tabWidget = nullptr;
    QWidget *dataTab = nullptr;
    QWidget *chartsTab = nullptr;
    QChartView *radiationChartView = nullptr;

    QComboBox *cityComboBox = nullptr;
    QDateTimeEdit *dateTimeEdit = nullptr;

    QSpinBox *radiationSpin = nullptr;   // ✅ исправлено

    QListWidget *cityOverlayList = nullptr;
    QComboBox *chartTypeCombo = nullptr;
    QTableWidget *table = nullptr;
    QPlainTextEdit *analysisText = nullptr;

    QPushButton *btnAdd = nullptr;
    QPushButton *btnAnalyze = nullptr;
    QPushButton *btnSave = nullptr;
    QPushButton *btnLoad = nullptr;
    QPushButton *btnUpdateCharts = nullptr;
    QPushButton *btnSelectAllCities = nullptr;
    QPushButton *btnClearAllCities = nullptr;
    QPushButton *btnFindMinMax = nullptr;
    QPushButton *btnTrend = nullptr;
    QComboBox *sortCombo = nullptr;
    QPushButton *btnApplySort = nullptr;

    int seqCounter = 0;
};

#endif

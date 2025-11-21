#include "mainwindow.h"
#include <cfloat>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolTip>
#include <QCursor>
#include <QGroupBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QHeaderView>
#include <QDateTime>
#include <cmath>
#include <QLabel>
#include <QFrame>
#include <QStatusBar>
#include <QScrollArea>
#include <QTabWidget>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QPushButton>

#include <QChartView>
#include <QLineSeries>
#include <QSplineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QCategoryAxis>
#include <QBarSet>
#include <QBarSeries>
#include <QHorizontalBarSeries>
#include <QBarCategoryAxis>
#include <QCheckBox>
#include <QClipboard>
#include <QApplication>
#include <limits>
#include <algorithm>
#include <QDate>
#include <QLegendMarker>

using namespace Qt::StringLiterals;

static qint64 toMs(const QDate &d) {
    // безопасное создание QDateTime без функционального кастинга
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return QDateTime(d.startOfDay()).toMSecsSinceEpoch();
#else
    return QDateTime(d, QTime(0,0)).toMSecsSinceEpoch();
#endif
}
static qint64 toMs(const QDateTime &dt) {
    return dt.toMSecsSinceEpoch();
}
static QString fmtDate(qint64 ms) {
    return QDateTime::fromMSecsSinceEpoch(ms).date().toString("yyyy-MM-dd");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1400, 900);
    setWindowTitle(u"🌤️ Анализатор Ионизирующего излучения"_s);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    dataTab = new QWidget;
    tabWidget->addTab(dataTab, u"📋 Данные"_s);

    chartsTab = new QWidget;
    tabWidget->addTab(chartsTab, u"📊 Графики"_s);

    // ===== Глобальный стиль =====
    this->setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f3f6fb, stop:1 #ffffff);
        }
        QGroupBox {
            font-weight: 600;
            font-size: 12px;
            color: #1f2937;
            border: 1px solid #e2e8f0;
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 15px;
            background: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 2px 10px;
            background: #eef2f7;
            border-radius: 6px;
            color: #334155;
        }
        QFrame {
            background: #ffffff;
            border-radius: 12px;
            border: 1px solid #e5e7eb;
        }
        QLabel { color: #1f2937; }
    )");

    QHBoxLayout *mainDataLayout = new QHBoxLayout(dataTab);
    mainDataLayout->setSpacing(20);
    mainDataLayout->setContentsMargins(20, 20, 20, 20);

    QFrame *leftPanel = new QFrame;
    leftPanel->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #f7fafc);
            border-radius: 12px;
            border: 1px solid #e5e7eb;
        }
    )");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(15, 15, 15, 15);

    // ===== Ввод =====
    QGroupBox *inputGroup = new QGroupBox(u"📝 Ввод данных радиации"_s);
    QFormLayout *formLayout = new QFormLayout;
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setContentsMargins(10, 15, 10, 15);

    cityComboBox = new QComboBox;
    initializeCities();
    cityComboBox->setStyleSheet(R"(
        QComboBox {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background: white;
            font-size: 11px;
        }
        QComboBox:hover { border-color: #3498db; }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #bdc3c7;
        }
    )");
    formLayout->addRow(u"🏙️ Город:"_s, cityComboBox);

    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDisplayFormat("yyyy-MM-dd");
    dateTimeEdit->setStyleSheet(R"(
        QDateTimeEdit {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background: #ffffff;
            color: #2c3e50;
            font-size: 11px;
        }
        QDateTimeEdit:hover { border-color: #3498db; }
        QDateTimeEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #bdc3c7;
        }
        QDateTimeEdit QAbstractItemView {
            background-color: white;
            border: 1px solid #bdc3c7;
            selection-background-color: #3498db;
            selection-color: white;
        }
    )");
    formLayout->addRow(u"📅 Дата:"_s, dateTimeEdit);

    radiationSpin = new QSpinBox;
    radiationSpin->setRange(0, 1000);
    radiationSpin->setSuffix(u" мкР/ч"_s);
    radiationSpin->setStyleSheet(R"(
        QSpinBox {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background: white;
            font-size: 11px;
        }
        QSpinBox:hover { border-color: #3498db; }
    )");
    formLayout->addRow(u"☢️ Радиация (мкР/ч):"_s, radiationSpin);

    inputGroup->setLayout(formLayout);
    leftLayout->addWidget(inputGroup);

    // ===== Сортировка =====
    QGroupBox *sortGroup = new QGroupBox(u"🔀 Сортировка таблицы"_s);
    QHBoxLayout *sortLayout = new QHBoxLayout;
    sortCombo = new QComboBox;
    sortCombo->addItems({
        u"Город A→Я"_s,
        u"Город Я→A"_s,
        u"Дата: старые→новые"_s,
        u"Дата: новые→старые"_s,
        u"Радиация: больше→меньше"_s,
        u"Радиация: меньше→больше"_s
    });
    btnApplySort = new QPushButton(u"Применить"_s);
    connect(btnApplySort, &QPushButton::clicked, this, &MainWindow::applySort);
    sortLayout->addWidget(sortCombo);
    sortLayout->addWidget(btnApplySort);
    sortGroup->setLayout(sortLayout);
    leftLayout->addWidget(sortGroup);

    // ===== Кнопки =====
    btnAdd = new QPushButton(u"➕ Добавить запись"_s);
    btnSave = new QPushButton(u"💾 Сохранить JSON"_s);
    btnLoad = new QPushButton(u"📂 Загрузить JSON"_s);

    QString buttonBaseStyle = R"(
        QPushButton {
            padding: 12px;
            font-weight: bold;
            border-radius: 8px;
            font-size: 11px;
            border: none;
            margin: 2px;
        }
        QPushButton:hover { opacity: 0.9; transform: translateY(-1px); }
        QPushButton:pressed { transform: translateY(1px); }
    )";

    btnAdd->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #27ae60, stop: 1 #2ecc71);
            color: white;
        }
    )");
    btnSave->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #f39c12, stop: 1 #f1c40f);
            color: white;
        }
    )");
    btnLoad->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #8e44ad, stop: 1 #9b59b6);
            color: white;
        }
    )");

    connect(btnAdd,  &QPushButton::clicked, this, &MainWindow::addRecord);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromJson);

    leftLayout->addWidget(btnAdd);

    btnAnalyze = new QPushButton(u"📊 Анализировать данные"_s);
    btnAnalyze->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2980b9, stop: 1 #3498db);
            color: white;
        }
    )");
    connect(btnAnalyze, &QPushButton::clicked, this, &MainWindow::analyzeData);

    leftLayout->addWidget(btnAnalyze);
    leftLayout->addWidget(btnSave);
    leftLayout->addWidget(btnLoad);
    leftLayout->addStretch();

    // ===== Правая часть (таблица + анализ) =====
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(15);

    QGroupBox *tableGroup = new QGroupBox(u"📋 Таблица измерений ионизирующего излучения"_s);
    QVBoxLayout *tableLayout = new QVBoxLayout;

    table = new QTableWidget(0, 3);
    QStringList headers = {u"🏙️ Город"_s, u"📅 Дата"_s, u"☢️ Ионизирующее излучение (мкР/ч)"_s};
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            gridline-color: #dce1e5;
            font-size: 11px;
        }
        QTableWidget::item { padding: 6px; border-bottom: 1px solid #ecf0f1; }
        QTableWidget::item:selected { background-color: #3498db; color: white; }
        QHeaderView::section {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #34495e, stop: 1 #2c3e50);
            color: white; padding: 8px; border: 1px solid #2c3e50; font-weight: bold; font-size: 11px;
        }
        QTableWidget QScrollBar:vertical { border: none; background: #ecf0f1; width: 12px; margin: 0px; }
        QTableWidget QScrollBar::handle:vertical { background: #bdc3c7; border-radius: 6px; min-height: 20px; }
    )");

    tableLayout->addWidget(table);
    tableGroup->setLayout(tableLayout);
    rightLayout->addWidget(tableGroup, 7);

    // Анализ
    QGroupBox *analysisBox = new QGroupBox(u"📊 Результаты анализа"_s);
    QVBoxLayout *analysisLayout = new QVBoxLayout;
    analysisText = new QPlainTextEdit;
    analysisText->setReadOnly(true);
    analysisText->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #0b132b;
            color: #e0e1dd;
            border: 2px solid #1c2541;
            border-radius: 8px;
            padding: 12px;
            font-family: 'Consolas', 'Monospace';
            font-size: 12px;
        }
        QPlainTextEdit:focus { border-color: #3a506b; }
    )");
    analysisText->setPlaceholderText(u"📈 Нажмите \"Анализировать данные\" для расчётов по текущему городу..."_s);
    analysisLayout->addWidget(analysisText);
    analysisBox->setLayout(analysisLayout);
    rightLayout->addWidget(analysisBox, 3);

    mainDataLayout->addWidget(leftPanel, 3);
    mainDataLayout->addLayout(rightLayout, 7);

    // ===== Вкладка графиков =====
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsTab);
    chartsLayout->setSpacing(15);
    chartsLayout->setContentsMargins(20, 20, 20, 20);

    QHBoxLayout *chartsButtonLayout = new QHBoxLayout;
    btnUpdateCharts = new QPushButton(u"🔄 Обновить график"_s);
    btnUpdateCharts->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #e74c3c, stop: 1 #c0392b);
            color: white;
        }
    )");
    connect(btnUpdateCharts, &QPushButton::clicked, this, &MainWindow::updateCharts);
    chartsButtonLayout->addStretch();
    chartsButtonLayout->addWidget(btnUpdateCharts);
    chartsButtonLayout->addStretch();
    chartsLayout->addLayout(chartsButtonLayout);

    // Панель управления графиком
    QHBoxLayout *chartAndControls = new QHBoxLayout;
    QWidget *controlsPanel = new QWidget;
    controlsPanel->setFixedWidth(260);
    controlsPanel->setStyleSheet(R"(
        QWidget { background: #ffffff; border: 1px solid #e5e7eb; border-radius: 10px; }
        QLabel { color: #1f2937; font-weight: 600; }
        QComboBox { padding: 8px; border: 1px solid #cbd5e1; border-radius: 8px; background: #ffffff; color: #1f2937; }
        QComboBox:hover { border-color: #3b82f6; }
        QCheckBox { color: #1f2937; }
        QPushButton { padding: 9px 12px; border-radius: 8px; background: #3b82f6; color: #ffffff; font-weight: 700; }
        QPushButton:hover { background: #2563eb; }
    )");
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsPanel);
    QLabel *chartTypeLbl = new QLabel(u"Тип графика:"_s);
    chartTypeCombo = new QComboBox;
    chartTypeCombo->addItems({u"Точки+линии"_s, u"Сглаженная"_s});
    chartTypeCombo->setCurrentIndex(0);
    controlsLayout->addWidget(chartTypeLbl);
    controlsLayout->addWidget(chartTypeCombo);

    btnFindMinMax = new QPushButton(u"MIN/MAX"_s);
    controlsLayout->addWidget(btnFindMinMax);
    connect(btnFindMinMax, &QPushButton::clicked, this, &MainWindow::findMinMax);
    btnTrend = new QPushButton(u"Тенденция"_s);
    controlsLayout->addWidget(btnTrend);
    connect(btnTrend, &QPushButton::clicked, this, &MainWindow::computeTrend);
    controlsLayout->addStretch();
    chartAndControls->addWidget(controlsPanel);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea { border: none; background: transparent; }
    )");

    QWidget *chartsContainer = new QWidget;
    QVBoxLayout *chartsContainerLayout = new QVBoxLayout(chartsContainer);
    chartsContainerLayout->setSpacing(20);

    QLabel *radLabel = new QLabel(u"☢️ Ионизирующее излучение (мкР/ч)"_s);
    radLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(radLabel);

    radiationChartView = new QChartView;
    QSize chartSize(900, 420);
    radiationChartView->setMinimumSize(chartSize);
    radiationChartView->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *chartRow = new QHBoxLayout;
    chartRow->addLayout(chartAndControls);
    chartRow->addWidget(radiationChartView, 1);
    chartsContainerLayout->addLayout(chartRow);

    // Список городов для наложения
    QGroupBox *overlayGroup = new QGroupBox(u"🔀 Наложение графиков по городам"_s);
    QVBoxLayout *overlayLayout = new QVBoxLayout;
    cityOverlayList = new QListWidget;
    cityOverlayList->setSelectionMode(QAbstractItemView::NoSelection);
    cityOverlayList->setStyleSheet(R"(
        QListWidget { border: 2px solid #dfe6e9; border-radius: 8px; }
        QListWidget::item { padding: 6px; }
        QListWidget::indicator { width: 16px; height: 16px; }
    )");
    overlayLayout->addWidget(new QLabel(u"Выберите города для наложения:"_s));
    overlayLayout->addWidget(cityOverlayList);
    QHBoxLayout *overlayBtns = new QHBoxLayout;
    btnSelectAllCities = new QPushButton(u"Выбрать все"_s);
    btnClearAllCities = new QPushButton(u"Снять все"_s);
    overlayBtns->addWidget(btnSelectAllCities);
    overlayBtns->addWidget(btnClearAllCities);
    overlayBtns->addStretch();
    overlayGroup->setLayout(overlayLayout);
    overlayLayout->addLayout(overlayBtns);
    chartsContainerLayout->addWidget(overlayGroup);

    scrollArea->setWidget(chartsContainer);
    chartsLayout->addWidget(scrollArea);

    setupCharts();

    // чекбоксы городов
    for (int i = 0; i < cityComboBox->count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(cityComboBox->itemText(i));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        cityOverlayList->addItem(item);
    }
    connect(btnSelectAllCities, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < cityOverlayList->count(); ++i)
            cityOverlayList->item(i)->setCheckState(Qt::Checked);
    });
    connect(btnClearAllCities, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < cityOverlayList->count(); ++i)
            cityOverlayList->item(i)->setCheckState(Qt::Unchecked);
    });

    statusBar()->setStyleSheet(R"(
        QStatusBar {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2c3e50, stop: 1 #34495e);
            color: white;
            font-weight: bold;
            padding: 5px;
        }
    )");
    statusBar()->showMessage(u"✅ Готов к работе. Добавьте записи и постройте график."_s);
}

void MainWindow::initializeCities()
{

    cityComboBox->addItem(u"Минск"_s,  "Minsk");
    cityComboBox->addItem(u"Гомель"_s, "Gomel");
    cityComboBox->addItem(u"Могилёв"_s,"Mogilev");
    cityComboBox->addItem(u"Витебск"_s,"Vitebsk");
    cityComboBox->addItem(u"Гродно"_s, "Grodno");
    cityComboBox->addItem(u"Брест"_s,  "Brest");
    cityComboBox->addItem(u"Брагин"_s, "Bragin");
    cityComboBox->addItem(u"Славгород"_s,"Slavgorod");
}

MainWindow::~MainWindow() = default;

// ============================
// ДАННЫЕ
// ============================

void MainWindow::addRecord()
{
    const QString city = cityComboBox->currentText();
    if (city.isEmpty()) {
        QMessageBox::warning(this, u"Ошибка"_s, u"Пожалуйста, выберите город."_s);
        return;
    }

    const int row = table->rowCount();
    table->insertRow(row);

    const int rad = radiationSpin->value();
    auto *cityItem = new QTableWidgetItem(city);
    cityItem->setData(Qt::UserRole, seqCounter);
    table->setItem(row, 0, cityItem);

    auto *dtItem = new QTableWidgetItem(dateTimeEdit->dateTime().date().toString("yyyy-MM-dd"));
    table->setItem(row, 1, dtItem);

    auto *radItem = new QTableWidgetItem(QString::number(rad) + u" мкР/ч"_s);

    // Цвет по уровням, мкР/ч: <=15 зелёный, <=30 жёлтый, <=60 оранжевый, иначе красный
    QColor bgColor;
    if (rad <= 15)       bgColor = QColor("#d1fae5");
    else if (rad <= 30)  bgColor = QColor("#fef3c7");
    else if (rad <= 60)  bgColor = QColor("#ffedd5");
    else                 bgColor = QColor("#fee2e2");

    cityItem->setBackground(bgColor);
    dtItem->setBackground(bgColor);
    radItem->setBackground(bgColor);

    table->setItem(row, 2, radItem);
    seqCounter++;

    statusBar()->showMessage(QString(u"✅ Добавлена запись для города %1"_s).arg(city), 3000);
}

void MainWindow::analyzeData()
{
    const int rows = table->rowCount();
    if (rows == 0) {
        QMessageBox::information(this, u"Нет данных"_s, u"Сначала добавьте записи."_s);
        statusBar()->showMessage(u"Ошибка: нет данных для анализа"_s);
        return;
    }

    const QString currentCity = cityComboBox->currentText();

    QVector<int> rads; rads.reserve(rows);
    int cityRecordCount = 0;

    for (int i = 0; i < rows; ++i) {
        const auto *cityIt = table->item(i, 0);
        const auto *radIt  = table->item(i, 2);
        if (!cityIt || !radIt) continue;
        if (cityIt->text() == currentCity) {
            const int rad = radIt->text().section(' ', 0, 0).toInt();
            rads.append(rad);
            cityRecordCount++;
        }
    }

    if (cityRecordCount == 0) {
        QMessageBox::information(this, u"Нет данных"_s, QString(u"Нет записей для города %1"_s).arg(currentCity));
        statusBar()->showMessage(QString(u"Нет данных для города %1"_s).arg(currentCity));
        return;
    }

    auto mean = [](const QVector<int>& v)->double {
        if (v.isEmpty()) return 0.0;
        long long s = 0;
        for (int x : v) s += x;
        return double(s) / v.size();
    };
    auto vmax = [](const QVector<int>& v)->int { int m=v[0]; for (int x:v) if (x>m) m=x; return m; };
    auto vmin = [](const QVector<int>& v)->int { int m=v[0]; for (int x:v) if (x<m) m=x; return m; };
    auto stddev = [&](const QVector<int>& v)->double {
        if (v.isEmpty()) return 0.0;
        double m = mean(v);
        double s = 0;
        for (int x : v) s += (x - m) * (x - m);
        return std::sqrt(s / v.size());
    };

    QString result;
    result += QString(u"📊 АНАЛИЗ ИОНИЗИРУЮЩЕГО ИЗЛУЧЕНИЯ ДЛЯ %1\n"_s).arg(currentCity.toUpper());
    result += QString(u"═══════════════════════════════\n\n"_s);
    result += QString(u"🏙️  Город: %1\n"_s).arg(currentCity);
    result += QString(u"📈 Количество записей: %1\n\n"_s).arg(cityRecordCount);

    result += QString(u"☢️  ИОНИЗИРУЮЩЕЕ ИЗЛУЧЕНИЕ (мкР/ч):\n"_s);
    result += QString(u"   • Среднее: %1\n"_s).arg(mean(rads), 0, 'f', 2);
    result += QString(u"   • Минимальное: %1\n"_s).arg(vmin(rads));
    result += QString(u"   • Максимальное: %1\n"_s).arg(vmax(rads));
    result += QString(u"   • Стандартное отклонение: %1\n"_s).arg(stddev(rads), 0, 'f', 2);

    analysisText->setPlainText(result);
    statusBar()->showMessage(QString(u"Анализ завершен для города %1. Обработано %2 записей"_s)
                                 .arg(currentCity).arg(cityRecordCount), 5000);
}

void MainWindow::saveToJson()
{
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, u"Нет данных"_s, u"Таблица пуста. Нечего сохранять."_s);
        statusBar()->showMessage(u"Ошибка: нет данных для сохранения"_s);
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this, u"Сохранить данные"_s, "", u"JSON файлы (*.json)"_s);
    if (fileName.isEmpty()) return;

    QJsonArray records;
    for (int i = 0; i < table->rowCount(); i++) {
        QJsonObject obj;
        obj["city"_L1] = table->item(i, 0)->text();
        obj["datetime"_L1] = table->item(i, 1)->text();

        // только число до пробела
        const QString radText = table->item(i, 2)->text().section(' ', 0, 0);
        obj["radiation"_L1] = radText.toInt();

        records.append(obj);
    }

    QJsonDocument doc(records);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, u"Ошибка"_s, u"Не удалось открыть файл для записи."_s);
        statusBar()->showMessage(u"Ошибка сохранения файла"_s);
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, u"Успех"_s, QString(u"Данные сохранены в файл:\n%1"_s).arg(fileName));
    statusBar()->showMessage(QString(u"Данные сохранены в: %1"_s).arg(fileName), 5000);
}

void MainWindow::loadFromJson()
{
    const QString fileName = QFileDialog::getOpenFileName(this, u"Загрузить данные"_s, "", u"JSON файлы (*.json)"_s);
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, u"Ошибка"_s, u"Не удалось открыть файл."_s);
        statusBar()->showMessage(u"Ошибка открытия файла"_s);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        QMessageBox::warning(this, u"Ошибка"_s, u"Формат файла неверный. Ожидался массив JSON."_s);
        statusBar()->showMessage(u"Ошибка: неверный формат JSON"_s);
        return;
    }

    QJsonArray records = doc.array();
    table->setRowCount(0);

    for (auto v : records) {
        QJsonObject obj = v.toObject();
        const int row = table->rowCount();
        table->insertRow(row);

        const QString city = obj.value("city"_L1).toString();
        const int rad = obj.value("radiation"_L1).toInt();

        auto *cityItem = new QTableWidgetItem(city);
        cityItem->setData(Qt::UserRole, seqCounter);
        table->setItem(row, 0, cityItem);

        auto *dtItem = new QTableWidgetItem(obj.value("datetime"_L1).toString());
        table->setItem(row, 1, dtItem);

        auto *radItem = new QTableWidgetItem(QString::number(rad) + u" мкР/ч"_s);

        QColor bgColor;
        if (rad <= 15)       bgColor = QColor("#d1fae5");
        else if (rad <= 30)  bgColor = QColor("#fef3c7");
        else if (rad <= 60)  bgColor = QColor("#ffedd5");
        else                 bgColor = QColor("#fee2e2");

        cityItem->setBackground(bgColor);
        dtItem->setBackground(bgColor);
        radItem->setBackground(bgColor);

        table->setItem(row, 2, radItem);
        seqCounter++;
    }

    QMessageBox::information(this, u"Успех"_s, QString(u"Загружено %1 записей из файла:\n%2"_s).arg(records.size()).arg(fileName));
    statusBar()->showMessage(QString(u"Загружено %1 записей из %2"_s).arg(records.size()).arg(fileName), 5000);
}

// ============================
// ЧАРТЫ
// ============================

void MainWindow::setupCharts()
{
    createRadiationChart();
}

void MainWindow::createRadiationChart()
{
    QChart *chart = new QChart();
    chart->setTitle(u"☢️ Радиация по городам"_s);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QFont legendFont; legendFont.setPointSize(10); legendFont.setBold(true);
    chart->legend()->setFont(legendFont);
    chart->setBackgroundBrush(QBrush(QColor(255,255,255)));
    chart->setPlotAreaBackgroundVisible(true);
    QLinearGradient bg(0,0,0,1); bg.setCoordinateMode(QGradient::ObjectBoundingMode);
    bg.setColorAt(0.0, QColor(248,250,252));
    bg.setColorAt(1.0, QColor(238,242,247));
    chart->setPlotAreaBackgroundBrush(bg);

    // Ось X — ДАТЫ
    auto *axisX = new QDateTimeAxis;
    axisX->setTitleText(u"Дата"_s);
    axisX->setFormat("yyyy-MM");
    axisX->setTickCount(8);
    axisX->setLabelsAngle(-45);
    QFont axisFont; axisFont.setPointSize(9);
    axisX->setLabelsFont(axisFont);


    // Ось Y
    auto *axisY = new QValueAxis;
    axisY->setTitleText(u"мкР/ч"_s);
    axisY->setLabelFormat("%.0f");
    axisY->setTickCount(7);
    axisY->setMinorTickCount(1);
    axisY->setGridLineColor(QColor("#dfe6e9"));
    axisY->setGridLinePen(QPen(QColor("#dfe6e9"), 1, Qt::SolidLine));
    axisY->setMinorGridLineColor(QColor("#ecf0f1"));
    axisY->setMinorGridLinePen(QPen(QColor("#ecf0f1"), 1, Qt::DashLine));
    axisY->setLabelsFont(axisFont);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    radiationChartView->setChart(chart);
    radiationChartView->setRubberBand(QChartView::RectangleRubberBand);
}

static void hideScatterLegend(QChart *chart, QScatterSeries *scatter) {
    if (!chart || !scatter || !chart->legend()) return;
    const auto markers = chart->legend()->markers(scatter);
    for (QLegendMarker *m : markers) if (m) m->setVisible(false);
}

void MainWindow::updateCharts()
{
    QList<QString> selectedCities;
    for (int i = 0; i < cityOverlayList->count(); ++i) {
        if (cityOverlayList->item(i)->checkState() == Qt::Checked)
            selectedCities.append(cityOverlayList->item(i)->text());
    }
    if (selectedCities.isEmpty())
        selectedCities = { cityComboBox->currentText() };

    QChart *chart = radiationChartView->chart();
    chart->removeAllSeries();

    QDateTimeAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    for (auto *ax : chart->axes(Qt::Horizontal))
        axisX = qobject_cast<QDateTimeAxis*>(ax);

    if (!axisX) {
        axisX = new QDateTimeAxis;
        axisX->setFormat("MMM yyyy");
        axisX->setTitleText("Дата");
        axisX->setLabelsAngle(-30);
        chart->addAxis(axisX, Qt::AlignBottom);
    }

    for (auto *ay : chart->axes(Qt::Vertical))
        axisY = qobject_cast<QValueAxis*>(ay);

    if (!axisY) {
        axisY = new QValueAxis;
        axisY->setTitleText("мкР/ч");
        axisY->setLabelFormat("%.0f");
        chart->addAxis(axisY, Qt::AlignLeft);
    }

    qint64 minTs = LLONG_MAX;
    qint64 maxTs = LLONG_MIN;
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    const QList<QColor> palette = {
        QColor("#2563eb"), QColor("#10b981"), QColor("#f59e0b"),
        QColor("#8b5cf6"), QColor("#ef4444"), QColor("#14b8a6"),
        QColor("#f97316")
    };

    int colorIndex = 0;
    bool useSpline = (chartTypeCombo && chartTypeCombo->currentText().startsWith("Сглаж"));

    for (const QString &city : selectedCities) {
        QColor color = palette[colorIndex % palette.size()];
        QPen pen(color);
        pen.setWidth(3);
        pen.setCosmetic(true);

        QXYSeries *curve = nullptr;
        if (useSpline) {
            auto *s = new QSplineSeries();
            s->setName(city);
            s->setPen(pen);
            s->setPointsVisible(true);
            curve = s;
        } else {
            auto *l = new QLineSeries();
            l->setName(city);
            l->setPen(pen);
            curve = l;
        }

        auto *scatter = new QScatterSeries();
        scatter->setMarkerSize(8);
        scatter->setColor(color);

        QVector<std::pair<qint64, int>> pts;
        pts.reserve(table->rowCount());

        for (int r = 0; r < table->rowCount(); r++) {
            if (!table->item(r,0) || !table->item(r,1) || !table->item(r,2)) continue;

            if (table->item(r,0)->text() == city) {
                QString ds = table->item(r,1)->text();
                QDate d = QDate::fromString(ds, "yyyy-MM-dd");
                if (!d.isValid()) continue;

                qint64 ts = QDateTime(d, QTime(0,0)).toMSecsSinceEpoch();
                int rad = table->item(r,2)->text().section(' ', 0, 0).toInt();
                pts.push_back({ts, rad});
            }
        }

        if (pts.isEmpty()) {
            delete scatter;
            delete curve;
            colorIndex++;
            continue;
        }

        std::sort(pts.begin(), pts.end(),
                  [](auto &a, auto &b){ return a.first < b.first; });

        for (auto &p : pts) {
            curve->append(p.first, p.second);
            scatter->append(p.first, p.second);

            minTs = std::min(minTs, p.first);
            maxTs = std::max(maxTs, p.first);
            minY = std::min(minY, double(p.second));
            maxY = std::max(maxY, double(p.second));
        }

        chart->addSeries(curve);
        chart->addSeries(scatter);
        curve->attachAxis(axisX);
        curve->attachAxis(axisY);
        scatter->attachAxis(axisX);
        scatter->attachAxis(axisY);

        // ✅ КРАСИВЫЙ ТУЛТИП (скруглённые углы, тень, не обрезает текст)
        auto *tipText = new QGraphicsTextItem();
        QFont f; f.setPointSize(10);
        tipText->setFont(f);
        tipText->setDefaultTextColor(Qt::black);
        tipText->setZValue(3001);
        tipText->setVisible(false);
        radiationChartView->scene()->addItem(tipText);

        auto *tipBg = new QGraphicsPathItem();
        tipBg->setBrush(QColor(255,255,255));
        tipBg->setPen(QPen(Qt::black, 1));
        tipBg->setZValue(3000);
        tipBg->setVisible(false);
        radiationChartView->scene()->addItem(tipBg);

        QObject::connect(
            scatter, &QScatterSeries::hovered,
            this,
            [this, tipText, tipBg, city](const QPointF &p, bool state)
            {
                if (state) {
                    QString text = QString("<b>%1</b><br>Дата: %2<br>Радиация: %3 мкР/ч")
                                       .arg(city)
                                       .arg(QDateTime::fromMSecsSinceEpoch(qint64(p.x())).toString("dd.MM.yyyy"))
                                       .arg(int(std::lround(p.y())));

                    tipText->setHtml(text);

                    QPointF pos = radiationChartView->chart()->mapToPosition(p);

                    // позиция тултипа
                    QPointF tipPos = pos + QPointF(12, -10);
                    tipText->setPos(tipPos);

                    QRectF r = tipText->boundingRect();
                    r.adjust(-6, -6, 6, 6);

                    // скругленный фон
                    QPainterPath path;
                    path.addRoundedRect(r, 6, 6);
                    tipBg->setPath(path);
                    tipBg->setPos(tipPos);

                    // делаем видимым
                    tipBg->setVisible(true);
                    tipText->setVisible(true);
                }
                else {
                    tipBg->setVisible(false);
                    tipText->setVisible(false);
                }
            }
            );

        colorIndex++;
    }

    if (minTs == LLONG_MAX) {
        QMessageBox::information(this, "Нет данных", "Нет данных для выбранных городов");
        return;
    }

    double pad = (maxY - minY) * 0.15;
    if (pad <= 0) pad = 1.0;
    axisY->setRange(std::max(0.0, minY - pad), maxY + pad);

    axisX->setMin(QDateTime::fromMSecsSinceEpoch(minTs));
    axisX->setMax(QDateTime::fromMSecsSinceEpoch(maxTs));

    QList<QDateTime> ticks;
    QDateTime cur = QDateTime::fromMSecsSinceEpoch(minTs);
    QDateTime end = QDateTime::fromMSecsSinceEpoch(maxTs);

    while (cur <= end) {
        ticks.append(cur);
        cur = cur.addMonths(2);
    }

    axisX->setTickCount(ticks.size());
    axisX->setFormat("MMM yyyy");
    axisX->setLabelsAngle(-30);

    chart->setTitle(
        QString("Ионизирующее излучение за период %1 — %2")
            .arg(QDateTime::fromMSecsSinceEpoch(minTs).toString("dd.MM.yyyy"))
            .arg(QDateTime::fromMSecsSinceEpoch(maxTs).toString("dd.MM.yyyy"))
        );

    statusBar()->showMessage("✅ График обновлен", 2000);
}





void MainWindow::findMinMax()
{
    QChart *chart = radiationChartView->chart();
    if (!chart) return;

    // Если линии уже существуют — удаляем их (режим переключения)
    QList<QAbstractSeries*> toRemove;
    for (auto *s : chart->series()) {
        if (s->name() == "MIN линия" || s->name() == "MAX линия")
            toRemove.append(s);
    }
    if (!toRemove.isEmpty()) {
        for (auto *s : toRemove) {
            chart->removeSeries(s);
            s->deleteLater();
        }
        statusBar()->showMessage("Линии MIN/MAX удалены", 2000);
        return;
    }

    // --- Находим минимум и максимум ---
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    QDateTimeAxis *axisX = nullptr;
    QValueAxis *axisY  = nullptr;

    // Ищем нужные оси
    for (auto *ax : chart->axes(Qt::Horizontal))
        axisX = qobject_cast<QDateTimeAxis*>(ax);

    for (auto *ay : chart->axes(Qt::Vertical))
        axisY = qobject_cast<QValueAxis*>(ay);

    if (!axisX || !axisY) {
        QMessageBox::warning(this, "Ошибка", "Оси графика не найдены");
        return;
    }

    // Обход всех серий, включая line + spline
    for (auto *s : chart->series()) {
        if (auto xy = qobject_cast<QXYSeries*>(s)) {
            for (const QPointF &p : xy->points()) {
                minY = std::min(minY, p.y());
                maxY = std::max(maxY, p.y());
            }
        }
    }

    if (!std::isfinite(minY)) {
        QMessageBox::information(this, "MIN/MAX", "На графике нет точек");
        return;
    }

    // --- рисуем горизонтальные линии ---
    auto drawLine = [&](double y, const QColor &color, const QString &name) {
        QLineSeries *line = new QLineSeries();
        line->setName(name);

        QPen pen(color);
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        pen.setCosmetic(true);
        line->setPen(pen);

        qint64 xMin = axisX->min().toMSecsSinceEpoch();
        qint64 xMax = axisX->max().toMSecsSinceEpoch();

        line->append(xMin, y);
        line->append(xMax, y);

        chart->addSeries(line);
        line->attachAxis(axisX);
        line->attachAxis(axisY);
    };

    drawLine(minY, QColor("#27ae60"), "MIN линия");
    drawLine(maxY, QColor("#c0392b"), "MAX линия");

    statusBar()->showMessage("Показаны MIN/MAX уровни", 2000);
}


void MainWindow::computeTrend()
{
    if (!radiationChartView || !radiationChartView->chart()) return;
    QChart *chart = radiationChartView->chart();

    // toggle: если уже есть — удалить
    QList<QAbstractSeries*> toRemove;
    for (QAbstractSeries *s : chart->series()) {
        if (s->name() == u"Тенденция"_s) toRemove.append(s);
    }
    if (!toRemove.isEmpty()) {
        for (QAbstractSeries *s : toRemove) { chart->removeSeries(s); s->deleteLater(); }
        return;
    }

    // собираем точки только из линий/сплайнов (scatter не нужен)
    QVector<QPointF> pts;
    for (QAbstractSeries *s : chart->series()) {
        if (s->name() == u"MIN линия"_s || s->name() == u"MAX линия"_s) continue;
        if (auto xy = qobject_cast<QXYSeries*>(s)) {
            const auto v = xy->pointsVector();
            pts += v;
        }
    }
    if (pts.size() < 2) {
        QMessageBox::information(this, u"Тенденция"_s, u"Недостаточно точек для расчёта тенденции."_s);
        return;
    }

    // регрессия y = a*x + b
    double sx = 0, sy = 0, sxx = 0, sxy = 0; const int n = pts.size();
    for (const QPointF &p : pts) {
        sx  += p.x();
        sy  += p.y();
        sxx += p.x()*p.x();
        sxy += p.x()*p.y();
    }
    const double denom = (n * sxx - sx * sx);
    if (denom == 0) return;
    const double a = (n * sxy - sx * sy) / denom;
    const double b = (sy - a * sx) / n;

    // диапазон X (поддержка QDateTimeAxis и QValueAxis)
    double xMin = 0.0, xMax = 0.0;
    if (auto *axisXdt = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).value(0))) {
        xMin = (double)toMs(axisXdt->min());
        xMax = (double)toMs(axisXdt->max());
    } else if (auto *axisXval = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).value(0))) {
        xMin = axisXval->min();
        xMax = axisXval->max();
    } else {
        return;
    }

    auto *trend = new QLineSeries();
    trend->setName(u"Тенденция"_s);
    QPen pen(QColor("#111827")); pen.setWidth(2); pen.setStyle(Qt::DotLine); pen.setCosmetic(true); trend->setPen(pen);
    trend->append(xMin, a * xMin + b);
    trend->append(xMax, a * xMax + b);
    chart->addSeries(trend);
    if (auto *ax = chart->axes(Qt::Horizontal).value(0)) trend->attachAxis(ax);
    if (auto *ay = chart->axes(Qt::Vertical).value(0))   trend->attachAxis(ay);
}

void MainWindow::applySort()
{
    if (!table) return;

    struct Row { QString city; QDate dt; int rad; int originalSeq; };
    QVector<Row> rows; rows.reserve(table->rowCount());

    for (int i = 0; i < table->rowCount(); ++i) {
        QTableWidgetItem *cityIt = table->item(i,0);
        QTableWidgetItem *dtIt   = table->item(i,1);
        QTableWidgetItem *radIt  = table->item(i,2);
        if (!cityIt || !dtIt || !radIt) continue;

        const int rad = radIt->text().section(' ', 0, 0).toInt();
        Row r{ cityIt->text(), QDate::fromString(dtIt->text(), "yyyy-MM-dd"), rad, cityIt->data(Qt::UserRole).toInt() };
        rows.push_back(r);
    }

    const QString mode = sortCombo ? sortCombo->currentText() : QString();
    auto byCityAsc = [](const Row &a, const Row &b){ return a.city.localeAwareCompare(b.city) < 0; };
    auto byCityDesc = [](const Row &a, const Row &b){ return a.city.localeAwareCompare(b.city) > 0; };
    auto byOldNew = [](const Row &a, const Row &b){ return a.dt < b.dt; };
    auto byNewOld = [](const Row &a, const Row &b){ return a.dt > b.dt; };
    auto byRadDesc = [](const Row &a, const Row &b){ return a.rad > b.rad; };
    auto byRadAsc  = [](const Row &a, const Row &b){ return a.rad < b.rad; };

    if (mode.startsWith(u"Город A"_s))             std::sort(rows.begin(), rows.end(), byCityAsc);
    else if (mode.startsWith(u"Город Я"_s))        std::sort(rows.begin(), rows.end(), byCityDesc);
    else if (mode.startsWith(u"Дата: старые"_s))   std::sort(rows.begin(), rows.end(), byOldNew);
    else if (mode.startsWith(u"Дата: новые"_s))    std::sort(rows.begin(), rows.end(), byNewOld);
    else if (mode.startsWith(u"Радиация: больше"_s)) std::sort(rows.begin(), rows.end(), byRadDesc);
    else if (mode.startsWith(u"Радиация: меньше"_s)) std::sort(rows.begin(), rows.end(), byRadAsc);

    table->setRowCount(0);
    for (const Row &r : rows) {
        const int row = table->rowCount(); table->insertRow(row);
        auto *cityItem = new QTableWidgetItem(r.city);
        cityItem->setData(Qt::UserRole, r.originalSeq);
        table->setItem(row, 0, cityItem);
        auto *dtItem = new QTableWidgetItem(r.dt.toString("yyyy-MM-dd"));
        table->setItem(row, 1, dtItem);

        auto *radItem = new QTableWidgetItem(QString::number(r.rad) + u" мкР/ч"_s);

        QColor bgColor;
        if (r.rad <= 15)       bgColor = QColor("#d1fae5");
        else if (r.rad <= 30)  bgColor = QColor("#fef3c7");
        else if (r.rad <= 60)  bgColor = QColor("#ffedd5");
        else                   bgColor = QColor("#fee2e2");

        cityItem->setBackground(bgColor);
        dtItem->setBackground(bgColor);
        radItem->setBackground(bgColor);

        table->setItem(row, 2, radItem);
    }
}

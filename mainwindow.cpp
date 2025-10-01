#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QDoubleSpinBox>
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1400, 900);
    setWindowTitle("🌤️ Анализатор погодных данных");

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    dataTab = new QWidget;
    tabWidget->addTab(dataTab, "📋 Данные");

    chartsTab = new QWidget;
    tabWidget->addTab(chartsTab, "☢️ Радиация");


    // Бот-вкладка удалена — фокус только на данных и графике радиации

    this->setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #f8f9fa, stop: 1 #e9ecef);
        }
        QGroupBox {
            font-weight: bold;
            font-size: 12px;
            color: #2c3e50;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 10px;
            background-color: #ecf0f1;
            border-radius: 4px;
        }
        QFrame {
            background-color: #ffffff;
            border-radius: 10px;
            border: 2px solid #dfe6e9;
        }
    )");

    QHBoxLayout *mainDataLayout = new QHBoxLayout(dataTab);
    mainDataLayout->setSpacing(20);
    mainDataLayout->setContentsMargins(20, 20, 20, 20);


    QFrame *leftPanel = new QFrame;
    leftPanel->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #ffffff, stop: 1 #f8f9fa);
            border-radius: 12px;
            border: 2px solid #e0e6ed;
        }
    )");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(15, 15, 15, 15);


    QGroupBox *inputGroup = new QGroupBox("📝 Ввод данных радиации");

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
        QComboBox:hover {
            border-color: #3498db;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #bdc3c7;
        }
    )");
    formLayout->addRow("🏙️ Город:", cityComboBox);

    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
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
        QAbstractSpinBox { background: #ffffff; color: #2c3e50; }
        QCalendarWidget QWidget { background-color: #ffffff; color: #2c3e50; }
        QCalendarWidget QToolButton { color: #2c3e50; background: #ecf0f1; border: none; border-radius: 4px; padding: 4px; }
        QCalendarWidget QMenu { background: #ffffff; color: #2c3e50; }
        QCalendarWidget QSpinBox { background: #ffffff; color: #2c3e50; }
        QCalendarWidget QAbstractItemView:enabled { background: #ffffff; color: #2c3e50; selection-background-color: #3498db; selection-color: #ffffff; }
    )");
    formLayout->addRow("🕐 Дата/время:", dateTimeEdit);

    radiationSpin = new QDoubleSpinBox;
    radiationSpin->setRange(0, 20000);
    radiationSpin->setDecimals(2);
    radiationSpin->setSuffix("мкР/ч");
    radiationSpin->setStyleSheet(R"(
        QDoubleSpinBox {
            padding: 8px;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background: white;
            font-size: 11px;
        }
        QDoubleSpinBox:hover {
            border-color: #3498db;
        }
    )");
    formLayout->addRow("☢️ Радиация:", radiationSpin);

    inputGroup->setLayout(formLayout);
    leftLayout->addWidget(inputGroup);


    btnAdd = new QPushButton("➕ Добавить запись");
    btnSave = new QPushButton("💾 Сохранить JSON");
    btnLoad = new QPushButton("📂 Загрузить JSON");


    QString buttonBaseStyle = R"(
        QPushButton {
            padding: 12px;
            font-weight: bold;
            border-radius: 8px;
            font-size: 11px;
            border: none;
            margin: 2px;
        }
        QPushButton:hover {
            opacity: 0.9;
            transform: translateY(-1px);
        }
        QPushButton:pressed {
            transform: translateY(1px);
        }
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

    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::addRecord);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromJson);

    leftLayout->addWidget(btnAdd);
    btnAnalyze = new QPushButton("📊 Анализировать данные");
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


    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(15);


    QGroupBox *tableGroup = new QGroupBox("📋 Таблица измерений радиации");

    QVBoxLayout *tableLayout = new QVBoxLayout;

    table = new QTableWidget(0, 3);
    QStringList headers = {"🏙️ Город", "🕐 Дата/время", "☢️ Радиация"};
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
        QTableWidget::item {
            padding: 6px;
            border-bottom: 1px solid #ecf0f1;
        }
        QTableWidget::item:selected {
            background-color: #3498db;
            color: white;
        }
        QHeaderView::section {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #34495e, stop: 1 #2c3e50);
            color: white;
            padding: 8px;
            border: 1px solid #2c3e50;
            font-weight: bold;
            font-size: 11px;
        }
        QTableWidget QScrollBar:vertical {
            border: none;
            background: #ecf0f1;
            width: 12px;
            margin: 0px;
        }
        QTableWidget QScrollBar::handle:vertical {
            background: #bdc3c7;
            border-radius: 6px;
            min-height: 20px;
        }
    )");

    tableLayout->addWidget(table);
    tableGroup->setLayout(tableLayout);
    rightLayout->addWidget(tableGroup, 7);


    // Блок анализа восстановлен
    QGroupBox *analysisBox = new QGroupBox("📊 Результаты анализа");
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
    analysisText->setPlaceholderText("📈 Нажмите \"Анализировать данные\" для расчётов по текущему городу...");

    analysisLayout->addWidget(analysisText);
    analysisBox->setLayout(analysisLayout);
    rightLayout->addWidget(analysisBox, 3);


    mainDataLayout->addWidget(leftPanel, 3);
    mainDataLayout->addLayout(rightLayout, 7);


    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsTab);
    chartsLayout->setSpacing(15);
    chartsLayout->setContentsMargins(20, 20, 20, 20);


    QHBoxLayout *chartsButtonLayout = new QHBoxLayout;
    btnUpdateCharts = new QPushButton("🔄 Обновить график");
    btnUpdateCharts->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #e74c3c, stop: 1 #c0392b);
            color: white;
        }
    )");
    connect(btnUpdateCharts, &QPushButton::clicked, this, &MainWindow::updateCharts);

    btnThemeToggle = new QPushButton("⚫/⚪ Тема");
    btnThemeToggle->setStyleSheet(R"(
        QPushButton { padding: 8px 12px; border-radius: 6px; background: #2c3e50; color: #ffffff; font-weight: bold; }
        QPushButton:hover { background: #1f2d3a; }
    )");
    connect(btnThemeToggle, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    chartsButtonLayout->addWidget(btnThemeToggle);
    chartsButtonLayout->addStretch();
    chartsButtonLayout->addWidget(btnUpdateCharts);
    chartsButtonLayout->addStretch();
    chartsLayout->addLayout(chartsButtonLayout);

    // Боковая панель управления типом графика и линиями MIN/MAX
    QHBoxLayout *chartAndControls = new QHBoxLayout;
    QWidget *controlsPanel = new QWidget;
    controlsPanel->setFixedWidth(260);
    controlsPanel->setStyleSheet(R"(
        QWidget { background: #ffffff; border: 2px solid #e0e6ed; border-radius: 8px; }
    )");
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsPanel);
    QLabel *chartTypeLbl = new QLabel("Тип графика:");
    chartTypeCombo = new QComboBox;
    chartTypeCombo->addItems({"Точки+линии", "Сглаженная"});
    chartTypeCombo->setCurrentIndex(0);
    controlsLayout->addWidget(chartTypeLbl);
    controlsLayout->addWidget(chartTypeCombo);

    showMinMaxCheck = new QCheckBox("Показывать линии MIN/MAX");
    controlsLayout->addWidget(showMinMaxCheck);
    btnFindMinMax = new QPushButton("Найти MIN/MAX и добавить линии");
    controlsLayout->addWidget(btnFindMinMax);
    connect(btnFindMinMax, &QPushButton::clicked, this, &MainWindow::findMinMax);

    controlsLayout->addStretch();

    chartAndControls->addWidget(controlsPanel);


    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: none;
            background: transparent;
        }
    )");

    QWidget *chartsContainer = new QWidget;
    QVBoxLayout *chartsContainerLayout = new QVBoxLayout(chartsContainer);
    chartsContainerLayout->setSpacing(20);

    QLabel *radLabel = new QLabel("☢️ Радиация (мкР/ч)");
    radLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(radLabel);

    radiationChartView = new QChartView;
    QSize chartSize(900, 420);
    radiationChartView->setMinimumSize(chartSize);
    radiationChartView->setRenderHint(QPainter::Antialiasing);
    // Вставляем график вместе с боковой панелью управления
    QHBoxLayout *chartRow = new QHBoxLayout;
    chartRow->addLayout(chartAndControls);
    chartRow->addWidget(radiationChartView, 1);
    chartsContainerLayout->addLayout(chartRow);

    // Список городов для наложения серий (чекбокс-лист)
    QGroupBox *overlayGroup = new QGroupBox("🔀 Наложение графиков по городам");
    QVBoxLayout *overlayLayout = new QVBoxLayout;
    cityOverlayList = new QListWidget;
    cityOverlayList->setSelectionMode(QAbstractItemView::NoSelection);
    cityOverlayList->setStyleSheet(R"(
        QListWidget { border: 2px solid #dfe6e9; border-radius: 8px; }
        QListWidget::item { padding: 6px; }
        QListWidget::indicator { width: 16px; height: 16px; }
    )");
    overlayLayout->addWidget(new QLabel("Выберите города для наложения:"));
    overlayLayout->addWidget(cityOverlayList);
    QHBoxLayout *overlayBtns = new QHBoxLayout;
    btnSelectAllCities = new QPushButton("Выбрать все");
    btnClearAllCities = new QPushButton("Снять все");
    overlayBtns->addWidget(btnSelectAllCities);
    overlayBtns->addWidget(btnClearAllCities);
    overlayBtns->addStretch();
    overlayGroup->setLayout(overlayLayout);
    overlayLayout->addLayout(overlayBtns);
    chartsContainerLayout->addWidget(overlayGroup);

    scrollArea->setWidget(chartsContainer);
    chartsLayout->addWidget(scrollArea);


    setupCharts();

    // Заполнение списка городов для наложения (чекбоксы)
    for (int i = 0; i < cityComboBox->count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(cityComboBox->itemText(i));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        cityOverlayList->addItem(item);
    }
    connect(btnSelectAllCities, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < cityOverlayList->count(); ++i) {
            cityOverlayList->item(i)->setCheckState(Qt::Checked);
        }
    });
    connect(btnClearAllCities, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < cityOverlayList->count(); ++i) {
            cityOverlayList->item(i)->setCheckState(Qt::Unchecked);
        }
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
    statusBar()->showMessage("✅ Готов к работе. Добавьте записи и постройте график.");
}


void MainWindow::initializeCities()
{
    cityComboBox->addItem("Минск", "Minsk");
    cityComboBox->addItem("Гомель", "Gomel");
    cityComboBox->addItem("Могилёв", "Mogilev");
    cityComboBox->addItem("Витебск", "Vitebsk");
    cityComboBox->addItem("Гродно", "Grodno");
    cityComboBox->addItem("Брест", "Brest");
    cityComboBox->addItem("Барановичи", "Baranovichi");
    cityComboBox->addItem("Борисов", "Borisov");
    cityComboBox->addItem("Пинск", "Pinsk");
    cityComboBox->addItem("Орша", "Orsha");
    cityComboBox->addItem("Мозырь", "Mozyr");
    cityComboBox->addItem("Солигорск", "Soligorsk");
    cityComboBox->addItem("Новополоцк", "Novopolotsk");
    cityComboBox->addItem("Лида", "Lida");
    cityComboBox->addItem("Молодечно", "Molodechno");
    cityComboBox->addItem("Полоцк", "Polotsk");
    cityComboBox->addItem("Жлобин", "Zhlobin");
    cityComboBox->addItem("Светлогорск", "Svetlogorsk");
    cityComboBox->addItem("Речица", "Rechitsa");
    cityComboBox->addItem("Жодино", "Zhodino");
}


MainWindow::~MainWindow() = default;

void MainWindow::addRecord()
{
    QString city = cityComboBox->currentText();
    if (city.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите город.");
        return;
    }

    int row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(city));
    table->setItem(row, 1, new QTableWidgetItem(dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm")));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(radiationSpin->value(), 'f', 2)));

    statusBar()->showMessage(QString("✅ Добавлена запись для города %1").arg(city), 3000);
}



void MainWindow::analyzeData()
{
    int rows = table->rowCount();
    if (rows == 0) {
        QMessageBox::information(this, "Нет данных", "Сначала добавьте записи.");
        statusBar()->showMessage("Ошибка: нет данных для анализа");
        return;
    }

    QString currentCity = cityComboBox->currentText();

    QVector<double> rads; rads.reserve(rows);
    int cityRecordCount = 0;

    for (int i = 0; i < rows; ++i) {
        QString recordCity = table->item(i, 0)->text();
        if (recordCity == currentCity) {
            rads.append(table->item(i, 2)->text().toDouble());
            cityRecordCount++;
        }
    }

    if (cityRecordCount == 0) {
        QMessageBox::information(this, "Нет данных", QString("Нет записей для города %1").arg(currentCity));
        statusBar()->showMessage(QString("Нет данных для города %1").arg(currentCity));
        return;
    }

    auto mean = [](const QVector<double>& v)->double {
        if (v.isEmpty()) return 0.0;
        double s = 0;
        for (double x : v) s += x;
        return s / v.size();
    };
    auto max = [](const QVector<double>& v)->double {
        if (v.isEmpty()) return 0.0;
        double m = v[0];
        for (double x : v) if (x > m) m = x;
        return m;
    };
    auto min = [](const QVector<double>& v)->double {
        if (v.isEmpty()) return 0.0;
        double m = v[0];
        for (double x : v) if (x < m) m = x;
        return m;
    };
    auto stddev = [&](const QVector<double>& v)->double {
        if (v.isEmpty()) return 0.0;
        double m = mean(v);
        double s = 0;
        for (double x : v) s += (x - m) * (x - m);
        return std::sqrt(s / v.size());
    };

    QString result;
    result += QString("📊 АНАЛИЗ РАДИАЦИИ ДЛЯ %1\n").arg(currentCity.toUpper());
    result += QString("═══════════════════════════════\n\n");
    result += QString("🏙️  Город: %1\n").arg(currentCity);
    result += QString("📈 Количество записей: %1\n\n").arg(cityRecordCount);

    result += QString("☢️  РАДИАЦИЯ (мкР/ч):\n");
    result += QString("   • Средняя: %1\n").arg(mean(rads), 0, 'f', 2);
    result += QString("   • Минимальная: %1\n").arg(min(rads), 0, 'f', 2);
    result += QString("   • Максимальная: %1\n").arg(max(rads), 0, 'f', 2);
    result += QString("   • Стандартное отклонение: %1\n").arg(stddev(rads), 0, 'f', 2);

    analysisText->setPlainText(result);
    statusBar()->showMessage(QString("Анализ завершен для города %1. Обработано %2 записей").arg(currentCity).arg(cityRecordCount), 5000);
}


void MainWindow::saveToJson()
{
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Нет данных", "Таблица пуста. Нечего сохранять.");
        statusBar()->showMessage("Ошибка: нет данных для сохранения");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить данные", "", "JSON файлы (*.json)");
    if (fileName.isEmpty()) return;

    QJsonArray records;
    for (int i = 0; i < table->rowCount(); i++) {
        QJsonObject obj;
        obj["city"] = table->item(i, 0)->text();
        obj["datetime"] = table->item(i, 1)->text();
        obj["radiation"] = table->item(i, 2)->text().toDouble();
        records.append(obj);
    }

    QJsonDocument doc(records);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        statusBar()->showMessage("Ошибка сохранения файла");
        return;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Успех", QString("Данные сохранены в файл:\n%1").arg(fileName));
    statusBar()->showMessage(QString("Данные сохранены в: %1").arg(fileName), 5000);
}


void MainWindow::loadFromJson()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Загрузить данные", "", "JSON файлы (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        statusBar()->showMessage("Ошибка открытия файла");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        QMessageBox::warning(this, "Ошибка", "Формат файла неверный. Ожидался массив JSON.");
        statusBar()->showMessage("Ошибка: неверный формат JSON");
        return;
    }

    QJsonArray records = doc.array();
    table->setRowCount(0);

    for (auto v : records) {
        QJsonObject obj = v.toObject();
        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(obj.value("city").toString()));
        table->setItem(row, 1, new QTableWidgetItem(obj.value("datetime").toString()));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(obj.value("radiation").toDouble(), 'f', 2)));
    }

    QMessageBox::information(this, "Успех", QString("Загружено %1 записей из файла:\n%2").arg(records.size()).arg(fileName));
    statusBar()->showMessage(QString("Загружено %1 записей из %2").arg(records.size()).arg(fileName), 5000);
}


void MainWindow::setupCharts()
{
    createRadiationChart();
}

void MainWindow::createRadiationChart()
{
    QChart *chart = new QChart();
    chart->setTitle("☢️ Радиация по городам");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QFont legendFont; legendFont.setPointSize(10); legendFont.setBold(true);
    chart->legend()->setFont(legendFont);
    chart->setBackgroundBrush(QBrush(QColor(255,255,255)));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setPlotAreaBackgroundBrush(QLinearGradient(0,0,0,1));

    // Индексная ось X без привязки к датам
    auto *axisX = new QValueAxis;
    axisX->setTitleText("Индекс точки");
    axisX->setLabelFormat("%.0f");
    axisX->setTickCount(8);
    QFont axisFont; axisFont.setPointSize(9);
    axisX->setLabelsFont(axisFont);

    auto *axisY = new QValueAxis;
    axisY->setTitleText("мкР/ч");
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

void MainWindow::updateCharts()
{
    // Собираем выбранные города для наложения
    QList<QString> selectedCities;
    for (int i = 0; i < cityOverlayList->count(); ++i) {
        QListWidgetItem *item = cityOverlayList->item(i);
        if (item->checkState() == Qt::Checked) selectedCities.append(item->text());
    }
    if (selectedCities.isEmpty()) {
        // Если города не выбраны, строим для текущего
        selectedCities = { cityComboBox->currentText() };
    }

    QChart *chart = radiationChartView->chart();
    chart->removeAllSeries();

    QValueAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    {
        const auto axesX = chart->axes(Qt::Horizontal);
        for (QAbstractAxis *ax : axesX) {
            axisX = qobject_cast<QValueAxis*>(ax);
            if (axisX) break;
        }
        const auto axesY = chart->axes(Qt::Vertical);
        for (QAbstractAxis *ay : axesY) {
            axisY = qobject_cast<QValueAxis*>(ay);
            if (axisY) break;
        }
    }
    if (!axisX) {
        axisX = new QValueAxis;
        axisX->setTitleText("Индекс точки");
        axisX->setLabelFormat("%.0f");
        chart->addAxis(axisX, Qt::AlignBottom);
    }
    if (!axisY) {
        axisY = new QValueAxis;
        axisY->setTitleText("мкР/ч");
        axisY->setLabelFormat("%.0f");
        chart->addAxis(axisY, Qt::AlignLeft);
    }

    qint64 minTs = std::numeric_limits<qint64>::max();
    qint64 maxTs = std::numeric_limits<qint64>::min();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::min();

    // Цвета для серий
    QList<QColor> palette = { QColor("#e74c3c"), QColor("#3498db"), QColor("#2ecc71"), QColor("#9b59b6"), QColor("#f1c40f"), QColor("#e67e22"), QColor("#1abc9c") };

    int colorIndex = 0;
    // Определяем тип графика
    const QString type = chartTypeCombo ? chartTypeCombo->currentText() : QString("Точки");
    QVector<QAbstractSeries*> createdSeries;

    for (const QString &city : selectedCities) {
        QColor color = palette[colorIndex % palette.size()];
        QPen pen(color); pen.setWidth(3); pen.setCosmetic(true);

        if (type == "Сглаженная") {
            QSplineSeries *spline = new QSplineSeries();
            spline->setName(city);
            spline->setColor(color);
            spline->setPen(pen);
            spline->setPointsVisible(true);

            int rowCount = table->rowCount();
            for (int i = 0, idx = 0; i < rowCount; ++i) {
                if (table->item(i, 0)->text() == city) {
                    double rad = table->item(i, 2)->text().toDouble();
                    spline->append(idx, rad);
                    qint64 t = idx;
                    minTs = std::min(minTs, t);
                    maxTs = std::max(maxTs, t);
                    minY = std::min(minY, rad);
                    maxY = std::max(maxY, rad);
                    idx++;
                }
            }

            chart->addSeries(spline);
            spline->attachAxis(axisX);
            spline->attachAxis(axisY);
            createdSeries.append(spline);
        } else { // Точки+линии
            // Линия
            QLineSeries *line = new QLineSeries();
            line->setName(city);
            line->setColor(color);
            line->setPen(pen);
            // Точки поверх
            QScatterSeries *scatter = new QScatterSeries();
            scatter->setName(city + " • точки");
            scatter->setColor(color);
            scatter->setMarkerSize(7.5);

    int rowCount = table->rowCount();
            for (int i = 0, idx = 0; i < rowCount; ++i) {
                if (table->item(i, 0)->text() == city) {
                    double rad = table->item(i, 2)->text().toDouble();
                    line->append(idx, rad);
                    scatter->append(idx, rad);
                    qint64 t = idx;
                    minTs = std::min(minTs, t);
                    maxTs = std::max(maxTs, t);
                    minY = std::min(minY, rad);
                    maxY = std::max(maxY, rad);
                    idx++;
                }
            }

            chart->addSeries(line);
            line->attachAxis(axisX);
            line->attachAxis(axisY);
            chart->addSeries(scatter);
            scatter->attachAxis(axisX);
            scatter->attachAxis(axisY);
            createdSeries.append(line);
            createdSeries.append(scatter);
        }

        colorIndex++;
    }

    if (minTs == std::numeric_limits<qint64>::max()) {
        QMessageBox::information(this, "Нет данных", "Нет данных для выбранных городов");
        return;
    }

    // Удаляем предыдущие линии MIN/MAX и повторно активируем кнопку
    for (QAbstractSeries *s : chart->series()) {
        if (s->name() == "MIN линия" || s->name() == "MAX линия") {
            chart->removeSeries(s);
            s->deleteLater();
        }
    }
    if (btnFindMinMax) btnFindMinMax->setEnabled(true);

    // Небольшие отступы по Y
    double padding = (maxY - minY) * 0.1;
    if (padding <= 0) padding = 1.0;
    axisY->setRange(std::max(0.0, minY - padding), maxY + padding);
    axisX->setRange(minTs, maxTs);

    statusBar()->showMessage("✅ График обновлен", 3000);
}

// applyTheme removed; using toggleTheme for light/dark switch

void MainWindow::toggleTheme()
{
    if (!radiationChartView || !radiationChartView->chart()) return;
    QChart::ChartTheme current = radiationChartView->chart()->theme();
    QChart::ChartTheme next = (current == QChart::ChartThemeDark) ? QChart::ChartThemeLight : QChart::ChartThemeDark;
    radiationChartView->chart()->setTheme(next);
}

void MainWindow::findMinMax()
{
    if (!radiationChartView || !radiationChartView->chart()) return;
    QChart *chart = radiationChartView->chart();
    if (btnFindMinMax && !btnFindMinMax->isEnabled()) return; // уже добавлено
    if (!showMinMaxCheck || !showMinMaxCheck->isChecked()) {
        QMessageBox::information(this, "MIN/MAX", "Включите опцию 'Показывать линии MIN/MAX'.");
        return;
    }

    // Собираем все Y из текущих серий линий/баров
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    for (QAbstractSeries *s : chart->series()) {
        if (auto ls = qobject_cast<QLineSeries*>(s)) {
            for (const QPointF &p : ls->points()) {
                minY = std::min(minY, p.y());
                maxY = std::max(maxY, p.y());
            }
        } else if (auto bs = qobject_cast<QBarSeries*>(s)) {
            for (QBarSet *set : bs->barSets()) {
                for (int i = 0; i < set->count(); ++i) {
                    minY = std::min(minY, set->at(i));
                    maxY = std::max(maxY, set->at(i));
                }
            }
        }
    }

    if (minY == std::numeric_limits<double>::max()) {
        QMessageBox::information(this, "MIN/MAX", "Нет данных на графике.");
        return;
    }

    // Рисуем горизонтальные линии MIN и MAX как отдельные серии
    auto drawHLine = [&](double y, const QColor &color, const QString &name) {
        QLineSeries *line = new QLineSeries();
        line->setName(name);
        QPen pen(color); pen.setWidth(2); pen.setStyle(Qt::DashLine); pen.setCosmetic(true);
        line->setPen(pen);
        // Диапазон X
        QValueAxis *axisX = nullptr;
        for (QAbstractAxis *ax : chart->axes(Qt::Horizontal)) {
            if ((axisX = qobject_cast<QValueAxis*>(ax))) break;
        }
        double xMin = 0.0, xMax = 0.0;
        if (axisX) { xMin = axisX->min(); xMax = axisX->max(); }
        line->append(xMin, y);
        line->append(xMax, y);
        chart->addSeries(line);
        line->attachAxis(axisX);
        if (auto axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).value(0))) {
            line->attachAxis(axisY);
        }
    };

    drawHLine(minY, QColor("#27ae60"), "MIN линия");
    drawHLine(maxY, QColor("#c0392b"), "MAX линия");
    if (btnFindMinMax) btnFindMinMax->setEnabled(false);
}


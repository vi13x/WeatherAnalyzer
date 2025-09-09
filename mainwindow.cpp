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
#include <QPushButton>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>



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
    tabWidget->addTab(chartsTab, "📊 Графики");


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


    QGroupBox *inputGroup = new QGroupBox("📝 Ввод данных измерений");

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
            background: white;
            font-size: 11px;
        }
        QDateTimeEdit:hover {
            border-color: #3498db;
        }
    )");
    formLayout->addRow("🕐 Дата/время:", dateTimeEdit);

    radiationSpin = new QDoubleSpinBox;
    radiationSpin->setRange(0, 20000);
    radiationSpin->setDecimals(2);
    radiationSpin->setSuffix(" Вт/м²");
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
    formLayout->addRow("☀️ Радиация:", radiationSpin);

    temperatureSpin = new QDoubleSpinBox;
    temperatureSpin->setRange(-100, 100);
    temperatureSpin->setDecimals(2);
    temperatureSpin->setSuffix(" °C");
    temperatureSpin->setStyleSheet(R"(
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
    formLayout->addRow("🌡️ Температура:", temperatureSpin);

    humiditySpin = new QDoubleSpinBox;
    humiditySpin->setRange(0, 100);
    humiditySpin->setDecimals(2);
    humiditySpin->setSuffix(" %");
    humiditySpin->setStyleSheet(R"(
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
    formLayout->addRow("💧 Влажность:", humiditySpin);

    pressureSpin = new QDoubleSpinBox;
    pressureSpin->setRange(300, 1200);
    pressureSpin->setDecimals(2);
    pressureSpin->setSuffix(" гПа");
    pressureSpin->setStyleSheet(R"(
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
    formLayout->addRow("📊 Давление:", pressureSpin);

    windSpin = new QDoubleSpinBox;
    windSpin->setRange(0, 200);
    windSpin->setDecimals(2);
    windSpin->setSuffix(" м/с");
    windSpin->setStyleSheet(R"(
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
    formLayout->addRow("🌬️ Ветер:", windSpin);

    uvSpin = new QDoubleSpinBox;
    uvSpin->setRange(0, 30);
    uvSpin->setDecimals(2);
    uvSpin->setStyleSheet(R"(
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
    formLayout->addRow("🟣 УФ-индекс:", uvSpin);

    inputGroup->setLayout(formLayout);
    leftLayout->addWidget(inputGroup);


    btnAdd = new QPushButton("➕ Добавить запись");
    btnAnalyze = new QPushButton("📊 Анализировать данные");
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

    btnAnalyze->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2980b9, stop: 1 #3498db);
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
    connect(btnAnalyze, &QPushButton::clicked, this, &MainWindow::analyzeData);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromJson);

    leftLayout->addWidget(btnAdd);
    leftLayout->addWidget(btnAnalyze);
    leftLayout->addWidget(btnSave);
    leftLayout->addWidget(btnLoad);
    leftLayout->addStretch();


    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(15);


    QGroupBox *tableGroup = new QGroupBox("📋 Таблица измерений");

    QVBoxLayout *tableLayout = new QVBoxLayout;

    table = new QTableWidget(0, 8);
    QStringList headers = {"🏙️ Город", "🕐 Дата/время", "☀️ Радиация", "🌡️ Темп.", "💧 Влажность", "📊 Давление", "🌬️ Ветер", "🟣 UV"};
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


    QGroupBox *analysisBox = new QGroupBox("📊 Результаты анализа");

    QVBoxLayout *analysisLayout = new QVBoxLayout;
    analysisText = new QPlainTextEdit;
    analysisText->setReadOnly(true);
    analysisText->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #f8f9fa;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 12px;
            font-family: 'Consolas', 'Monospace';
            font-size: 11px;
            color: #2c3e50;
        }
        QPlainTextEdit:focus {
            border-color: #3498db;
        }
    )");
    analysisText->setPlaceholderText("📈 Здесь будут отображаться результаты анализа данных...");

    analysisLayout->addWidget(analysisText);
    analysisBox->setLayout(analysisLayout);
    rightLayout->addWidget(analysisBox, 3);


    mainDataLayout->addWidget(leftPanel, 3);
    mainDataLayout->addLayout(rightLayout, 7);


    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsTab);
    chartsLayout->setSpacing(15);
    chartsLayout->setContentsMargins(20, 20, 20, 20);


    QHBoxLayout *chartsButtonLayout = new QHBoxLayout;
    btnUpdateCharts = new QPushButton("🔄 Обновить графики");
    btnUpdateCharts->setStyleSheet(buttonBaseStyle + R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #e74c3c, stop: 1 #c0392b);
            color: white;
        }
    )");
    connect(btnUpdateCharts, &QPushButton::clicked, this, &MainWindow::updateCharts);

    chartsButtonLayout->addWidget(btnUpdateCharts);
    chartsButtonLayout->addStretch();
    chartsLayout->addLayout(chartsButtonLayout);


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


    temperatureChartView = new QChartView;
    radiationChartView = new QChartView;
    humidityChartView = new QChartView;
    windChartView = new QChartView;
    pressureChartView = new QChartView;
    uvChartView = new QChartView;


    QSize chartSize(600, 300);
    temperatureChartView->setMinimumSize(chartSize);
    radiationChartView->setMinimumSize(chartSize);
    humidityChartView->setMinimumSize(chartSize);
    windChartView->setMinimumSize(chartSize);
    pressureChartView->setMinimumSize(chartSize);
    uvChartView->setMinimumSize(chartSize);

    temperatureChartView->setRenderHint(QPainter::Antialiasing);
    radiationChartView->setRenderHint(QPainter::Antialiasing);
    humidityChartView->setRenderHint(QPainter::Antialiasing);
    windChartView->setRenderHint(QPainter::Antialiasing);
    pressureChartView->setRenderHint(QPainter::Antialiasing);
    uvChartView->setRenderHint(QPainter::Antialiasing);


    QLabel *tempLabel = new QLabel("🌡️ Температура (°C)");
    tempLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(tempLabel);
    chartsContainerLayout->addWidget(temperatureChartView);

    QLabel *radLabel = new QLabel("☀️ Солнечная радиация (Вт/м²)");
    radLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(radLabel);
    chartsContainerLayout->addWidget(radiationChartView);

    QLabel *humLabel = new QLabel("💧 Влажность (%)");
    humLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(humLabel);
    chartsContainerLayout->addWidget(humidityChartView);

    QLabel *windLabel = new QLabel("🌬️ Скорость ветра (м/с)");
    windLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(windLabel);
    chartsContainerLayout->addWidget(windChartView);

    QLabel *pressLabel = new QLabel("📊 Давление (гПа)");
    pressLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(pressLabel);
    chartsContainerLayout->addWidget(pressureChartView);

    QLabel *uvLabel = new QLabel("🟣 УФ-индекс");
    uvLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; }");
    chartsContainerLayout->addWidget(uvLabel);
    chartsContainerLayout->addWidget(uvChartView);

    scrollArea->setWidget(chartsContainer);
    chartsLayout->addWidget(scrollArea);


    setupCharts();


    statusBar()->setStyleSheet(R"(
        QStatusBar {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #2c3e50, stop: 1 #34495e);
            color: white;
            font-weight: bold;
            padding: 5px;
        }
    )");
    statusBar()->showMessage("✅ Готов к работе. Выберите город для начала.");
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
    table->setItem(row, 3, new QTableWidgetItem(QString::number(temperatureSpin->value(), 'f', 2)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(humiditySpin->value(), 'f', 2)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(pressureSpin->value(), 'f', 2)));
    table->setItem(row, 6, new QTableWidgetItem(QString::number(windSpin->value(), 'f', 2)));
    table->setItem(row, 7, new QTableWidgetItem(QString::number(uvSpin->value(), 'f', 2)));

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

    QVector<double> temps; temps.reserve(rows);
    QVector<double> rads; rads.reserve(rows);
    QVector<double> uvs; uvs.reserve(rows);
    QVector<double> humidities; humidities.reserve(rows);
    QVector<double> pressures; pressures.reserve(rows);
    QVector<double> winds; winds.reserve(rows);

    int cityRecordCount = 0;

    for (int i = 0; i < rows; ++i) {
        QString recordCity = table->item(i, 0)->text();


        if (recordCity == currentCity) {
            temps.append(table->item(i, 3)->text().toDouble());
            rads.append(table->item(i, 2)->text().toDouble());
            uvs.append(table->item(i, 7)->text().toDouble());
            humidities.append(table->item(i, 4)->text().toDouble());
            pressures.append(table->item(i, 5)->text().toDouble());
            winds.append(table->item(i, 6)->text().toDouble());
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
    result += QString("📊 АНАЛИЗ ПОГОДНЫХ ДАННЫХ ДЛЯ %1\n").arg(currentCity.toUpper());
    result += QString("══════════════════════════════════\n\n");
    result += QString("🏙️  Город: %1\n").arg(currentCity);
    result += QString("📈 Количество записей: %1\n\n").arg(cityRecordCount);

    result += QString("🌡️  ТЕМПЕРАТУРА:\n");
    result += QString("   • Средняя: %1 °C\n").arg(mean(temps), 0, 'f', 2);
    result += QString("   • Минимальная: %1 °C\n").arg(min(temps), 0, 'f', 2);
    result += QString("   • Максимальная: %1 °C\n").arg(max(temps), 0, 'f', 2);
    result += QString("   • Стандартное отклонение: %1\n\n").arg(stddev(temps), 0, 'f', 2);

    result += QString("☀️  СОЛНЕЧНАЯ РАДИАЦИЯ:\n");
    result += QString("   • Средняя: %1 Вт/м²\n").arg(mean(rads), 0, 'f', 2);
    result += QString("   • Минимальная: %1 Вт/м²\n").arg(min(rads), 0, 'f', 2);
    result += QString("   • Максимальная: %1 Вт/м²\n").arg(max(rads), 0, 'f', 2);
    result += QString("   • Стандартное отклонение: %1\n\n").arg(stddev(rads), 0, 'f', 2);

    result += QString("🟣 УФ-ИНДЕКС:\n");
    result += QString("   • Средний: %1\n").arg(mean(uvs), 0, 'f', 2);
    result += QString("   • Минимальный: %1\n").arg(min(uvs), 0, 'f', 2);
    result += QString("   • Максимальный: %1\n").arg(max(uvs), 0, 'f', 2);
    result += QString("   • Стандартное отклонение: %1\n\n").arg(stddev(uvs), 0, 'f', 2);

    result += QString("💧 ВЛАЖНОСТЬ:\n");
    result += QString("   • Средняя: %1%%\n").arg(mean(humidities), 0, 'f', 2);
    result += QString("   • Минимальная: %1%%\n").arg(min(humidities), 0, 'f', 2);
    result += QString("   • Максимальная: %1%%\n").arg(max(humidities), 0, 'f', 2);
    result += QString("   • Стандартное отклонение: %1\n\n").arg(stddev(humidities), 0, 'f', 2);

    result += QString("🌬️  ВЕТЕР:\n");
    result += QString("   • Средняя скорость: %1 м/с\n").arg(mean(winds), 0, 'f', 2);
    result += QString("   • Минимальная: %1 м/с\n").arg(min(winds), 0, 'f', 2);
    result += QString("   • Максимальная: %1 м/с\n").arg(max(winds), 0, 'f', 2);

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
        obj["temperature"] = table->item(i, 3)->text().toDouble();
        obj["humidity"] = table->item(i, 4)->text().toDouble();
        obj["pressure"] = table->item(i, 5)->text().toDouble();
        obj["wind"] = table->item(i, 6)->text().toDouble();
        obj["uv"] = table->item(i, 7)->text().toDouble();
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
        table->setItem(row, 3, new QTableWidgetItem(QString::number(obj.value("temperature").toDouble(), 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(obj.value("humidity").toDouble(), 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(obj.value("pressure").toDouble(), 'f', 2)));
        table->setItem(row, 6, new QTableWidgetItem(QString::number(obj.value("wind").toDouble(), 'f', 2)));
        table->setItem(row, 7, new QTableWidgetItem(QString::number(obj.value("uv").toDouble(), 'f', 2)));
    }

    QMessageBox::information(this, "Успех", QString("Загружено %1 записей из файла:\n%2").arg(records.size()).arg(fileName));
    statusBar()->showMessage(QString("Загружено %1 записей из %2").arg(records.size()).arg(fileName), 5000);
}


void MainWindow::setupCharts()
{
    createTemperatureChart();
    createRadiationChart();
    createHumidityChart();
    createWindChart();
    createPressureChart();
    createUVChart();
}

void MainWindow::createTemperatureChart()
{
    QChart *chart = new QChart();
    chart->setTitle("🌡️ Температура");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("Температура (°C)");

    chart->addSeries(series);
    chart->createDefaultAxes();

    temperatureChartView->setChart(chart);
}

void MainWindow::createRadiationChart()
{
    QChart *chart = new QChart();
    chart->setTitle("☀️ Солнечная радиация");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("Радиация (Вт/м²)");

    chart->addSeries(series);
    chart->createDefaultAxes();

    radiationChartView->setChart(chart);
}

void MainWindow::createHumidityChart()
{
    QChart *chart = new QChart();
    chart->setTitle("💧 Влажность");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("Влажность (%)");

    chart->addSeries(series);
    chart->createDefaultAxes();

    humidityChartView->setChart(chart);
}

void MainWindow::createWindChart()
{
    QChart *chart = new QChart();
    chart->setTitle("🌬️ Скорость ветра");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("Ветер (м/с)");

    chart->addSeries(series);
    chart->createDefaultAxes();

    windChartView->setChart(chart);
}

void MainWindow::createPressureChart()
{
    QChart *chart = new QChart();
    chart->setTitle("📊 Давление");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("Давление (гПа)");

    chart->addSeries(series);
    chart->createDefaultAxes();

    pressureChartView->setChart(chart);
}

void MainWindow::createUVChart()
{
    QChart *chart = new QChart();
    chart->setTitle("🟣 УФ-индекс");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setTheme(QChart::ChartThemeLight);

    QLineSeries *series = new QLineSeries();
    series->setName("УФ-индекс");

    chart->addSeries(series);
    chart->createDefaultAxes();

    uvChartView->setChart(chart);
}

void MainWindow::updateCharts()
{
    QString currentCity = cityComboBox->currentText();

    QVector<double> temperatures, radiations, humidities, winds, pressures, uvs;

    int rowCount = table->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        if (table->item(i, 0)->text() == currentCity) {
            temperatures.append(table->item(i, 3)->text().toDouble());
            radiations.append(table->item(i, 2)->text().toDouble());
            humidities.append(table->item(i, 4)->text().toDouble());
            winds.append(table->item(i, 6)->text().toDouble());
            pressures.append(table->item(i, 5)->text().toDouble());
            uvs.append(table->item(i, 7)->text().toDouble());
        }
    }

    if (temperatures.isEmpty()) {
        QMessageBox::information(this, "Нет данных", "Нет данных для построения графиков");
        return;
    }


    QLineSeries *tempSeries = new QLineSeries();
    tempSeries->setName("Температура");
    for (int i = 0; i < temperatures.size(); ++i) {
        tempSeries->append(i, temperatures[i]);
    }

    QChart *tempChart = temperatureChartView->chart();
    tempChart->removeAllSeries();
    tempChart->addSeries(tempSeries);
    tempChart->createDefaultAxes();


    QLineSeries *radSeries = new QLineSeries();
    for (int i = 0; i < radiations.size(); ++i) {
        radSeries->append(i, radiations[i]);
    }
    radiationChartView->chart()->removeAllSeries();
    radiationChartView->chart()->addSeries(radSeries);
    radiationChartView->chart()->createDefaultAxes();

    QLineSeries *humSeries = new QLineSeries();
    for (int i = 0; i < humidities.size(); ++i) {
        humSeries->append(i, humidities[i]);
    }
    humidityChartView->chart()->removeAllSeries();
    humidityChartView->chart()->addSeries(humSeries);
    humidityChartView->chart()->createDefaultAxes();

    QLineSeries *windSeries = new QLineSeries();
    for (int i = 0; i < winds.size(); ++i) {
        windSeries->append(i, winds[i]);
    }
    windChartView->chart()->removeAllSeries();
    windChartView->chart()->addSeries(windSeries);
    windChartView->chart()->createDefaultAxes();

    QLineSeries *pressSeries = new QLineSeries();
    for (int i = 0; i < pressures.size(); ++i) {
        pressSeries->append(i, pressures[i]);
    }
    pressureChartView->chart()->removeAllSeries();
    pressureChartView->chart()->addSeries(pressSeries);
    pressureChartView->chart()->createDefaultAxes();

    QLineSeries *uvSeries = new QLineSeries();
    for (int i = 0; i < uvs.size(); ++i) {
        uvSeries->append(i, uvs[i]);
    }
    uvChartView->chart()->removeAllSeries();
    uvChartView->chart()->addSeries(uvSeries);
    uvChartView->chart()->createDefaultAxes();

    statusBar()->showMessage("✅ Графики обновлены", 3000);
}


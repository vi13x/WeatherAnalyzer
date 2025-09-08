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

// === Конструктор ===
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 800);
    setWindowTitle("Анализатор погодных данных");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // === ЛЕВАЯ ПАНЕЛЬ (Ввод данных) ===
    QFrame *leftPanel = new QFrame;
    leftPanel->setFrameStyle(QFrame::StyledPanel);
    leftPanel->setStyleSheet("QFrame { background-color: #f8f9fa; border-radius: 8px; }");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);

    // Группа ввода данных
    QGroupBox *inputGroup = new QGroupBox("Ввод данных измерений");
    inputGroup->setStyleSheet("QGroupBox { font-weight: bold; }");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignRight);

    dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    dateTimeEdit->setCalendarPopup(true);
    dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    dateTimeEdit->setStyleSheet("QDateTimeEdit { padding: 5px; }");
    formLayout->addRow("Дата/время:", dateTimeEdit);

    radiationSpin = new QDoubleSpinBox;
    radiationSpin->setRange(0, 20000);
    radiationSpin->setDecimals(2);
    radiationSpin->setSuffix(" Вт/м²");
    radiationSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("Солнечная радиация:", radiationSpin);

    temperatureSpin = new QDoubleSpinBox;
    temperatureSpin->setRange(-100, 100);
    temperatureSpin->setDecimals(2);
    temperatureSpin->setSuffix(" °C");
    temperatureSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("Температура:", temperatureSpin);

    humiditySpin = new QDoubleSpinBox;
    humiditySpin->setRange(0, 100);
    humiditySpin->setDecimals(2);
    humiditySpin->setSuffix(" %");
    humiditySpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("Влажность:", humiditySpin);

    pressureSpin = new QDoubleSpinBox;
    pressureSpin->setRange(300, 1200);
    pressureSpin->setDecimals(2);
    pressureSpin->setSuffix(" гПа");
    pressureSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("Давление:", pressureSpin);

    windSpin = new QDoubleSpinBox;
    windSpin->setRange(0, 200);
    windSpin->setDecimals(2);
    windSpin->setSuffix(" м/с");
    windSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("Скорость ветра:", windSpin);

    uvSpin = new QDoubleSpinBox;
    uvSpin->setRange(0, 30);
    uvSpin->setDecimals(2);
    uvSpin->setStyleSheet("QDoubleSpinBox { padding: 5px; }");
    formLayout->addRow("УФ-индекс:", uvSpin);

    inputGroup->setLayout(formLayout);
    leftLayout->addWidget(inputGroup);

    // Кнопки
    QPushButton *btnAdd = new QPushButton("➕ Добавить запись");
    QPushButton *btnAnalyze = new QPushButton("📊 Анализировать данные");
    QPushButton *btnSave = new QPushButton("💾 Сохранить JSON");
    QPushButton *btnLoad = new QPushButton("📂 Загрузить JSON");

    // Стилизация кнопок
    QString buttonStyle = "QPushButton { padding: 10px; font-weight: bold; border-radius: 5px; }";
    btnAdd->setStyleSheet(buttonStyle + "QPushButton { background-color: #4CAF50; color: white; }");
    btnAnalyze->setStyleSheet(buttonStyle + "QPushButton { background-color: #2196F3; color: white; }");
    btnSave->setStyleSheet(buttonStyle + "QPushButton { background-color: #FF9800; color: white; }");
    btnLoad->setStyleSheet(buttonStyle + "QPushButton { background-color: #9C27B0; color: white; }");

    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::addRecord);
    connect(btnAnalyze, &QPushButton::clicked, this, &MainWindow::analyzeData);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveToJson);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadFromJson);

    leftLayout->addWidget(btnAdd);
    leftLayout->addWidget(btnAnalyze);
    leftLayout->addWidget(btnSave);
    leftLayout->addWidget(btnLoad);
    leftLayout->addStretch();

    // === ПРАВАЯ ПАНЕЛЬ (Таблица + Анализ) ===
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(15);

    // Таблица данных
    QGroupBox *tableGroup = new QGroupBox("Таблица измерений");
    tableGroup->setStyleSheet("QGroupBox { font-weight: bold; }");

    QVBoxLayout *tableLayout = new QVBoxLayout;

    table = new QTableWidget(0, 7);
    QStringList headers = {"Дата/время", "Радиация (Вт/м²)", "Темп. (°C)", "Влажность (%)", "Давление (гПа)", "Ветер (м/с)", "UV"};
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet(
        "QTableWidget { gridline-color: #ddd; }"
        "QHeaderView::section { background-color: #e3e3e3; padding: 5px; border: 1px solid #ccc; }"
        );

    tableLayout->addWidget(table);
    tableGroup->setLayout(tableLayout);
    rightLayout->addWidget(tableGroup, 7);

    // Панель анализа
    QGroupBox *analysisBox = new QGroupBox("Результаты анализа");
    analysisBox->setStyleSheet("QGroupBox { font-weight: bold; }");

    QVBoxLayout *analysisLayout = new QVBoxLayout;
    analysisText = new QPlainTextEdit;
    analysisText->setReadOnly(true);
    analysisText->setStyleSheet(
        "QPlainTextEdit { background-color: #f8f9fa; border: 1px solid #ddd; border-radius: 5px; padding: 10px; }"
        );
    analysisText->setPlaceholderText("Здесь будут отображаться результаты анализа данных...");

    analysisLayout->addWidget(analysisText);
    analysisBox->setLayout(analysisLayout);
    rightLayout->addWidget(analysisBox, 3);

    // === Итоговый Layout ===
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addLayout(rightLayout, 7);

    // Статус бар
    statusBar()->showMessage("Готов к работе");
}

// === Деструктор ===
MainWindow::~MainWindow() = default;

// === Добавление записи ===
void MainWindow::addRecord()
{
    int row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm")));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(radiationSpin->value(), 'f', 2)));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(temperatureSpin->value(), 'f', 2)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(humiditySpin->value(), 'f', 2)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(pressureSpin->value(), 'f', 2)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(windSpin->value(), 'f', 2)));
    table->setItem(row, 6, new QTableWidgetItem(QString::number(uvSpin->value(), 'f', 2)));

    statusBar()->showMessage(QString("Добавлена запись №%1").arg(row + 1), 3000);
}

// === Анализ данных ===
void MainWindow::analyzeData()
{
    int rows = table->rowCount();
    if (rows == 0) {
        QMessageBox::information(this, "Нет данных", "Сначала добавьте записи.");
        statusBar()->showMessage("Ошибка: нет данных для анализа");
        return;
    }

    QVector<double> temps; temps.reserve(rows);
    QVector<double> rads; rads.reserve(rows);
    QVector<double> uvs; uvs.reserve(rows);
    QVector<double> humidities; humidities.reserve(rows);
    QVector<double> pressures; pressures.reserve(rows);
    QVector<double> winds; winds.reserve(rows);

    for (int i = 0; i < rows; ++i) {
        temps.append(table->item(i, 2)->text().toDouble());
        rads.append(table->item(i, 1)->text().toDouble());
        uvs.append(table->item(i, 6)->text().toDouble());
        humidities.append(table->item(i, 3)->text().toDouble());
        pressures.append(table->item(i, 4)->text().toDouble());
        winds.append(table->item(i, 5)->text().toDouble());
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
    result += QString("📊 АНАЛИЗ ПОГОДНЫХ ДАННЫХ\n");
    result += QString("══════════════════════════\n\n");
    result += QString("📈 Общее количество записей: %1\n\n").arg(rows);

    result += QString("🌡️  ТЕМПЕРАТУРА:\n");
    result += QString("   • Среднее: %1 °C\n").arg(mean(temps), 0, 'f', 2);
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

    analysisText->setPlainText(result);
    statusBar()->showMessage(QString("Анализ завершен. Обработано %1 записей").arg(rows), 5000);
}

// === Сохранение JSON ===
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
        obj["datetime"] = table->item(i, 0)->text();
        obj["radiation"] = table->item(i, 1)->text().toDouble();
        obj["temperature"] = table->item(i, 2)->text().toDouble();
        obj["humidity"] = table->item(i, 3)->text().toDouble();
        obj["pressure"] = table->item(i, 4)->text().toDouble();
        obj["wind"] = table->item(i, 5)->text().toDouble();
        obj["uv"] = table->item(i, 6)->text().toDouble();
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

// === Загрузка JSON ===
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

        table->setItem(row, 0, new QTableWidgetItem(obj.value("datetime").toString()));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(obj.value("radiation").toDouble(), 'f', 2)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(obj.value("temperature").toDouble(), 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(obj.value("humidity").toDouble(), 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(obj.value("pressure").toDouble(), 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(obj.value("wind").toDouble(), 'f', 2)));
        table->setItem(row, 6, new QTableWidgetItem(QString::number(obj.value("uv").toDouble(), 'f', 2)));
    }

    QMessageBox::information(this, "Успех", QString("Загружено %1 записей из файла:\n%2").arg(records.size()).arg(fileName));
    statusBar()->showMessage(QString("Загружено %1 записей из %2").arg(records.size()).arg(fileName), 5000);
}

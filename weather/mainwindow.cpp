#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <cmath>
#include <vector>
#include <algorithm>


    struct WeatherData {
    QDateTime datetime;
    double radiation;   // Вт/м^2
    double temperature; // °C
    double humidity;    // %
    double pressure;    // hPa
    double windSpeed;   // m/s
    double uvIndex;     // unitless
};

// Helper functions for statistics
namespace stats {

static double mean(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += x;
    return s / double(v.size());
}

static double variance(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    double m = mean(v);
    double s = 0.0;
    for (double x : v) s += (x - m) * (x - m);
    return s / double(v.size());
}

static double stddev(const std::vector<double>& v) {
    return std::sqrt(variance(v));
}

static double minv(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    return *std::min_element(v.begin(), v.end());
}

static double maxv(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    return *std::max_element(v.begin(), v.end());
}

static double correlation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.empty()) return 0.0;
    double mx = mean(x), my = mean(y);
    double num = 0.0, dx2 = 0.0, dy2 = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        double dx = x[i] - mx;
        double dy = y[i] - my;
        num += dx * dy;
        dx2 += dx * dx;
        dy2 += dy * dy;
    }
    double den = std::sqrt(dx2 * dy2);
    if (den == 0.0) return 0.0;
    return num / den;
}

} // namespace stats

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        setWindowTitle("SunSafety — мониторинг метеоусловий");
        resize(1100, 700);

        QWidget *central = new QWidget;
        QHBoxLayout *mainLayout = new QHBoxLayout;

        // Left: Input form + controls
        QVBoxLayout *leftLayout = new QVBoxLayout;

        QGroupBox *inputBox = new QGroupBox("Добавить запись");
        QFormLayout *form = new QFormLayout;

        dateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
        dateEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
        dateEdit->setCalendarPopup(true);
        form->addRow("Дата и время:", dateEdit);

        radiationSpin = makeDoubleSpin(0, 20000, 0.01);
        form->addRow("Солнечная радиация (Вт/м²):", radiationSpin);

        temperatureSpin = makeDoubleSpin(-100, 100, 0.01);
        form->addRow("Температура (°C):", temperatureSpin);

        humiditySpin = makeDoubleSpin(0, 100, 0.01);
        form->addRow("Влажность (%):", humiditySpin);

        pressureSpin = makeDoubleSpin(300, 1100, 0.01);
        form->addRow("Давление (гПа):", pressureSpin);

        windSpin = makeDoubleSpin(0, 200, 0.01);
        form->addRow("Скорость ветра (м/с):", windSpin);

        uvSpin = makeDoubleSpin(0, 20, 0.01);
        form->addRow("УФ индекс:", uvSpin);

        inputBox->setLayout(form);
        leftLayout->addWidget(inputBox);

        QHBoxLayout *btns = new QHBoxLayout;
        addBtn = new QPushButton("Добавить");
        editBtn = new QPushButton("Редактировать");
        removeBtn = new QPushButton("Удалить");
        btns->addWidget(addBtn);
        btns->addWidget(editBtn);
        btns->addWidget(removeBtn);
        leftLayout->addLayout(btns);

        connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAdd);
        connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEdit);
        connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemove);

        // File controls
        QHBoxLayout *fileBtns = new QHBoxLayout;
        loadBtn = new QPushButton("Загрузить JSON");
        saveBtn = new QPushButton("Сохранить JSON");
        exportBtn = new QPushButton("Экспорт JSON");
        fileBtns->addWidget(loadBtn);
        fileBtns->addWidget(saveBtn);
        fileBtns->addWidget(exportBtn);
        leftLayout->addLayout(fileBtns);

        connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoad);
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
        connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExport);

        // Analysis buttons
        QHBoxLayout *analysisBtns = new QHBoxLayout;
        analyzeBtn = new QPushButton("Анализировать");
        warnBtn = new QPushButton("Проверить предупреждения");
        analysisBtns->addWidget(analyzeBtn);
        analysisBtns->addWidget(warnBtn);
        leftLayout->addLayout(analysisBtns);

        connect(analyzeBtn, &QPushButton::clicked, this, &MainWindow::onAnalyze);
        connect(warnBtn, &QPushButton::clicked, this, &MainWindow::onWarnings);

        // Quick presets (buttons for common temps, radiation)
        QGroupBox *presetsBox = new QGroupBox("Быстрые пресеты");
        QHBoxLayout *presetLayout = new QHBoxLayout;
        QPushButton *presetCold = new QPushButton("0 °C");
        QPushButton *presetHot = new QPushButton("30 °C");
        QPushButton *presetSunny = new QPushButton("1000 Вт/м²");
        QPushButton *presetUVHigh = new QPushButton("UV 8");
        presetLayout->addWidget(presetCold);
        presetLayout->addWidget(presetHot);
        presetLayout->addWidget(presetSunny);
        presetLayout->addWidget(presetUVHigh);
        presetsBox->setLayout(presetLayout);
        leftLayout->addWidget(presetsBox);

        connect(presetCold, &QPushButton::clicked, [=](){ temperatureSpin->setValue(0.00); });
        connect(presetHot, &QPushButton::clicked, [=](){ temperatureSpin->setValue(30.00); });
        connect(presetSunny, &QPushButton::clicked, [=](){ radiationSpin->setValue(1000.00); });
        connect(presetUVHigh, &QPushButton::clicked, [=](){ uvSpin->setValue(8.00); });

        leftLayout->addStretch();

        // Right: Table + analysis display
        QVBoxLayout *rightLayout = new QVBoxLayout;

        table = new QTableWidget(0, 7);
        QStringList headers = {"Дата и время","Радиация","Темп. (°C)","Влажность","Давление","Ветер","UV"};
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        rightLayout->addWidget(table);

        // Analysis output area
        QGroupBox *analysisBox = new QGroupBox("Результаты анализа");
        QVBoxLayout *analysisLayout = new QVBoxLayout;
        analysisText = new QPlainTextEdit;
        analysisText->setReadOnly(true);
        analysisLayout->addWidget(analysisText);
        analysisBox->setLayout(analysisLayout);
        rightLayout->addWidget(analysisBox);

        mainLayout->addLayout(leftLayout, 3);
        mainLayout->addLayout(rightLayout, 5);

        central->setLayout(mainLayout);
        setCentralWidget(central);

        // Initialize
        connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelect);
        editBtn->setEnabled(false);
        removeBtn->setEnabled(false);

        // Try to load default file on startup
        defaultFilename = QDir::currentPath() + "/weather.json";
        loadFromFile(defaultFilename);
        refreshTable();
    }

private slots:
    void onAdd() {
        WeatherData d = readInputs();
        dataset.push_back(d);
        refreshTable();
    }

    void onEdit() {
        auto sel = table->selectedRanges();
        if (sel.isEmpty()) return;
        int r = sel.first().topRow();
        if (r < 0 || r >= int(dataset.size())) return;
        dataset[r] = readInputs();
        refreshTable();
    }

    void onRemove() {
        auto sel = table->selectedRanges();
        if (sel.isEmpty()) return;
        int r = sel.first().topRow();
        if (r < 0 || r >= int(dataset.size())) return;
        dataset.erase(dataset.begin() + r);
        refreshTable();
    }

    void onLoad() {
        QString fn = QFileDialog::getOpenFileName(this, "Открыть JSON", QDir::homePath(), "JSON Files (*.json)");
        if (fn.isEmpty()) return;
        loadFromFile(fn);
        refreshTable();
    }

    void onSave() {
        QString fn = QFileDialog::getSaveFileName(this, "Сохранить JSON", defaultFilename, "JSON Files (*.json)");
        if (fn.isEmpty()) return;
        saveToFile(fn);
    }

    void onExport() {
        QString fn = QFileDialog::getSaveFileName(this, "Экспортировать JSON", defaultFilename, "JSON Files (*.json)");
        if (fn.isEmpty()) return;
        saveToFile(fn);
    }

    void onAnalyze() {
        if (dataset.empty()) {
            analysisText->setPlainText("Нет данных для анализа.");
            return;
        }
        // Collect vectors
        std::vector<double> rad, tmp, hum, pres, wnd, uv;
        for (auto &d : dataset) {
            rad.push_back(d.radiation);
            tmp.push_back(d.temperature);
            hum.push_back(d.humidity);
            pres.push_back(d.pressure);
            wnd.push_back(d.windSpeed);
            uv.push_back(d.uvIndex);
        }
        QString out;
        out += "Записей: " + QString::number(dataset.size()) + "\n\n";

        auto appendStat = [&](const QString &name, const std::vector<double>& v){
            out += name + "\n";
            out += "  Среднее: " + QString::number(stats::mean(v),'f',2) + "\n";
            out += "  Мин: " + QString::number(stats::minv(v),'f',2) + "\n";
            out += "  Макс: " + QString::number(stats::maxv(v),'f',2) + "\n";
            out += "  Стд. откл.: " + QString::number(stats::stddev(v),'f',2) + "\n\n";
        };

        appendStat("Солнечная радиация (Вт/м²):", rad);
        appendStat("Температура (°C):", tmp);
        appendStat("Влажность (%):", hum);
        appendStat("Давление (гПа):", pres);
        appendStat("Скорость ветра (м/с):", wnd);
        appendStat("УФ индекс:", uv);

        // Correlations
        out += "Корреляции:\n";
        out += "  Радиaция / Темп.: " + QString::number(stats::correlation(rad, tmp),'f',3) + "\n";
        out += "  Радиaция / УФ: " + QString::number(stats::correlation(rad, uv),'f',3) + "\n";
        out += "  Темп. / Влажность: " + QString::number(stats::correlation(tmp, hum),'f',3) + "\n";

        analysisText->setPlainText(out);
    }

    void onWarnings() {
        if (dataset.empty()) {
            QMessageBox::information(this, "Предупреждения", "Нет данных.");
            return;
        }
        QStringList warns;
        for (auto &d : dataset) {
            if (d.uvIndex >= 8.0) warns << d.datetime.toString("yyyy-MM-dd HH:mm") + ": Высокий УФ — рекомендуется защита";
            if (d.radiation >= 1000.0) warns << d.datetime.toString("yyyy-MM-dd HH:mm") + ": Сильная солнечная радиация — избегать прямого солнца";
            if (d.temperature >= 35.0) warns << d.datetime.toString("yyyy-MM-dd HH:mm") + ": Высокая температура — риск теплового удара";
            if (d.temperature <= -10.0) warns << d.datetime.toString("yyyy-MM-dd HH:mm") + ": Низкая температура — риск переохлаждения";
        }
        if (warns.isEmpty()) {
            QMessageBox::information(this, "Предупреждения", "Специальных предупреждений не выявлено.");
        } else {
            QMessageBox::warning(this, "Предупреждения", warns.join("\n"));
        }
    }

    void onTableSelect() {
        auto sel = table->selectedRanges();
        bool ok = !sel.isEmpty();
        editBtn->setEnabled(ok);
        removeBtn->setEnabled(ok);
        if (ok) {
            int r = sel.first().topRow();
            if (r >= 0 && r < int(dataset.size())) {
                populateInputs(dataset[r]);
            }
        }
    }

private:
    // UI elements
    QDateTimeEdit *dateEdit;
    QDoubleSpinBox *radiationSpin;
    QDoubleSpinBox *temperatureSpin;
    QDoubleSpinBox *humiditySpin;
    QDoubleSpinBox *pressureSpin;
    QDoubleSpinBox *windSpin;
    QDoubleSpinBox *uvSpin;

    QPushButton *addBtn;
    QPushButton *editBtn;
    QPushButton *removeBtn;
    QPushButton *loadBtn;
    QPushButton *saveBtn;
    QPushButton *exportBtn;
    QPushButton *analyzeBtn;
    QPushButton *warnBtn;

    QTableWidget *table;
    QPlainTextEdit *analysisText;

    std::vector<WeatherData> dataset;
    QString defaultFilename;

    QDoubleSpinBox* makeDoubleSpin(double minv, double maxv, double step) {
        QDoubleSpinBox *s = new QDoubleSpinBox;
        s->setRange(minv, maxv);
        s->setSingleStep(step);
        s->setDecimals(2);
        s->setValue(0.0);
        return s;
    }

    WeatherData readInputs() {
        WeatherData d;
        d.datetime = dateEdit->dateTime();
        d.radiation = radiationSpin->value();
        d.temperature = temperatureSpin->value();
        d.humidity = humiditySpin->value();
        d.pressure = pressureSpin->value();
        d.windSpeed = windSpin->value();
        d.uvIndex = uvSpin->value();
        return d;
    }

    void populateInputs(const WeatherData &d) {
        dateEdit->setDateTime(d.datetime);
        radiationSpin->setValue(d.radiation);
        temperatureSpin->setValue(d.temperature);
        humiditySpin->setValue(d.humidity);
        pressureSpin->setValue(d.pressure);
        windSpin->setValue(d.windSpeed);
        uvSpin->setValue(d.uvIndex);
    }

    void refreshTable() {
        table->setRowCount(int(dataset.size()));
        for (int i = 0; i < int(dataset.size()); ++i) {
            const auto &d = dataset[i];
            table->setItem(i, 0, new QTableWidgetItem(d.datetime.toString("yyyy-MM-dd HH:mm")));
            table->setItem(i, 1, new QTableWidgetItem(QString::number(d.radiation,'f',2)));
            table->setItem(i, 2, new QTableWidgetItem(QString::number(d.temperature,'f',2)));
            table->setItem(i, 3, new QTableWidgetItem(QString::number(d.humidity,'f',2)));
            table->setItem(i, 4, new QTableWidgetItem(QString::number(d.pressure,'f',2)));
            table->setItem(i, 5, new QTableWidgetItem(QString::number(d.windSpeed,'f',2)));
            table->setItem(i, 6, new QTableWidgetItem(QString::number(d.uvIndex,'f',2)));
        }
    }

    bool saveToFile(const QString &filename) {
        QFile f(filename);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи: " + filename);
            return false;
        }
        QJsonArray arr;
        for (auto &d : dataset) {
            QJsonObject obj;
            obj["datetime"] = d.datetime.toString(Qt::ISODate);
            obj["radiation"] = d.radiation;
            obj["temperature"] = d.temperature;
            obj["humidity"] = d.humidity;
            obj["pressure"] = d.pressure;
            obj["windSpeed"] = d.windSpeed;
            obj["uvIndex"] = d.uvIndex;
            arr.append(obj);
        }
        QJsonObject root;
        root["data"] = arr;
        QJsonDocument doc(root);
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
        QMessageBox::information(this, "Сохранено", "Файл сохранён: " + filename);
        return true;
    }

    bool loadFromFile(const QString &filename) {
        QFile f(filename);
        if (!f.exists()) return false;
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл: " + filename);
            return false;
        }
        QByteArray b = f.readAll();
        f.close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(b, &err);
        if (err.error != QJsonParseError::NoError) {
            QMessageBox::critical(this, "Ошибка парсинга JSON", err.errorString());
            return false;
        }
        dataset.clear();
        QJsonObject root = doc.object();
        QJsonArray arr = root.value("data").toArray();
        for (auto val : arr) {
            QJsonObject obj = val.toObject();
            WeatherData d;
            d.datetime = QDateTime::fromString(obj.value("datetime").toString(), Qt::ISODate);
            d.radiation = obj.value("radiation").toDouble();
            d.temperature = obj.value("temperature").toDouble();
            d.humidity = obj.value("humidity").toDouble();
            d.pressure = obj.value("pressure").toDouble();
            d.windSpeed = obj.value("windSpeed").toDouble();
            d.uvIndex = obj.value("uvIndex").toDouble();
            dataset.push_back(d);
        }
        QMessageBox::information(this, "Загружено", "Данные загружены из: " + filename);
        return true;
    }
};




#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("ExampleOrg");
    QApplication::setApplicationName("RadiationAnalyzer");

    MainWindow w;
    w.show();
    return app.exec();
}

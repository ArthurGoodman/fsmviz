#include <QtWidgets/QApplication>
#include "View.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    fsmviz::View w;
    w.show();

    return app.exec();
}

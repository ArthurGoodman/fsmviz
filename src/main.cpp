#include <QtWidgets/QApplication>
#include "Widget.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    fsmviz::Widget w;
    w.show();

    return app.exec();
}

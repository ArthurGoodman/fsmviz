#include <QtWidgets/QApplication>
#include "Controller.hpp"
#include "View.hpp"
#include "gcp/GenericCommandProcessor.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    gcp::GenericCommandProcessor processor;

    fsmviz::Controller controller(processor);

    fsmviz::View view(processor, controller);
    controller.setView(&view);

    view.show();

    return app.exec();
}

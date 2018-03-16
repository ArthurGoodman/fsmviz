#include <functional>
#include <QtWidgets/QApplication>
#include "Controller.hpp"
#include "View.hpp"
#include "gcp/GenericCommandProcessor.hpp"
#include "qconsole/QConsole.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    gcp::GenericCommandProcessor processor;

    qconsole::QConsole *console = new qconsole::QConsole;

    static constexpr int c_console_alpha = 128;
    console->setStyleSheet(("background-color: rgba(0, 0, 0, " +
                            std::to_string(c_console_alpha) + ");")
                               .c_str());

    console->setPrompt("$ ");

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(16);
    console->setFont(font);

    console->setProcessor(std::bind(
        &gcp::GenericCommandProcessor::process,
        &processor,
        std::placeholders::_1));

    fsmviz::Controller controller(processor, *console);

    fsmviz::View view(processor, *console, controller);
    controller.setView(&view);

    view.show();

    return app.exec();
}

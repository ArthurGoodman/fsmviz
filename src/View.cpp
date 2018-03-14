#include "View.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <limits>
#include <sstream>
#include <QtWidgets/QtWidgets>
#include "StateGraphicsObject.hpp"
#include "TransitionGraphicsObject.hpp"

namespace fsmviz {

View::View(gcp::GenericCommandProcessor &processor, Controller &controller)
    : m_processor{processor}
    , m_controller{controller}
    , m_selected_object{nullptr}
    , m_translating{false}
    , m_moving{false}
    , m_run{false}
    , m_antialias{true}
    , m_scale{1}
    , m_time{Clock::now()}
    , m_console{this}
    , m_console_visible{false}
    , m_editing{false} ///@ refactor (maybe...)
{
    std::srand(std::time(nullptr));

    QSize screen_size = qApp->primaryScreen()->size();
    QPoint screen_center =
        QPoint(screen_size.width() / 2, screen_size.height() / 2);

    resize(screen_size * 0.75);
    move(screen_center - rect().center());

    setMouseTracking(true);
    setFocus();

    ////////////////////////////////////////////////////////////////////////////
    // Console setup
    ////////////////////////////////////////////////////////////////////////////

    m_console.setPrompt("$ ");

    m_console.setStyleSheet("background-color: rgba(0, 0, 0, 128);");

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(16);
    m_console.setFont(font);

    m_console.setProcessor(std::bind(
        &gcp::GenericCommandProcessor::process,
        &m_processor,
        std::placeholders::_1));

    ////////////////////////////////////////////////////////////////////////////
    // Key bindings
    ////////////////////////////////////////////////////////////////////////////

    bind("space", [&]() { m_processor.process("toggle_run"); });
    bind("backspace", [&]() { m_processor.process("reset"); });
    bind("a", [&]() { m_processor.process("antialias"); });
    bind("delete", [&]() { m_processor.process("delete"); });
    bind("f11", [&]() { m_processor.process("toggle_fullscreen"); });
    bind("[", [&]() { m_processor.process("toggle_starting"); });
    bind("]", [&]() { m_processor.process("toggle_final"); });
    bind("return", [&]() { m_processor.process("edit"); });

    bind("r", [&]() { m_processor.process("rev"); });
    bind("d", [&]() { m_processor.process("det"); });
    bind("m", [&]() { m_processor.process("min"); });

    ////////////////////////////////////////////////////////////////////////////

    startTimer(16);
}

View::~View()
{
}

void View::bind(std::string str, const std::function<void()> &handler)
{
    ///@todo Add more keys (and key sequences)
    static const std::vector<std::string> c_special_keys{
        "f1",     "f2",  "f3",        "f4",     "f5",    "f6",   "f7",
        "f8",     "f9",  "f10",       "f11",    "f12",   "home", "pgup",
        "pgdown", "end", "backspace", "delete", "up",    "left", "down",
        "right",  "tab", "space",     "return", "enter", "pause"};

    std::transform(std::begin(str), std::end(str), std::begin(str), ::tolower);

    QKeySequence key_sequence = QKeySequence::fromString(str.c_str());

    if (((str.size() != 1 || !isprint(str[0])) &&
         std::find(std::begin(c_special_keys), std::end(c_special_keys), str) ==
             std::end(c_special_keys)) ||
        key_sequence[0] == Qt::Key_unknown || str[0] == '`')
    {
        m_console << "error: invalid key\n";
        return;
    }

    QAction *action;

    if (m_actions.find(key_sequence) != m_actions.end())
    {
        action = m_actions[key_sequence].first;
        action->setText(str.c_str());
        QObject::disconnect(m_actions[key_sequence].second);
    }
    else
    {
        action = new QAction(str.c_str(), this);
        addAction(action);
    }

    action->setShortcut(key_sequence);

    QMetaObject::Connection connection =
        QObject::connect(action, &QAction::triggered, [=]() {
            if (!m_editing)
            {
                handler();
            }
            else
            {
                QCoreApplication::postEvent(
                    this,
                    new QKeyEvent(
                        QEvent::KeyPress,
                        action->shortcut()[0],
                        Qt::NoModifier,
                        action->shortcut().toString().toLower()));
            }
        });

    m_actions[key_sequence] = std::make_pair(action, connection);
}

void View::unbind(const std::string &str)
{
    QKeySequence key_sequence = QKeySequence::fromString(str.c_str());

    const auto &it = m_actions.find(key_sequence);
    if (it != m_actions.end())
    {
        QObject::disconnect(it->second.second);
    }
}

void View::renderImage(const std::string &file_name)
{
    static constexpr int c_border = 200;

    int w = m_max.x() - m_min.x() + c_border;
    int h = m_max.y() - m_min.y() + c_border;

    QImage image(w, h, QImage::Format_RGB32);

    QPainter p(&image);
    QPointF translation =
        image.rect().center() - ((m_min + m_max) / 2).toPointF();
    render(p, image.rect(), translation);

    image.save(file_name.c_str());
}

GraphicsObjectPtr View::getSelectedObject() const
{
    return m_selected_object;
}

void View::deselect()
{
    m_selected_object = nullptr;
}

qconsole::QConsole &View::getConsole()
{
    return m_console;
}

void View::toggleFullscreen()
{
    isFullScreen() ? showNormal() : showFullScreen();
}

void View::toggleAntialiasing()
{
    m_antialias = !m_antialias;
}

void View::toggleRun()
{
    m_run = !m_run;
}

void View::run()
{
    m_run = true;
}

void View::stop()
{
    m_run = false;
}

void View::reset()
{
    m_translation = QPointF();
    // m_scale = 1;
}

void View::edit()
{
    TransitionGraphicsObjectPtr transition =
        cast<TransitionGraphicsObject>(m_selected_object);
    if (transition)
    {
        m_editing = true;
        transition->setSymbol(-1); ///@ refactor
    }
}

void View::timerEvent(QTimerEvent *)
{
    applyForces();
    tick();
    repaint();
}

void View::resizeEvent(QResizeEvent *)
{
    resizeConsole();
}

void View::mousePressEvent(QMouseEvent *e)
{
    setFocus();

    if (!(e->button() & Qt::LeftButton) && !(e->button() & Qt::RightButton))
    {
        return;
    }

    if (m_selected_object)
    {
        m_selected_object->deselect();
        m_selected_object = nullptr;
    };

    QVector2D pos(e->pos() - m_translation - rect().center());

    m_selected_object = m_controller.objectAt(pos);
    if (m_selected_object)
    {
        m_selected_object->select();
    }

    if (e->button() & Qt::LeftButton)
    {
        if (m_selected_object)
        {
            m_moving = true;
        }
        else
        {
            m_translating = true;
        }
    }
    else if (
        e->button() & Qt::RightButton &&
        !cast<TransitionGraphicsObject>(m_selected_object))
    {
        StateGraphicsObjectPtr state =
            cast<StateGraphicsObject>(m_selected_object);
        if (state)
        {
            m_selected_object->deselect();

            m_selected_object = m_controller.createTransition(state, pos);
            m_selected_object->select();

            m_moving = true;
        }
        else
        {
            m_selected_object = m_controller.createState(pos);
            m_selected_object->select();
        }
    }

    m_last_pos = e->pos();
}

void View::mouseReleaseEvent(QMouseEvent *e)
{
    TransitionGraphicsObjectPtr transition =
        cast<TransitionGraphicsObject>(m_selected_object);

    if (e->button() & Qt::RightButton && m_moving && transition)
    {
        QVector2D pos(e->pos() - m_translation - rect().center());
        StateGraphicsObjectPtr end = m_controller.stateAt(pos);

        if (m_selected_object)
        {
            m_selected_object->deselect();
            m_selected_object = nullptr;
        }

        transition->select();
        m_selected_object = transition;

        if (!end)
        {
            end = m_controller.createState(pos);
        }

        m_controller.connectTransition(transition, end);
    }

    m_translating = false;
    m_moving = false;
}

void View::mouseMoveEvent(QMouseEvent *e)
{
    QPointF delta = e->pos() - m_last_pos;

    if (m_translating)
    {
        m_translation += delta;
    }
    else if (m_moving)
    {
        m_selected_object->move(QVector2D(delta));
    }

    m_last_pos = e->pos();
}

void View::wheelEvent(QWheelEvent *e)
{
    if (!hasFocus())
    {
        return;
    }

    if (e->delta() > 0)
    {
        m_scale *= 1.1;
    }
    else
    {
        m_scale /= 1.1;
    }
}

void View::keyPressEvent(QKeyEvent *e)
{
    TransitionGraphicsObjectPtr transition =
        cast<TransitionGraphicsObject>(m_selected_object);

    if (m_editing)
    {
        if ((e->key() >= Qt::Key_A && e->key() <= Qt::Key_Z) ||
            (e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9) ||
            e->key() == Qt::Key_Space)
        {
            if (e->key() == Qt::Key_Space)
            {
                transition->setSymbol('\0');
            }
            else
            {
                transition->setSymbol(e->text().toUtf8().data()[0]);
            }
            m_editing = false;
            return;
        }
    }

    switch (e->key())
    {
    case Qt::Key_Escape:
        if (m_editing)
        {
            m_editing = false;
            transition->setSymbol('\0');
        }
        else
        {
            m_console_visible ? toggleConsole()
                              : isFullScreen() ? showNormal() : qApp->quit();
        }
        break;

    case Qt::Key_QuoteLeft:
        toggleConsole();
        break;
    }
}

void View::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setClipRect(rect());
    render(p, rect(), m_translation + rect().center());
}

void View::render(QPainter &p, const QRect &rect, const QPointF &translation)
{
    p.fillRect(rect, Qt::lightGray);

    p.translate(translation);

    if (m_antialias)
    {
        p.setRenderHint(QPainter::Antialiasing);
    }

    static constexpr int c_num_passes = 3;
    for (int pass = 0; pass < c_num_passes; pass++)
    {
        for (GraphicsObjectPtr obj : m_controller.getObjects())
        {
            obj->render(p, pass);
        }
    }
}

void View::interact(GraphicsObjectPtr a, GraphicsObjectPtr b, bool attract)
{
    static constexpr float c_edge_length = 25;
    const float anti_gravity = 100 * m_scale;

    QVector2D v(a->getPos() - b->getPos());
    float dist = v.length();
    v.normalize();

    float power;
    if (attract)
    {
        power = std::abs(dist / c_edge_length - 1);
    }
    else
    {
        power = anti_gravity / (dist > 0 ? dist : 1);
    }

    QVector2D force;

    if (attract)
    {
        force = v * power;

        if (dist < c_edge_length)
        {
            force *= -1;
        }
    }
    else
    {
        static constexpr float c_max_repulsion = 10;
        force = -v * std::min(power, c_max_repulsion);
    }

    if (!a->isSelected() || !m_moving)
    {
        a->applyForce(-force);
    }

    if (!b->isSelected() || !m_moving)
    {
        b->applyForce(force);
    }
}

void View::applyForces()
{
    if (!m_run)
    {
        return;
    }

    for (GraphicsObjectPtr a : m_controller.getObjects())
    {
        TransitionGraphicsObjectPtr a_tr = cast<TransitionGraphicsObject>(a);

        if (a_tr && a_tr->getEnd())
        {
            interact(a_tr, a_tr->getStart(), true);
            interact(a_tr, a_tr->getEnd(), true);
        }

        for (GraphicsObjectPtr b : m_controller.getObjects())
        {
            TransitionGraphicsObjectPtr b_tr =
                cast<TransitionGraphicsObject>(b);

            if (a >= b || (a_tr && !a_tr->getEnd()) ||
                (b_tr && !b_tr->getEnd()))
            {
                continue;
            }

            if (a->getTag() == b->getTag())
            {
                interact(b, a, false);
            }
        }
    }
}

void View::tick()
{
    long double float_max = std::numeric_limits<float>::max();
    m_min = QVector2D(float_max, float_max);
    m_max = QVector2D(-float_max, -float_max);

    for (GraphicsObjectPtr obj : m_controller.getObjects())
    {
        obj->tick(Duration(Clock::now() - m_time).count());

        QVector2D pos = obj->getPos();
        m_min = QVector2D(
            std::min(m_min.x(), pos.x()), std::min(m_min.y(), pos.y()));
        m_max = QVector2D(
            std::max(m_max.x(), pos.x()), std::max(m_max.y(), pos.y()));
    }

    m_time = Clock::now();
}

void View::toggleConsole()
{
    QPropertyAnimation *animation =
        new QPropertyAnimation(&m_console, "geometry");
    connect(
        animation, &QPropertyAnimation::finished, this, &View::resizeConsole);
    animation->setEasingCurve(QEasingCurve::InOutSine);
    animation->setDuration(200);
    animation->setStartValue(m_console.rect());

    if (m_console_visible)
    {
        animation->setEndValue(QRect(0, 0, width(), 0));
        m_console.clearFocus();
    }
    else
    {
        animation->setEndValue(QRect(0, 0, width(), height() / 3));
        m_console.setFocus();
    }

    animation->start();

    m_console_visible = !m_console_visible;
}

void View::resizeConsole()
{
    m_console.resize(width(), m_console_visible ? height() / 3 : 0);
}

} // namespace fsmviz

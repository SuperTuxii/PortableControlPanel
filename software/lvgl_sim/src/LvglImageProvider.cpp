#include "LvglImageProvider.h"
#include "LvglRenderer.h"

static void mouseRead(lv_indev_t* device, lv_indev_data_t* data);
static void keyboardRead(lv_indev_t* device, lv_indev_data_t* data);

LvglImageProvider::LvglImageProvider(LvglRenderer& renderer) :
        QQuickImageProvider(Pixmap),
        renderer(renderer),
        mouseArea(nullptr),
        key() {
    keyboardDevice = lv_indev_create();
    lv_indev_set_type(keyboardDevice, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_user_data(keyboardDevice, this);
    lv_indev_set_read_cb(keyboardDevice, &keyboardRead);

    auto* default_group = lv_group_create();
    lv_group_set_default(default_group);
    lv_indev_set_group(keyboardDevice, default_group);

    mouseDevice = lv_indev_create();
    lv_indev_set_type(mouseDevice, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(mouseDevice, this);
    lv_indev_set_read_cb(mouseDevice, &mouseRead);
}

LvglImageProvider::~LvglImageProvider() {
    lv_indev_delete(keyboardDevice);
    lv_indev_delete(mouseDevice);
}

QPixmap LvglImageProvider::requestPixmap(const QString&, QSize* size, const QSize& requestedSize) {
    if (size != nullptr)
        *size = { renderer.getWidth(), renderer.getHeight() };

    lv_tick_inc(renderer.getTickPeriodMs());
    lv_task_handler();

    return renderer.getPixmap().scaled(
        requestedSize.width() > 0 ? requestedSize.width() : renderer.getWidth(),
        requestedSize.height() > 0 ? requestedSize.height() : renderer.getHeight());
}

void LvglImageProvider::setMouseArea(QObject* mouseAreaObj) {
    mouseArea = mouseAreaObj;
}

QPointF LvglImageProvider::getMousePosition() const {
    QPointF position{};
    if (mouseArea != nullptr) {
        auto xScale = renderer.getWidth() / mouseArea->property("width").toFloat();
        auto yScale = renderer.getHeight() / mouseArea->property("height").toFloat();
        position.setX(mouseArea->property("mouseX").toFloat() * xScale);
        position.setY(mouseArea->property("mouseY").toFloat() * yScale);
    }
    return position;
}

bool LvglImageProvider::isMousePressed() const {
    bool isPressed = false;
    if (mouseArea != nullptr) {
        isPressed = mouseArea->property("pressedButtons").toInt() != 0;
    }
    return isPressed;
}

Qt::Key LvglImageProvider::getKey() const {
    return key;
}

void LvglImageProvider::onKeyEvent(Qt::Key keyVal, bool isPressed) {
    if (isPressed) {
        key = keyVal;
    } else {
        key = {};
    }
}

static void mouseRead(lv_indev_t* device, lv_indev_data_t* data) {
    auto* view = static_cast<LvglImageProvider*>(lv_indev_get_user_data(device));
    auto  mousePoint = view->getMousePosition();
    data->point.x = mousePoint.x();
    data->point.y = mousePoint.y();
    data->state = view->isMousePressed() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

static void keyboardRead(lv_indev_t* device, lv_indev_data_t* data) {
    auto* view = static_cast<LvglImageProvider*>(lv_indev_get_user_data(device));
    data->key = LvglRenderer::toAscii(view->getKey());
    data->state = (data->key == 0) ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
}

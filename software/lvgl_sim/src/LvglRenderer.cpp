#include "LvglRenderer.h"

static void lvglFlushCallback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map);

LvglRenderer::LvglRenderer(QObject *parent) :
        QObject(parent),
        width(800),
        height(480),
        tickPeriodMs(16),
        displayBufferRatio(10),
        displayFrame1(nullptr),
        displayFrame2(nullptr),
        currentFrame(nullptr),
        display(nullptr),
        screen(nullptr),
        name(QStringLiteral("LvglImageProvider")),
        lvglImageProvider(nullptr) {
}

LvglRenderer::~LvglRenderer() {
    if (lvglImageProvider && engine)
        engine->removeImageProvider(name);
    if (screen)
        lv_obj_delete(screen);
    if (display)
        lv_display_delete(display);
    free(currentFrame);
    free(displayFrame2);
    free(displayFrame1);
}

void LvglRenderer::componentComplete() {
    size_t frameBufferSize = width * height * BYTES_PER_PIXEL / displayBufferRatio;
    displayFrame1 = static_cast<uint8_t*>(malloc(frameBufferSize));
    displayFrame2 = static_cast<uint8_t*>(malloc(frameBufferSize));
    currentFrame = static_cast<lv_color32_t*>(malloc(width * height * BYTES_PER_PIXEL));
    if (!displayFrame1 || !displayFrame2 || !currentFrame)
        return;
    image = QImage(reinterpret_cast<uint8_t*>(currentFrame), width, height, IMAGE_FORMAT);
    if (!lv_is_initialized())
        lv_init();
    lv_lock();
    display = lv_display_create(width, height);
    lv_display_set_user_data(display, this);
    lv_display_set_buffers(display, displayFrame1, displayFrame2, frameBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, &lvglFlushCallback);
    lv_display_set_default(display);
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_screen_load(screen);
    engine = qmlEngine(this);
    lvglImageProvider = new LvglImageProvider(*this);
    engine->addImageProvider(name, lvglImageProvider);
    lv_unlock();
    emit propertiesInitialized();
}

void LvglRenderer::flush(const lv_display_t*, const lv_area_t* area, const uint8_t* px_map) const {
    const auto areaWidth = lv_area_get_width(area);
    const auto lineWidthBytes = areaWidth * BYTES_PER_PIXEL;

    for (auto y = area->y1; y <= area->y2; ++y) {
        memcpy(currentFrame + ((y * width) + area->x1), px_map, lineWidthBytes);
        px_map += lineWidthBytes;
    }
}

uint32_t LvglRenderer::toAscii(Qt::Key key) {
    uint32_t ascii = 0;
    switch (key) {
    case Qt::Key_Up:
        ascii = LV_KEY_UP;
        break;
    case Qt::Key_Down:
        ascii = LV_KEY_DOWN;
        break;
    case Qt::Key_Right:
        ascii = LV_KEY_RIGHT;
        break;
    case Qt::Key_Left:
        ascii = LV_KEY_LEFT;
        break;
    case Qt::Key_Escape:
        ascii = LV_KEY_ESC;
        break;
    case Qt::Key_Delete:
        ascii = LV_KEY_DEL;
        break;
    case Qt::Key_Backspace:
        ascii = LV_KEY_BACKSPACE;
        break;
    case Qt::Key_Enter:
        ascii = LV_KEY_ENTER;
        break;
    case Qt::Key_Tab:
        ascii = LV_KEY_NEXT;
        break;
    case Qt::Key_Home:
        ascii = LV_KEY_HOME;
        break;
    case Qt::Key_End:
        ascii = LV_KEY_END;
        break;
    default:
        ascii = key;
        break;
    }
    return ascii;
}

static void lvglFlushCallback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map) {
    auto* renderer = static_cast<LvglRenderer*>(lv_display_get_user_data(display));
    renderer->flush(display, area, px_map);
    lv_display_flush_ready(display);
}

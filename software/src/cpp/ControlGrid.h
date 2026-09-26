#ifndef CONTROLPANELSOFTWARE_CONTROLGRID_H
#define CONTROLPANELSOFTWARE_CONTROLGRID_H
#include <QDir>
#include <QObject>
#include <qqmlintegration.h>
#include "lv_control_grid.h"
#include "src/LvglRenderer.h"

class ControlGrid : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(LvglRenderer* lvglRenderer READ getLvglRenderer WRITE setLvglRenderer)
    Q_PROPERTY(int32_t controlGridWidth READ getCGWidth NOTIFY sizeChanged)
    Q_PROPERTY(int32_t controlGridHeight READ getCGHeight NOTIFY sizeChanged)
    Q_PROPERTY(uint8_t rows READ getRowCount NOTIFY layoutChanged)
    Q_PROPERTY(uint8_t columns READ getColumnCount NOTIFY layoutChanged)
    Q_PROPERTY(int32_t outerPad READ getOuterPad WRITE setOuterPad NOTIFY outerPadChanged)
    Q_PROPERTY(int32_t rowPad READ getRowPad WRITE setRowPad NOTIFY rowPadChanged)
    Q_PROPERTY(int32_t columnPad READ getColumnPad WRITE setColumnPad NOTIFY columnPadChanged)

    LvglRenderer *lvglRenderer;
    lv_control_grid_t *controlGrid;
    QString loadedImages[256];

    void reinitControlGrid();
public:
    explicit ControlGrid(QObject *parent = nullptr);
    ~ControlGrid() override;

    static bool parseStyleElement(const QVariant &styleElement, uint8_t *&buffer, const uint8_t *bufferEnd, int &part);

    [[nodiscard]] LvglRenderer* getLvglRenderer() const { return lvglRenderer; }
    [[nodiscard]] int32_t getCGWidth() const;
    [[nodiscard]] int32_t getCGHeight() const;
    [[nodiscard]] uint8_t getRowCount() const;
    [[nodiscard]] uint8_t getColumnCount() const;
    [[nodiscard]] int32_t getOuterPad() const;
    [[nodiscard]] int32_t getRowPad() const;
    [[nodiscard]] int32_t getColumnPad() const;
    void setLvglRenderer(LvglRenderer* lvglRenderer_) {
        if (lvglRenderer_ == nullptr)
            lvglRenderer = nullptr;
        else if (lvglRenderer == nullptr) {
            lvglRenderer = lvglRenderer_;
            lv_lock();
            controlGrid = lv_control_grid_create(lvglRenderer->getScreen());
            lv_unlock();
            connect(lvglRenderer, &LvglRenderer::displaySizeRefreshed, this, &ControlGrid::reinitControlGrid);
        }
        emit sizeChanged();
        emit layoutChanged();
        emit outerPadChanged();
        emit rowPadChanged();
        emit columnPadChanged();
    }
    void setOuterPad(int32_t pad);
    void setRowPad(int32_t pad);
    void setColumnPad(int32_t pad);
public slots:
    void setScreenStyle(lv_style_selector_t styleSelector, const QJSValue& data);
    void removeScreenStyle(lv_style_selector_t styleSelector) const;
    void removeScreenStyles() const;
    void setLayout(int rows, int columns);
    void testFill() const;
    void clear() const;
    void move(uint8_t fromIndex, uint8_t toIndex) const;
    void changeSize(uint8_t index, uint8_t index2) const;
    void remove(uint8_t index, uint8_t subIndex) const;
    int16_t findImage(const QString& key) const;
    int16_t loadImage(const QString& key, int16_t index = -1);
    void loadImages(const QSet<QString>& data);
    void loadImages(QStringList data) {
        loadImages(QSet(data.begin(), data.end()));
    }
    void removeUnusedImages(const QSet<QString>& data);
    void removeUnusedImages(QStringList data) {
        removeUnusedImages(QSet(data.begin(), data.end()));
    }
    void removeImage(const QString& key);
    void clearImages();
    bool addWidget(const QString& type, uint8_t index, uint8_t index2) const;
    bool addButton(uint8_t index, uint8_t index2) const;
    uint8_t subWidget(const QString& type, uint8_t index, uint8_t subIndex, const QJSValue& data);
    uint8_t subText(uint8_t index, uint8_t subIndex, const QString& text) const;
    uint8_t subImage(uint8_t index, uint8_t subIndex, const QString& key);
    void setStyle(uint8_t index, uint8_t subIndex, lv_style_selector_t styleSelector, const QJSValue& data);
    void removeStyle(uint8_t index, uint8_t subIndex, lv_style_selector_t styleSelector) const;
    void removeStyles(uint8_t index, uint8_t subIndex) const;

    void insertCoordsData(QJSValue data) const;
signals:
    void sizeChanged();
    void layoutChanged();
    void outerPadChanged();
    void rowPadChanged();
    void columnPadChanged();
};

#endif //CONTROLPANELSOFTWARE_CONTROLGRID_H

#ifndef CONTROLPANELSOFTWARE_CONTROLGRID_H
#define CONTROLPANELSOFTWARE_CONTROLGRID_H
#include <QObject>
#include <qqmlintegration.h>

#include "Connection.h"
#include "src/LvglRenderer.h"
extern "C" {
#include "lv_control_grid.h"
}

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
private:
    LvglRenderer *lvglRenderer;
    lv_control_grid_t *controlGrid;
public:
    explicit ControlGrid(QObject *parent = nullptr);
    ~ControlGrid() override;

    static bool parseStyleElement(const QJSValue &styleElement, uint8_t *&buffer, const uint8_t *bufferEnd, int &part);

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
    void setLayout(int rows, int columns);
    void testFill() const;
    void clear() const;
    void move(uint8_t fromIndex, uint8_t toIndex) const;
    void changeSize(uint8_t index, uint8_t index2) const;
    void remove(uint8_t index, uint8_t subIndex) const;
    // TODO: Add Image Methods
    bool addWidget(const QString& type, uint8_t index, uint8_t index2) const;
    bool addButton(uint8_t index, uint8_t index2) const;
    void subText(uint8_t index, uint8_t subIndex, const QString& text) const;
    void subImage(uint8_t index, uint8_t subIndex, uint8_t imageIndex) const;
    void setStyle(uint8_t index, uint8_t subIndex, lv_style_selector_t styleSelector, const QJSValue& data) const;

    void setupDragTarget(QJSValue data) const;
signals:
    void sizeChanged();
    void layoutChanged();
    void outerPadChanged();
    void rowPadChanged();
    void columnPadChanged();
};

#endif //CONTROLPANELSOFTWARE_CONTROLGRID_H

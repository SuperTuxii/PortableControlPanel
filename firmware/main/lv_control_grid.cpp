#import "lv_control_grid.h"

#include <cstring>

LVControlGrid::LVControlGrid(lv_obj_t* parent, uint8_t rows, uint8_t columns, int32_t outerPad, int32_t rowPad,
                             int32_t columnPad, bool doTestFill)
            : rowDsc(nullptr), columnDsc(nullptr), outerPad(outerPad), rowPad(rowPad), columnPad(columnPad) {
    gridContainer = lv_obj_create(parent);
    rowCount = 0;
    columnCount = 0;
    lv_obj_null_on_delete(&gridContainer);
    // lv_obj_add_event_cb(gridContainer, freeControlGridOnDelete, LV_EVENT_DELETE, active);
    lv_obj_set_scrollbar_mode(gridContainer, LV_SCROLLBAR_MODE_OFF);
    setLayout(rows, columns);
    if (doTestFill) testFill();
}

LVControlGrid::~LVControlGrid() {
    if (gridContainer) {
        clear();
        lv_obj_delete(gridContainer);
    }
}

void LVControlGrid::updateGridSize() const {
    const lv_obj_t *screen = lv_obj_get_parent(gridContainer);
    const int cell_width = ((lv_obj_get_width(screen) - (2 * outerPad) + columnPad) / columnCount) - columnPad;
    const int cell_height = ((lv_obj_get_height(screen) - (2 * outerPad) + rowPad) / rowCount) - rowPad;
    const int cell_size = cell_height < cell_width ? cell_height : cell_width;
    lv_obj_set_size(gridContainer, ((cell_size + columnPad) * columnCount) - columnPad + (2 * outerPad), ((cell_size + rowPad) * rowCount) - rowPad + (2 * outerPad));
}
void LVControlGrid::setLayout(const uint8_t rows, const uint8_t columns) {
    clear();
    if (rowDsc != nullptr)
        free(rowDsc);
    if (columnDsc != nullptr)
        free(columnDsc);
    rowDsc = static_cast<lv_coord_t*>(malloc((rows + 1) * sizeof(lv_coord_t)));
    columnDsc = static_cast<lv_coord_t*>(malloc((columns + 1) * sizeof(lv_coord_t)));
    rowCount = rows;
    columnCount = columns;

    for (int i = 0; i < rows; ++i) {
        rowDsc[i] = LV_GRID_FR(1);
    }
    rowDsc[rows] = LV_GRID_TEMPLATE_LAST;
    for (int i = 0; i < columns; ++i) {
        columnDsc[i] = LV_GRID_FR(1);
    }
    columnDsc[columns] = LV_GRID_TEMPLATE_LAST;
    lv_obj_set_grid_dsc_array(gridContainer, columnDsc, rowDsc);

    updateGridSize();
    lv_obj_center(gridContainer);
    lv_obj_set_style_bg_opa(gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(gridContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(gridContainer, outerPad, LV_PART_MAIN);
    lv_obj_set_style_pad_row(gridContainer, rowPad, LV_PART_MAIN);
    lv_obj_set_style_pad_column(gridContainer, columnPad, LV_PART_MAIN);
}
void LVControlGrid::setOuterPad(int32_t outerPad) {}
void LVControlGrid::setRowPad(int32_t rowPad) {}
void LVControlGrid::setColumnPad(int32_t columnPad) {}

void LVControlGrid::testFill() {}
void LVControlGrid::clear() {}

void LVControlGrid::move(uint8_t fromIndex, uint8_t toIndex) {}
void LVControlGrid::changeSize(uint8_t index, uint8_t index2) {}
void LVControlGrid::remove(uint8_t index, uint8_t subIndex) {}

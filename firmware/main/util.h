#ifndef CONTROLPANELFIRMWARE_UTIL_H
#define CONTROLPANELFIRMWARE_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_control_grid.h"

int32_t convertDataToInt32(const uint8_t *data);
uint16_t convertDataToUInt16(const uint8_t *data);
void alignPivotForScaleForced(const lv_obj_t *object, lv_style_t *style, lv_style_selector_t styleSelector);
void alignPivotForScale(const lv_obj_t *object, lv_style_t *style, lv_style_selector_t styleSelector);
void alignPivotForScaleCB(lv_event_t *event);
void alignPivotForScaleForcedCB(lv_event_t *event);
void freeUserDataOnDelete(lv_event_t *event);
void addUserData(lv_obj_t *object, lv_user_data_type_t type, void *data);
void removeUserData(lv_user_data_t *list, lv_user_data_node_t *node);
void removeStyle(lv_obj_t *object, lv_style_selector_t styleSelector);
void removeStyles(lv_obj_t *object);
lv_style_t *getStyleData(lv_obj_t *object, lv_style_selector_t styleSelector, bool createIfNotFound);
lv_grad_dsc_t *getBgGradDsc(lv_obj_t *object, lv_style_t *style, lv_style_selector_t styleSelector, bool createIfNotFound);
uint8_t *handle_style_data(const lv_control_grid_t *cg, lv_obj_t *object, lv_style_t **style, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //CONTROLPANELFIRMWARE_UTIL_H

#include "lvgl.h"
#include <string.h>
#ifndef ESP_PLATFORM
#include <stdlib.h>
#endif
#include "lv_control_grid.h"
#include "protocol.h"
#include "util.h"

int32_t inline convertDataToInt32(const uint8_t *data) {
    return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

uint16_t inline convertDataToUInt16(const uint8_t *data) {
    return data[0] << 8 | data[1];
}

void alignPivotForScaleForced(const lv_obj_t *object, lv_style_t *style, const lv_style_selector_t styleSelector) {
    const lv_align_t align = lv_obj_get_style_align(object, styleSelector);
    if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_BOTTOM_LEFT)
        lv_style_set_transform_pivot_x(style, 0);
    if (align == LV_ALIGN_TOP_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_BOTTOM_MID)
        lv_style_set_transform_pivot_x(style, lv_obj_get_width(object) / 2);
    if (align == LV_ALIGN_TOP_RIGHT || align == LV_ALIGN_RIGHT_MID || align == LV_ALIGN_BOTTOM_RIGHT)
        lv_style_set_transform_pivot_x(style, lv_obj_get_width(object));
    if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_TOP_MID || align == LV_ALIGN_TOP_RIGHT)
        lv_style_set_transform_pivot_y(style, 0);
    if (align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_RIGHT_MID)
        lv_style_set_transform_pivot_y(style, lv_obj_get_height(object) / 2);
    if (align == LV_ALIGN_BOTTOM_LEFT || align == LV_ALIGN_BOTTOM_MID || align == LV_ALIGN_BOTTOM_RIGHT)
        lv_style_set_transform_pivot_y(style, lv_obj_get_height(object));
}

void alignPivotForScale(const lv_obj_t *object, lv_style_t *style, const lv_style_selector_t styleSelector) {
    const lv_align_t align = lv_obj_get_style_align(object, styleSelector);
    if (lv_obj_get_style_transform_scale_x(object, styleSelector) != LV_SCALE_NONE) {
        if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_BOTTOM_LEFT)
            lv_style_set_transform_pivot_x(style, 0);
        if (align == LV_ALIGN_TOP_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_BOTTOM_MID)
            lv_style_set_transform_pivot_x(style, lv_obj_get_width(object) / 2);
        if (align == LV_ALIGN_TOP_RIGHT || align == LV_ALIGN_RIGHT_MID || align == LV_ALIGN_BOTTOM_RIGHT)
            lv_style_set_transform_pivot_x(style, lv_obj_get_width(object));
    }
    if (lv_obj_get_style_transform_scale_y(object, styleSelector) != LV_SCALE_NONE) {
        if (align == LV_ALIGN_TOP_LEFT || align == LV_ALIGN_TOP_MID || align == LV_ALIGN_TOP_RIGHT)
            lv_style_set_transform_pivot_y(style, 0);
        if (align == LV_ALIGN_LEFT_MID || align == LV_ALIGN_CENTER || align == LV_ALIGN_RIGHT_MID)
            lv_style_set_transform_pivot_y(style, lv_obj_get_height(object) / 2);
        if (align == LV_ALIGN_BOTTOM_LEFT || align == LV_ALIGN_BOTTOM_MID || align == LV_ALIGN_BOTTOM_RIGHT)
            lv_style_set_transform_pivot_y(style, lv_obj_get_height(object));
    }
}

void alignPivotForScaleCB(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED) return;
    lv_obj_t *object = lv_event_get_target(event);
    alignPivotForScale(object, getStyleData(object, 0, true), 0);
    for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
        alignPivotForScale(object, getStyleData(object, selector, true), selector);
    }
}

void alignPivotForScaleForcedCB(lv_event_t *event) {
    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED) return;
    lv_obj_t *object = lv_event_get_target(event);
    alignPivotForScaleForced(object, getStyleData(object, 0, true), 0);
    for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
        alignPivotForScaleForced(object, getStyleData(object, selector, true), selector);
    }
}

void freeUserDataOnDelete(lv_event_t *event) {
    lv_obj_t *object = lv_event_get_target(event);
    lv_user_data_t *list = lv_obj_get_user_data(object);
    if (lv_event_get_code(event) != LV_EVENT_DELETE || list == nullptr) return;
    lv_user_data_node_t *node = list->head;
    while (node != nullptr) {
        lv_user_data_node_t* next = node->next;
        free(node->data);
        free(node);
        node = next;
    }
    free(list);
    lv_obj_set_user_data(object, nullptr);
}

void removeUserData(lv_user_data_t *list, lv_user_data_node_t *node) {
    if (node->prev)
        node->prev->next = node->next;
    if (node->next)
        node->next->prev = node->prev;
    if (list->head == node)
        list->head = node->next;
    if (list->tail == node)
        list->tail = node->prev;
    free(node->data);
    free(node);
}

void removeStyle(lv_obj_t *object, const lv_style_selector_t styleSelector) {
    lv_user_data_t *userData = lv_obj_get_user_data(object);
    if (userData != nullptr) {
        lv_user_data_node_t *node = userData->head;
        while (node != nullptr) {
            if (node->type == LV_USER_DATA_TYPE_STYLE_DATA) {
                lv_user_data_style_t *styleData = node->data;
                if (styleSelector == styleData->selector) {
                    lv_obj_remove_style(object, &styleData->style, LV_PART_ANY | LV_STATE_ANY);
                    lv_style_reset(&styleData->style);
                    lv_user_data_node_t *next = node->next;
                    removeUserData(userData, node);
                    node = next;
                    continue;
                }
            } else if (node->type == LV_USER_DATA_TYPE_GRAD_DSC) {
                lv_user_data_grad_dsc_t *gradDscData = node->data;
                if (styleSelector == gradDscData->selector) {
                    lv_user_data_node_t *next = node->next;
                    removeUserData(userData, node);
                    node = next;
                    continue;
                }
            }
            node = node->next;
        }
    }
}

void removeStyles(lv_obj_t *object) {
    lv_user_data_t *userData = lv_obj_get_user_data(object);
    if (userData != nullptr) {
        lv_user_data_node_t *node = userData->head;
        while (node != nullptr) {
            if (node->type == LV_USER_DATA_TYPE_STYLE_DATA) {
                lv_user_data_style_t *styleData = node->data;
                lv_obj_remove_style(object, &styleData->style, LV_PART_ANY | LV_STATE_ANY);
                lv_style_reset(&styleData->style);
                lv_user_data_node_t *next = node->next;
                removeUserData(userData, node);
                node = next;
                continue;
            }
            if (node->type == LV_USER_DATA_TYPE_GRAD_DSC) {
                lv_user_data_node_t *next = node->next;
                removeUserData(userData, node);
                node = next;
                continue;
            }
            node = node->next;
        }
    }
}

void addUserData(lv_obj_t *object, const lv_user_data_type_t type, void *data) {
    lv_user_data_t *list = lv_obj_get_user_data(object);
    if (list == nullptr) {
        list = malloc(sizeof(lv_user_data_t));
        lv_obj_set_user_data(object, list);
        lv_obj_remove_event_cb(object, freeUserDataOnDelete);
        lv_obj_add_event_cb(object, freeUserDataOnDelete, LV_EVENT_DELETE, nullptr);
        list->head = nullptr;
        list->tail = nullptr;
    }
    lv_user_data_node_t *node = malloc(sizeof(lv_user_data_node_t));
    node->data = data;
    node->type = type;
    node->next = nullptr;
    node->prev = list->tail;
    if (list->tail != nullptr)
        list->tail->next = node;
    else
        list->head = node;
    list->tail = node;
}

lv_style_t *getStyleData(lv_obj_t *object, const lv_style_selector_t styleSelector, const bool createIfNotFound) {
    lv_style_t *style = nullptr;
    const lv_user_data_t *userData = lv_obj_get_user_data(object);
    if (userData != nullptr) {
        const lv_user_data_node_t *node = userData->head;
        while (node != nullptr) {
            if (node->type == LV_USER_DATA_TYPE_STYLE_DATA) {
                lv_user_data_style_t *styleData = node->data;
                if (styleSelector == styleData->selector) return &styleData->style;
            }
            node = node->next;
        }
    }
    if (!createIfNotFound) return nullptr;
    style = malloc(sizeof(lv_style_t));
    lv_user_data_style_t *styleData = malloc(sizeof(lv_user_data_style_t));
    styleData->selector = styleSelector;
    style = &styleData->style;
    lv_style_init(style);
    addUserData(object, LV_USER_DATA_TYPE_STYLE_DATA, styleData);
    lv_obj_add_style(object, style, styleSelector);
    return style;
}

lv_grad_dsc_t *getBgGradDsc(lv_obj_t *object, lv_style_t *style, const lv_style_selector_t styleSelector, const bool createIfNotFound) {
    lv_grad_dsc_t *gradDsc = nullptr;
    const lv_user_data_t *userData = lv_obj_get_user_data(object);
    if (userData != nullptr) {
        const lv_user_data_node_t *node = userData->head;
        while (node != nullptr) {
            if (node->type == LV_USER_DATA_TYPE_GRAD_DSC) {
                lv_user_data_grad_dsc_t *gradDscData = node->data;
                if (styleSelector == gradDscData->selector) return &gradDscData->grad_dsc;
                if (createIfNotFound && gradDsc == nullptr && ((styleSelector & gradDscData->selector) != 0 ||
                    ((gradDscData->selector & 0xFFFF) == 0 && (styleSelector & 0xFF0000) == (gradDscData->selector & 0xFF0000))))
                    gradDsc = &gradDscData->grad_dsc;
            }
            node = node->next;
        }
    }
    if (!createIfNotFound) return nullptr;
    lv_user_data_grad_dsc_t *gradDscData = malloc(sizeof(lv_user_data_grad_dsc_t));
    gradDscData->selector = styleSelector;
    if (gradDsc != nullptr) {
        memcpy(&gradDscData->grad_dsc, gradDsc, sizeof(lv_grad_dsc_t));
        gradDsc = &gradDscData->grad_dsc;
    } else {
        gradDsc = &gradDscData->grad_dsc;
        memset(gradDsc, 0, sizeof(lv_grad_dsc_t));
        gradDsc->stops[0].color = lv_obj_get_style_bg_color(object, styleSelector);
        gradDsc->stops[0].opa = lv_obj_get_style_bg_main_opa(object, styleSelector);
        gradDsc->stops[0].frac = lv_obj_get_style_bg_main_stop(object, styleSelector);
        gradDsc->stops[1].color = lv_obj_get_style_bg_grad_color(object, styleSelector);
        gradDsc->stops[1].opa = lv_obj_get_style_bg_grad_opa(object, styleSelector);
        gradDsc->stops[1].frac = lv_obj_get_style_bg_grad_stop(object, styleSelector);
        gradDsc->stops_count = LV_GRADIENT_MAX_STOPS;
        gradDsc->dir = lv_obj_get_style_bg_grad_dir(object, styleSelector);
        gradDsc->params.radial.end_extent.x = INT32_MIN;
    }
    addUserData(object, LV_USER_DATA_TYPE_GRAD_DSC, gradDscData);
    lv_style_set_bg_grad(style, gradDsc);
    return gradDsc;
}

uint8_t *handle_style_data(const lv_control_grid_t *cg, lv_obj_t *object, lv_style_t **style, lv_style_selector_t *styleSelector, uint8_t *data, uint8_t *end) {
    lv_grad_dsc_t *gradDsc = nullptr;
    const bool considerScale = (*data == ConsiderScale);
    if (considerScale) data++;
    const int styleDataSize = end - data;
    if (*data <= NumberStyleKeyMax) { // Number Keys
        if (styleDataSize < 4) goto styleFormatError;
        int32_t styleData = convertDataToInt32(data + 1);
        if (considerScale) {
            if (*data == PadTop || *data == PadBottom || *data == MarginTop || *data == MarginBottom ||
                *data == ShadowOffsetY || *data == TextLineSpace || *data == DropShadowOffsetY || (*data >= Height && *data <= MaxHeight)) {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_y(object, *styleSelector);
            } else {
                styleData = (styleData << 8) / lv_obj_get_style_transform_scale_x(object, *styleSelector);
            }
        }
        switch (*data) {
        case SetStyleSelector:
            lv_obj_refresh_style(object, lv_obj_style_get_selector_part(*styleSelector), LV_STYLE_PROP_ANY);
            *styleSelector = styleData;
            *style = getStyleData(object, *styleSelector, true);
            break;
        case PadAll:
            lv_style_set_pad_all(*style, styleData);
            break;
        case PadTop:
            lv_style_set_pad_top(*style, styleData);
            break;
        case PadBottom:
            lv_style_set_pad_bottom(*style, styleData);
            break;
        case PadLeft:
            lv_style_set_pad_left(*style, styleData);
            break;
        case PadRight:
            lv_style_set_pad_right(*style, styleData);
            break;
        case MarginAll:
            lv_style_set_margin_all(*style, styleData);
            break;
        case MarginTop:
            lv_style_set_margin_top(*style, styleData);
            break;
        case MarginBottom:
            lv_style_set_margin_bottom(*style, styleData);
            break;
        case MarginLeft:
            lv_style_set_margin_left(*style, styleData);
            break;
        case MarginRight:
            lv_style_set_margin_right(*style, styleData);
            break;
        case BorderWidth:
            lv_style_set_border_width(*style, styleData);
            break;
        case OutlineWidth:
            lv_style_set_outline_width(*style, styleData);
            break;
        case OutlinePad:
            lv_style_set_outline_pad(*style, styleData);
            break;
        case ShadowWidth:
            lv_style_set_shadow_width(*style, styleData);
            break;
        case ShadowOffsetX:
            lv_style_set_shadow_offset_x(*style, styleData);
            break;
        case ShadowOffsetY:
            lv_style_set_shadow_offset_y(*style, styleData);
            break;
        case ShadowSpread:
            lv_style_set_shadow_spread(*style, styleData);
            break;
        case TextLetterSpace:
            lv_style_set_text_letter_space(*style, styleData);
            break;
        case TextLineSpace:
            lv_style_set_text_line_space(*style, styleData);
            break;
        case TextOutlineStrokeWidth:
            lv_style_set_text_outline_stroke_width(*style, styleData);
            break;
        case BlurRadius:
            lv_style_set_blur_radius(*style, styleData);
            break;
        case DropShadowRadius:
            lv_style_set_drop_shadow_radius(*style, styleData);
            break;
        case DropShadowOffsetX:
            lv_style_set_drop_shadow_offset_x(*style, styleData);
            break;
        case DropShadowOffsetY:
            lv_style_set_drop_shadow_offset_y(*style, styleData);
            break;
        case Width:
            lv_style_set_width(*style, styleData);
            break;
        case MinWidth:
            lv_style_set_min_width(*style, styleData);
            break;
        case MaxWidth:
            lv_style_set_max_width(*style, styleData);
            break;
        case Height:
            lv_style_set_height(*style, styleData);
            break;
        case MinHeight:
            lv_style_set_min_height(*style, styleData);
            break;
        case MaxHeight:
            lv_style_set_max_height(*style, styleData);
            break;
        case Length:
            lv_style_set_length(*style, styleData);
            break;
        case X:
            lv_style_set_x(*style, styleData);
            break;
        case Y:
            lv_style_set_y(*style, styleData);
            break;
        case TransformWidth:
            lv_style_set_transform_width(*style, styleData);
            break;
        case TransformHeight:
            lv_style_set_transform_height(*style, styleData);
            break;
        case TranslateX:
            lv_style_set_translate_x(*style, styleData);
            break;
        case TranslateY:
            lv_style_set_translate_y(*style, styleData);
            break;
        case TranslateRadial:
            lv_style_set_translate_radial(*style, styleData);
            break;
        case TranslateScale:
            lv_style_set_transform_scale(*style, styleData);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, nullptr);
            alignPivotForScale(object, *style, *styleSelector);
            break;
        case TranslateScaleX:
            lv_style_set_transform_scale_x(*style, styleData);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, nullptr);
            alignPivotForScale(object, *style, *styleSelector);
            break;
        case TranslateScaleY:
            lv_style_set_transform_scale_y(*style, styleData);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, nullptr);
            alignPivotForScale(object, *style, *styleSelector);
            break;
        case TransformRotation:
            lv_style_set_transform_rotation(*style, styleData);
            break;
        case TransformPivotX:
            lv_style_set_transform_pivot_x(*style, styleData);
            break;
        case TransformPivotY:
            lv_style_set_transform_pivot_y(*style, styleData);
            break;
        case TransformSkewX:
            lv_style_set_transform_skew_x(*style, styleData);
            break;
        case TransformSkewY:
            lv_style_set_transform_skew_y(*style, styleData);
            break;
        case Radius:
            lv_style_set_radius(*style, styleData);
            break;
        case RadiusOffset:
            lv_style_set_radial_offset(*style, styleData);
            break;
        case RotarySensitivity:
            lv_style_set_rotary_sensitivity(*style, styleData);
            break;
        case BackgroundGradParams1:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            if (gradDsc->params.radial.focal_extent.x >= gradDsc->params.radial.focal.x)
                gradDsc->params.radial.focal_extent.x = styleData + (gradDsc->params.radial.focal_extent.x - gradDsc->params.radial.focal.x);
            gradDsc->params.linear.start.x = styleData;
            break;
        case BackgroundGradParams2:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            if (gradDsc->params.radial.focal_extent.x >= gradDsc->params.radial.focal.x)
                gradDsc->params.radial.focal_extent.y = styleData;
            gradDsc->params.linear.start.y = styleData;
            break;
        case BackgroundGradLinearEndX:
            getBgGradDsc(object, *style, *styleSelector, true)->params.linear.end.x = styleData;
            break;
        case BackgroundGradLinearEndY:
            getBgGradDsc(object, *style, *styleSelector, true)->params.linear.end.y = styleData;
            break;
        case BackgroundGradRadialEndX:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            if (gradDsc->params.radial.end_extent.x > gradDsc->params.radial.end.x)
                gradDsc->params.radial.end_extent.x = styleData + (gradDsc->params.radial.end_extent.x - gradDsc->params.radial.end.x);
            gradDsc->params.radial.end.x = styleData;
            break;
        case BackgroundGradRadialEndY:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            if (gradDsc->params.radial.end_extent.x > gradDsc->params.radial.end.x)
                gradDsc->params.radial.end_extent.y = styleData;
            gradDsc->params.radial.end.y = styleData;
            break;
        case BackgroundGradRadialFocalRadius:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            styleData = abs(styleData);
            if (styleData == abs(gradDsc->params.radial.end_extent.x - gradDsc->params.radial.end.x)
                && gradDsc->params.radial.focal.x == gradDsc->params.radial.end.x
                && gradDsc->params.radial.focal.y == gradDsc->params.radial.end.y)
                styleData++;
            gradDsc->params.radial.focal_extent.x = gradDsc->params.radial.focal.x + styleData;
            gradDsc->params.radial.focal_extent.y = gradDsc->params.radial.focal.y;
            break;
        case BackgroundGradRadialEndRadius:
            gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
            styleData = abs(styleData);
            if (styleData == 0 || (styleData == abs(gradDsc->params.radial.focal_extent.x - gradDsc->params.radial.focal.x)
                && gradDsc->params.radial.focal.x == gradDsc->params.radial.end.x
                && gradDsc->params.radial.focal.y == gradDsc->params.radial.end.y))
                styleData++;
            gradDsc->params.radial.end_extent.x = gradDsc->params.radial.end.x + styleData;
            gradDsc->params.radial.end_extent.y = gradDsc->params.radial.end.y;
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= ColorOpacityStyleKeyMax) { // Color + Opacity Keys
        if (styleDataSize < 4) goto styleFormatError;
        const uint32_t rawData = (uint32_t) convertDataToInt32(data + 1);
        const lv_color_t color = lv_color_hex(rawData >> 8);
        const lv_opa_t opa = rawData & 0xFF;
        switch (*data) {
        case BackgroundColor:
            lv_style_set_bg_color(*style, color);
            lv_style_set_bg_opa(*style, opa);
            gradDsc = getBgGradDsc(object, *style, *styleSelector, false);
            if (gradDsc != nullptr)
                gradDsc->stops[0].color = color;
            break;
        case BackgroundGradColor:
            lv_style_set_bg_grad_color(*style, color);
            lv_style_set_bg_grad_opa(*style, opa);
            gradDsc = getBgGradDsc(object, *style, *styleSelector, false);
            if (gradDsc != nullptr) {
                gradDsc->stops[1].color = color;
                gradDsc->stops[1].opa = opa;
            }
            break;
        case BackgroundImageRecolor:
            lv_style_set_bg_image_recolor(*style, color);
            lv_style_set_bg_image_recolor_opa(*style, opa);
            break;
        case BorderColor:
            lv_style_set_border_color(*style, color);
            lv_style_set_border_opa(*style, opa);
            break;
        case OutlineColor:
            lv_style_set_outline_color(*style, color);
            lv_style_set_outline_opa(*style, opa);
            break;
        case ShadowColor:
            lv_style_set_shadow_color(*style, color);
            lv_style_set_shadow_opa(*style, opa);
            break;
        case TextColor:
            lv_style_set_text_color(*style, color);
            lv_style_set_text_opa(*style, opa);
            break;
        case TextOutlineStrokeColor:
            lv_style_set_text_outline_stroke_color(*style, color);
            lv_style_set_text_outline_stroke_opa(*style, opa);
            break;
        case DropShadowColor:
            lv_style_set_drop_shadow_color(*style, color);
            lv_style_set_drop_shadow_opa(*style, opa);
            break;
        case Recolor:
            lv_style_set_recolor(*style, color);
            lv_style_set_recolor_opa(*style, opa);
            break;
        case ImageRecolor:
            lv_style_set_image_recolor(*style, color);
            lv_style_set_image_recolor_opa(*style, opa);
            break;
        default: ;
        }
        return data + 5;
    }
    if (*data <= Number16StyleKeyMax) { // Number (16) Keys
        if (styleDataSize < 2) goto styleFormatError;
        const int16_t value = (int16_t) convertDataToUInt16(data + 1);
        switch (*data) {
        case BackgroundGradConicalStartAngle:
            getBgGradDsc(object, *style, *styleSelector, true)->params.conical.start_angle = value;
            break;
        case BackgroundGradConicalEndAngle:
            getBgGradDsc(object, *style, *styleSelector, true)->params.conical.end_angle = value;
            break;
        default: ;
        }
        return data + 3;
    }
    if (*data <= ByteStyleKeyMax) { // Byte Keys
        if (styleDataSize < 1)
            goto styleFormatError;
        const uint8_t value = *(data + 1);
        switch (*data) {
        case BackgroundImageIndex:
            lv_style_set_bg_image_src(*style, cg->images + value);
            break;
        case BackgroundMainOpacity:
            lv_style_set_bg_main_opa(*style, value);
            gradDsc = getBgGradDsc(object, *style, *styleSelector, false);
            if (gradDsc != nullptr)
                gradDsc->stops[0].opa = value;
            break;
        case BackgroundMainStop:
            lv_style_set_bg_main_stop(*style, value);
            gradDsc = getBgGradDsc(object, *style, *styleSelector, false);
            if (gradDsc != nullptr)
                gradDsc->stops[0].frac = value;
            break;
        case BackgroundGradStop:
            lv_style_set_bg_grad_stop(*style, value);
            gradDsc = getBgGradDsc(object, *style, *styleSelector, false);
            if (gradDsc != nullptr)
                gradDsc->stops[1].frac = value;
            break;
        case BackgroundImageOpacity:
            lv_style_set_bg_image_opa(*style, value);
            break;
        case FontSizeScaled:
            lv_style_set_text_font(*style, &lv_font_montserrat_48);
            lv_style_set_transform_scale(*style, (value << 8) / 48);
            lv_obj_remove_event_cb(object, alignPivotForScaleCB);
            lv_obj_add_event_cb(object, alignPivotForScaleCB, LV_EVENT_SIZE_CHANGED, nullptr);
            alignPivotForScale(object, *style, *styleSelector);
            break;
        case Opacity:
            lv_style_set_opa(*style, value);
            break;
        case OpacityLayered:
            lv_style_set_opa_layered(*style, value);
            break;
        case ColorFilterOpacity:
            lv_style_set_color_filter_opa(*style, value);
            break;
        case ImageOpacity:
            lv_style_set_image_opa(*style, value);
            break;
        default: ;
        }
        return data + 2;
    }
    // Non Type Keys
    if (*data >= BackgroundGradDirNone && *data <= BackgroundGradDirConical) {
        lv_style_set_bg_grad_dir(*style, *data - BackgroundGradDirNone);
        if (*data >= BackgroundGradDirLinear) gradDsc = getBgGradDsc(object, *style, *styleSelector, true);
        if (gradDsc != nullptr)
            gradDsc->dir = *data - BackgroundGradDirNone;
        goto nonTypeSwitchEnd;
    }
    if (*data >= BackgroundGradExtendPad && *data <= BackgroundGradExtendReflect) {
        getBgGradDsc(object, *style, *styleSelector, true)->extend = *data - BackgroundGradExtendPad;
        goto nonTypeSwitchEnd;
    }
    if (*data >= TextAlignAuto && *data <= TextAlignRight) {
        lv_style_set_text_align(*style, *data - LV_TEXT_ALIGN_AUTO);
        goto nonTypeSwitchEnd;
    }
    if (*data == BlurBackdropOff || *data == BlurBackdropOn) {
        lv_style_set_blur_backdrop(*style, *data - BlurBackdropOff);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BlurQualityAuto && *data <= BlurQualityPrecision) {
        lv_style_set_blur_quality(*style, *data - BlurQualityAuto);
        goto nonTypeSwitchEnd;
    }
    if (*data >= DropShadowQualityAuto && *data <= DropShadowQualityPrecision) {
        lv_style_set_drop_shadow_quality(*style, *data - DropShadowQualityAuto);
        goto nonTypeSwitchEnd;
    }
    if (*data >= AlignTopLeft && *data <= AlignCenter) {
        lv_style_set_align(*style, *data - AlignTopLeft + 1);
        alignPivotForScale(object, *style, *styleSelector);
        goto nonTypeSwitchEnd;
    }
    if (*data == BackgroundImageTiledOff || *data == BackgroundImageTiledOn) {
        lv_style_set_bg_image_tiled(*style, *data - BackgroundImageTiledOff);
        goto nonTypeSwitchEnd;
    }
    if (*data == ClipCornerOff || *data == ClipCornerOn) {
        lv_style_set_clip_corner(*style, *data - ClipCornerOff);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BlendModeNormal && *data <= BlendModeDifference) {
        lv_style_set_blend_mode(*style, *data - BlendModeNormal);
        goto nonTypeSwitchEnd;
    }
    if (*data >= BaseDirLTR && *data <= BaseDirAuto) {
        lv_style_set_base_dir(*style, *data - BaseDirLTR);
        goto nonTypeSwitchEnd;
    }
    switch (*data) {
    case BorderSideNone:
        lv_style_set_border_side(*style, LV_BORDER_SIDE_NONE);
        break;
    case BorderSideBottom:
        lv_style_set_border_side(*style, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_BOTTOM);
        break;
    case BorderSideTop:
        lv_style_set_border_side(*style, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_TOP);
        break;
    case BorderSideLeft:
        lv_style_set_border_side(*style, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_LEFT);
        break;
    case BorderSideRight:
        lv_style_set_border_side(*style, lv_obj_get_style_border_side(object, *styleSelector) | LV_BORDER_SIDE_RIGHT);
        break;
    case BorderSideFull:
        lv_style_set_border_side(*style, LV_BORDER_SIDE_FULL);
        break;
    case TextDecorNone:
        lv_style_set_text_decor(*style, LV_TEXT_DECOR_NONE);
        break;
    case TextDecorUnderline:
        lv_style_set_text_decor(*style, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_UNDERLINE);
        break;
    case TextDecorStrikethrough:
        lv_style_set_text_decor(*style, lv_obj_get_style_text_decor(object, *styleSelector) | LV_TEXT_DECOR_STRIKETHROUGH);
        break;
    case AlignTransformPivot:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, *style, *styleSelector);
        break;
    case AlignTransformPivotAll:
        lv_obj_update_layout(object);
        alignPivotForScaleForced(object, getStyleData(object, 0, true), 0);
        for (uint32_t selector = 1; selector <= 0x0FFFFF; selector <<= 1) {
            alignPivotForScaleForced(object, getStyleData(object, selector, true), selector);
        }
        break;
    case AlignTransformPivotEvent:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        lv_obj_add_event_cb(object, alignPivotForScaleForcedCB, LV_EVENT_SIZE_CHANGED, nullptr);
        break;
    case AlignTransformPivotAllEvent:
        lv_obj_remove_event_cb(object, alignPivotForScaleForcedCB);
        break;
    case FontMontserrat14:
        lv_style_set_text_font(*style, &lv_font_montserrat_14);
        lv_style_set_transform_scale(*style, 256);
        lv_obj_remove_event_cb(object, alignPivotForScaleCB);
        break;
    case FontMontserrat48:
        lv_style_set_text_font(*style, &lv_font_montserrat_48);
        lv_style_set_transform_scale(*style, 256);
        lv_obj_remove_event_cb(object, alignPivotForScaleCB);
        break;
    case ColorFilterUnset:
        lv_style_set_color_filter_dsc(*style, nullptr);
        break;
    case ColorFilterShade:
        lv_style_set_color_filter_dsc(*style, &lv_color_filter_shade);
        break;
    default: ;
    }
nonTypeSwitchEnd:
    return data + 1;
styleFormatError:
    return end + 2;
}
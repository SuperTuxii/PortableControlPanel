pragma Singleton

import QtCore
import QtQuick

QtObject {
    property Settings settings

    function getStyleKeyDirections(styleKey: int): int {
       if (styleKey === Connection.PadAll || styleKey === Connection.MarginAll)
           return 4;
       if (Connection.styleKeyString(styleKey).endsWith("X")
           || styleKey === Connection.TransformWidth
           || styleKey === Connection.TranslateScale
           || (styleKey >= Connection.BackgroundGradParams1 && styleKey <= Connection.BackgroundGradParams2)
           || (styleKey >= Connection.Width && styleKey <= Connection.MaxWidth))
           return 2;
       return 0;
    }
    function getStyleKeyMin(styleKey: int): int {
        if (styleKey >= Connection.NumberStyleKeyMin && styleKey <= Connection.NumberStyleKeyMax)
            return -(1 << 31);
        if (styleKey >= Connection.Number16StyleKeyMin && styleKey <= Connection.Number16StyleKeyMax)
            return -(1 << 15);
        return 0;
    }
    function getStyleKeyMax(styleKey: int): int {
        if (styleKey >= Connection.NumberStyleKeyMin && styleKey <= Connection.NumberStyleKeyMax)
            return (1 << 31) - 1;
        if (styleKey >= Connection.Number16StyleKeyMin && styleKey <= Connection.Number16StyleKeyMax)
            return (1 << 15) - 1;
        return (1 << 8) - 1;
    }
    function parseIntCalc(string: string, min: int, max: int): variant {
        if (string.length === 0) return undefined;
        string = string.replace(/\s+/g, "");
        if (!/^[\d+\-*/().?:<>=&|!]+$/.test(string))
            return undefined;
        try {
            const value = Math.floor(Function("\"use strict\"; return (" + string + ")")());
            if (isNaN(value) || value < min || value > max) return undefined;
            return value;
        } catch (e) {}
    }
    function parseDirectionsCalc(string: string, directions: int, min: int, max: int): variant {
        if (string.length === 0) return undefined;
        string = string.replace(/\s+/g, "");
        let values = string.split(";");
        if (values.length !== 1 && values.length !== directions && !(directions === 4 && values.length === 2))
            return undefined;
        for (const i in values) {
            values[i] = parseIntCalc(values[i], min, max);
            if (values[i] === undefined) return undefined;
        }
        return values;
    }
    function buildScreenMacros(macros, allStyleData, width: int, height: int, rows: int, columns: int, outerPad: int, rowPad: int, columnPad: int) {
        macros.w = width;
        macros.width = width;
        macros.h = height;
        macros.height = height;
        macros.r = rows;
        macros.rows = rows;
        macros.c = columns;
        macros.columns = columns;
        macros.outerPad = outerPad;
        macros.rowPad = rowPad;
        macros.columnPad = columnPad;
        buildStyleMacros(macros, allStyleData);
    }
    function buildMacros(macros, allStyleData, width: int, height: int, row: int, column: int, rowSpan: int, columnSpan: int) {
        buildDimensionMacros(macros, width, height, row, column, rowSpan, columnSpan);
        buildStyleMacros(macros, allStyleData);
    }
    function buildDimensionMacros(macros, width: int, height: int, row: int, column: int, rowSpan: int, columnSpan: int) {
        macros.w = width;
        macros.width = width;
        macros.h = height;
        macros.height = height;
        macros.r = row;
        macros.row = row;
        macros.c = column;
        macros.column = column;
        macros.rs = rowSpan;
        macros.rowspan = rowSpan;
        macros.cs = columnSpan;
        macros.columnspan = columnSpan;
    }
    function buildSelectStyleMacros(selectStyleMacros, selectStyleData) {
        for (const styleData of selectStyleData) {
            if (!styleData.value) continue;
            if (typeof styleData.value === "number")
                selectStyleMacros[styleData.name.toLowerCase()] = styleData.value;
            else if (styleData.value.length && typeof styleData.value !== "string")
                selectStyleMacros[styleData.name.toLowerCase()] = styleData.value.length === 1 ? styleData.value[0] : styleData.value;
        }
    }
    function buildStyleMacros(macros, allStyleData) {
        macros.style = {};
        for (const styleSelector in allStyleData) {
            macros.style[styleSelector] = {};
            buildSelectStyleMacros(macros.style[styleSelector], allStyleData[styleSelector])
        }
    }
    function combineSizeMacros(macros, width: int, height: int): variant {
        if (!macros) return { w: width, width: width, h: height, height: height };
        const widgetWidth = "parent" in macros ? macros.parent.w : macros.w;
        const widgetHeight = "parent" in macros ? macros.parent.h : macros.h;
        return Object.assign({}, macros, {
            w: width, width: width, h: height, height: height,
            widget: [widgetWidth, widgetHeight],
            ww: widgetWidth, wwidth: widgetWidth,
            wh: widgetHeight, wheight: widgetHeight
        });
    }
    function macroPreprocessor(macros, text: string, currentStyleSelector: int, isDirection = false): string {
        if (!macros) return text;
        if (isDirection)
            text = text.replace(/(\d+p?)%/g, "$1d%");
        return text.replace(/^(.*\d+p?)d%(.*)$/, "$1w%$2;$1h%$2")
            .replace(/(\d+)(p?[wh]|\$[{(][^)}:]+:?[^)}]*[)}])?%/g, (_, digits, modifier) => {
            if (!modifier)
                return `(${digits}/100)*($(w)+$(h))/2`;
            if (modifier === "p")
                return `(${digits}/100)*($(w:p)+$(h:p))/2`;
            return modifier.startsWith("$") ?
                `(${digits}/100)*${modifier}` :
                modifier.startsWith("p") ?
                    `(${digits}/100)*$(${modifier.substring(1)}:p)` :
                    `(${digits}/100)*$(${modifier})`;
        }).replace(/\$[{(]([^)}:]+):?([^)}]*)[)}]/g, (_, key, modifierString) => {
            key = key.toLowerCase();
            let modifiers = {};
            for (let modifier of modifierString.toLowerCase().split(",").map(modifier => modifier.split("="))) {
                if (!modifier[0]) continue;
                if (!modifier[1]) {
                    modifiers[modifier[0]] = "";
                    continue;
                }
                if (modifier[0] === "s" || modifier[0] === "state" || modifier[0] === "p" || modifier[0] === "part")
                    modifier[1] = modifier[1].charAt(0).toUpperCase() + modifier[1].substring(1);
                modifiers[modifier[0]] = modifier[1];
            }
            let styleSelector = currentStyleSelector;
            if ("s" in modifiers || "state" in modifiers) {
                let state = Connection.styleStateFromString("state" in modifiers ? modifiers.state : modifiers.s);
                if (state !== -1)
                    styleSelector = (styleSelector & 0xFF0000) | state;
            }
            if (modifiers.p || "part" in modifiers) {
                let part = Connection.stylePartFromString("part" in modifiers ? modifiers.part : modifiers.p);
                if (part !== -1)
                    styleSelector = (styleSelector & 0xFFFF) | part;
            }
            const usedMacros = ("p" in modifiers && !modifiers.p) || "parent" in modifiers ? macros.parent : macros;
            if (!usedMacros) return "";
            if (key in usedMacros) {
                return usedMacros[key];
            } else if ("style" in usedMacros && styleSelector in usedMacros.style && key in usedMacros.style[styleSelector]) {
                let value = usedMacros.style[styleSelector][key];
                if (typeof value !== "string" && value.length) {
                    if ("x" in modifiers || "0" in modifiers || "v" in modifiers || "vertical" in modifiers
                        || "t" in modifiers || "top" in modifiers)
                        return value[0];
                    else if ("y" in modifiers || "1" in modifiers || "h" in modifiers || "horizontal" in modifiers
                        || "r" in modifiers || "right" in modifiers)
                        return value.length >= 2 ? value[1] : value[0];
                    else if ("2" in modifiers || "b" in modifiers || "bottom" in modifiers)
                        return value.length === 4 ? value[2] : value[0];
                    else if ("3" in modifiers || "l" in modifiers || "left" in modifiers)
                        return value.length === 4 ? value[3] : value.length === 2 ? value[1] : value[0];
                    else
                        return value[0];
                } else {
                    return value;
                }
            } else {
                return "";
            }
        }).replace(/(\d+)°/g, "$1*10")
            .replace(/(\d+)x/g, "$1*256");
    }
    function refreshStyleData(macros, allStyleData): boolean {
        for (let refreshes = 0; refreshes < 10; refreshes++) {
            if (!refreshStyleDataSingle(macros, allStyleData)) return refreshes !== 0;
        }
        tooManyStyleDataRecursions(macros, allStyleData);
        return true;
    }
    function refreshSubWidget(macros, subWidget): boolean {
        let changed = false;
        if ("style" in subWidget)
            changed |= refreshStyleData(macros, subWidget.style);
        if (subWidget.type === "Image" && subWidget.image && subWidget.image.imageKey)
            changed |= refreshImageValue(macros, subWidget.image, 0);
        return changed;
    }
    function refreshImageValue(macros, image, styleSelector): boolean {
        const imageData = settings.loadImage(image.key);
        if ("cropSizeText" in image) {
            const newValue = parseDirectionsCalc(
                macroPreprocessor(
                    Utils.combineSizeMacros(
                        macros,
                        imageData.initialSize[0],
                        imageData.initialSize[1]
                    ),
                    image.cropSizeText, styleSelector, true
                ),
                2, 1, Math.max(imageData.initialSize[0], imageData.initialSize[1])
            );
            if (newValue !== undefined)
                image.cropSize = newValue;
        }
        const cropSize = image.cropSize ?? imageData.cropSize;
        const cropWidth = cropSize[0];
        const cropHeight = cropSize.length === 2 ? cropSize[1] : cropSize[0];
        if ("cropPosText" in image) {
            const newValue = parseDirectionsCalc(
                macroPreprocessor(
                    Utils.combineSizeMacros(
                        macros,
                        imageData.initialSize[0] - cropWidth,
                        imageData.initialSize[1] - cropHeight
                    ),
                    image.cropPosText, styleSelector, true
                ),
                2, 0, Math.max(imageData.initialSize[0], imageData.initialSize[1])
            );
            if (newValue !== undefined)
                image.cropPos = newValue;
        }
        if ("resizeText" in image) {
            const newValue = parseDirectionsCalc(
                macroPreprocessor(
                    Utils.combineSizeMacros(
                        macros,
                        cropWidth,
                        cropHeight
                    ),
                    image.resizeText, styleSelector, true
                ),
                2, 1, Math.max(imageData.initialSize[0], imageData.initialSize[1], 1000)
            );
            if (newValue !== undefined)
                image.resize = newValue;
        }
        const newImageKey = createDefaultedImageKey(image, imageData);
        if (newImageKey !== image.imageKey) {
            image.imageKey = newImageKey;
            return true;
        }
        return false;
    }
    function refreshStyleDataSingle(macros, allStyleData, exceptStyleSelector = -1): boolean {
        let changed = false;
        for (let styleSelector in allStyleData) {
            styleSelector = parseInt(styleSelector);
            if (styleSelector === exceptStyleSelector) continue;
            for (const styleData of allStyleData[styleSelector]) {
                if (!("text" in styleData) && !(typeof styleData.value === "object" && "imageKey" in styleData.value)) continue;
                if (typeof styleData.value === "number") {
                    const newValue = parseIntCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector),
                        getStyleKeyMin(styleData.attrKey),
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (newValue === undefined) continue;
                    if (newValue !== styleData.value) {
                        macros.style[styleSelector][styleData.name.toLowerCase()] = newValue;
                        styleData.value = newValue;
                        changed = true;
                    }
                } else if (styleData.value.length && typeof styleData.value !== "string") {
                    const newValue = parseDirectionsCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector, true),
                        getStyleKeyDirections(styleData.attrKey),
                        getStyleKeyMin(styleData.attrKey),
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (newValue === undefined) continue;
                    if (newValue.length !== styleData.value.length
                        || !newValue.every((value, index) => value === styleData.value[index])) {
                        macros.style[styleSelector][styleData.name.toLowerCase()] = newValue;
                        styleData.value = newValue.length === 1 ? newValue[0] : newValue;
                        changed = true;
                    }
                } else if (typeof styleData.value === "object" && "imageKey" in styleData.value) {
                    changed |= refreshImageValue(macros, styleData.value, styleSelector);
                }
            }
        }
        return changed;
    }
    function tooManyStyleDataRecursions(macros, allStyleData, exceptStyleSelector = -1) {
        for (let styleSelector in allStyleData) {
            styleSelector = parseInt(styleSelector);
            if (styleSelector === exceptStyleSelector) continue;
            for (const styleData of allStyleData[styleSelector]) {
                if (!("text" in styleData)) continue;
                if (typeof styleData.value === "number") {
                    const min = getStyleKeyMin(styleData.attrKey)
                    let newValue = parseIntCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector),
                        min,
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (!newValue) continue;
                    if (newValue !== styleData.value) {
                        delete macros.style[styleSelector][styleData.name.toLowerCase()];
                        styleData.value = min > 0 ? min : 0;
                    }
                } else if (styleData.value.length && typeof styleData.value !== "string") {
                    const min = getStyleKeyMin(styleData.attrKey);
                    let newValue = parseDirectionsCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector, true),
                        getStyleKeyDirections(styleData.attrKey),
                        min,
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (!newValue) continue;
                    if (newValue.length !== styleData.value.length
                        || !newValue.every((value, index) => value === styleData.value[index])) {
                        delete macros.style[styleSelector][styleData.name.toLowerCase()];
                        styleData.value = [min > 0 ? min : 0]
                    }
                }
            }
        }
    }

    function createImageKey(data: variant, allowedAttrs): string {
        return createDefaultedImageKey(data, Object(), allowedAttrs);
    }

    function createDefaultedImageKey(data: variant, imageData = Object(), allowedAttrs = ["cropPos", "cropSize", "resize", "colorFormat"]): string {
        let attributes = [];
        for (const key of new Set(allowedAttrs)) {
            const value = data[key] ?? imageData[key];
            if (!value) continue;
            if (typeof value === "object" && value.length)
                if (value.every(val => value[0] === val))
                    attributes.push(`${key}=${value[0]}`);
                else
                    attributes.push(`${key}=${value.join(";")}`);
            else
                attributes.push(`${key}=${value}`);
        }
        return `${data.image ?? data.key}?${attributes.join(",")}`;
    }

    function overrideImageKey(imageKey: string, overrides: variant): string {
        let attrDelimiter = imageKey.lastIndexOf("?")
        let imageAttrs = imageKey.substring(attrDelimiter + 1);
        for (const key in overrides) {
            let value = overrides[key];
            if (!value) continue;
            if (typeof value === "object" && value.length)
                if (value.every(val => value[0] === val))
                    value = value[0];
                else
                    value = value.join(";");
            const attrPattern = RegExp(`(,${key}=)[^,]*(,|$)`);
            if (attrPattern.test(imageAttrs)) {
                imageAttrs = imageAttrs.replace(attrPattern, `$1${value}$2`);
            } else {
                if (imageAttrs.length > 0)
                    imageAttrs += ",";
                imageAttrs += `${key}=${value}`;
            }
        }
        return imageKey.substring(0, attrDelimiter + 1) + imageAttrs;
    }
}
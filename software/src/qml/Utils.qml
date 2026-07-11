pragma Singleton

import QtQuick

QtObject {
    function getStyleKeyDirections(styleKey: int): int {
       if (styleKey === Connection.PadAll || styleKey === Connection.MarginAll)
           return 4;
       if (Connection.styleKeyString(styleKey).endsWith("X")
           || styleKey === Connection.TransformWidth
           || styleKey === Connection.TranslateScale
           || (styleKey >= Connection.BackgroundGradParams1 && styleKey <= Connection.BackgroundGradParams2))
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
        if (!/^[\d+\-*/().]+$/.test(string))
            return undefined;
        try {
            const value = Function("\"use strict\"; return (" + string + ")")();
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
    function buildStyleMacros(macros, allStyleData) {
        macros.style = {};
        for (const styleSelector in allStyleData) {
            macros.style[styleSelector] = {};
            for (let styleData of allStyleData[styleSelector]) {
                if (typeof styleData.value === "number")
                    macros.style[styleSelector][styleData.name.toLowerCase()] = styleData.value;
                else if (styleData.value.length && typeof styleData.value !== "string")
                    macros.style[styleSelector][styleData.name.toLowerCase()] = styleData.value.length === 1 ? styleData.value[0] : styleData.value;
            }
        }
    }
    function macroPreprocessor(macros, text: string, currentStyleSelector: int): string {
        if (!macros) return text;
        return text.replace(/\$[{(]([^)}:]+):?([^)}]*)[)}]/g, (_, key, modifierString) => {
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
            if ("p" in modifiers || "part" in modifiers) {
                let part = Connection.stylePartFromString("part" in modifiers ? modifiers.part : modifiers.p);
                if (part !== -1)
                    styleSelector = (styleSelector & 0xFFFF) | part;
            }
            if (key in macros) {
                return macros[key];
            } else if ("style" in macros && styleSelector in macros.style && key in macros.style[styleSelector]) {
                let value = macros.style[styleSelector][key];
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
        });
    }
    function refreshStyleData(macros, allStyleData): boolean {
        for (let refreshes = 0; refreshes < 10; refreshes++) {
            if (!refreshStyleDataSingle(macros, allStyleData)) return refreshes !== 0;
        }
        tooManyStyleDataRecursions(macros, allStyleData);
        return true;
    }
    function refreshStyleDataSingle(macros, allStyleData, exceptStyleSelector = -1): boolean {
        let changed = false;
        for (let styleSelector in allStyleData) {
            styleSelector = parseInt(styleSelector);
            if (styleSelector === exceptStyleSelector) continue;
            for (const styleData of allStyleData[styleSelector]) {
                if (!("text" in styleData)) continue;
                if (typeof styleData.value === "number") {
                    let newValue = parseIntCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector),
                        getStyleKeyMin(styleData.attrKey),
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (!newValue) continue;
                    if (newValue !== styleData.value) {
                        macros.style[styleSelector][styleData.name.toLowerCase()] = newValue;
                        styleData.value = newValue;
                        changed = true;
                    }
                } else if (styleData.value.length && typeof styleData.value !== "string") {
                    let newValue = parseDirectionsCalc(
                        macroPreprocessor(macros, styleData.text, styleSelector),
                        getStyleKeyDirections(styleData.attrKey),
                        getStyleKeyMin(styleData.attrKey),
                        getStyleKeyMax(styleData.attrKey)
                    );
                    if (!newValue) continue;
                    if (newValue.length !== styleData.value.length
                        || !newValue.every((value, index) => value === styleData.value[index])) {
                        macros.style[styleSelector][styleData.name.toLowerCase()] = newValue;
                        styleData.value = newValue.length === 1 ? newValue[0] : newValue;
                        changed = true;
                    }
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
                        macroPreprocessor(macros, styleData.text, styleSelector),
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
}
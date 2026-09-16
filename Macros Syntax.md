## Percentage
### Default (`%`)
When a `%` is placed after a number, the expression will become the percentage of the average of width and height (percentage set by the number before `%`). By adding a `p` after the number, the width and height of the parent will be used if available.

If the value is directional, the default `%` will be turned into a `d%`.
### Directional (`d%`)
A `d%` is applied to the whole text value. For the first direction the percentage of the width is used and for the second direction the percentage of the height is used. The operations before and after the expression are copied for both directions. A `p` can also be added after the number to use the width and height of the parent if available.
### Width and Height (`w%` & `h%`)
When added after a number, it will become the percentage of the width for `w%` and the percentage of the height for `h%`. Additionally, by adding a `p` after the number, the width and height of the parent will be used if available.
### Macro (`$(macro)%`)
To apply a percentage to any dollar macro, the macro followed by a `%` can be added after a number representing the percentage.
## Dollar Macro (`$(macro:modifier,modifier=value)`)
Instead of `$(...)`, `${...}` can also be used. Macros can be `width`/`w`, `height`/`h`,  `row`/`r`, `column`/`c`, `rowSpan`/`rs`, `columnSpan`/`cs` or the name of any style if available.

There are modifiers with value and without value. Modifiers with no value can be `x`/`0`/`vertical`/`v`/`top`/`t` or `y`/`1`/`horizontal`/`h`/`r`/`right` or `2`/`bottom`/`b` or `3`/`left`/`l` or `parent`/`p`. Modifiers with value can be `state`/`s` or `part`/`p`. They can be set with the name of a state or a part to reference style values from different states and parts.
## Scale (`x`)
This is useful to bring a scale into the format used by transform scale. When added after a number, it will multiply the number by 256.
## Angle (`°`)
This is useful to bring an angle (in degrees) into the format used by transform rotation. When added after a number, it will multiply the number by 10.
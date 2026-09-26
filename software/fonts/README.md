## How to update MaterialSymbols
### Update the font file
1. Download all MaterialSymbols variants from [here](https://fonts.google.com/download?family=Material+Symbols+Outlined|Material+Symbols+Rounded|Material+Symbols+Sharp)
2. Pick the desired variant of MaterialSymbols from the downloaded archive (MaterialSymbolsRounded is used here)
3. Copy the **Variable Font** to this directory and rename it to `MaterialSymbols.ttf`
### Update the configuration file
This file includes information on what symbols are included in the font and is also used by SymbolListMenu to search for symbols.
It should always be downloaded and updated alongside the font file.
1. Get the configuration file from [here](https://fonts.google.com/metadata/icons?key=material_symbols&incomplete=true)
2. Open the downloaded file, remove the first line (`)]}'`) and save it
3. Copy the modified file to this directory and rename it to `MaterialSymbolsConfig.json`


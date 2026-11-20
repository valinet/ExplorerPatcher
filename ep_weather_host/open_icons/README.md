# ExplorerPatcher weather artwork

The **ExplorerPatcher** option in the weather icon picker uses the original shapes in `artwork.js` for the taskbar, current conditions, and daily forecast. The same 48 × 48 vector geometry can be used to export standalone SVGs.

The artwork in `artwork.js` is dedicated under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/). See `CC0-1.0.txt` for the legal text. The integration code follows the project license.

`forecast.js` maps recognized forecast labels to individual icons and compositions. An arrow marks a change later in the period. Different markers show brief, intermittent, and simultaneous conditions. The forecast table uses names from the [Google Weather API condition reference](https://developers.google.com/maps/documentation/weather/reference/rest/v1/WeatherCondition). Google Search supplies the widget's localized labels and image sources.

`conditions.js` maps known Google filenames and inline image fingerprints to artwork IDs. To register an inline image, use the base64 payload's length, a colon, and its 32-bit FNV-1a hash as eight lowercase hexadecimal digits. When both the label and image are unfamiliar, `render.js` draws the neutral `unknown` icon and preserves the forecast label.

# ExplorerPatcher weather artwork

The **ExplorerPatcher** option in the weather icon picker uses the original shapes in `artwork.js` for the taskbar, current conditions, and daily forecast. The same 48 × 48 vector geometry can be used to export standalone SVGs.

The artwork in `artwork.js` is dedicated under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/). See `CC0-1.0.txt` for the legal text. The integration code follows the project license.

`conditions.js` maps known Google filenames and inline image fingerprints to artwork IDs. To register an inline image, compute its base64 payload length and its 32-bit FNV-1a hash, encoded as eight lowercase hexadecimal digits. Use the neutral `unknown` icon if the image cannot be identified reliably. Keep the original forecast text and image accessibility labels.

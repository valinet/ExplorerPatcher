function epOpenWeatherIconName(source) {
    const inline = /^data:image\/(png|svg\+xml);base64,/.exec(source);
    if (inline) {
        const payload = source.slice(inline[0].length);
        let hash = 2166136261;
        for (let i = 0; i < payload.length; ++i) {
            hash = Math.imul(hash ^ payload.charCodeAt(i), 16777619);
        }
        const fingerprint = payload.length + ':' + (hash >>> 0).toString(16).padStart(8, '0');
        const name = epOpenWeatherFingerprints[fingerprint];
        if (name) return name;
        if (inline[1] === 'png') {
            for (const [legacyName, ...suffixes] of epOpenWeatherLegacyPngs) {
                if (suffixes.some(suffix => payload.endsWith(suffix))) return legacyName;
            }
        }
        return 'unknown';
    }

    const match = /^https:\/\/(?:www|ssl)\.gstatic\.com\/(?:weather\/conditions\/v\d+\/svg\/([a-z_]+)_(?:light|dark)\.svg|onebox\/weather\/\d+\/([a-z_]+)\.png)(?:[?#]|$)/.exec(source);
    const condition = match && (match[1] || match[2]);
    return condition && Object.hasOwn(epOpenWeatherNames, condition) ? epOpenWeatherNames[condition] : 'unknown';
}

function epOpenWeatherDraw(name) {
    const canvas = document.createElement('canvas');
    canvas.width = canvas.height = 48;
    const context = canvas.getContext('2d');
    context.lineCap = 'round';
    context.lineJoin = 'round';
    for (const { shapes, x, y, scale } of epOpenWeatherLayers(name)) {
        context.save();
        context.translate(x, y);
        context.scale(scale, scale);
        for (const { path, fill, stroke, width } of shapes) {
            const outline = new Path2D(path);
            if (fill) {
                context.fillStyle = fill;
                context.fill(outline);
            }
            if (stroke) {
                context.strokeStyle = stroke;
                context.lineWidth = width;
                context.stroke(outline);
            }
        }
        context.restore();
    }
    return { canvas, display: canvas.toDataURL('image/png') };
}

const state = window.epOpenWeatherState ||= { images: new WeakMap(), artwork: new Map() };
state.observer?.disconnect();

function epOpenWeatherReplace(image) {
    const prior = state.images.get(image);
    const label = image.alt;
    const painted = prior?.artwork.display === image.src;
    if (painted && prior.label === label) return prior.artwork.canvas;
    const source = painted ? prior.source : image.src;
    const name = epOpenWeatherForecastIcon(label, epOpenWeatherIconName(source));
    if (!state.artwork.has(name)) state.artwork.set(name, epOpenWeatherDraw(name));
    const artwork = state.artwork.get(name);
    const tooltip = image.getAttribute('title');
    if (!tooltip || tooltip === prior?.label) {
        if (label) image.title = label;
        else if (tooltip) image.removeAttribute('title');
    }
    state.images.set(image, { artwork, source, label, name });
    if (image.src !== artwork.display) image.src = artwork.display;
    return artwork.canvas;
}

function epOpenWeatherRefresh() {
    const current = document.querySelector('#wob_wc img') || document.querySelector('img.YQ4gaf.zr758c');
    if (current) window.final_im = epOpenWeatherReplace(current);
    for (const image of document.querySelectorAll('.uW5pk img')) epOpenWeatherReplace(image);
}

window.final_im = 0;
const current = ep_weather_icon();
epOpenWeatherRefresh();
state.observer = new MutationObserver(epOpenWeatherRefresh);
const observe = { subtree: true, childList: true, attributeFilter: ['src', 'alt'] };
state.observer.observe(document.getElementById('wob_wc') || current, observe);
const forecast = document.querySelector('.wob_df')?.parentElement;
if (forecast) state.observer.observe(forecast, observe);
return 'run_part_0';

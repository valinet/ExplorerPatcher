// SPDX-License-Identifier: CC0-1.0
// Original weather artwork. Shapes use a 48 by 48 grid.
const epOpenWeatherArtwork = (() => {
    const ink = '#182c3b';
    const cloudColor = '#bce6e9';
    const water = '#36addf';
    const snowColor = '#e2f6ff';
    const sunColor = '#ffbe54';
    const amber = '#f28940';
    const violet = '#c6b7f3';
    const teal = '#52c9bb';

    const solid = (path, color) => ({ path, fill: color, stroke: ink, width: 2 });
    const line = (path, color, width = 2.6) => ({ path, fill: null, stroke: color, width });
    const layer = (shapes, x = 0, y = 0, scale = 1) => ({ shapes, x, y, scale });

    const sun = [
        line('M24 3v5m0 32v5M3 24h5m32 0h5M9 9l4 4m22 22 4 4M39 9l-4 4M13 35l-4 4', amber, 3.3),
        solid('M24 11a13 13 0 1 1 0 26 13 13 0 0 1 0-26Z', sunColor),
        line('M18 20a7 7 0 0 1 6-3.6', '#fff2ac', 2),
    ];
    const moon = [
        solid('M30 9c-2.9 4.1-3.1 9.9-.1 14.4 2.7 4 7 6.2 11.9 5.4-2.4 7.4-9.1 11.8-16.7 10.4C16.9 37.7 11.8 30.3 13 22c1.3-8.5 8.7-13.9 17-13Z', violet),
        line('M35 7v8m-4-4h8m2 6v4m-2-2h4', '#f7d881', 1.9),
        line('M18.8 27.6c.5 2.7 2.2 4.6 4.9 5.7', '#f1eaff', 1.7),
    ];
    const cloudShape = 'M12 31.5a7 7 0 1 1 .9-13.9 10.7 10.7 0 0 1 20.1 1.9 6.1 6.1 0 1 1 3.3 12H12Z';
    const cloud = [
        solid(cloudShape, cloudColor),
        line('M13.1 25.3h19.1c2.9 0 4.3-1.1 4.8-2.3', '#83bcc8', 1.7),
        line('M16.4 17.4a6.6 6.6 0 0 1 4.7-2.9', '#f4ffff', 1.8),
    ];
    const backCloud = [solid(cloudShape, '#84b2c4')];
    const drop = 'M0-3.8c-1.8 2.3-2.8 3.7-2.8 5.1a2.8 2.8 0 0 0 5.6 0C2.8-.1 1.8-1.5 0-3.8Z';
    const dropShape = [solid(drop, water)];
    const flake = [
        line('M0-4v8m-3.5-6L3.5 2m0-4-7 4', ink, 3.2),
        line('M0-4v8m-3.5-6L3.5 2m0-4-7 4', snowColor, 1.6),
    ];
    const pellet = [solid('M0-3 2.8-1.4v2.8L0 3l-2.8-1.6v-2.8Z', violet)];
    const bolt = [solid('m22 27-5 10h6l-1 8 11-13h-7l3-5Z', sunColor)];
    const gusts = [
        line('M5 16h20c6 0 6-7 1-7-2 0-3.4 1.1-3.9 2.6M3 23h33c8 0 8-10 1-10-2.5 0-3.8 1.5-4.5 3.1M8 31h21c6 0 6 7 1 7-2 0-3.5-1.1-4-2.8', ink, 5.4),
        line('M5 16h20c6 0 6-7 1-7-2 0-3.4 1.1-3.9 2.6M3 23h33c8 0 8-10 1-10-2.5 0-3.8 1.5-4.5 3.1M8 31h21c6 0 6 7 1 7-2 0-3.5-1.1-4-2.8', teal, 3),
    ];
    const lowCloud = layer(cloud, 0, -3);
    const dayTop = layer(sun, 0, -4, .63);
    const nightTop = layer(moon, -7, -6, .73);
    const mixedCloud = layer(cloud, 6, 6, .9);
    const drops = (positions) => positions.map(([x, y, scale = 1]) => layer(dropShape, x, y, scale));
    const flakes = (positions) => positions.map(([x, y, scale = 1]) => layer(flake, x, y, scale));
    const threeRain = [[13, 37], [24, 40], [35, 36]];
    const threeSnow = [[13, 36], [24, 40], [35, 35]];
    const fogLines = [
        line('M7 33h34M11 39h26M16 44h16', ink, 4.4),
        line('M7 33h34M11 39h26M16 44h16', '#9ccbd9', 2.5),
    ];
    const thermometer = (color) => [
        solid('M20 29V11a4 4 0 0 1 8 0v18a8.4 8.4 0 1 1-8 0Z', '#e2f2f5'),
        line('M24 15v20', color, 3.2),
        {path: 'M24 32a4 4 0 1 1 0 8 4 4 0 0 1 0-8Z', fill: color, stroke: null},
        line('M30 13h3m-3 5h3m-3 5h3', ink, 1.4),
    ];

    return {
        'clear-day': [layer(sun)],
        'clear-night': [layer(moon)],
        'mostly-clear-day': [layer(sun, 0, -2, .84), layer(cloud, 19, 17, .58)],
        'mostly-clear-night': [layer(moon, -6, -2, .95), layer(cloud, 20, 18, .55)],
        'partly-cloudy-day': [dayTop, mixedCloud],
        'partly-cloudy-night': [nightTop, mixedCloud],
        'mostly-cloudy-day': [layer(sun, 13, -1, .57), layer(backCloud, -1, 1, .84), layer(cloud, 3, 7, .97)],
        'mostly-cloudy-night': [layer(moon, 13, -3, .63), layer(backCloud, -1, 1, .84), layer(cloud, 3, 7, .97)],
        'overcast': [layer(backCloud, -1, -2, .83), layer(cloud, 3, 4, .98)],
        'fog': [layer(cloud, 4, -1, .83), layer(fogLines)],
        'haze': [layer(sun, 9, -2, .62), layer([
            line('M8 28h32m-27 6h22M7 40h34', ink, 4.4),
            line('M8 28h32m-27 6h22M7 40h34', '#edb381', 2.5),
        ])],
        'wind': [layer(gusts)],
        'drizzle': [lowCloud, ...drops([[16, 35, .64], [24, 39, .64], [32, 35, .64]])],
        'rain': [lowCloud, ...drops(threeRain)],
        'heavy-rain': [lowCloud, ...drops([[11, 35], [20, 41], [28, 35], [37, 41]])],
        'rain-showers': [layer(cloud, 3, -6, .94), ...drops([[15, 33, .8], [24, 39, .85], [33, 34, .8]])],
        'rain-showers-day': [dayTop, layer(cloud, 5, 2, .92), ...drops([[19, 37, .9], [30, 40, .9], [39, 36, .85]])],
        'rain-showers-night': [nightTop, layer(cloud, 5, 2, .92), ...drops([[19, 37, .9], [30, 40, .9], [39, 36, .85]])],
        'freezing-rain': [lowCloud, ...drops([[13, 36], [33, 36]]), layer(pellet, 23, 41)],
        'icy': [layer([
            solid('M24 5 41 17v18l-17 9L7 35V17Z', '#b6e6f5'),
            line('m24 6 4 15 12-3M8 18l15 7 1 18m4-22 12 13M8 34l14-9', '#65aecb', 2.2),
            line('m12 18 12-8 1 7', '#f3fdff', 2.1),
        ])],
        'rain-and-snow': [lowCloud, ...drops([[13, 37], [33, 36]]), layer(flake, 23, 40)],
        'sleet-hail': [lowCloud, layer(pellet, 13, 36), layer(pellet, 24, 41), layer(pellet, 35, 36)],
        'flurries': [lowCloud, ...flakes([[17, 35, .73], [28, 40, .73]])],
        'snow': [lowCloud, ...flakes(threeSnow)],
        'heavy-snow': [lowCloud, ...flakes([[10, 36], [20, 41], [29, 35], [38, 41]])],
        'snow-showers': [layer(cloud, 3, -6, .94), ...flakes(threeSnow)],
        'snow-showers-day': [dayTop, layer(cloud, 5, 2, .92), ...flakes([[19, 37], [30, 40], [39, 36]])],
        'snow-showers-night': [nightTop, layer(cloud, 5, 2, .92), ...flakes([[19, 37], [30, 40], [39, 36]])],
        'blowing-snow': [layer(gusts, 0, -1, .85), ...flakes([[14, 38], [29, 39], [40, 31, .8]])],
        'blizzard': [layer(backCloud, 0, -4, .94), layer(cloud, 6, 1, .85),
            ...flakes([[11, 34], [20, 42], [31, 35], [39, 42]]), layer([line('m4 37 6-3m20 8 7-4', teal, 2)])],
        'isolated-storm-day': [dayTop, layer(cloud, 5, 1, .92), layer(bolt, 5, 0, .96)],
        'isolated-storm-night': [nightTop, layer(cloud, 5, 1, .92), layer(bolt, 5, 0, .96)],
        'thunderstorms': [lowCloud, ...drops([[11, 36], [37, 37]]), layer(bolt)],
        'severe-thunderstorms': [layer(backCloud, 0, -3, .89), layer(cloud, 2, -2, .95),
            layer(bolt, -6, 1, .95), layer(bolt, 12, 1, .9)],
        'tropical-storm': [layer([
            solid('M43 10c-5.9 9.4-9.1 11.4-13.5 10 2.4 2.7 2.6 6.2.5 9C25.7 35.1 13 38.8 5 38c6.4-9.1 9.6-11.8 14-10.3-2.2-2.9-2.4-6.6-.2-9.3C23.5 12.8 35.4 8.7 43 10Z', teal),
            solid('M24 20a4 4 0 1 1 0 8 4 4 0 0 1 0-8Z', '#effefb'),
        ])],
        'tornado': [layer([
            solid('M5 13c1.6-5.4 32.3-6.9 37 0 1.9 2.7-3.4 5.5-8.1 6.6-3.8 1 2.9 1.5.4 5.4-1.4 2.3-7.7 3.9-8.4 5.7-.7 1.6 3.1 1.5 1.6 4.2-1.1 1.9-4.6 4.4-8 6.9 1-2.1 2.5-5 .2-5.5-4-.8-7.7-3.2-4.1-6.1 1.7-1.4 1.5-2.1-2.7-3.6-4.6-1.7-5.8-4.6-.9-6.2C8.1 19.7 3.6 16.8 5 13Z', '#a2c0d6'),
            line('M10 13c4 2.5 19.5 3.2 28.5-.6M15 23c4.1 1.4 9.6 1.1 15.3-.6', '#e8f4fb', 1.9),
        ])],
        'hot': [layer(sun, 1, 2, .67), layer(thermometer('#e36e51'), 9, 5, .94)],
        'cold': [layer(thermometer('#55a9dd'), -3, 2, .96), layer(flake, 35, 12, 1.45)],
        'umbrella': [layer([
            solid('M5 25c1.1-8.7 9.2-15.1 19-15.1S41.9 16.3 43 25c-3.1-1.7-6.6-1.7-9.7 0-3-1.7-6.4-1.7-9.3 0-3.2-1.7-6.5-1.7-9.7 0-3-1.7-6.4-1.7-9.3 0Z', teal),
            line('M24 7v3m0 16v12c0 5.4 7 5.1 7 .3', ink, 4.2),
            line('M24 7v3m0 16v12c0 5.4 7 5.1 7 .3', teal, 2),
            line('M14 23c1.2-6 4.6-9.5 10-12m9 12c-1.2-6-4.6-9.5-9-12', '#cafff1', 1.65),
        ])],
        'unknown': [layer(cloud, -2, -1, 1), layer([
            solid('M36 24a8.5 8.5 0 1 1 0 17 8.5 8.5 0 0 1 0-17Z', violet),
            line('M33.5 29.6c.6-2.1 4.3-2 4.4.2.1 1.5-2 1.7-2 3.6m0 2.9v.1', ink, 1.8),
        ])],
    };
})();

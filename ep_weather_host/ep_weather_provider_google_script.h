#ifndef _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#define _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#include <Windows.h>
// many thanks to https://stackoverflow.com/questions/23202966/google-weather-widget-on-my-website
#define EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN 30000

#define EP_WEATHER_GOOGLE_ICON_HELPER \
    L"function ep_weather_icon() {\n" \
    L"    var im = document.querySelector('#wob_wc img') || document.querySelector('img.YQ4gaf.zr758c');\n" \
    L"    if (!im) { throw new Error('Google weather icon not found; page layout may have changed'); }\n" \
    L"    return im;\n" \
    L"}\n"

LPCWSTR ep_weather_provider_google_script10 = EP_WEATHER_GOOGLE_ICON_HELPER
    L"window.ep_weather_final_image = null; \"run_part_0\";";

// reference: https://github.com/Triggertrap/sun-js
LPCWSTR ep_weather_provider_google_script00 = L"\
Date.prototype.sunrise = function(latitude, longitude, zenith) {\n\
	return this.sunriseSet(latitude, longitude, true, zenith);\n\
}\n\
Date.prototype.sunset = function(latitude, longitude, zenith) {\n\
	return this.sunriseSet(latitude, longitude, false, zenith);\n\
}\n\
Date.prototype.sunriseSet = function(latitude, longitude, sunrise, zenith) {\n\
	if(!zenith) {\n\
		zenith = 90.8333;\n\
	}\n\
	var hoursFromMeridian = longitude / Date.DEGREES_PER_HOUR,\n\
		dayOfYear = this.getDayOfYear(),\n\
		approxTimeOfEventInDays,\n\
		sunMeanAnomaly,\n\
		sunTrueLongitude,\n\
		ascension,\n\
		rightAscension,\n\
		lQuadrant,\n\
		raQuadrant,\n\
		sinDec,\n\
		cosDec,\n\
		localHourAngle,\n\
		localHour,\n\
		localMeanTime,\n\
		time;\n\
	if (sunrise) {\n\
        approxTimeOfEventInDays = dayOfYear + ((6 - hoursFromMeridian) / 24);\n\
    } else {\n\
        approxTimeOfEventInDays = dayOfYear + ((18.0 - hoursFromMeridian) / 24);\n\
    }\n\
	sunMeanAnomaly = (0.9856 * approxTimeOfEventInDays) - 3.289;\n\
	sunTrueLongitude = sunMeanAnomaly + (1.916 * Math.sinDeg(sunMeanAnomaly)) + (0.020 * Math.sinDeg(2 * sunMeanAnomaly)) + 282.634;\n\
	sunTrueLongitude =  Math.mod(sunTrueLongitude, 360);\n\
	ascension = 0.91764 * Math.tanDeg(sunTrueLongitude);\n\
    rightAscension = 360 / (2 * Math.PI) * Math.atan(ascension);\n\
    rightAscension = Math.mod(rightAscension, 360);\n\
    lQuadrant = Math.floor(sunTrueLongitude / 90) * 90;\n\
    raQuadrant = Math.floor(rightAscension / 90) * 90;\n\
    rightAscension = rightAscension + (lQuadrant - raQuadrant);\n\
    rightAscension /= Date.DEGREES_PER_HOUR;\n\
    sinDec = 0.39782 * Math.sinDeg(sunTrueLongitude);\n\
	cosDec = Math.cosDeg(Math.asinDeg(sinDec));\n\
	cosLocalHourAngle = ((Math.cosDeg(zenith)) - (sinDec * (Math.sinDeg(latitude)))) / (cosDec * (Math.cosDeg(latitude)));\n\
	localHourAngle = Math.acosDeg(cosLocalHourAngle)\n\
	if (sunrise) {\n\
		localHourAngle = 360 - localHourAngle;\n\
	}\n\
	localHour = localHourAngle / Date.DEGREES_PER_HOUR;\n\
	localMeanTime = localHour + rightAscension - (0.06571 * approxTimeOfEventInDays) - 6.622;\n\
	time = localMeanTime - (longitude / Date.DEGREES_PER_HOUR);\n\
	time = Math.mod(time, 24);\n\
	var midnight = new Date(0);\n\
		midnight.setUTCFullYear(this.getUTCFullYear());\n\
		midnight.setUTCMonth(this.getUTCMonth());\n\
		midnight.setUTCDate(this.getUTCDate());\n\
	var milli = midnight.getTime() + (time * 60 *60 * 1000);\n\
	return new Date(milli);\n\
}\n\
Date.DEGREES_PER_HOUR = 360 / 24;\n\
Date.prototype.getDayOfYear = function() {\n\
	var onejan = new Date(this.getFullYear(),0,1);\n\
	return Math.ceil((this - onejan) / 86400000);\n\
}\n\
Math.degToRad = function(num) {\n\
	return num * Math.PI / 180;\n\
}\n\
Math.radToDeg = function(radians){\n\
    return radians * 180.0 / Math.PI;\n\
}\n\
Math.sinDeg = function(deg) {\n\
    return Math.sin(deg * 2.0 * Math.PI / 360.0);\n\
}\n\
Math.acosDeg = function(x) {\n\
    return Math.acos(x) * 360.0 / (2 * Math.PI);\n\
}\n\
Math.asinDeg = function(x) {\n\
    return Math.asin(x) * 360.0 / (2 * Math.PI);\n\
}\n\
Math.tanDeg = function(deg) {\n\
    return Math.tan(deg * 2.0 * Math.PI / 360.0);\n\
}\n\
Math.cosDeg = function(deg) {\n\
    return Math.cos(deg * 2.0 * Math.PI / 360.0);\n\
}\n\
Math.mod = function(a, b) {\n\
	var result = a % b;\n\
	if(result < 0) {\n\
		result += b;\n\
	}\n\
	return result;\n\
}\n\
\n\
\n\
var is_first_time = 1;\n\
window.ep_weather_final_image = null;\n\
var final_img2 = 0;\n\
var final_int;\n\
var final_cnt = 0;\n\
function ep_set_final_img(){\n\
//ep_weather_icon().src = final_img2;\n\
//final_cnt++;\n\
//if (final_cnt == 20)\n\
is_first_time=0;\n\
clearInterval(final_int);\n\
}\n\
const ep_weather_replaced_sources = new Set();\n\
function IsDay(has_time, hrs, mins) {\n\
var weatherLink = document.querySelector('.YfftMc a[href], a.YfftMc[href]');\n\
if (!weatherLink) return null;\n\
var w_url = weatherLink.href;\n\
var lat = 0.0;\n\
var lon = 0.0;\n\
if (w_url.includes('weathernews.jp')) {\n\
lat = parseFloat(w_url.split('/')[4]);\n\
lon = parseFloat(w_url.split('/')[5]);\n\
} else {\n\
var urlParams = new URLSearchParams(w_url);\n\
lat = parseFloat(urlParams.get('https://www.weather.com/wx/today/?lat'));\n\
lon = parseFloat(urlParams.get('lon'));\n\
}\n\
if (!Number.isFinite(lat) || !Number.isFinite(lon)) return null;\n\
var current = new Date();\n\
if (has_time) { \n\
let off = Math.round(lon/15);\n\
current.setUTCHours(hrs - off); current.setMinutes(mins); }\n\
current.setSeconds(0);\n\
current.setMilliseconds(0);\n\
var sunrise = new Date().sunrise(lat, lon);\n\
sunrise.setFullYear(current.getFullYear());\n\
sunrise.setMonth(current.getMonth());\n\
sunrise.setDate(current.getDate());\n\
sunrise.setSeconds(0);\n\
sunrise.setMilliseconds(0);\n\
var sunset = new Date().sunset(lat, lon);\n\
sunset.setFullYear(current.getFullYear());\n\
sunset.setMonth(current.getMonth());\n\
sunset.setDate(current.getDate());\n\
sunset.setSeconds(0);\n\
sunset.setMilliseconds(0);\n\
if (!Number.isFinite(sunrise.getTime()) || !Number.isFinite(sunset.getTime())) return null;\n\
return (current >= sunrise && current < sunset) ? 1 : 0;\n\
}\n\
is_day1 = IsDay(0, 0, 0);\n\
";

LPCWSTR ep_weather_provider_google_script010 = L"\
function replaceImage(im, is_day) {\n\
if (!im || !im.src || ep_weather_replaced_sources.has(im.src)) return;\n\
const originalSrc = im.src;\n\
const svgName = ep_google_svg_name(originalSrc);\n\
const svgIcon = svgName && Object.hasOwn(ep_google_svg_icons, svgName) && ep_google_svg_icons[svgName];\n\
const src = svgIcon ? '/' + svgIcon[0] + '.png' : originalSrc;\n\
if (svgIcon) is_day = !svgName.includes('_night');\n\
if (is_day == null) return;\n\
let final_img = 0;\n\
if (       src.endsWith('/sunny_s_cloudy.png') || src.endsWith('AAAAAAAAAAAAAAAA+Pf6Bm5v6/1bJGiwAAAAAElFTkSuQmCC') || src.endsWith('MYe78v+ACIiACIiACJTEEyDCTi8sMWUSAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = 'mostly-sunny-day';\n\
    else        final_img = 'mostly-sunny-night';\n\
} else if (src.endsWith('/sunny.png')          || src.endsWith('Ls8fJgAAAAAAAADg1nkDlR7XfJiH1ggAAAAASUVORK5CYII=') || src.endsWith('NzjYbzHpLqUCFKCAJfkAq7RimK7qUtAAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'sunny';\n\
    else        final_img = 'clear-night';\n\
} else if (src.endsWith('/cloudy.png')         || src.endsWith('AiAAAiAAAiAAAiAAl48fFVnRpiVnD+AAAAAASUVORK5CYII=') || src.endsWith('gAAIgAAIgAAIgAC86wECCuvGtH3EIQAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = 'cloudy';\n\
    else        final_img = 'cloudy';\n\
} else if (src.endsWith('/rain_s_cloudy.png')  || src.endsWith('CQABIAAEgAAQAAJAAB6iPy4FckaI/hnJAAAAAElFTkSuQmCC') || src.endsWith('lFRe7S+mBtAAGkADeFsvKpKWeAy6FowAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'light-rain';\n\
    else        final_img = 'light-rain';\n\
} else if (src.endsWith('/cloudy_s_rain.png')  || src.endsWith('1S6F6f8CBIAAEAACcEP9AsAciK6YtipuAAAAAElFTkSuQmCC') || src.endsWith('A8DsIAvZ0J98BEAA//oBqG5a70gUjuIAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'light-rain';\n\
    else        final_img = 'light-rain';\n\
} else if (src.endsWith('/rain_light.png')     || src.endsWith('LTAzVDExOjEzOjA0KzAyOjAw/lWSQAAAAABJRU5ErkJggg==') || src.endsWith('LTAzVDExOjEzOjA0KzAyOjAw/lWSQAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = 'light-rain';\n\
    else        final_img = 'light-rain';\n\
} else if (src.endsWith('/rain.png')           || src.endsWith('FEABFEABFEABFOB3fgDsHp230RVQOwAAAABJRU5ErkJggg==') || src.endsWith('MeRa9W/WClABKkAFiKofRnoGaQBkK9wAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'rain';\n\
    else        final_img = 'rain';\n\
} else if (src.endsWith('/rain_heavy.png')     || src.endsWith('oP1rrAFoABqABqAS/QFSaJKDZ1ZY3QAAAABJRU5ErkJggg==') || src.endsWith('U4t1idb1T/0FcAF86g/VgX+0644VVAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = 'rain';\n\
    else        final_img = 'rain';\n\
} else if (src.endsWith('/thunderstorms.png')  || src.endsWith('6n+NVQAVQAVQAfwS/QNrOJYuXyjUGAAAAABJRU5ErkJggg==') || src.endsWith('DTTcSfRpAO//1L8B/hnAD9B4AcpTDEFdAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = 'thunderstorms';\n\
    else        final_img = 'thunderstorms';\n\
} else if (src.endsWith('/cloudy_s_sunny.png') || src.endsWith('gAAQAAJAAAgAASAATqNfCT6dqrbpoPYAAAAASUVORK5CYII=') || src.endsWith('VwAEQAAEQAAEQAD+KcA3AodABBLnBp0AAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'mostly-cloudy-day';\n\
    else        final_img = 'mostly-cloudy-night';\n\
";

LPCWSTR ep_weather_provider_google_script011 = L"\
} else if (src.endsWith('/partly_cloudy.png')  || src.endsWith('AlAACkABKAAFoABOp1+6Bd0LJ+BorgAAAABJRU5ErkJggg==') || src.endsWith('UD8fqAJVoApUgSrwqfwCJ6xaZshM+xMAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'partly-cloudy-day';\n\
    else        final_img = 'partly-cloudy-night';\n\
} else if (src.endsWith('/sunny_s_rain.png')   || src.endsWith('+/JUWP5fQAAIAAEgAA6iPyWP6kEiMYUSAAAAAElFTkSuQmCC') || src.endsWith('EphMZLo+8U++KBAFpnwBEAqlYwd30/wAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'sun-showers-day';\n\
    else        final_img = 'sun-showers-night';\n\
} else if (src.endsWith('/rain_s_sunny.png')   || src.endsWith('gAgQASJABIgAESACXpxfGkmKjZKKoRQAAAAASUVORK5CYII=') || src.endsWith('S30fyJ+YskAWyAJZIIUf+u496Ifu2YkAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'sun-showers-day';\n\
    else        final_img = 'sun-showers-night';\n\
} else if (src.endsWith('/fog.png')            || src.endsWith('/wsIQAACEIAABCCAb6hfRzSoz4hgnE4AAAAASUVORK5CYII=') || src.endsWith('EC30ZRbiv5QMwAAM8PF6AtMRIwMJwimNAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = 'fog';\n\
    else        final_img = 'fog';\n\
} else if (src.endsWith('/snow.png')           || src.endsWith('dMq/xgqAAqAAKABW4g+Q1luByt+ugAAAAABJRU5ErkJggg==') || src.endsWith('kKRD6Xzx+qvBCyCw/ABgjEYCwIT8wQAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = 'snow';\n\
    else        final_img = 'snow';\n\
} else if (src.endsWith('/snow_heavy.png')     || src.endsWith('+jJWB6R/m0sAEoAEIBb7BcA+hV/gbM+pAAAAAElFTkSuQmCC') || src.endsWith('cUH9DYB6xvNfDZ4GIuMbUY1SYGBaleEAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'heavy-snow';\n\
    else        final_img = 'heavy-snow';\n\
} else if (src.endsWith('/snow_light.png')     || src.endsWith('AP5lhgAEIAABCEAAAnzlBYgKVujL4rwkAAAAAElFTkSuQmCC') || src.endsWith('88Tmfn8L1e/EFVABFZA0LkcjEH9Eoxa+AAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = 'snow';\n\
    else        final_img = 'snow';\n\
} else if (src.endsWith('/snow_s_cloudy.png')  || src.endsWith('SwMAAAAAAAAAAAAAAFxEL1Fe7CEOIBaKAAAAAElFTkSuQmCC') || src.endsWith('lfcC1L+YKkAFqAAVIFa/mfArO5CvUjAAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'snow-showers-day';\n\
    else        final_img = 'snow-showers-night';\n\
} else if (src.endsWith('/snow_s_rain.png')    || src.endsWith('+Lc4AAAAAAAAAAAA+AGv/oIwYDUGyQAAAABJRU5ErkJggg==') || src.endsWith('1/kcsNXU/8gqQAWoAGXpGzmRTGmgvhAvAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = 'snow-and-rain';\n\
    else        final_img = 'snow-and-rain';\n\
} else if (src.endsWith('/cloudy_s_snow.png')  || src.endsWith('ASAABIAAEAACQABepj8dv4hCnMCVbgAAAABJRU5ErkJggg==') || src.endsWith('EuzIGPqHhgAIgAAIAE1vnG7TKhwgdU4AAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = 'snow-showers-day';\n\
    else        final_img = 'snow-showers-night';\n\
} else if (src.endsWith('/rain_s_snow.png')    || src.endsWith('ABAAAkAACAABIAAEwIfoF/D2gpORVfhJAAAAAElFTkSuQmCC') || src.endsWith('i/HUv5gqQAWoABUgWj/U6dmtXHN0sAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = 'rain-and-snow';\n\
    else        final_img = 'rain-and-snow';\n\
}\n\
const png = ep_weather_microsoft_icons[final_img];\n\
if (png) {\n\
    const replacement = new Image();\n\
    replacement.src = 'data:image/png;base64,' + png;\n\
    replacement.decode().then(() => {\n\
        if (im.src !== originalSrc) return;\n\
        if (im === ep_weather_icon()) window.ep_weather_final_image = replacement;\n\
        else { im.width = 48; im.height = 48; }\n\
        ep_weather_replaced_sources.add(replacement.src);\n\
        im.src = replacement.src;\n\
    }).catch(() => {});\n\
}\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script020 = L"\
// Google Search serves these SVGs both by URL and as base64 data URIs.\n\
// Each fingerprint is the base64 length and FNV-1a hash of an asset's light or dark variant.\n\
const ep_google_svg_icons = {\n\
    sunny: ['sunny', '1188:5c38525b', '1196:ad502638'],\n\
    clear_night: ['sunny', '724:5cac18ed', '724:2e128e7c'],\n\
    mostly_sunny: ['sunny_s_cloudy', '1956:eaf3d53d', '1948:e2bc8c81'],\n\
    mostly_clear_night: ['sunny_s_cloudy', '1460:15cf1d97', '1460:b11be506'],\n\
    sunny_with_cloudy: ['sunny_s_cloudy', '3564:18f56424', '1848:77ec5d18'],\n\
    partly_cloudy: ['partly_cloudy', '1860:3f6e2617', '1860:7c5ea34c'],\n\
    partly_cloudy_night: ['partly_cloudy', '1452:12e1f41a', '1396:90a3b966'],\n\
    mostly_cloudy_day: ['cloudy_s_sunny', '1980:67fe695e', '1980:e11d8c02'],\n\
    mostly_cloudy_night: ['cloudy_s_sunny', '1436:9e978951', '1388:03ce68d6'],\n\
    cloudy_with_sunny: ['cloudy_s_sunny', '3176:45d8cdb0', '1756:d6352238'],\n\
    cloudy: ['cloudy', '3772:dae58750', '852:8f9c6fca'],\n\
    drizzle: ['rain_light', '3940:27059ac8', '916:dbf42ba6'],\n\
    cloudy_with_rain: ['rain_light', '2744:fb3aa119', '1084:ce27a72b'],\n\
    rain_with_cloudy: ['rain_light', '3160:4d295634', '1124:5691de5b'],\n\
    rain_showers: ['rain', '1324:d05e9bb6', '1220:5d68be67'],\n\
    heavy_rain: ['rain_heavy', '1624:1a916eac', '1520:214352fd'],\n\
    sunny_with_rain: ['sunny_s_rain', '1772:3df379be', '1520:86323c41'],\n\
    rain_with_sunny: ['rain_s_sunny', '1740:81aa196b', '1504:f964ea1b'],\n\
    thunderstorms: ['thunderstorms', '1632:637d63fd', '1528:503b6133'],\n\
    strong_thunderstorms: ['thunderstorms', '2304:ad188c8d', '2208:0cc9848c'],\n\
    flurries: ['snow_light', '4140:54ce1738', '1072:19e918f0'],\n\
    snow_showers: ['snow', '1884:eb884f97', '1780:4c06edb7'],\n\
    heavy_snow: ['snow_heavy', '2568:bd6f1703', '2460:fad6df5b'],\n\
    blizzard: ['snow_heavy', '920:883ce325', '912:b20de39a'],\n\
    blowing_snow: ['snow', '640:4836bbd0', '1264:676c6acd'],\n\
    cloudy_with_snow: ['cloudy_s_snow', '4588:638eaa3c', '3056:39b134ff'],\n\
    snow_with_cloudy: ['snow_s_cloudy', '5092:4030e150', '1924:189ba0e7'],\n\
    scattered_snow_showers_day: ['cloudy_s_snow', '2888:043e603d', '2836:314cb616'],\n\
    scattered_snow_showers_night: ['cloudy_s_snow', '2440:43389152', '2376:de783832'],\n\
    sunny_with_snow: ['cloudy_s_snow', '3272:79e3e016', '3284:bc234b62'],\n\
    snow_with_sunny: ['snow_s_cloudy', '3708:ae80fb34', '2228:9aa42376'],\n\
    snow_with_rain: ['snow_s_rain', '3296:dfc18ad5', '1564:9b03636f'],\n\
    rain_with_snow: ['rain_s_snow', '3160:9f905065', '2904:b6dcb9de'],\n\
};\n\
const ep_google_svg_names = new Map();\n\
for (const [name, [, ...fingerprints]] of Object.entries(ep_google_svg_icons)) {\n\
    for (const fingerprint of fingerprints) ep_google_svg_names.set(fingerprint, name);\n\
}\n\
function ep_google_svg_name(src) {\n\
    const prefix = 'data:image/svg+xml;base64,';\n\
    if (src.startsWith(prefix)) {\n\
        let hash = 2166136261;\n\
        for (let i = prefix.length; i < src.length; ++i) {\n\
            hash = Math.imul(hash ^ src.charCodeAt(i), 16777619);\n\
        }\n\
        const fingerprint = (src.length - prefix.length) + ':' + (hash >>> 0).toString(16).padStart(8, '0');\n\
        return ep_google_svg_names.get(fingerprint);\n\
    }\n\
    const match = /^https:\\/\\/(?:www|ssl)\\.gstatic\\.com\\/weather\\/conditions\\/v\\d+\\/svg\\/([a-z_]+)_(?:light|dark)\\.svg(?:[?#]|$)/.exec(src);\n\
    return match && match[1];\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script03 = L"\
const currentIcon = ep_weather_icon();\n\
replaceImage(currentIcon, is_day1);\n\
window.ep_weather_icon_observer?.disconnect();\n\
const observer = new MutationObserver(changes => {\n\
    for (const { target } of changes) {\n\
        if (ep_weather_replaced_sources.has(target.src)) continue;\n\
        if (target !== currentIcon) {\n\
            replaceImage(target, 1);\n\
            continue;\n\
        }\n\
        window.ep_weather_final_image = null;\n\
        const time = document.getElementById('wob_dts')?.innerText.match(/(\\d+):(\\d+)/);\n\
        const isDay = time ? (is_first_time ? is_day1 : IsDay(1, Number(time[1]), Number(time[2]))) : 1;\n\
        is_first_time = 0;\n\
        replaceImage(target, isDay);\n\
    }\n\
});\n\
window.ep_weather_icon_observer = observer;\n\
observer.observe(currentIcon, {attributeFilter: ['src']});\n\
for (const element of document.getElementsByClassName('uW5pk')) {\n\
    const icon = element.querySelector('img');\n\
    if (!icon) continue;\n\
    replaceImage(icon, 1);\n\
    observer.observe(icon, {attributeFilter: ['src']});\n\
}\n\
return \"run_part_0\";\n\
";

LPCWSTR ep_weather_provider_google_script = L"\
function changeCSSStyle(ssMain, selector, cssProp, cssVal) {\n\
var cssRules = (document.all) ? 'rules': 'cssRules'; \n\
//console.log(ssMain);\n\
  for (i=0, len=document.styleSheets[ssMain][cssRules].length; i<len; i++)\n\
  {\n\
    if (document.styleSheets[ssMain][cssRules][i].selectorText === selector) {\n\
      document.styleSheets[ssMain][cssRules][i].style[cssProp] = cssVal; \n\
//console.log('DA');\n\
      return; \n\
    }\n\
  }\n\
}\n\
function ep_weather_utf8ToHex(str) {\n\
  return Array.from(str).map(c => \n\
    c.charCodeAt(0) < 128 ? c.charCodeAt(0).toString(16) : \n\
    encodeURIComponent(c).replace(/\\%%/g,'').toLowerCase()\n\
  ).join('');\n\
}\n\
function ep_weather_drawImageToCanvas(image, w, h) {\n\
  const canvas = document.createElement('canvas');\n\
  canvas.width = w;\n\
  canvas.height = h;\n\
  canvas.getContext('2d').drawImage(image, 0, 0, w, h);\n\
  return canvas;\n\
}\n\
function ep_weather_toHexString (byteArray) {\n\
  //const chars = new Buffer(byteArray.length * 2);\n\
  const chars = new Uint8Array(byteArray.length * 2);\n\
  const alpha = 'a'.charCodeAt(0) - 10;\n\
  const digit = '0'.charCodeAt(0);\n\
  \n\
  let p = 0;\n\
  for (let i = 0; i < byteArray.length; i++) {\n\
      let nibble = byteArray[i] >>> 4;\n\
      chars[p++] = nibble > 9 ? nibble + alpha : nibble + digit;\n\
      nibble = byteArray[i] & 0xF;\n\
      chars[p++] = nibble > 9 ? nibble + alpha : nibble + digit;\n\
  }\n\
  \n\
  //return chars.toString('utf8');\n\
  return String.fromCharCode.apply(null, chars);\n\
}\n\
function ep_weather_getData(image, w, h, ch) {\n\
  const canvas = ep_weather_drawImageToCanvas(image, w, h);\n\
  const ctx = canvas.getContext('2d');\n\
  \n\
  let result = [];\n\
  for (let y = 0; y < canvas.height; y++) {\n\
    for (let x = 0; x < canvas.width; x++) {\n\
      let data = ctx.getImageData(x, y, 1, 1).data;\n\
      result.push(data[2] * data[3] / 255);\n\
      result.push(data[1] * data[3] / 255);\n\
      result.push(data[0] * data[3] / 255);\n\
      result.push(data[3]);\n\
    }\n\
  }\n\
  let res = (\n\
    document.documentElement.getAttribute(\"dir\") + \"#\" + \n\
    document.getElementsByClassName(\"ULSxyf\")[0].offsetHeight + \"#\" + \n\
    document.getElementById(ch.includes('x') ? \"wob_ttm\" : \"wob_tm\").innerText + \"#\" + \n\
    Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t').filter(e => !e.style.display.toString().includes(\"none\"))[0].innerText + \"#\" + \n\
    ep_weather_icon().alt + \"#\" + \n\
    document.getElementById(\"wob_loc\").innerText + \"#\" + \n\
    ep_weather_toHexString(result)\n\
  );\n\
  //console.log(res);\n\
  document.body.style.backgroundColor='transparent';\n\
  document.body.style.backgroundColor='transparent';\n\
  Array.from(document.getElementsByClassName(\"Ww4FFb\")).forEach((element) => {element.style.backgroundColor = \"transparent\";});\n\
  return res;\n\
}\n\
var ep_result;\n\
function ep_weather_part1() {\n\
let unit = Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t')[0].innerText;\n\
let p = '%c';\n\
if (!unit.includes(p)) {\n\
    Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('a')).filter(e => e.className == 'wob_t').filter(e => e.innerText.includes(p))[0].click();\n\
    unit = 'x';\n\
}\n\
ep_result = ep_weather_getData(\nwindow.ep_weather_final_image || ep_weather_icon(), %d, %d, unit);\n\
return \"run_part_2\";\n\
}\n\
ep_weather_part1();\n\
";

LPCWSTR ep_weather_provider_google_script2 = L"\
function scrolldisable() {\n\
TopScroll = window.pageYOffset || document.documentElement.scrollTop;\n\
LeftScroll = window.pageXOffset || document.documentElement.scrollLeft;\n\
window.onscroll = function() {\n\
window.scrollTo(LeftScroll, TopScroll);\n\
        };\n\
}\n\
function ep_weather_part2() {\n\
let banner1 = document.getElementById(\"taw\"); if (banner1) { banner1.style = \"display: none\"; }\n\
let wob_gsp = document.getElementById(\"wob_gsp\"); if (wob_gsp) { wob_gsp.style = \"width: 648.04px\"; }\n\
let weird_line = document.getElementsByClassName(\"v5jHUb\")[0]; if (weird_line) { weird_line.style = \"display: none\"; }\n\
let slim_appbar = document.getElementById(\"slim_appbar\"); if (slim_appbar) { slim_appbar.style = \"display: none\"; }\n\
scrolldisable();\n\
const header = document.getElementsByClassName('KFFQ0c')[0];\n\
if (header) header.style.display = 'none';\n\
if (document.getElementsByClassName(\"QS5gu sy4vM\").length > 1) { document.getElementsByClassName(\"QS5gu sy4vM\")[1].click(); return \"run_part_1\"; }\n\
if (document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\").length > 1) { document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\")[1].click(); return \"run_part_1\"; }\n\
//document.getElementById(\"search\").scrollIntoView(true);\n\
return ep_result;\n\
}\n\
ep_weather_part2();\n\
";
#endif

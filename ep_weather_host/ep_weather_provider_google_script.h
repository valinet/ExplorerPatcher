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
    L"window.ep_weather_reset_microsoft_icons?.(); var final_im = 0; \"run_part_0\";";

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
window.ep_weather_reset_microsoft_icons?.();\n\
window.final_im = 0;\n\
let is_first_time = true;\n\
let ep_weather_microsoft_active = true;\n\
const ep_weather_replaced_icons = new Map();\n\
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
const is_day1 = IsDay(0, 0, 0);\n\
";

LPCWSTR ep_weather_provider_google_script010 = L"\
function replaceImage(im, is_day) {\n\
if (!im || !im.src || ep_weather_replaced_icons.get(im)?.replacement === im.src) return;\n\
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
        if (!ep_weather_microsoft_active || im.src !== originalSrc) return;\n\
        const dimensions = [im.getAttribute('width'), im.getAttribute('height')];\n\
        ep_weather_replaced_icons.set(im, { original: originalSrc, replacement: replacement.src, dimensions });\n\
        if (im === ep_weather_icon()) window.final_im = replacement;\n\
        else { im.width = 48; im.height = 48; }\n\
        im.src = replacement.src;\n\
    }).catch(() => {});\n\
}\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script020 = L"\
// Google Search serves these SVGs both by URL and as base64 data URIs.\n\
// Each fingerprint is the base64 length and FNV-1a hash of an asset's light or dark variant.\n\
const ep_google_svg_icons = {\n\
    sunny: ['sunny', '1160:96c17eb2', '1164:fd8a0da3', '1172:d2193429', '1188:5c38525b', '1192:cab06be9', '1196:ad502638', '844:49c6786f', '844:ec73f8f9'],\n\
    clear_night: ['sunny', '492:00c0081e', '492:cfb3c3ec', '724:02b3e26a', '724:2e128e7c', '724:5cac18ed', '724:d06d1957', '756:48d8b615', '756:d2ae81e2'],\n\
    mostly_sunny: ['sunny_s_cloudy', '1948:383bff75', '1948:e2bc8c81', '1956:c80963c0', '1956:eaf3d53d', '2012:18d6a535', '2412:1dbbf038', '2412:3699ff9d', '3100:933c2d6d'],\n\
    mostly_clear_night: ['sunny_s_cloudy', '1000:561fad38', '1000:7f54c77a', '1460:15cf1d97', '1460:b11be506', '1488:f6de586f', '1600:10b4f624', '1600:912ab3cc'],\n\
    sunny_with_cloudy: ['sunny_s_cloudy', '1848:77ec5d18', '1980:c49c8b6b', '3564:18f56424', '3668:df225ccc'],\n\
    partly_cloudy: ['partly_cloudy', '1860:3f6e2617', '1860:7c5ea34c', '1960:f2208656', '1980:4e7b2d43', '1980:aa22a4a0', '2224:630be2cc', '2224:f1939b42', '2908:4ac687cc'],\n\
    partly_cloudy_night: ['partly_cloudy', '1396:90a3b966', '1452:12e1f41a', '1480:89119207', '1560:59201b0e', '1560:8b0e0bdb', '960:08b3c448', '960:e5da217f'],\n\
    mostly_cloudy_day: ['cloudy_s_sunny', '1980:67fe695e', '1980:e11d8c02', '2104:da6812bd'],\n\
    mostly_cloudy_night: ['cloudy_s_sunny', '1000:4bf2e984', '1388:03ce68d6', '1436:9e978951', '1464:69e92352', '1584:6e57101d', '1584:abf19278', '984:efba1b7a'],\n\
    cloudy_with_sunny: ['cloudy_s_sunny', '1756:ba146916', '1756:d6352238', '3176:31dede11', '3176:45d8cdb0'],\n\
    cloudy: ['cloudy', '1016:0499de37', '1016:387f211f', '3772:dae58750', '3796:59d9197d', '660:db41bd8c', '676:f8d386b6', '852:8f9c6fca', '852:cf4e99f5'],\n\
    drizzle: ['rain_light', '1472:502a42be', '1472:8d33df45', '3940:27059ac8', '3968:4b133faa', '868:df86965a', '880:eb483959', '916:b91b30b8', '916:dbf42ba6'],\n\
    cloudy_with_rain: ['rain_light', '1084:5cc9b352', '1084:ce27a72b', '2744:8d145adb', '2744:fb3aa119'],\n\
    rain_with_cloudy: ['rain_light', '1124:36453880', '1124:5691de5b', '3132:2e91e8c7', '3160:4d295634'],\n\
    rain_showers: ['rain', '1220:5d68be67', '1324:d05e9bb6'],\n\
    heavy_rain: ['rain_heavy', '1520:214352fd', '1520:72f99a21', '1624:1a916eac', '1624:a84de534', '1700:c76cb181', '1700:cbd80607', '3856:a16a89fe', '3856:cabd899f'],\n\
    sunny_with_rain: ['sunny_s_rain', '1520:86323c41', '1648:c03b6d07', '1772:3df379be', '1900:32c400c4'],\n\
    rain_with_sunny: ['rain_s_sunny', '1504:9e37d4e4', '1504:f964ea1b', '1740:7f672427', '1740:81aa196b'],\n\
    thunderstorms: ['thunderstorms', '1464:7e1be2ac', '1464:9f63ea81', '1528:503b6133', '1556:aab26e24', '1632:637d63fd', '1656:972a87d1'],\n\
    strong_thunderstorms: ['thunderstorms', '1100:7311ee29', '1100:f493d73d', '1876:3d74d56a', '1876:659212f5', '2208:0cc9848c', '2288:90e3ca64', '2304:ad188c8d', '2384:fee6631e'],\n\
    flurries: ['snow_light', '1072:19e918f0', '1124:c311f48c', '1140:0c75f89c', '1464:34906cea', '1464:b9b88f68', '3088:2789c5c3', '4140:54ce1738', '6184:6e0d34f9'],\n\
    snow_showers: ['snow', '1704:6fc0dc7f', '1704:8641b0b5', '1780:4c06edb7', '1884:eb884f97'],\n\
    heavy_snow: ['snow_heavy', '12580:a136d3aa', '12684:dcb97e06', '2288:3a5d805f', '2288:b887e8a6', '2460:fad6df5b', '2568:bd6f1703', '3068:2b9727f7', '3068:c534a3f5'],\n\
    blizzard: ['snow_heavy', '1456:5dba8fef', '1460:af191314', '912:6fbfcc7d', '912:b20de39a', '920:883ce325', '948:74f0f0c9'],\n\
    blowing_snow: ['snow', '1264:676c6acd', '1320:8fb7afb0', '640:4836bbd0', '668:f3314c9a', '940:4f1a9d98', '940:d219a794', '940:d6cf8248'],\n\
    cloudy_with_snow: ['cloudy_s_snow', '3056:39b134ff', '3056:ac292dca', '4588:638eaa3c', '4588:6996002d'],\n\
    snow_with_cloudy: ['snow_s_cloudy', '1924:189ba0e7', '1952:c295bbb7', '5092:4030e150', '5092:4bb5c07d'],\n\
    scattered_snow_showers_day: ['cloudy_s_snow', '2836:314cb616', '2888:043e603d', '8968:7622b343'],\n\
    scattered_snow_showers_night: ['cloudy_s_snow', '2376:de783832', '2440:43389152', '8548:1958d052'],\n\
    sunny_with_snow: ['cloudy_s_snow', '3272:79e3e016', '3284:bc234b62', '3404:d2e5b830', '3416:ce567951'],\n\
    snow_with_sunny: ['snow_s_cloudy', '2228:9aa42376', '2360:bdccb039', '3708:ae80fb34', '3840:e2a5769e'],\n\
    snow_with_rain: ['snow_s_rain', '1564:9b03636f', '1716:2fe16b24', '3296:dfc18ad5', '3428:3b299806'],\n\
    rain_with_snow: ['rain_s_snow', '2904:5dd79dc8', '2904:b6dcb9de', '3160:9f905065', '3160:f0ed9ad7'],\n\
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
const observer = new MutationObserver(changes => {\n\
    for (const { target } of changes) {\n\
        if (ep_weather_replaced_icons.get(target)?.replacement === target.src) continue;\n\
        if (target !== currentIcon) {\n\
            replaceImage(target, 1);\n\
            continue;\n\
        }\n\
        window.final_im = 0;\n\
        const time = document.getElementById('wob_dts')?.innerText.match(/(\\d+):(\\d+)/);\n\
        const isDay = time ? (is_first_time ? is_day1 : IsDay(1, Number(time[1]), Number(time[2]))) : 1;\n\
        is_first_time = false;\n\
        replaceImage(target, isDay);\n\
    }\n\
});\n\
observer.observe(currentIcon, {attributeFilter: ['src']});\n\
for (const element of document.getElementsByClassName('uW5pk')) {\n\
    const icon = element.querySelector('img');\n\
    if (!icon) continue;\n\
    replaceImage(icon, 1);\n\
    observer.observe(icon, {attributeFilter: ['src']});\n\
}\n\
window.ep_weather_reset_microsoft_icons = () => {\n\
    ep_weather_microsoft_active = false;\n\
    observer.disconnect();\n\
    for (const [image, { original, replacement, dimensions }] of ep_weather_replaced_icons) {\n\
        if (image.src !== replacement) continue;\n\
        image.src = original;\n\
        ['width', 'height'].forEach((name, index) => {\n\
            if (dimensions[index] === null) image.removeAttribute(name);\n\
            else image.setAttribute(name, dimensions[index]);\n\
        });\n\
    }\n\
    window.ep_weather_reset_microsoft_icons = null;\n\
    window.final_im = 0;\n\
};\n\
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
ep_result = ep_weather_getData(\n(final_im != 0) ? final_im : ep_weather_icon(), %d, %d, unit);\n\
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

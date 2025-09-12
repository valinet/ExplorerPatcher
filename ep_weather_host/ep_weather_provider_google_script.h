#ifndef _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#define _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#include <Windows.h>
// many thanks to https://stackoverflow.com/questions/23202966/google-weather-widget-on-my-website
#define EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN 30000

LPCWSTR ep_weather_provider_google_script10 = L"var final_im = 0; function ep_weather_part0() { return \"run_part_0\"; }; ep_weather_part0();";

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
var final_im = 0;\n\
var final_img2 = 0;\n\
var final_int;\n\
var final_cnt = 0;\n\
function ep_set_final_img(){\n\
//document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src = final_img2;\n\
//final_cnt++;\n\
//if (final_cnt == 20)\n\
is_first_time=0;\n\
clearInterval(final_int);\n\
}\n\
function ep_download_image_blob(url) {\n\
    var request = new XMLHttpRequest();\n\
    request.open('GET', url, false);\n\
    request.overrideMimeType('text/plain; charset=x-user-defined');\n\
    request.send(null);\n\
    var binary = new Uint8Array(request.responseText.length);\n\
    for(var i=0;i<request.responseText.length;i++){ binary[i] = request.responseText.charCodeAt(i) & 0xff; }\n\
    return URL.createObjectURL(new Blob([binary.buffer]));\n\
}\n\
function IsDay(has_time, hrs, mins) {\n\
var w_url = document.querySelector('.YfftMc').childNodes[0].href;\n\
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
var is_day2 = 0;\n\
//console.log(sunrise);\n\
//console.log(sunset);\n\
//console.log(current);\n\
while (1) {\n\
if (sunrise.getTime() == current.getTime()) { is_day2 = 1; break; }\n\
if (sunrise.getHours() == 23 && sunrise.getMinutes() == 59) { sunrise.setHours(0); sunrise.setMinutes(0); }\n\
else if (sunrise.getMinutes() == 59) { sunrise.setHours(sunrise.getHours() + 1); sunrise.setMinutes(0); }\n\
else sunrise.setMinutes(sunrise.getMinutes() + 1);\n\
if (sunrise.getTime() == sunset.getTime()) break;\n\
}\n\
//console.log(is_day2);\n\
return is_day2;\n\
}\n\
is_day1 = IsDay(0, 0, 0);\n\
";

LPCWSTR ep_weather_provider_google_script010 = L"\
function replaceImage(im, is_day) {\n\
var final_img = 0;\n\
if (!im.src) { return; }\n\
if (       im.src.endsWith('/sunny_s_cloudy.png') || im.src.endsWith('AAAAAAAAAAAAAAAA+Pf6Bm5v6/1bJGiwAAAAAElFTkSuQmCC') || im.src.endsWith('MYe78v+ACIiACIiACJTEEyDCTi8sMWUSAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949438-4e0c0e0d-67bc-4c76-b75e-e0ffcead3f48.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949442-63f14d44-ec0e-40b2-aa1b-8e4a27ec10f5.png');\n\
} else if (im.src.endsWith('/sunny.png')          || im.src.endsWith('Ls8fJgAAAAAAAADg1nkDlR7XfJiH1ggAAAAASUVORK5CYII=') || im.src.endsWith('NzjYbzHpLqUCFKCAJfkAq7RimK7qUtAAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949449-9320c6f5-15ef-4c17-9e72-740708f4828c.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949451-269d02a3-08cb-4237-9789-f1e60fdc723d.png');\n\
} else if (im.src.endsWith('/cloudy.png')         || im.src.endsWith('AiAAAiAAAiAAAiAAl48fFVnRpiVnD+AAAAAASUVORK5CYII=') || im.src.endsWith('gAAIgAAIgAAIgAC86wECCuvGtH3EIQAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949452-f347fe27-5005-48f2-9c9a-899bb7b8825e.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949452-f347fe27-5005-48f2-9c9a-899bb7b8825e.png');\n\
} else if (im.src.endsWith('/rain_s_cloudy.png')  || im.src.endsWith('CQABIAAEgAAQAAJAAB6iPy4FckaI/hnJAAAAAElFTkSuQmCC') || im.src.endsWith('lFRe7S+mBtAAGkADeFsvKpKWeAy6FowAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
} else if (im.src.endsWith('/cloudy_s_rain.png')  || im.src.endsWith('1S6F6f8CBIAAEAACcEP9AsAciK6YtipuAAAAAElFTkSuQmCC') || im.src.endsWith('A8DsIAvZ0J98BEAA//oBqG5a70gUjuIAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
} else if (im.src.endsWith('/rain_light.png')     || im.src.endsWith('LTAzVDExOjEzOjA0KzAyOjAw/lWSQAAAAABJRU5ErkJggg==') || im.src.endsWith('LTAzVDExOjEzOjA0KzAyOjAw/lWSQAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949458-dc66775d-8bb9-4d04-838e-7f550d305c26.png');\n\
} else if (im.src.endsWith('/rain.png')           || im.src.endsWith('FEABFEABFEABFOB3fgDsHp230RVQOwAAAABJRU5ErkJggg==') || im.src.endsWith('MeRa9W/WClABKkAFiKofRnoGaQBkK9wAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949460-7c132d89-efb7-457f-8810-9bf235f5737f.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949460-7c132d89-efb7-457f-8810-9bf235f5737f.png');\n\
} else if (im.src.endsWith('/rain_heavy.png')     || im.src.endsWith('oP1rrAFoABqABqAS/QFSaJKDZ1ZY3QAAAABJRU5ErkJggg==') || im.src.endsWith('U4t1idb1T/0FcAF86g/VgX+0644VVAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949460-7c132d89-efb7-457f-8810-9bf235f5737f.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949460-7c132d89-efb7-457f-8810-9bf235f5737f.png');\n\
} else if (im.src.endsWith('/thunderstorms.png')  || im.src.endsWith('6n+NVQAVQAVQAfwS/QNrOJYuXyjUGAAAAABJRU5ErkJggg==') || im.src.endsWith('DTTcSfRpAO//1L8B/hnAD9B4AcpTDEFdAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949450-7e03a3f5-580e-4414-aaeb-3a0898afd1da.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949450-7e03a3f5-580e-4414-aaeb-3a0898afd1da.png');\n\
} else if (im.src.endsWith('/cloudy_s_sunny.png') || im.src.endsWith('gAAQAAJAAAgAASAATqNfCT6dqrbpoPYAAAAASUVORK5CYII=') || im.src.endsWith('VwAEQAAEQAAEQAD+KcA3AodABBLnBp0AAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949462-f50c21dd-85dd-4d9c-a4eb-516e6cddfb1f.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949463-a427edfb-3d7f-4167-bd6f-f5019c482ea1.png');\n\
";

LPCWSTR ep_weather_provider_google_script011 = L"\
} else if (im.src.endsWith('/partly_cloudy.png')  || im.src.endsWith('AlAACkABKAAFoABOp1+6Bd0LJ+BorgAAAABJRU5ErkJggg==') || im.src.endsWith('UD8fqAJVoApUgSrwqfwCJ6xaZshM+xMAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949465-54dd31c6-7e3a-464a-8e64-8b54b6fb7a65.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949461-1f058cf3-6fdd-4aeb-80b7-68fa27b02845.png');\n\
} else if (im.src.endsWith('/sunny_s_rain.png')   || im.src.endsWith('+/JUWP5fQAAIAAEgAA6iPyWP6kEiMYUSAAAAAElFTkSuQmCC') || im.src.endsWith('EphMZLo+8U++KBAFpnwBEAqlYwd30/wAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949443-062a0fa9-88c1-4e07-b6b1-8e52ff64f4f3.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949444-d3aea936-4c22-4f17-a201-02155396684d.png');\n\
} else if (im.src.endsWith('/rain_s_sunny.png')   || im.src.endsWith('gAgQASJABIgAESACXpxfGkmKjZKKoRQAAAAASUVORK5CYII=') || im.src.endsWith('S30fyJ+YskAWyAJZIIUf+u496Ifu2YkAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949443-062a0fa9-88c1-4e07-b6b1-8e52ff64f4f3.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949444-d3aea936-4c22-4f17-a201-02155396684d.png');\n\
} else if (im.src.endsWith('/fog.png')            || im.src.endsWith('/wsIQAACEIAABCCAb6hfRzSoz4hgnE4AAAAASUVORK5CYII=') || im.src.endsWith('EC30ZRbiv5QMwAAM8PF6AtMRIwMJwimNAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949454-81d5d47d-1f33-4859-a112-5a64ceb549a1.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949454-81d5d47d-1f33-4859-a112-5a64ceb549a1.png');\n\
} else if (im.src.endsWith('/snow.png')           || im.src.endsWith('dMq/xgqAAqAAKABW4g+Q1luByt+ugAAAAABJRU5ErkJggg==') || im.src.endsWith('kKRD6Xzx+qvBCyCw/ABgjEYCwIT8wQAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949459-dfe70eba-6c2c-4b1c-b51b-27c13ce7c08c.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949459-dfe70eba-6c2c-4b1c-b51b-27c13ce7c08c.png');\n\
} else if (im.src.endsWith('/snow_heavy.png')     || im.src.endsWith('+jJWB6R/m0sAEoAEIBb7BcA+hV/gbM+pAAAAAElFTkSuQmCC') || im.src.endsWith('cUH9DYB6xvNfDZ4GIuMbUY1SYGBaleEAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949456-04a4bdbd-ff3b-4484-bb30-8909baff8aa8.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949456-04a4bdbd-ff3b-4484-bb30-8909baff8aa8.png');\n\
} else if (im.src.endsWith('/snow_light.png')     || im.src.endsWith('AP5lhgAEIAABCEAAAnzlBYgKVujL4rwkAAAAAElFTkSuQmCC') || im.src.endsWith('88Tmfn8L1e/EFVABFZA0LkcjEH9Eoxa+AAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949459-dfe70eba-6c2c-4b1c-b51b-27c13ce7c08c.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949459-dfe70eba-6c2c-4b1c-b51b-27c13ce7c08c.png');\n\
} else if (im.src.endsWith('/snow_s_cloudy.png')  || im.src.endsWith('SwMAAAAAAAAAAAAAAFxEL1Fe7CEOIBaKAAAAAElFTkSuQmCC') || im.src.endsWith('lfcC1L+YKkAFqAAVIFa/mfArO5CvUjAAAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949447-a6658710-567e-4977-9316-a80007df3076.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949448-cd1b69af-4028-4153-8e40-288526577b58.png');\n\
} else if (im.src.endsWith('/snow_s_rain.png')    || im.src.endsWith('+Lc4AAAAAAAAAAAA+AGv/oIwYDUGyQAAAABJRU5ErkJggg==') || im.src.endsWith('1/kcsNXU/8gqQAWoAGXpGzmRTGmgvhAvAAAAAElFTkSuQmCC')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156950233-ccaadb4a-2e9a-4934-b41c-acd36a7f0d9c.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156950233-ccaadb4a-2e9a-4934-b41c-acd36a7f0d9c.png');\n\
} else if (im.src.endsWith('/cloudy_s_snow.png')  || im.src.endsWith('ASAABIAAEAACQABepj8dv4hCnMCVbgAAAABJRU5ErkJggg==') || im.src.endsWith('EuzIGPqHhgAIgAAIAE1vnG7TKhwgdU4AAAAASUVORK5CYII=')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949447-a6658710-567e-4977-9316-a80007df3076.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949448-cd1b69af-4028-4153-8e40-288526577b58.png');\n\
} else if (im.src.endsWith('/rain_s_snow.png')    || im.src.endsWith('ABAAAkAACAABIAAEwIfoF/D2gpORVfhJAAAAAElFTkSuQmCC') || im.src.endsWith('i/HUv5gqQAWoABUgWj/U6dmtXHN0sAAAAABJRU5ErkJggg==')) {\n\
    if (is_day) final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949445-60d12efa-a21d-40e0-b9a8-1b7a84e58944.png');\n\
    else        final_img = ep_download_image_blob('https://user-images.githubusercontent.com/6503598/156949445-60d12efa-a21d-40e0-b9a8-1b7a84e58944.png');\n\
}\n\
if (final_img != 0) {\n\
    if (im.id != document.getElementsByClassName(\"YQ4gaf zr758c\")[0].id) { im.width = 48; im.height = 48; }\n\
    im.src = final_img;\n\
}\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script020 = L"\
";

LPCWSTR ep_weather_provider_google_script021 = L"\
";

LPCWSTR ep_weather_provider_google_script03 = L"\
replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], is_day1);\n\
for (const element of document.getElementsByClassName(\"uW5pk\")){ replaceImage(element.children[0], 1); }\n\
var observer = new MutationObserver((changes) => {\n\
  changes.forEach(change => {\n\
      if(change.attributeName.includes('src') && (document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src.includes('gstatic.com') || document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src.includes('data:image/png;base64,'))){\n\
        let includes_time = document.getElementById(\"wob_dts\").innerText.includes(\":\");\n\
        if (includes_time) {\n\
            let sp = document.getElementById(\"wob_dts\").innerText.split(':');\n\
            let hrs = parseInt(sp[0].split(' ')[1]);\n\
            let mins = parseInt(sp[1]);\n\
            if (is_first_time) { replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], is_day1); is_first_time = 0 }\n\
            else replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], IsDay(1, hrs, mins));\n\
        } else { replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], 1); }\n\
      }\n\
  });\n\
});\n\
observer.observe(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], {attributes : true});\n\
function ep_weather_part0() {\n\
return \"run_part_0\";\n\
}\n\
ep_weather_part0();\n\
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
    document.getElementsByClassName(\"YQ4gaf zr758c\")[0].alt + \"#\" + \n\
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
let unit = Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t')[0].innerText;\n\
let p = '%c';\n\
if (!unit.includes(p)) {\n\
    Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('a')).filter(e => e.className == 'wob_t').filter(e => e.innerText.includes(p))[0].click();\n\
    unit = 'x';\n\
}\n\
ep_result = ep_weather_getData(\n\(final_im != 0) ? final_im : document.getElementsByClassName(\"YQ4gaf zr758c\")[0], %d, %d, unit);\n\
function ep_weather_part1() {\n\
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
let h = document.getElementsByClassName(\"ULSxyf\")[0].offsetHeight;\n\
////document.getElementsByClassName(\"google-weather-place\")[0].style.height = h + 'px';\n\
////document.getElementsByClassName(\"google-weather-crop\")[0].style.height = h + 'px';\n\
//if (1) for (let j = 0; j < document.styleSheets.length; j++) changeCSSStyle(j, '.wob_ds', 'background-color', '#303134');\n\
document.getElementsByClassName(\"KFFQ0c\")[0].style.display = 'none';\n\
if (document.getElementsByClassName(\"QS5gu sy4vM\").length > 1) { document.getElementsByClassName(\"QS5gu sy4vM\")[1].click(); return \"run_part_1\"; }\n\
if (document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\").length > 1) { document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\")[1].click(); return \"run_part_1\"; }\n\
//document.getElementById(\"search\").scrollIntoView(true);\n\
return ep_result;\n\
}\n\
let banner1 = document.getElementById(\"taw\"); if (banner1) { banner1.style = \"display: none\"; }\n\
let wob_gsp = document.getElementById(\"wob_gsp\"); if (wob_gsp) { wob_gsp.style = \"width: 648.04px\"; }\n\
let weird_line = document.getElementsByClassName(\"v5jHUb\")[0]; if (weird_line) { weird_line.style = \"display: none\"; }\n\
let slim_appbar = document.getElementById(\"slim_appbar\"); if (slim_appbar) { slim_appbar.style = \"display: none\"; }\n\
scrolldisable();\n\
ep_weather_part2();\n\
";
#endif
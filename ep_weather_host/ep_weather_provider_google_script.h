#ifndef _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#define _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#include <Windows.h>
// many thanks to https://stackoverflow.com/questions/23202966/google-weather-widget-on-my-website
#define EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN 30000

LPCWSTR ep_weather_provider_google_script10 = L"var final_im = 0; function ep_weather_part0() { return \"run_part_0\"; }; ep_weather_part0();";

LPCWSTR ep_weather_provider_google_script00 = L"\
\n\
\n\
var final_im = 0;\n\
var final_img2 = 0;\n\
var final_int;\n\
var final_cnt = 0;\n\
function ep_set_final_img(){\n\
//document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src = final_img2;\n\
//final_cnt++;\n\
//if (final_cnt == 20)\n\
clearInterval(final_int);\n\
}\n\
function ep_download_svg_blob(url) {\n\
    var request = new XMLHttpRequest();\n\
    request.open('GET', url, false);\n\
    request.overrideMimeType('image/svg+xml');\n\
    request.send(null);\n\
    var blob = new Blob([request.responseText], { type: 'image/svg+xml;charset=utf-8' });\n\
    return URL.createObjectURL(blob);\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script010 = L"\
function replaceImage(im) {\n\
var final_img = 0;\n\
if (!im.src) { return; }\n\
if (im.src.endsWith('/mostly_cloudy_day_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PGcgY2xpcC1wYXRoPSJ1cmwoI2EpIj48cGF0aCBmaWxsPSJ1cmwoI2IpIiBkPSJNMj')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyCloudyDayV2.svg');\n\
} else if (im.src.endsWith('/mostly_cloudy_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyCloudyNightV2.svg');\n\
} else if (im.src.endsWith('/mostly_clear_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyClearNight.svg');\n\
} else if (im.src.endsWith('/mostly_sunny_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZD0iTTMzLjk5NCA3LjE2M2E4LjAxMyA4LjAxMyAwIDAgMSA3LjQ1IDcuNDUx')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlySunnyDay.svg');\n\
} else if (im.src.endsWith('/sunny_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZD0iTTMzLjg3NiA2Ljc2YTcuOTE4IDcuOTE4IDAgMCAxIDcuMzY0IDcuMzY0')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SunnyDayV3.svg');\n\
} else if (im.src.endsWith('/clear_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ClearNightV3.svg');\n\
} else if (im.src.endsWith('/cloudy_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PG1hc2sgaWQ9ImMiIGZpbGw9IiNmZmYiPjxwYXRoIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDI')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/CloudyV3.svg');\n\
} else if (im.src.endsWith('/drizzle_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PG1hc2sgaWQ9ImMiIGZpbGw9IiNmZmYiPjxwYXRoIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDE2LjVjMCA4LjM1OC02LjY2MiAxNS4xNzEtMTUgMTUuNDg4VjMySDExdi0uMDAyaC0uMTQzQzUuNDEzIDMxLjk5OSAxIDI3LjYxNyA')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/LightRainV3.svg');\n\
} else if (im.src.endsWith('/rain_showers_light.svg') || im.src.endsWith('/heavy_rain_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZmlsbC1vcGFjaXR5PSIuOTUiIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDE2LjVjMCA4LjM1OC02LjY2MiAxNS4xNzEtMTUgMTUuNDg4VjMySDExdi0uMDAya')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ModerateRainV2.svg');\n\
} else if (im.src.endsWith('/strong_thunderstorms_light.svg') || im.src.endsWith('/thunderstorms_day_light.svg') || im.src.endsWith('/thunderstorms_night_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMzAuNjY3IDIxIDI0IDM0Ljc1N2g2TDI5LjI1NyA0NyA0NCAzMC45NjJsLTUuMzM1LS4wMkw0NCAyMUgzMC42NjdaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSJ1cmwoI2IpIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xOC42NjcgMzcgMTYgNDIuMjkxaDIuNEwxOC4xMDM') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZD0iTTIwLjc2MSA0Ljk0NGE0LjYxOSA0LjYxOSAwIDAgMSA0LjI5NiA0LjI5NSA0LjYxOSA0LjYxOSAwIDAgMCAxLjEyOCAyLjcyNCA0LjYyIDQuNjIgMCAwIDEgMCA2LjA3NCA0LjYyIDQuNjIgMCAwIDAtMS4xMjggMi43MjQgNC42MTkgNC42MTkgMCAwIDEtNC4yOTYgNC4yOTYgNC42MiA0LjYyIDAgMCAwLTIuNzI0IDEuMTI4IDQ')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ThunderstormsV2.svg');\n\
";

LPCWSTR ep_weather_provider_google_script011 = L"\
} else if (im.src.endsWith('/partly_cloudy_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PGcgY2xpcC1wYXRoPSJ1cmwoI2EpIj48cGF0aCBmaWxsPSJ1cmwoI2IpIiBkPSJNMz')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/PartlyCloudyDayV3.svg');\n\
} else if (im.src.endsWith('/partly_cloudy_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/PartlyCloudyNightV2.svg');\n\
} else if (im.src.endsWith('/flurries_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PG1hc2sgaWQ9ImMiIGZpbGw9IiNmZmYiPjxwYXRoIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDE2LjVjMCA4LjM1OC02LjY2MiAxNS4xNzEtMTUgMTUuNDg4VjMySDExdi0uMDAybC0uMTQzLjAwMUM1LjQxMyAzMS45OTkgMSAyNy4')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/LightSnowV2.svg');\n\
} else if (im.src.endsWith('/wintry_mix_light.svg')  || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZmlsbC1vcGFjaXR5PSIuOTUiIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDE2LjVjMCA4LjM1OC02LjY2MiAxNS4xNzItMTUgMTUuNDg4VjMySDExdi0uMDAyb')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/RainSnowV2.svg');\n\
} else if (im.src.endsWith('/scattered_snow_showers_day_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZD0iTTE5Ljc2MSAzLjk0NGE0LjYxOSA0LjYxOSAwIDAgMSA0LjI5NiA0LjI5NSA0LjYxOSA0LjYxOSAwIDAgMCAxLjEyOCAyLjcyNCA0LjYyIDQuNjIgMCAwIDEgMCA2LjA3NCA0LjYyIDQuNjIgMCAwIDAtMS4xMjggMi43MjQgNC42MTkgNC42MTkgMCAwIDEtNC4yOTYgNC4yOTYgNC42MiA0LjYyIDAgMCAwLTIuNzI0IDEuMTI4IDQuNjE5IDQuNjE5IDAgMCAxLTYuMDc0IDAgNC42MiA0LjYyIDAgMCAwLTIuNzI0LTEuMTI4IDQuNjE5IDQuNjE5IDAgMC')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SnowShowersDayV2.svg');\n\
} else if (im.src.endsWith('/scattered_snow_showers_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SnowShowersNightV2.svg');\n\
} else if (im.src.endsWith('/snow_showers_light.svg') || im.src.endsWith('/heavy_snow_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgZmlsbD0ibm9uZSI+PHBhdGggZmlsbD0idXJsKCNhKSIgZmlsbC1vcGFjaXR5PSIuOTUiIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTQ3IDE2LjVjMCA4LjM1OC02LjY2MiAxNS4xNzEtMTUgMTUuNDg4VjMySDExdi0uMDAybC0uMTQzLjAwMUM1LjQxMyAzMS45OTkgMSAyNy42MTYgMSAyMi4yMDljMC01LjQwNiA0LjQxMy05Ljc4OCA5Ljg1Ny05Ljc4OCAxLjg5IDAgMy42NTQuNTI3IDUuMTU0IDEuNDQzQzE3LjI3MSA2LjU1OSAyMy42NzkgMSAzMS4zOTMgMSA0MC4wMTMgMSA0NyA3Ljk0IDQ3IDE2LjVaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSIjNUU1RTVFIiBmaW')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/HeavySnowV2.svg');\n\
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
replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0]);\n\
for (const element of document.getElementsByClassName(\"uW5pk\")){ replaceImage(element.children[0], 1); }\n\
var observer = new MutationObserver((changes) => {\n\
  changes.forEach(change => {\n\
      if(change.attributeName.includes('src') && (document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src.includes('gstatic.com') || document.getElementsByClassName(\"YQ4gaf zr758c\")[0].src.includes('data:image/svg+xml;base64,'))){\n\
        replaceImage(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], 1); }\n\
  });\n\
});\n\
observer.observe(document.getElementsByClassName(\"YQ4gaf zr758c\")[0], {attributes : true});\n\
const ep_style2 = document.createElement('style');\n\
ep_style2.innerHTML = `.uW5pk{width: 41px !important;height: 40px !important;padding: 4px !important;}`;\n\
document.documentElement.insertBefore(ep_style2, document.head);\n\
function ep_weather_part0() {\n\
return \"run_part_0\";\n\
}\n\
ep_weather_part0();\n\
";

LPCWSTR ep_weather_provider_google_script = L"\
const ep_reloadhtml = `<a class=\"qm9ZDf\" aria-selected=\"false\" tabindex=\"0\" id=\"wob_ep_reload\" role=\"tab\" href=\"epweather://refresh\"><svg xmlns=\"http://www.w3.org/2000/svg\" fill=\"currentColor\" viewBox=\"0 -960 960 960\" width=\"20\" height=\"20\"><path d=\"M480-160q-134 0-227-93t-93-227 93-227 227-93q69 0 132 28.5T720-690v-110h80v280H520v-80h168q-32-56-87.5-88T480-720q-100 0-170 70t-70 170 70 170 170 70q77 0 139-44t87-116h84q-28 106-114 173t-196 67\"></path></svg></a>`;\n\
const ep_reloadhtmlTarget = document.querySelector('#wob_wind');\n\
if (ep_reloadhtmlTarget) {ep_reloadhtmlTarget.insertAdjacentHTML(\"afterend\", ep_reloadhtml);}\n\
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
function ep_wait_image_loaded(img, callback) {\n\
    if (!img) {\n\
        callback();\n\
        return;\n\
    }\n\
    if (img.complete && img.naturalWidth !== 0) {\n\
        callback();\n\
        return;\n\
    }\n\
    img.onload = function() {\n\
        callback();\n\
    };\n\
    img.onerror = function() {\n\
        callback();\n\
    };\n\
}\n\
var ep_result;\n\
let unit = Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t')[0].innerText;\n\
let p = '%c';\n\
if (!unit.includes(p)) {\n\
    Array.from(document.getElementsByClassName('wob-unit')[0].getElementsByTagName('a')).filter(e => e.className == 'wob_t').filter(e => e.innerText.includes(p))[0].click();\n\
    unit = 'x';\n\
}\n\
let target_img = (final_im != 0) ? final_im : document.getElementsByClassName(\"YQ4gaf zr758c\")[0];\n\
ep_wait_image_loaded(target_img, function() {\n\
    ep_result = ep_weather_getData(target_img, %d, %d, unit);\n\
});\n\
function ep_weather_part1() {\n\
return \"run_part_2\";\n\
}\n\
ep_weather_part1();\n\
";

LPCWSTR ep_weather_provider_google_script2 = L"\
function ep_weather_part2() {\n\
let h = document.getElementsByClassName(\"ULSxyf\")[0].offsetHeight;\n\
////document.getElementsByClassName(\"google-weather-place\")[0].style.height = h + 'px';\n\
////document.getElementsByClassName(\"google-weather-crop\")[0].style.height = h + 'px';\n\
//if (1) for (let j = 0; j < document.styleSheets.length; j++) changeCSSStyle(j, '.wob_ds', 'background-color', '#303134');\n\
if (document.getElementsByClassName(\"QS5gu sy4vM\").length > 1) { document.getElementsByClassName(\"QS5gu sy4vM\")[1].click(); return \"run_part_1\"; }\n\
if (document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\").length > 1) { document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\")[1].click(); return \"run_part_1\"; }\n\
//document.getElementById(\"search\").scrollIntoView(true);\n\
return ep_result;\n\
}\n\
const ep_style = document.createElement('style');\n\
ep_style.innerHTML = `body{overflow:hidden}.wHYlTd,.YzCcne,.cUnQKe,.Lv2Cle,.oIk2Cb,#oFNiHe,#taw,.v5jHUb,#slim_appbar,.KFFQ0c {display:none !important}.AaVjTc,.YNk70c.iFBYke,#sfooter,.rfiSsc.YNk70c,.LoygGf {visibility:hidden !important}.wtsRwe {position:absolute !important;}#wob_gsp {width: 649px;}div[role=\"tablist\"] {display:flex;flex-direction: row;}#wob_ep_reload {margin-left:auto;}.WQNOrf {height:17px;margin: auto 11px 10px 11px !important;}`;\n\
document.documentElement.insertBefore(ep_style, document.head);\n\
ep_weather_part2();\n\
";
#endif

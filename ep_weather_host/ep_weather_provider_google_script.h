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
function ep_download_svg_blob(url) {\n\
    var request = new XMLHttpRequest();\n\
    request.open('GET', url, false);\n\
    request.overrideMimeType('image/svg+xml');\n\
    request.send(null);\n\
    var blob = new Blob([request.responseText], { type: 'image/svg+xml;charset=utf-8' });\n\
    return URL.createObjectURL(blob);\n\
}\n\
function ep_weather_icon() {\n\
    var im = document.querySelector('#wob_wc img') || document.querySelector('img.YQ4gaf.zr758c');\n\
    if (!im) { throw new Error('Google weather icon not found; page layout may have changed'); }\n\
    return im;\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script010 = L"\
function replaceImage(im) {\n\
var final_img = 0;\n\
if (!im.src) { return; }\n\
if (im.src.endsWith('/mostly_cloudy_day_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2IpIiBkPSJNMj')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyCloudyDayV2.svg');\n\
} else if (im.src.endsWith('/mostly_cloudy_night_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0yM')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyCloudyNightV2.svg');\n\
} else if (im.src.endsWith('/mostly_clear_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlyClearNight.svg');\n\
} else if (im.src.endsWith('/mostly_sunny_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBkPSJNMzMuOT')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/MostlySunnyDay.svg');\n\
} else if (im.src.endsWith('/sunny_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBkPSJNMzMuOD')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SunnyDayV3.svg');\n\
} else if (im.src.endsWith('/clear_night_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0yN')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ClearNightV3.svg');\n\
} else if (im.src.endsWith('/cloudy_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48bWFzayBpZD0iYyIgZmlsbD0iI2ZmZiI+PHBhdGggZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMj')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/CloudyV3.svg');\n\
} else if (im.src.endsWith('/drizzle_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48bWFzayBpZD0iYyIgZmlsbD0iI2ZmZiI+PHBhdGggZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMTYuNWMwIDguMzU4LTYuNjYyIDE1LjE3MS0xNSAxNS40ODhWMzJIMTF2LS4wMDJo')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/LightRainV3.svg');\n\
} else if (im.src.endsWith('/rain_showers_light.svg') || im.src.endsWith('/heavy_rain_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLW9wYWNpdHk9Ii45NSIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMTYuNWMwIDguMzU4LTYuNjYyIDE1LjE3MS0xNSAxNS40ODhWMzJIMTF2LS4wMDJo')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ModerateRainV2.svg');\n\
} else if (im.src.endsWith('/strong_thunderstorms_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0z')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ThunderstormsV2.svg');\n\
} else if (im.src.endsWith('/thunderstorms_day_light.svg') || im.src.endsWith('/thunderstorms_night_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBkPSJNMj')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/ThunderstormsV2.svg');\n\
";

LPCWSTR ep_weather_provider_google_script011 = L"\
} else if (im.src.endsWith('/partly_cloudy_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2IpIiBkPSJNMz')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/PartlyCloudyDayV3.svg');\n\
} else if (im.src.endsWith('/partly_cloudy_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/PartlyCloudyNightV2.svg');\n\
} else if (im.src.endsWith('/flurries_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48bWFzayBpZD0iYyIgZmlsbD0iI2ZmZiI+PHBhdGggZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMTYuNWMwIDguMzU4LTYuNjYyIDE1LjE3MS0xNSAxNS40ODhWMzJIMTF2LS4wMDJs')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/LightSnowV2.svg');\n\
} else if (im.src.endsWith('/wintry_mix_light.svg')  || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLW9wYWNpdHk9Ii45NSIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMTYuNWMwIDguMzU4LTYuNjYyIDE1LjE3Mi')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/RainSnowV2.svg');\n\
} else if (im.src.endsWith('/scattered_snow_showers_day_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBkPSJNMT')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SnowShowersDayV2.svg');\n\
} else if (im.src.endsWith('/scattered_snow_showers_night_light.svg')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/SnowShowersNightV2.svg');\n\
} else if (im.src.endsWith('/snow_showers_light.svg') || im.src.endsWith('/heavy_snow_light.svg') || im.src.startsWith('data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgNDggNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjQ4IiBoZWlnaHQ9IjQ4IiBmaWxsPSJub25lIj48cGF0aCBmaWxsPSJ1cmwoI2EpIiBmaWxsLW9wYWNpdHk9Ii45NSIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNDcgMTYuNWMwIDguMzU4LTYuNjYyIDE1LjE3MS0xNSAxNS40ODhWMzJIMTF2LS4wMDJs')) {\n\
    final_img = ep_download_svg_blob('https://assets.msn.com/bundles/v1/weather/latest/HeavySnowV2.svg');\n\
}\n\
if (final_img != 0) {\n\
    if (!ep_weather_icon() || im.id !== ep_weather_icon().id) { im.width = 48; im.height = 48; }\n\
    im.src = final_img;\n\
}\n\
}\n\
";

LPCWSTR ep_weather_provider_google_script020 = L"\
";

LPCWSTR ep_weather_provider_google_script021 = L"\
";

LPCWSTR ep_weather_provider_google_script03 = L"\
replaceImage(ep_weather_icon());\n\
for (const element of document.getElementsByClassName(\"uW5pk\")){ replaceImage(element.children[0], 1); }\n\
var observer = new MutationObserver((changes) => {\n\
  changes.forEach(change => {\n\
      if(change.attributeName.includes('src') && (ep_weather_icon().src.includes('gstatic.com') || ep_weather_icon().src.includes('data:image/svg+xml;base64,'))){\n\
        replaceImage(ep_weather_icon(), 1); }\n\
  });\n\
});\n\
observer.observe(ep_weather_icon(), {attributes : true});\n\
const ep_style2 = document.createElement('style');\n\
ep_style2.innerHTML = `.uW5pk,.uW5pk img {width: 41px !important;height: 40px !important;}`;\n\
document.documentElement.insertBefore(ep_style2, document.head);\n\
function ep_weather_part0() {\n\
return \"run_part_0\";\n\
}\n\
ep_weather_part0();\n\
";

LPCWSTR ep_weather_provider_google_script = L"\
document.querySelectorAll('.logo,[jscontroller=\"gOTY1\"],[jsaction=\"QyLbLe:OMITjf;ewaord:qsYrDe;xd28Mb:A6j43c\"],[jscontroller=\"Da4hkd\"],#botstuff,#searchform,g-section-with-header').forEach(element => element.remove());\n\
document.querySelector('.LSCOAf .QS5gu.sy4vM')?.click();\n\
const ep_reloadhtml = `<a class=\"qm9ZDf\" aria-selected=\"false\" tabindex=\"0\" id=\"wob_ep_reload\" role=\"tab\" href=\"epweather://refresh\"><svg xmlns=\"http://www.w3.org/2000/svg\" fill=\"currentColor\" viewBox=\"0 -960 960 960\" width=\"20\" height=\"20\"><path d=\"M480-160q-134 0-227-93t-93-227 93-227 227-93q69 0 132 28.5T720-690v-110h80v280H520v-80h168q-32-56-87.5-88T480-720q-100 0-170 70t-70 170 70 170 170 70q77 0 139-44t87-116h84q-28 106-114 173t-196 67\"></path></svg></a>`;\n\
const ep_reloadhtmlTarget = document.querySelector('#wob_wind');\n\
if (ep_reloadhtmlTarget) {ep_reloadhtmlTarget.insertAdjacentHTML(\"afterend\", ep_reloadhtml);}\n\
window.addEventListener('dragstart', (e) => e.preventDefault());\n\
window.scrollTo(0, 0);\n\
window.addEventListener(\"blur\",()=>{document.querySelector(\"#wob_temp.qm9ZDf:nth-child(1)\").click(),document.querySelector(\".wob_df:nth-child(1)\")?.click(),setTimeout(()=>{document.querySelector(\".wob_t.wob_gs_l0:nth-child(1)\")?.dispatchEvent(new MouseEvent(\"click\",{bubbles:!0,cancelable:!0,view:window}))},100)});\n\
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
let target_img = (final_im != 0) ? final_im : ep_weather_icon();\n\
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
if (document.getElementsByClassName(\"QS5gu sy4vM\").length > 1) { document.getElementsByClassName(\"QS5gu sy4vM\")[1].click(); return \"run_part_1\"; }\n\
if (document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\").length > 1) { document.getElementsByClassName(\"Gfzyee VDgVie DKlyaf Loxgyb\")[1].click(); return \"run_part_1\"; }\n\
return ep_result;\n\
}\n\
const ep_style = document.createElement('style');\n\
ep_style.innerHTML = `.UQt4rd,.VQF4g{width:auto!important}.QrNVmd.ZXCv8e,.gNCp2e{font-size:16px}body,html{overflow:hidden}#appbar,#oFNiHe,#slim_appbar,#taw,.KFFQ0c,.Lv2Cle,.SLPe5b.M8OgIe,.YzCcne,.cUnQKe,.oIk2Cb,.v5jHUb,.vk_gbt .Uekwlc>a,.wHYlTd{display:none!important}#sfooter,.AaVjTc,.Fgyi2e.rZj61,.LoygGf,.YNk70c.iFBYke,.rfiSsc.YNk70c{visibility:hidden!important}#wob_gsp{width:649px}div[role=tablist]{display:flex;flex-direction:row;width:100%}#wob_ep_reload{margin-left:auto}img{-webkit-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none;-webkit-user-drag:none}.Ab33Nc,.ESCdRb,.TylWce,.UQt4rd,.VQF4g,.wob-unit{display:inline-flex!important}.Ab33Nc,.ESCdRb,.TylWce,.UQt4rd{flex-direction:row!important}.VQF4g{flex-direction:row-reverse;float:right}.gNCp2e{font-weight:500}.wNE31c{line-height:10px!important}#wob_wc{padding-bottom:500px}`;\n\
document.documentElement.insertBefore(ep_style, document.head);\n\
ep_weather_part2();\n\
";
#endif
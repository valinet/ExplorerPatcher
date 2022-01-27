#ifndef _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#define _H_EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_H_
#include <Windows.h>
// many thanks to https://stackoverflow.com/questions/23202966/google-weather-widget-on-my-website
#define EP_WEATHER_PROVIDER_GOOGLE_SCRIPT_LEN 4000
LPCWSTR ep_weather_provider_google_script = L"\
function ep_weather_utf8ToHex(str) {\n\
  return Array.from(str).map(c => \n\
    c.charCodeAt(0) < 128 ? c.charCodeAt(0).toString(16) : \n\
    encodeURIComponent(c).replace(/\\%%/g,'').toLowerCase()\n\
  ).join('');\n\
}\n\
function ep_weather_drawImageToCanvas(image) {\n\
  const canvas = document.createElement('canvas');\n\
  const w = image.width;\n\
  const h = image.height;\n\
  canvas.width = w;\n\
  canvas.height = h;\n\
  canvas.getContext('2d').drawImage(image, 0, 0);\n\
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
function ep_weather_getData(imageBitmap, ch) {\n\
  const canvas = ep_weather_drawImageToCanvas(imageBitmap);\n\
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
    document.getElementById(\"frame\").contentWindow.document.getElementById(ch.includes('x') ? \"wob_ttm\" : \"wob_tm\").innerText + \"#\" + \n\
    Array.from(document.getElementById(\"frame\").contentWindow.document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t').filter(e => !e.style.display.toString().includes(\"none\"))[0].innerText + \"#\" + \n\
    document.getElementById(\"frame\").contentWindow.document.getElementById(\"wob_tci\").alt + \"#\" + \n\
    document.getElementById(\"frame\").contentWindow.document.getElementById(\"wob_loc\").innerText + \"#\" + \n\
    ep_weather_toHexString(result)\n\
  );\n\
  console.log(res);\n\
  document.body.style.backgroundColor='transparent';\n\
  document.getElementById(\"frame\").contentWindow.document.body.style.backgroundColor='transparent';\n\
  return res;\n\
}\n\
var ep_result;\n\
let unit = Array.from(document.getElementById(\"frame\").contentWindow.document.getElementsByClassName('wob-unit')[0].getElementsByTagName('span')).filter(e => e.className == 'wob_t')[0].innerText;\n\
let p = '%c';\n\
if (!unit.includes(p)) {\n\
    Array.from(document.getElementById(\"frame\").contentWindow.document.getElementsByClassName('wob-unit')[0].getElementsByTagName('a')).filter(e => e.className == 'wob_t').filter(e => e.innerText.includes(p))[0].click();\n\
    unit = 'x';\n\
}\n\
createImageBitmap(\n\
    document.getElementById(\"frame\").contentWindow.document.getElementById('wob_tci'), \n\
    { resizeWidth: %d, resizeHeight: %d, resizeQuality: 'high' }\n\
)\n\
.then(imageBitmap => \n\
    ep_result = ep_weather_getData(imageBitmap, unit)\n\
);\n\
function ep_weather_part1() {\n\
return \"run_part_2\";\n\
}\n\
ep_weather_part1();\n\
";

LPCWSTR ep_weather_provider_google_script2 = L"\
function ep_weather_part2() {\n\
document.getElementById(\"frame\").contentWindow.document.getElementsByClassName(\"KFFQ0c\")[0].style.display = 'none';\n\
document.getElementById(\"frame\").contentWindow.document.getElementById(\"search\").scrollIntoView(true);\n\
return ep_result;\n\
}\n\
ep_weather_part2();\n\
";
#endif
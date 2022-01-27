#ifndef _H_EP_WEATHER_PROVIDER_GOOGLE_HTML_H_
#define _H_EP_WEATHER_PROVIDER_GOOGLE_HTML_H_
#include <Windows.h>
#define EP_WEATHER_PROVIDER_GOOGLE_HTML_LEN 2000
LPCWSTR ep_weather_provider_google_html = L"\
<!DOCTYPE html>\n\
<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">\n\
<head>\n\
<meta charset=\"utf-8\" />\n\
<title>Weather</title>\n\
<style>\n\
html {\n\
  background-color: transparent !important;\n\
}\n\
.google-weather-place {\n\
    width: calc(655px); height: calc(370px);\n\
}\n\
.google-weather-crop {\n\
    width: calc(655px); height: calc(370px);\n\
    overflow: hidden;\n\
    transition: 0.3s; \n\
    position: absolute;\n\
}\n\
.google-weather-crop:hover {\n\
    width: 655px; height: 370px;\n\
}\n\
.google-weather {\n\
    position: relative;\n\
    left: -180px; top: -180px;\n\
    width: 2560px; height: 5120px;\n\
    transform: scale(1.0);\n\
    transform-origin: 180px 180px;\n\
    transition: 0.3s;\n\
    position: absolute;\n\
}\n\
.google-weather:hover { \n\
    transform: scale(1); \n\
    z-index: 1000; \n\
}\n\
</style>\n\
</head>\n\
<body>\n\
<div class=\"google-weather-place\">\n\
<div class=\"google-weather-crop\">\n\
<iframe id=\"frame\" allowtransparency=\"true\" class=\"google-weather\" src=\"https://www.google.com/search?igu=1&amp;hl=%s&amp;q=weather%s%s\">\n\
</iframe>\n\
</div>\n\
</div>\n\
</body>\n\
</html>";
#endif
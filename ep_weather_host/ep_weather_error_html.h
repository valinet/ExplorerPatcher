#ifndef _H_EP_WEATHER_ERROR_HTML_H_
#define _H_EP_WEATHER_ERROR_HTML_H_
#include <Windows.h>
#define EP_WEATHER_ERROR_LEN 2000
LPCWSTR ep_weather_error_html = L"\
<!DOCTYPE html>\n\
<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">\n\
<head>\n\
<meta charset=\"utf-8\" />\n\
<meta name=\"color-scheme\" content=\"light dark\">\n\
<title>Weather</title>\n\
<style>\n\
html {\n\
  background-color: transparent !important;\n\
}\n\
body {\n\
  font-family: 'Segoe UI';\n\
  display: flex;\n\
  justify-content: center;\n\
  align-items: center;\n\
}\n\
@media (prefers-color-scheme: dark) {\n\
  .refreshLink   { color: #43a7ff; }\n\
}\n\
@media (prefers-color-scheme: light) {\n\
  .refreshLink   { color: #096bda; }\n\
}\n\
a:link {\n\
  text-decoration: none;\n\
}\n\
a:visited {\n\
  text-decoration: none;\n\
}\n\
a:hover {\n\
  text-decoration: underline;\n\
}\n\
a:active {\n\
  text-decoration: underline;\n\
}\n\
</style>\n\
</head>\n\
<body><center>\n\
<h1>&#128240;</h1>\n\
<h2>Unable to load weather information</h2>\n\
<p>Make sure that the location you have entered is correct.<br/>\n\
Verify that you are connected to the Internet.</p>\n\
<a class=\"refreshLink\" href=\"epweather://refresh\">Reload</a>\n\
</center></body>\n\
</html>";
#endif

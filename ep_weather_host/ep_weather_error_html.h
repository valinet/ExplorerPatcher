#ifndef _H_EP_WEATHER_ERROR_HTML_H_
#define _H_EP_WEATHER_ERROR_HTML_H_
#include <Windows.h>
#include <tchar.h>
#define EP_WEATHER_ERROR_LEN 2000
LPCWSTR ep_weather_error_html = L"\
<!DOCTYPE html>\n\
<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">\n\
<head>\n\
<meta charset=\"utf-8\" />\n\
<meta name=\"color-scheme\" content=\"light dark\">\n\
<title>" _T(CLSID_EPWeather_TEXT) L"_ErrorPage</title>\n\
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
  a   { color: #43a7ff; }\n\
}\n\
@media (prefers-color-scheme: light) {\n\
  a   { color: #096bda; }\n\
}\n\
a:link,a:visited {\n\
  text-decoration: none;\n\
}\n\
a:hover,a:active {\n\
  text-decoration: underline;\n\
}\n\
.newsIcon {user-select: none;-webkit-user-select: none;}\n\
</style>\n\
<script>window.addEventListener('dragstart', (e) => e.preventDefault());</script>\n\
<script>fetch(\"https://www.google.com/gen_204\",{mode:\"no-cors\",cache:\"no-store\"}).then(()=>{const e=document.querySelector(\".refreshWarn\");e&&(e.style.display=\"block\"),setTimeout(()=>{window.location.href=\"epweather://refresh\"},300000)}).catch(e=>{console.log(\"Cant load gen204:\",e)});</script>\n\
</head>\n\
<body><center>\n\
<h1 class=\"newsIcon\">&#128240;</h1>\n\
<h2>Unable to load weather information</h2>\n\
<p>Make sure that the location you have entered is correct.<br/>\n\
Verify that you are connected to the Internet.</p>\n\
<p class=\"refreshWarn\" style=\"display:none;\">Weather will reload in 5 minutes.</p>\n\
<a class=\"refreshLink\" href=\"epweather://refresh\">Reload now</a>\n\
</center>\n\
</body>\n\
</html>";
#endif

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
<body><center><div class=\"mainPage\">\n\
<a class=\"googleLogin\"style=\"position: absolute;top: 0;left: 50%;transform: translateX(-50%);width: 100%;\">Google thinks you're bot? Click here to login with Google Account</a>\n\
<h1 class=\"newsIcon\">&#128240;</h1>\n\
<h2>Unable to load weather information</h2>\n\
<p>Make sure that the location you have entered is correct.<br/>\n\
Verify that you are connected to the Internet.</p>\n\
</div>\n\
<div class=\"ep_GoogleLogin\" style=\"display:none;\">\n\
<h1 class=\"newsIcon\">&#x1F510;</h1>\n\
<h2>Google Account Login</h2>\n\
<p>Click Google login button below and a new window will appear.</p>\n\
<p>After you logged in, close the window and click the reload button.</p>\n\
<p>ExplorerPatcher Never shares your data with anyone. All data is saved locally, which you can clean in EP Settings.</p>\n\
<a class=\"LoginLink\" href=\"https://accounts.google.com/login\" onclick=\"window.open(this.href, '_blank', 'width=800,height=600'); return false;\">Google Login</a>\n\
<center style=\"height: 5px;\"></center>\n\
</div>\n\
<p class=\"refreshWarn\" style=\"display:none;\">Weather will reload after 5 minutes.</p>\n\
<a class=\"refreshLink\" href=\"epweather://refresh\">Reload now</a>\n\
</center>\n\
<script>document.querySelector(\".googleLogin\").addEventListener(\"click\",function(e){e.preventDefault(),document.querySelector(\".mainPage\").style.display=\"none\",document.querySelector(\".ep_GoogleLogin\").style.display=\"block\"});</script>\n\
</body>\n\
</html>";
#endif

# Magic Explorer Button

Este botón se añade mediante una versión extendida de ExplorerPatcher.

## Configuración

Ruta en el registro:
`HKCU\Software\ExplorerPatcher\MagicExplorerButton`

Valores (REG_SZ):
- Label
- Command
- Args
- Icon
- HotIcon (opcional)

Ejemplo de valores por defecto:
- Label = * Configurar *
- Command = notepad.exe
- Args = MagicExplorerButton\Readme.es.md
- Icon = MagicExplorerButton\favicon.ico
- HotIcon = MagicExplorerButton\favicon2.ico

Si la clave de registro no puede crearse o leerse, el botón abre este README (modo sólo lectura).

Interacciones:
- Clic izquierdo: ejecuta el comando (expandiendo %PATH% si aparece en Args).
- Clic derecho O Shift + clic izquierdo: recarga la configuración.
- Ctrl + clic izquierdo: abre este README (ignora el comando).
- En modo sólo lectura cualquier clic izquierdo abre este README.

Puedes usar %PATH% dentro de Args para inyectar la ruta de la carpeta activa (sólo pestaña actual).

No se emplean timers ni hilos en segundo plano.

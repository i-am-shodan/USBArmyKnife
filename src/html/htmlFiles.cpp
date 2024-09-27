#include <string>
#include <unordered_map>
#include <cstdint>
#include <pgmspace.h>

#include "bootstrapMinCss.h"
#include "bootstrapMinJs.h"
#include "stylesMinCss.h"
#include "scriptMinJs.h"
#include "indexHtml.h"
#include "vnc/indexhtml.h"
#include "vnc/error_handlerjs.h"
#include "vnc/localizationjs.h"
#include "vnc/uijs.h"
#include "vnc/webutiljs.h"
#include "vnc/altsvg.h"
#include "vnc/clipboardsvg.h"
#include "vnc/connectsvg.h"
#include "vnc/ctrlsvg.h"
#include "vnc/ctrlaltdelsvg.h"
#include "vnc/disconnectsvg.h"
#include "vnc/dragsvg.h"
#include "vnc/errorsvg.h"
#include "vnc/escsvg.h"
#include "vnc/expandersvg.h"
#include "vnc/fullscreensvg.h"
#include "vnc/handlesvg.h"
#include "vnc/handle_bgsvg.h"
#include "vnc/infosvg.h"
#include "vnc/keyboardsvg.h"
#include "vnc/powersvg.h"
#include "vnc/settingssvg.h"
#include "vnc/tabsvg.h"
#include "vnc/toggleextrakeyssvg.h"
#include "vnc/warningsvg.h"
#include "vnc/windowssvg.h"
#include "vnc/novnc_icon_smsvg.h"
#include "vnc/novnc_iconsvg.h"
#include "vnc/novnc_ios_120png.h"
#include "vnc/novnc_ios_152png.h"
#include "vnc/novnc_ios_167png.h"
#include "vnc/novnc_ios_180png.h"
#include "vnc/novnc_ios_40png.h"
#include "vnc/novnc_ios_58png.h"
#include "vnc/novnc_ios_60png.h"
#include "vnc/novnc_ios_80png.h"
#include "vnc/novnc_ios_87png.h"
#include "vnc/novnc_ios_iconsvg.h"
#include "vnc/novncico.h"
#include "vnc/bellmp3.h"
#include "vnc/belloga.h"
#include "vnc/basecss.h"
#include "vnc/inputcss.h"
#include "vnc/Orbitron700ttf.h"
#include "vnc/Orbitron700woff.h"
#include "vnc/base64js.h"
#include "vnc/deflatorjs.h"
#include "vnc/desjs.h"
#include "vnc/displayjs.h"
#include "vnc/encodingsjs.h"
#include "vnc/inflatorjs.h"
#include "vnc/ra2js.h"
#include "vnc/rfbjs.h"
#include "vnc/websockjs.h"
#include "vnc/copyrectjs.h"
#include "vnc/hextilejs.h"
#include "vnc/jpegjs.h"
#include "vnc/rawjs.h"
#include "vnc/rrejs.h"
#include "vnc/tightjs.h"
#include "vnc/tightpngjs.h"
#include "vnc/zrlejs.h"
#include "vnc/domkeytablejs.h"
#include "vnc/fixedkeysjs.h"
#include "vnc/gesturehandlerjs.h"
#include "vnc/keyboardjs.h"
#include "vnc/keysymjs.h"
#include "vnc/keysymdefjs.h"
#include "vnc/utiljs.h"
#include "vnc/vkeysjs.h"
#include "vnc/xtscancodesjs.h"
#include "vnc/browserjs.h"
#include "vnc/cursorjs.h"
#include "vnc/elementjs.h"
#include "vnc/eventsjs.h"
#include "vnc/eventtargetjs.h"
#include "vnc/intjs.h"
#include "vnc/loggingjs.h"
#include "vnc/md5js.h"
#include "vnc/stringsjs.h"
#include "vnc/commonjs.h"
#include "vnc/adler32js.h"
#include "vnc/constantsjs.h"
#include "vnc/crc32js.h"
#include "vnc/deflatejs.h"
#include "vnc/gzheaderjs.h"
#include "vnc/inffastjs.h"
#include "vnc/inflatejs.h"
#include "vnc/inftreesjs.h"
#include "vnc/messagesjs.h"
#include "vnc/treesjs.h"
#include "vnc/zstreamjs.h"

std::unordered_map<const char*,std::pair<const uint8_t*, size_t>> staticHtmlFilesLookup = {
    {"/assets/bootstrap/css/bootstrap.min.css",{ bootstrapMinCssGz,sizeof(bootstrapMinCssGz) } },
    {"/assets/bootstrap/js/bootstrap.min.js",{ bootstrapMinJsGz,sizeof(bootstrapMinJsGz) } },
    {"/assets/css/styles.min.css",{ stylesMinCssGz,sizeof(stylesMinCssGz) } },
    {"/assets/js/script.min.js",{ scriptMinJsGz,sizeof(scriptMinJsGz) } },
    {"/index.html",{ indexHtmlGz,sizeof(indexHtmlGz) } },
    {"/vnc/index.html",{ noVNCindexhtmlGz,sizeof(noVNCindexhtmlGz) } },
    {"/vnc/app/error-handler.js",{ noVNCerror_handlerjsGz,sizeof(noVNCerror_handlerjsGz) } },
    {"/vnc/app/localization.js",{ noVNClocalizationjsGz,sizeof(noVNClocalizationjsGz) } },
    {"/vnc/app/ui.js",{ noVNCuijsGz,sizeof(noVNCuijsGz) } },
    {"/vnc/app/webutil.js",{ noVNCwebutiljsGz,sizeof(noVNCwebutiljsGz) } },
    {"/vnc/app/images/alt.svg",{ noVNCaltsvgGz,sizeof(noVNCaltsvgGz) } },
    {"/vnc/app/images/clipboard.svg",{ noVNCclipboardsvgGz,sizeof(noVNCclipboardsvgGz) } },
    {"/vnc/app/images/connect.svg",{ noVNCconnectsvgGz,sizeof(noVNCconnectsvgGz) } },
    {"/vnc/app/images/ctrl.svg",{ noVNCctrlsvgGz,sizeof(noVNCctrlsvgGz) } },
    {"/vnc/app/images/ctrlaltdel.svg",{ noVNCctrlaltdelsvgGz,sizeof(noVNCctrlaltdelsvgGz) } },
    {"/vnc/app/images/disconnect.svg",{ noVNCdisconnectsvgGz,sizeof(noVNCdisconnectsvgGz) } },
    {"/vnc/app/images/drag.svg",{ noVNCdragsvgGz,sizeof(noVNCdragsvgGz) } },
    {"/vnc/app/images/error.svg",{ noVNCerrorsvgGz,sizeof(noVNCerrorsvgGz) } },
    {"/vnc/app/images/esc.svg",{ noVNCescsvgGz,sizeof(noVNCescsvgGz) } },
    {"/vnc/app/images/expander.svg",{ noVNCexpandersvgGz,sizeof(noVNCexpandersvgGz) } },
    {"/vnc/app/images/fullscreen.svg",{ noVNCfullscreensvgGz,sizeof(noVNCfullscreensvgGz) } },
    {"/vnc/app/images/handle.svg",{ noVNChandlesvgGz,sizeof(noVNChandlesvgGz) } },
    {"/vnc/app/images/handle_bg.svg",{ noVNChandle_bgsvgGz,sizeof(noVNChandle_bgsvgGz) } },
    {"/vnc/app/images/info.svg",{ noVNCinfosvgGz,sizeof(noVNCinfosvgGz) } },
    {"/vnc/app/images/keyboard.svg",{ noVNCkeyboardsvgGz,sizeof(noVNCkeyboardsvgGz) } },
    {"/vnc/app/images/power.svg",{ noVNCpowersvgGz,sizeof(noVNCpowersvgGz) } },
    {"/vnc/app/images/settings.svg",{ noVNCsettingssvgGz,sizeof(noVNCsettingssvgGz) } },
    {"/vnc/app/images/tab.svg",{ noVNCtabsvgGz,sizeof(noVNCtabsvgGz) } },
    {"/vnc/app/images/toggleextrakeys.svg",{ noVNCtoggleextrakeyssvgGz,sizeof(noVNCtoggleextrakeyssvgGz) } },
    {"/vnc/app/images/warning.svg",{ noVNCwarningsvgGz,sizeof(noVNCwarningsvgGz) } },
    {"/vnc/app/images/windows.svg",{ noVNCwindowssvgGz,sizeof(noVNCwindowssvgGz) } },
    {"/vnc/app/images/icons/novnc-icon-sm.svg",{ noVNCnovnc_icon_smsvgGz,sizeof(noVNCnovnc_icon_smsvgGz) } },
    {"/vnc/app/images/icons/novnc-icon.svg",{ noVNCnovnc_iconsvgGz,sizeof(noVNCnovnc_iconsvgGz) } },
    {"/vnc/app/images/icons/novnc-ios-120.png",{ noVNCnovnc_ios_120pngGz,sizeof(noVNCnovnc_ios_120pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-152.png",{ noVNCnovnc_ios_152pngGz,sizeof(noVNCnovnc_ios_152pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-167.png",{ noVNCnovnc_ios_167pngGz,sizeof(noVNCnovnc_ios_167pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-180.png",{ noVNCnovnc_ios_180pngGz,sizeof(noVNCnovnc_ios_180pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-40.png",{ noVNCnovnc_ios_40pngGz,sizeof(noVNCnovnc_ios_40pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-58.png",{ noVNCnovnc_ios_58pngGz,sizeof(noVNCnovnc_ios_58pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-60.png",{ noVNCnovnc_ios_60pngGz,sizeof(noVNCnovnc_ios_60pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-80.png",{ noVNCnovnc_ios_80pngGz,sizeof(noVNCnovnc_ios_80pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-87.png",{ noVNCnovnc_ios_87pngGz,sizeof(noVNCnovnc_ios_87pngGz) } },
    {"/vnc/app/images/icons/novnc-ios-icon.svg",{ noVNCnovnc_ios_iconsvgGz,sizeof(noVNCnovnc_ios_iconsvgGz) } },
    {"/vnc/app/images/icons/novnc.ico",{ noVNCnovncicoGz,sizeof(noVNCnovncicoGz) } },
    {"/vnc/app/sounds/bell.mp3",{ noVNCbellmp3Gz,sizeof(noVNCbellmp3Gz) } },
    {"/vnc/app/sounds/bell.oga",{ noVNCbellogaGz,sizeof(noVNCbellogaGz) } },
    {"/vnc/app/styles/base.css",{ noVNCbasecssGz,sizeof(noVNCbasecssGz) } },
    {"/vnc/app/styles/input.css",{ noVNCinputcssGz,sizeof(noVNCinputcssGz) } },
    {"/vnc/app/styles/Orbitron700.ttf",{ noVNCOrbitron700ttfGz,sizeof(noVNCOrbitron700ttfGz) } },
    {"/vnc/app/styles/Orbitron700.woff",{ noVNCOrbitron700woffGz,sizeof(noVNCOrbitron700woffGz) } },
    {"/vnc/core/base64.js",{ noVNCbase64jsGz,sizeof(noVNCbase64jsGz) } },
    {"/vnc/core/deflator.js",{ noVNCdeflatorjsGz,sizeof(noVNCdeflatorjsGz) } },
    {"/vnc/core/des.js",{ noVNCdesjsGz,sizeof(noVNCdesjsGz) } },
    {"/vnc/core/display.js",{ noVNCdisplayjsGz,sizeof(noVNCdisplayjsGz) } },
    {"/vnc/core/encodings.js",{ noVNCencodingsjsGz,sizeof(noVNCencodingsjsGz) } },
    {"/vnc/core/inflator.js",{ noVNCinflatorjsGz,sizeof(noVNCinflatorjsGz) } },
    {"/vnc/core/ra2.js",{ noVNCra2jsGz,sizeof(noVNCra2jsGz) } },
    {"/vnc/core/rfb.js",{ noVNCrfbjsGz,sizeof(noVNCrfbjsGz) } },
    {"/vnc/core/websock.js",{ noVNCwebsockjsGz,sizeof(noVNCwebsockjsGz) } },
    {"/vnc/core/decoders/copyrect.js",{ noVNCcopyrectjsGz,sizeof(noVNCcopyrectjsGz) } },
    {"/vnc/core/decoders/hextile.js",{ noVNChextilejsGz,sizeof(noVNChextilejsGz) } },
    {"/vnc/core/decoders/jpeg.js",{ noVNCjpegjsGz,sizeof(noVNCjpegjsGz) } },
    {"/vnc/core/decoders/raw.js",{ noVNCrawjsGz,sizeof(noVNCrawjsGz) } },
    {"/vnc/core/decoders/rre.js",{ noVNCrrejsGz,sizeof(noVNCrrejsGz) } },
    {"/vnc/core/decoders/tight.js",{ noVNCtightjsGz,sizeof(noVNCtightjsGz) } },
    {"/vnc/core/decoders/tightpng.js",{ noVNCtightpngjsGz,sizeof(noVNCtightpngjsGz) } },
    {"/vnc/core/decoders/zrle.js",{ noVNCzrlejsGz,sizeof(noVNCzrlejsGz) } },
    {"/vnc/core/input/domkeytable.js",{ noVNCdomkeytablejsGz,sizeof(noVNCdomkeytablejsGz) } },
    {"/vnc/core/input/fixedkeys.js",{ noVNCfixedkeysjsGz,sizeof(noVNCfixedkeysjsGz) } },
    {"/vnc/core/input/gesturehandler.js",{ noVNCgesturehandlerjsGz,sizeof(noVNCgesturehandlerjsGz) } },
    {"/vnc/core/input/keyboard.js",{ noVNCkeyboardjsGz,sizeof(noVNCkeyboardjsGz) } },
    {"/vnc/core/input/keysym.js",{ noVNCkeysymjsGz,sizeof(noVNCkeysymjsGz) } },
    {"/vnc/core/input/keysymdef.js",{ noVNCkeysymdefjsGz,sizeof(noVNCkeysymdefjsGz) } },
    {"/vnc/core/input/util.js",{ noVNCutiljsGz,sizeof(noVNCutiljsGz) } },
    {"/vnc/core/input/vkeys.js",{ noVNCvkeysjsGz,sizeof(noVNCvkeysjsGz) } },
    {"/vnc/core/input/xtscancodes.js",{ noVNCxtscancodesjsGz,sizeof(noVNCxtscancodesjsGz) } },
    {"/vnc/core/util/browser.js",{ noVNCbrowserjsGz,sizeof(noVNCbrowserjsGz) } },
    {"/vnc/core/util/cursor.js",{ noVNCcursorjsGz,sizeof(noVNCcursorjsGz) } },
    {"/vnc/core/util/element.js",{ noVNCelementjsGz,sizeof(noVNCelementjsGz) } },
    {"/vnc/core/util/events.js",{ noVNCeventsjsGz,sizeof(noVNCeventsjsGz) } },
    {"/vnc/core/util/eventtarget.js",{ noVNCeventtargetjsGz,sizeof(noVNCeventtargetjsGz) } },
    {"/vnc/core/util/int.js",{ noVNCintjsGz,sizeof(noVNCintjsGz) } },
    {"/vnc/core/util/logging.js",{ noVNCloggingjsGz,sizeof(noVNCloggingjsGz) } },
    {"/vnc/core/util/md5.js",{ noVNCmd5jsGz,sizeof(noVNCmd5jsGz) } },
    {"/vnc/core/util/strings.js",{ noVNCstringsjsGz,sizeof(noVNCstringsjsGz) } },
    {"/vnc/vendor/pako/lib/utils/common.js",{ noVNCcommonjsGz,sizeof(noVNCcommonjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/adler32.js",{ noVNCadler32jsGz,sizeof(noVNCadler32jsGz) } },
    {"/vnc/vendor/pako/lib/zlib/constants.js",{ noVNCconstantsjsGz,sizeof(noVNCconstantsjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/crc32.js",{ noVNCcrc32jsGz,sizeof(noVNCcrc32jsGz) } },
    {"/vnc/vendor/pako/lib/zlib/deflate.js",{ noVNCdeflatejsGz,sizeof(noVNCdeflatejsGz) } },
    {"/vnc/vendor/pako/lib/zlib/gzheader.js",{ noVNCgzheaderjsGz,sizeof(noVNCgzheaderjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/inffast.js",{ noVNCinffastjsGz,sizeof(noVNCinffastjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/inflate.js",{ noVNCinflatejsGz,sizeof(noVNCinflatejsGz) } },
    {"/vnc/vendor/pako/lib/zlib/inftrees.js",{ noVNCinftreesjsGz,sizeof(noVNCinftreesjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/messages.js",{ noVNCmessagesjsGz,sizeof(noVNCmessagesjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/trees.js",{ noVNCtreesjsGz,sizeof(noVNCtreesjsGz) } },
    {"/vnc/vendor/pako/lib/zlib/zstream.js",{ noVNCzstreamjsGz,sizeof(noVNCzstreamjsGz) } }
};
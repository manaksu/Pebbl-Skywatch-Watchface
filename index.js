/* SkyWatch — PebbleKit JS
 *
 * CloudPebble AppMessage keys (Automatic, alphabetical order):
 *   clockstyle = 0
 *   sunrise    = 1  (minutes since midnight, local)
 *   sunset     = 2  (minutes since midnight, local)
 *   widget0    = 3
 *   widget1    = 4
 *   widget2    = 5
 *
 * JS always sends using NUMERIC keys 0-5 to bypass name resolution.
 */

var CLOCK_LABELS  = ['Dot', 'Pixel', 'Digital'];
var WIDGET_LABELS = ['Steps', 'Heart Rate', 'Battery', 'Calories', 'Distance'];

/* ---------------------------------------------------------- */
/*  Sunrise/Sunset — sunrise-sunset.org (free, no key)        */
/* ---------------------------------------------------------- */

function fetchSunTimes(lat, lon, callback) {
  var url = 'https://api.sunrise-sunset.org/json?lat=' + lat
          + '&lng=' + lon + '&formatted=0';
  var xhr = new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.onload = function() {
    try {
      var data = JSON.parse(xhr.responseText);
      if (data.status === 'OK') {
        /* API returns UTC ISO strings — convert to LOCAL minutes */
        var rise   = new Date(data.results.sunrise);
        var set    = new Date(data.results.sunset);
        var riseMn = rise.getHours() * 60 + rise.getMinutes();
        var setMn  = set.getHours()  * 60 + set.getMinutes();
        localStorage.setItem('sunrise', String(riseMn));
        localStorage.setItem('sunset',  String(setMn));
        callback(riseMn, setMn);
      } else {
        callback(getSunCached());
      }
    } catch(e) { callback(getSunCached()); }
  };
  xhr.onerror = function() { callback(getSunCached()); };
  xhr.send();
}

function getSunCached() {
  return [
    +(localStorage.getItem('sunrise') || '360'),
    +(localStorage.getItem('sunset')  || '1200')
  ];
}

function fetchAndSend() {
  var cfg = loadCfg();
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      fetchSunTimes(pos.coords.latitude, pos.coords.longitude, function(rise, set) {
        sendMsg(cfg, rise, set);
      });
    },
    function() {
      var sun = getSunCached();
      sendMsg(cfg, sun[0], sun[1]);
    },
    { timeout: 10000 }
  );
}

/* ---------------------------------------------------------- */
/*  Send — always numeric keys, never string names            */
/* ---------------------------------------------------------- */

function sendMsg(cfg, rise, set) {
  Pebble.sendAppMessage(
    { 0: cfg.clock, 1: rise, 2: set, 3: cfg.w0, 4: cfg.w1, 5: cfg.w2 },
    function() { console.log('SkyWatch: sent ok'); },
    function(e) { console.log('SkyWatch: send fail ' + JSON.stringify(e)); }
  );
}

/* ---------------------------------------------------------- */
/*  Local storage helpers                                      */
/* ---------------------------------------------------------- */

function loadCfg() {
  return {
    clock: +(localStorage.getItem('clock') || '0'),
    w0:    +(localStorage.getItem('w0')    || '0'),
    w1:    +(localStorage.getItem('w1')    || '1'),
    w2:    +(localStorage.getItem('w2')    || '2'),
  };
}

function saveCfg(cfg) {
  localStorage.setItem('clock', String(cfg.clock));
  localStorage.setItem('w0',    String(cfg.w0));
  localStorage.setItem('w1',    String(cfg.w1));
  localStorage.setItem('w2',    String(cfg.w2));
}

/* ---------------------------------------------------------- */
/*  Config page — data:text/html (NOT base64)                 */
/* ---------------------------------------------------------- */

function buildConfig(cfg) {
  function radios(name, labels, sel) {
    return labels.map(function(l, i) {
      return '<label><input type="radio" name="' + name + '" value="' + i + '"'
        + (i === sel ? ' checked' : '') + '> ' + l + '</label>';
    }).join('');
  }

  var html = '<!DOCTYPE html><html><head>'
    + '<meta name="viewport" content="width=device-width,initial-scale=1">'
    + '<style>'
    + '*{box-sizing:border-box}'
    + 'body{font:14px/1.4 sans-serif;background:#111;color:#eee;padding:14px;margin:0}'
    + 'h3{font-size:11px;color:#888;text-transform:uppercase;letter-spacing:.06em;margin:20px 0 8px 0}'
    + 'label{display:flex;align-items:center;gap:10px;background:#1c1c1c;border-radius:8px;'
    + '      padding:11px 14px;margin-bottom:6px;cursor:pointer;font-size:15px}'
    + 'input[type=radio]{accent-color:#e02020;width:18px;height:18px;flex-shrink:0}'
    + '#save{display:block;width:100%;margin-top:24px;padding:14px;background:#e02020;'
    + '      color:#fff;border:none;border-radius:8px;font-size:16px;font-weight:bold;cursor:pointer}'
    + '</style></head><body>'
    + '<h3>Clock Style</h3>'
    + radios('cs', CLOCK_LABELS, cfg.clock)
    + '<h3>Widget Slot 1</h3>'
    + radios('w0', WIDGET_LABELS, cfg.w0)
    + '<h3>Widget Slot 2</h3>'
    + radios('w1', WIDGET_LABELS, cfg.w1)
    + '<h3>Widget Slot 3</h3>'
    + radios('w2', WIDGET_LABELS, cfg.w2)
    + '<button id="save">Save</button>'
    + '<script>'
    + 'document.getElementById("save").onclick = function() {'
    + '  function g(n) { var r = document.querySelector("input[name=" + n + "]:checked"); return r ? parseInt(r.value) : 0; }'
    + '  var out = { clock: g("cs"), w0: g("w0"), w1: g("w1"), w2: g("w2") };'
    + '  location.href = "pebblejs://close#" + encodeURIComponent(JSON.stringify(out));'
    + '};'
    + '<\/script></body></html>';

  return 'data:text/html,' + encodeURIComponent(html);
}

/* ---------------------------------------------------------- */
/*  Pebble events                                              */
/* ---------------------------------------------------------- */

Pebble.addEventListener('ready', function() {
  console.log('SkyWatch: ready');
  /* Small delay ensures watch inbox is open before first send */
  setTimeout(function() {
    fetchAndSend();
  }, 500);
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(buildConfig(loadCfg()));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === '' || e.response === 'CANCELLED') return;
  var raw = e.response;
  /* Strip everything before the # */
  if (raw.indexOf('#') !== -1) raw = raw.substring(raw.lastIndexOf('#') + 1);
  var cfg;
  try { cfg = JSON.parse(decodeURIComponent(raw)); } catch(err) {
    console.log('SkyWatch: parse error ' + err);
    return;
  }
  saveCfg(cfg);
  /* Re-fetch sun times and send everything */
  fetchAndSend();
});

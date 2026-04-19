/* SkyWatch — PebbleKit JS settings */

var CLOCK_LABELS  = ['Dot', 'Pixel', 'Digital'];
var WIDGET_LABELS = ['Steps', 'Heart Rate', 'Battery', 'Calories', 'Distance'];

function buildConfig(cfg) {
  function radios(name, labels, sel) {
    return labels.map(function(l, i) {
      return '<label><input type="radio" name="' + name + '" value="' + i + '"'
        + (i === sel ? ' checked' : '') + '><span>' + l + '</span></label>';
    }).join('');
  }

  var html = '<!DOCTYPE html><html><head>'
    + '<meta name="viewport" content="width=device-width,initial-scale=1">'
    + '<style>'
    + 'body{font:14px sans-serif;background:#111;color:#eee;padding:14px;margin:0}'
    + 'h3{font-size:11px;color:#888;text-transform:uppercase;letter-spacing:.08em;margin:18px 0 8px}'
    + 'label{display:flex;align-items:center;gap:10px;background:#1c1c1c;border-radius:8px;padding:10px 12px;margin-bottom:6px;cursor:pointer}'
    + 'input[type=radio]{accent-color:#e02020;width:16px;height:16px;flex-shrink:0}'
    + '#save{display:block;width:100%;margin-top:22px;padding:13px;background:#e02020;color:#fff;border:none;border-radius:8px;font-size:15px;cursor:pointer}'
    + '</style></head><body>'
    + '<h3>Clock Style</h3>'
    + '<div id="clock">' + radios('clock', CLOCK_LABELS, cfg.clock) + '</div>'
    + '<h3>Widget Slot 1</h3>'
    + '<div id="w0">' + radios('w0', WIDGET_LABELS, cfg.w0) + '</div>'
    + '<h3>Widget Slot 2</h3>'
    + '<div id="w1">' + radios('w1', WIDGET_LABELS, cfg.w1) + '</div>'
    + '<h3>Widget Slot 3</h3>'
    + '<div id="w2">' + radios('w2', WIDGET_LABELS, cfg.w2) + '</div>'
    + '<button id="save">Save</button>'
    + '<script>'
    + 'document.getElementById("save").addEventListener("click",function(){'
    + '  function sel(n){var r=document.querySelector("input[name="+n+"]:checked");return r?+r.value:0;}'
    + '  var d={clock:sel("clock"),w0:sel("w0"),w1:sel("w1"),w2:sel("w2")};'
    + '  location.href="pebblejs://close#"+encodeURIComponent(JSON.stringify(d));'
    + '});'
    + '<\/script></body></html>';

  return 'data:text/html,' + encodeURIComponent(html);
}

function load() {
  return {
    clock: +( localStorage.getItem('clock') || '0' ),
    w0:    +( localStorage.getItem('w0')    || '0' ),
    w1:    +( localStorage.getItem('w1')    || '1' ),
    w2:    +( localStorage.getItem('w2')    || '2' ),
  };
}

function send(cfg) {
  Pebble.sendAppMessage(
    { 0: cfg.clock, 1: cfg.w0, 2: cfg.w1, 3: cfg.w2 },
    function() {},
    function() {}
  );
}

Pebble.addEventListener('ready', function() {
  send(load());
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(buildConfig(load()));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === '' || e.response === 'CANCELLED') return;
  var raw = e.response;
  if (raw.indexOf('#') !== -1) raw = raw.split('#').pop();
  var cfg;
  try { cfg = JSON.parse(decodeURIComponent(raw)); } catch(err) { return; }
  if (cfg.clock != null) localStorage.setItem('clock', String(cfg.clock));
  if (cfg.w0    != null) localStorage.setItem('w0',    String(cfg.w0));
  if (cfg.w1    != null) localStorage.setItem('w1',    String(cfg.w1));
  if (cfg.w2    != null) localStorage.setItem('w2',    String(cfg.w2));
  send(load());
});

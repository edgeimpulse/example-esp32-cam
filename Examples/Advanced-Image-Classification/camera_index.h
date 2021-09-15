
const uint8_t index_custom_html[] = R"=====(
<!doctype html>
<html>

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>Edge Impulse on ESP32</title>
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            background: #ffffff;
            color: #000000;
            font-size: 16px
        }

        h2 {
            font-size: 18px
        }

        section.main {
            display: flex
        }

        #menu,
        section.main {
            flex-direction: column
        }

        #menu {
            display: none;
            flex-wrap: nowrap;
            min-width: 340px;
            background: #39BFCE;
            padding: 8px;
            border-radius: 4px;
            margin-top: -10px;
            margin-right: 10px
        }

        #content {
            display: flex;
            flex-wrap: wrap;
            align-items: stretch
        }

        figure {
            padding: 0;
            margin: 0;
            -webkit-margin-before: 0;
            margin-block-start: 0;
            -webkit-margin-after: 0;
            margin-block-end: 0;
            -webkit-margin-start: 0;
            margin-inline-start: 0;
            -webkit-margin-end: 0;
            margin-inline-end: 0
        }

        figure img {
            display: block;
            width: 100%;
            height: auto;
            border-radius: 4px;
            margin-top: 8px
        }

        @media (min-width: 800px) and (orientation:landscape) {
            #content {
                display: flex;
                flex-wrap: nowrap;
                align-items: stretch
            }

            figure img {
                display: block;
                max-width: 100%;
                max-height: calc(100vh - 40px);
                width: auto;
                height: auto
            }

            figure {
                padding: 0;
                margin: 0;
                -webkit-margin-before: 0;
                margin-block-start: 0;
                -webkit-margin-after: 0;
                margin-block-end: 0;
                -webkit-margin-start: 0;
                margin-inline-start: 0;
                -webkit-margin-end: 0;
                margin-inline-end: 0
            }
        }

        section#buttons {
            display: flex;
            flex-wrap: nowrap;
            justify-content: space-between
        }

        #nav-toggle {
            cursor: pointer;
            display: block
        }

        #nav-toggle-cb {
            outline: 0;
            opacity: 0;
            width: 0;
            height: 0
        }

        #nav-toggle-cb:checked+#menu {
            display: flex
        }

        .input-group {
            display: flex;
            flex-wrap: nowrap;
            line-height: 22px;
            margin: 5px 0
        }

        .input-group>label {
            display: inline-block;
            padding-right: 10px;
            min-width: 47%
        }

        .input-group input,
        .input-group select {
            flex-grow: 1
        }

        .range-max,
        .range-min {
            display: inline-block;
            padding: 0 5px
        }

        button {
            display: block;
            margin: 5px;
            padding: 0 12px;
            border: 0;
            line-height: 28px;
            cursor: pointer;
            color: #fff;
            background: #A3D008;
            border-radius: 5px;
            font-size: 16px;
            outline: 0
        }

        button:hover {
            background: #F8562C
        }

        button:active {
            background: #f21c21
        }

        button.disabled {
            cursor: default;
            background: #a0a0a0
        }

        input[type=range] {
            -webkit-appearance: none;
            width: 100%;
            height: 22px;
            background: transparent;
            cursor: pointer;
            margin: 0
        }

        input[type=range]:focus {
            outline: 0
        }

        input[type=range]::-webkit-slider-runnable-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: #EFEFEF;
            border-radius: 0;
            border: 0 solid #EFEFEF
        }

        input[type=range]::-webkit-slider-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #F8562C;
            cursor: pointer;
            -webkit-appearance: none;
            margin-top: -11.5px
        }

        input[type=range]:focus::-webkit-slider-runnable-track {
            background: #EFEFEF
        }

        input[type=range]::-moz-range-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: #EFEFEF;
            border-radius: 0;
            border: 0 solid #EFEFEF
        }

        input[type=range]::-moz-range-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #F8562C;
            cursor: pointer
        }

        input[type=range]::-ms-track {
            width: 100%;
            height: 2px;
            cursor: pointer;
            background: 0 0;
            border-color: transparent;
            color: transparent
        }

        input[type=range]::-ms-fill-lower {
            background: #EFEFEF;
            border: 0 solid #EFEFEF;
            border-radius: 0
        }

        input[type=range]::-ms-fill-upper {
            background: #EFEFEF;
            border: 0 solid #EFEFEF;
            border-radius: 0
        }

        input[type=range]::-ms-thumb {
            border: 1px solid rgba(0, 0, 30, 0);
            height: 22px;
            width: 22px;
            border-radius: 50px;
            background: #F8562C;
            cursor: pointer;
            height: 2px
        }

        input[type=range]:focus::-ms-fill-lower {
            background: #EFEFEF
        }

        input[type=range]:focus::-ms-fill-upper {
            background: #363636
        }

        .switch {
            display: block;
            position: relative;
            line-height: 22px;
            font-size: 16px;
            height: 22px
        }

        .switch input {
            outline: 0;
            opacity: 0;
            width: 0;
            height: 0
        }

        .slider {
            width: 50px;
            height: 22px;
            border-radius: 22px;
            cursor: pointer;
            background-color: grey
        }

        .slider,
        .slider:before {
            display: inline-block;
            transition: .4s
        }

        .slider:before {
            position: relative;
            content: "";
            border-radius: 50%;
            height: 16px;
            width: 16px;
            left: 4px;
            top: 3px;
            background-color: #fff
        }

        input:checked+.slider {
            background-color: #F8562C
        }

        input:checked+.slider:before {
            -webkit-transform: translateX(26px);
            transform: translateX(26px)
        }

        select {
            border: 1px solid #363636;
            font-size: 14px;
            height: 22px;
            outline: 0;
            border-radius: 5px
        }

        .image-container {
            position: relative;
            min-width: 160px; 
        }

        .output-container {
            position: relative;
            max-width: 400px;
        }

        .output {
            position: relative;
            min-width: 350px;
        }

        .close {
            position: absolute;
            right: 5px;
            top: 5px;
            background: #F8562C;
            width: 16px;
            height: 16px;
            border-radius: 100px;
            color: #fff;
            text-align: center;
            line-height: 18px;
            cursor: pointer
        }

        .hidden {
            display: none
        }

        .center {
            text-align: center;
        }

        /* CSS Simple Pre Code */
        pre {
            background: #444;
            white-space: pre;
            word-wrap: break-word;
            overflow: scroll;
        }

        pre.code {
            margin: 10px 0px;
            border-radius: 4px;
            border: 1px solid #292929;
            position: relative;
            max-width: 750px;
        }

        pre.code code {
            display: block;
            margin: 0 0 0 5px;
            padding: 15px 16px 14px;
            
            overflow-x: auto;
            font-size: 13px;
            line-height: 19px;
            color: #FEBA06;
        }
        
    </style>
</head>

<body>
    <section class="main">
        <div id="logo">
            <div class="center">
                
                    <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAMAAAAp4XiDAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAACBFBMVEUAAAA5v846v806v8wAsv+o0QCj0Amh0Azc2QD/twD6ugr9ugf+ugaj0Ail0AWczxaTzSXJxwfWxAfOxgfRxQfPxQevzQii0QijzwjWhx7Kxwf4VizvYyjxYCmR6QE5v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v845v8+j0Aij0Aij0Aij0Aij0Aj+ugb+ugb+ugb+ugb+ugb+ugaj0Aij0Aij0Aij0Aj+ugb+ugb+ugaj0Aij0Aj+ugb+ugb+ugaj0Aij0Aj+ugb+ugb+ugaj0Aij0Aij0Aij0Aj+ugb+ugb+ugaj0Aij0Aij0Aij0Aij0Aij0Aij0Aj+ugb+ugb+ugb+ugaS6AGh0wej0Aj+ugb5VC36Uy34Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viz4Viw5v86j0Aj+ugb4Viz///9+b0UfAAAAp3RSTlMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQMLXaSrdRoJlPu/XlhaQ1H5/v3gJZBskbxV+vU/msmCgIE4DmWstH0eBTCAk3gkIXaTlIUXMtrHHxvCXIZlXq0Gi2pj6jA95tUnI9F+AUKarKuSM5Crrn0CAwMDBQcQbLO6hSIMn8x7d3gwWPueApLhJI9qT/i7CwiO+rxkYGFWCVedpW4XAVKKAw4AAAABYktHRKvJAWcoAAAACXBIWXMAAC4jAAAuIwF4pT92AAAAB3RJTUUH5QQNFTgMiXPPxwAAAX5JREFUSMft0eN/w0AYwPHcjNzsDO1s27Zt27Zt287Wv3LXscldm/V9vy+ffH55kjuKUlGhKPBJycBBpFQEgNjRydnFFZARCzd3D5ZlPb28MT6+InLi589KBQRi2KBg0hoAQlj5QslJmIIkXExoAIiIlJ9ERRMTcUwgehgbF0+QkEj+sqTklNS0dAD+fczSqShDTV1DU+ubtg4a6erpS9EI3kADLUMjbchhTMvACwgzs7Jl5eRCEzovv0CqsKgYbyAsKX3jKis3rah8/1ZFSqrf+GrMan+K97p6QtKAJY1mTb9Jcwu2BsLWNl7R3mHe2fXbdBOSnt6+/gEZg0PQgm4ZHvkyOkb4Mksr6/GJyT/oDG3Q2U79wLYwyPSMrZ29pcy9UDSt4GYYZnZufmFxiVGIWyyvSJDVtfUNeTb5yda2RMDOLi/ZEyok+wecNQxzKJhIjnjJ8YlgcnrGTc4vLtH06vpGrtu7Zd7P3D88Pj2/MMqcMvLKnwrAX6OigvkAw7BeHKYxObUAAAAldEVYdGRhdGU6Y3JlYXRlADIwMjEtMDQtMTNUMjE6NTY6MDgrMDA6MDAv7P8rAAAAJXRFWHRkYXRlOm1vZGlmeQAyMDIxLTA0LTEzVDIxOjU2OjA4KzAwOjAwXrFHlwAAAABJRU5ErkJggg=="
                    alt="logo" />
                    <h2 class="bold">Edge Impulse on ESP32 Cam</h2>
            </div>
            <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle settings</label>
        </div>
        <div id="content">
            <div id="sidebar">
                <input type="checkbox" id="nav-toggle-cb" checked="checked">
                <nav id="menu">
                    <div class="input-group" id="framesize-group">
                        <label for="framesize">Original Resolution</label>
                        <select id="framesize" class="default-action">
                            <option value="13">UXGA(1600x1200)</option>
                            <option value="12">SXGA(1280x1024)</option>
                            <option value="11">HD(1280x720)</option>
                            <option value="10">XGA(1024x768)</option>
                            <option value="9">SVGA(800x600)</option>
                            <option value="8">VGA(640x480)</option>
                            <option value="7">HVGA(480x320)</option>
                            <option value="6">CIF(400x296)</option>
                            <option value="5">QVGA(320x240)</option>
                            <option value="4" selected>240x240</option>
                            <option value="3">HQVGA(240x176)</option>
                            <option value="2">QCFIF(176x144)</option>
                            <option value="1">QQVGA(160x120)</option>
                            <option value="0">96x96</option>
                        </select>
                    </div>
                    <div class="input-group" id="quality-group">
                        <label for="quality">Quality</label>
                        <div class="range-min">10</div>
                        <input type="range" id="quality" min="10" max="63" value="10" class="default-action">
                        <div class="range-max">63</div>
                    </div>
                    <div class="input-group" id="brightness-group">
                        <label for="brightness">Brightness</label>
                        <div class="range-min">-2</div>
                        <input type="range" id="brightness" min="-2" max="2" value="0" class="default-action">
                        <div class="range-max">2</div>
                    </div>
                    <div class="input-group" id="contrast-group">
                        <label for="contrast">Contrast</label>
                        <div class="range-min">-2</div>
                        <input type="range" id="contrast" min="-2" max="2" value="0" class="default-action">
                        <div class="range-max">2</div>
                    </div>
                    <div class="input-group" id="saturation-group">
                        <label for="saturation">Saturation</label>
                        <div class="range-min">-2</div>
                        <input type="range" id="saturation" min="-2" max="2" value="0" class="default-action">
                        <div class="range-max">2</div>
                    </div>
                    <div class="input-group" id="special_effect-group">
                        <label for="special_effect">Special Effect</label>
                        <select id="special_effect" class="default-action">
                            <option value="0" selected="selected">No Effect</option>
                            <option value="1">Negative</option>
                            <option value="2">Grayscale</option>
                            <option value="3">Red Tint</option>
                            <option value="4">Green Tint</option>
                            <option value="5">Blue Tint</option>
                            <option value="6">Sepia</option>
                        </select>
                    </div>
                    <div class="input-group" id="awb-group">
                        <label for="awb">AWB</label>
                        <div class="switch">
                            <input id="awb" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="awb"></label>
                        </div>
                    </div>
                    <div class="input-group" id="awb_gain-group">
                        <label for="awb_gain">AWB Gain</label>
                        <div class="switch">
                            <input id="awb_gain" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="awb_gain"></label>
                        </div>
                    </div>
                    <div class="input-group" id="wb_mode-group">
                        <label for="wb_mode">WB Mode</label>
                        <select id="wb_mode" class="default-action">
                            <option value="0" selected="selected">Auto</option>
                            <option value="1">Sunny</option>
                            <option value="2">Cloudy</option>
                            <option value="3">Office</option>
                            <option value="4">Home</option>
                        </select>
                    </div>
                    <div class="input-group" id="aec-group">
                        <label for="aec">AEC SENSOR</label>
                        <div class="switch">
                            <input id="aec" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="aec"></label>
                        </div>
                    </div>
                    <div class="input-group" id="aec2-group">
                        <label for="aec2">AEC DSP</label>
                        <div class="switch">
                            <input id="aec2" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="aec2"></label>
                        </div>
                    </div>
                    <div class="input-group" id="ae_level-group">
                        <label for="ae_level">AE Level</label>
                        <div class="range-min">-2</div>
                        <input type="range" id="ae_level" min="-2" max="2" value="0" class="default-action">
                        <div class="range-max">2</div>
                    </div>
                    <div class="input-group" id="aec_value-group">
                        <label for="aec_value">Exposure</label>
                        <div class="range-min">0</div>
                        <input type="range" id="aec_value" min="0" max="1200" value="204" class="default-action">
                        <div class="range-max">1200</div>
                    </div>
                    <div class="input-group" id="agc-group">
                        <label for="agc">AGC</label>
                        <div class="switch">
                            <input id="agc" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="agc"></label>
                        </div>
                    </div>
                    <div class="input-group hidden" id="agc_gain-group">
                        <label for="agc_gain">Gain</label>
                        <div class="range-min">1x</div>
                        <input type="range" id="agc_gain" min="0" max="30" value="5" class="default-action">
                        <div class="range-max">31x</div>
                    </div>
                    <div class="input-group" id="gainceiling-group">
                        <label for="gainceiling">Gain Ceiling</label>
                        <div class="range-min">2x</div>
                        <input type="range" id="gainceiling" min="0" max="6" value="0" class="default-action">
                        <div class="range-max">128x</div>
                    </div>
                    <div class="input-group" id="bpc-group">
                        <label for="bpc">BPC</label>
                        <div class="switch">
                            <input id="bpc" type="checkbox" class="default-action">
                            <label class="slider" for="bpc"></label>
                        </div>
                    </div>
                    <div class="input-group" id="wpc-group">
                        <label for="wpc">WPC</label>
                        <div class="switch">
                            <input id="wpc" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="wpc"></label>
                        </div>
                    </div>
                    <div class="input-group" id="raw_gma-group">
                        <label for="raw_gma">Raw GMA</label>
                        <div class="switch">
                            <input id="raw_gma" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="raw_gma"></label>
                        </div>
                    </div>
                    <div class="input-group" id="lenc-group">
                        <label for="lenc">Lens Correction</label>
                        <div class="switch">
                            <input id="lenc" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="lenc"></label>
                        </div>
                    </div>
                    <div class="input-group" id="hmirror-group">
                        <label for="hmirror">H-Mirror</label>
                        <div class="switch">
                            <input id="hmirror" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="hmirror"></label>
                        </div>
                    </div>
                    <div class="input-group" id="vflip-group">
                        <label for="vflip">V-Flip</label>
                        <div class="switch">
                            <input id="vflip" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="vflip"></label>
                        </div>
                    </div>
                    <div class="input-group" id="dcw-group">
                        <label for="dcw">DCW (Downsize EN)</label>
                        <div class="switch">
                            <input id="dcw" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="dcw"></label>
                        </div>
                    </div>
                    <div class="input-group" id="colorbar-group">
                        <label for="colorbar">Color Bar</label>
                        <div class="switch">
                            <input id="colorbar" type="checkbox" class="default-action">
                            <label class="slider" for="colorbar"></label>
                        </div>
                    </div>
                    <!-- <div class="input-group" id="face_detect-group">
                        <label for="face_detect">Face Detection</label>
                        <div class="switch">
                            <input id="face_detect" type="checkbox" class="default-action">
                            <label class="slider" for="face_detect"></label>
                        </div>
                    </div> -->
                    <div class="input-group" id="ei_activate-group">
                        <label for="ei_activate">Edge Impulse Inference</label>
                        <div class="switch">
                            <input id="ei_activate" type="checkbox" class="default-action">
                            <label class="slider" for="ei_activate"></label>
                        </div>
                    </div>
                    <section id="buttons">
                        <button id="get-still">Capture Image</button>
                        <button id="toggle-stream">Start Stream </button>
                        <button id="ei_run" class="disabled" disabled="disabled">Run inference</button>
                        <button id="status">Get status</button>
                    </section>
                </nav>
            </div>
            <figure>
                <div id="stream-container" class="image-container hidden">
                    <div class="close" id="close-stream">×</div>
                    <img id="stream" src="">
                </div>
                <div id="output-container" class="image-container hidden">
                    <p>Output: </p>
                    <div class="output">
                        <pre id="output" class="code"></pre>
                    </div>
                </div>
            </figure>
        </div>
    </section>
    <script>
        document.addEventListener('DOMContentLoaded',
                function () {
                    function b(B) {
                        let C;
                        switch (B.type) {
                            case 'checkbox': C = B.checked ? 1 : 0; break;
                            case 'range': case 'select-one': C = B.value; break;
                            case 'button': case 'submit': C = '1'; break;
                            default: return;
                        }
                        const D = `${c}/control?var=${B.id}&val=${C}`;
                        fetch(D).then(E => { console.log(`request to ${D} finished, status: ${E.status}`) })
                    }
                    var c = document.location.origin;
                    const
                        e = B => { B.classList.add('hidden') },
                        f = B => { B.classList.remove('hidden') },
                        g = B => { B.classList.add('disabled'), B.disabled = !0 },
                        h = B => { B.classList.remove('disabled'), B.disabled = !1 },
                        i = (B, C, D) => { D = !(null != D) || D; let E; 'checkbox' === B.type ? (E = B.checked, C = !!C, B.checked = C) : (E = B.value, B.value = C), D && E !== C ? b(B) : !D && ('aec' === B.id ? C ? e(v) : f(v) : 'agc' === B.id ? C ? (f(t), e(s)) : (e(t), f(s)) : 'awb_gain' === B.id ? C ? f(x) : e(x) : 'ei_activate' === B.id && (C ? h(n) : g(n))) };
                    document.querySelectorAll('.close')
                        .forEach(B => { B.onclick = () => { e(B.parentNode) } }),
                        fetch(`${c}/status`).then(function (B) { return B.json() })
                            .then(function (B) {
                                document.querySelectorAll('.default-action')
                                    .forEach(C => { i(C, B[C.id], !1) })
                            });
                    const
                        j = document.getElementById('stream'),
                        k = document.getElementById('stream-container'),
                        l = document.getElementById('get-still'),
                        m = document.getElementById('toggle-stream'),
                        n = document.getElementById('ei_run'),
                        o = document.getElementById('close-stream'),
                        ei = document.getElementById('output-container'),
                        status = document.getElementById('status'),
                        p = () => { window.stop(), m.innerHTML = 'Start Stream' },
                        q = () => { j.src = `${c + ':9601'}/stream`, f(k), m.innerHTML = 'Stop Stream' };
                    l.onclick = () => { p(), j.src = `${c}/capture?_cb=${Date.now()}`, f(k) },
                        o.onclick = () => { p(), e(k) }, 
                        m.onclick = () => { const B = 'Stop Stream' === m.innerHTML; B ? p() : q() },
                        n.onclick = () => { p(), j.src = `${c}/inference?_cb=${Date.now()}`, f(k), 
                            j.addEventListener('load', (event) => {
                                if(j.complete){
                                    console.log('image has been loaded!');
                                    httpReq('inference-results');
                                   
                                }
                            }, { once: true });
                        },
                        status.onclick = () => { httpReq('status') };
                    const r = document.getElementById('agc'),
                        s = document.getElementById('agc_gain-group'),
                        t = document.getElementById('gainceiling-group');
                    r.onchange = () => { b(r), r.checked ? (f(t), e(s)) : (e(t), f(s)) };
                    const u = document.getElementById('aec'),
                        v = document.getElementById('aec_value-group');
                    u.onchange = () => { b(u), u.checked ? e(v) : f(v) };
                    const w = document.getElementById('awb_gain'),
                        x = document.getElementById('wb_mode-group');
                    w.onchange = () => { b(w), w.checked ? f(x) : e(x) };
                    const //y = document.getElementById('face_detect'),
                        z = document.getElementById('ei_activate'),
                        A = document.getElementById('framesize');
                    A.onchange = () => { b(A), 5 < A.value && (i(y, !1), i(z, !1)) },
                        //y.onchange = () => { return 6 < A.value ? (alert('Please select CIF or lower resolution before enabling this feature!'), void i(y, !1)) : void (b(y), !y.checked) }, 
                        z.onchange = () => { return 5 < A.value ? (alert('Please select QVGA or lower resolution before enabling this feature!'), void i(z, !1)) : void (b(z), z.checked ? (h(n)) : g(n)) };

                    
                    function httpReq(endpoint){    
                        fetch(`${window.location.href}`+endpoint)
                        //fetch(`http://192.168.1.198/`+endpoint)
                        .then(data => {return data.json()})
                        .then(res=>{
                            console.log(res); 
                            displayOutput(res)
                        })
                    };
                    
                    function displayOutput(data) {
                        f(ei); 
                        var pre = document.getElementById("output");
                            pre.innerHTML = '';
                        var code = document.createElement("code");
                            code.innerHTML = JSON.stringify(data, null, '\t');
                            pre.appendChild(code);
                    };

                    
                });
        </script>
</body>
</html>
)=====";

size_t index_custom_html_len = sizeof(index_custom_html)-1;

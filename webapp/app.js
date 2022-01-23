let serverAddr;
let webSocketServerAddr; //todo decide if needed
let refreshQuery;
let webSocketUrl;
let state;
let webSocket;

/**
 * Set up the webSocket connection and associated hooks to socket events
 * @return {undefined}
 */
function establishWebSocketConnection() {
    webSocket = new WebSocket(webSocketUrl);
    webSocket.onopen = function() {
        console.debug('WebSocket connected.');
    };

    webSocket.onmessage = handleWebSocketMessage;

    webSocket.onclose = function(e) {
        if(!document.hidden){
            console.debug('WebSocket is closed. Reconnect will be attempted in 1 second.', e.reason);
            setTimeout(function() {
                refreshState();
                establishWebSocketConnection();
            }, 1000);
        }
    };

    webSocket.onerror = function(err) {
        console.error('Closing socket due to error', err);
        disconnectWebSocket();
    };
}

/**
 * Renders the webapp <div>s within the <body>.
 * Adds some <link>s which are required to load CSS and favicons.
 * @return {undefined}
 */
function renderSkeleton(){
//    const webRoot = 'https://jackhiggins.ie/on-air'; //todo fix
    const webRoot = '.';

    document.body.innerHTML = `
  <div class="grid-container">
      <div id="red">
            <div class="sensor infoText" id="red-sensor"></div>
         </div>
      <div id="green">
            <div class="sensor infoText" id="green-sensor"></div>
         </div>
    </div>
  `;

    const appleIcon = document.createElement('link');
    appleIcon.rel = 'apple-touch-icon';
    appleIcon.sizes = '180x180';
    appleIcon.href = webRoot + '/favicon_io/apple-touch-icon.png?version=latest';
    document.head.appendChild(appleIcon);

    const icon = document.createElement('link');
    icon.rel = 'icon';
    icon.type = 'image/png';
    icon.href = webRoot + '/favicon_io/favicon-32x32.png?version=latest';
    document.head.appendChild(icon);

    const stylesheet = document.createElement('link');
    stylesheet.rel = 'stylesheet';
    stylesheet.type = 'text/css';
    stylesheet.href = webRoot + '/style.css?version=latest';
    document.head.appendChild(stylesheet);

    document.title = "ON AIR";
}

/**
 * Entry point to application from HTML. Initialises global state, renders app, and creates event hooks.
 * @return {undefined}
 */
function app(){ // eslint-disable-line no-unused-vars
//    serverAddr = '/api'; //todo fix
    serverAddr = 'http://on-air.fritz.box/api';
//    webSocketServerAddr = window.location.host; //todo fix
    webSocketServerAddr = 'on-air.fritz.box';

    state = {
        'bpm' : 100,
        'red' : false,
        'green' : false,
        'redTemperature' : 0.0,
        'greenTemperature' : 0.0,
        'title': '',
        'album': '',
        'artist': ''
    };

    renderSkeleton();

    const supportsTouch = 'ontouchstart' in window || navigator.msMaxTouchPoints;
    const eventName = supportsTouch ? 'touchend' : 'click';

    $('green').addEventListener(eventName, green);
    $('red').addEventListener(eventName, red);

    webSocketUrl = 'ws://' + webSocketServerAddr + ':81';
    refreshQuery = new XMLHttpRequest();

    refreshState();
    establishWebSocketConnection();
    document.addEventListener('visibilitychange', onBrowserShownOrHidden);
    window.onunload = window.onbeforeunload = disconnectWebSocket();
}

function disconnectWebSocket(){
    if(webSocket.readyState === WebSocket.OPEN){
        webSocket.close();
    }
}

/**
 * Handler for browser visibility change events
 * @return {undefined}
 */
function onBrowserShownOrHidden(){
    if(document.hidden){
        disconnectWebSocket();
    }else{
        refreshState();
        establishWebSocketConnection();
    }
}

/**
 * Parse each incoming webSocket message and update the global state
 * @param {MessageEvent} event WebSocket event message
 * @return {undefined}
 */
function handleWebSocketMessage(event){
    const messageContents = JSON.parse(event.data);

    for(const key of Object.keys(messageContents)){
        state[key] = messageContents[key];
    }

    updateScreen();
}

/**
 * Use the updated state to adjust on-screen elements and cause their redraw
 * @return {undefined}
 */
function updateScreen(){
    const greenDiv = $('green');
    const redDiv = $('red');
    const redSensorDiv = $('red-sensor');
    const greenSensorDiv = $('green-sensor');

    if(state['green']){
        greenDiv.className = 'bright-green';
    } else {
        greenDiv.className = 'dark-green';
    }
    if(state['red']){
        redDiv.className = 'bright-red';
    } else {
        redDiv.className = 'dark-red';
    }

    greenSensorDiv.innerHTML = "" + Math.round(state['greenTemperature']) + "°C";
    redSensorDiv.innerHTML = "" + Math.round(state['redTemperature']) + "°C";
}

/**
 * Manually perform a GET request to the server to get a full state update
 * @return {undefined}
 */
function refreshState(){
    refreshQuery = new XMLHttpRequest();
    refreshQuery.open('GET', serverAddr + '/status', true);
    refreshQuery.onreadystatechange = function(){
        if (refreshQuery.readyState === XMLHttpRequest.DONE) {
            if (refreshQuery.status === 200) {
                state =  JSON.parse(refreshQuery.responseText);
                updateScreen();
            }
        }
    };
    refreshQuery.send();
}

/**
 * Notify the server that the user wants to change the green light state
 * @return {undefined}
 */
function green(){
    refreshQuery.abort();
    const xhr = new XMLHttpRequest();
    if(state['green']){
        xhr.open('DELETE', serverAddr + '/green', true);
    }else{
        xhr.open('PUT', serverAddr + '/green', true);
    }

    xhr.send();
    state['green'] = !state['green'];
    updateScreen();
}

/**
 * Notify the server that the user wants to change the red light state
 * @return {undefined}
 */
function red(){
    refreshQuery.abort();
    const xhr = new XMLHttpRequest();
    if(state['red']){
        xhr.open('DELETE', serverAddr + '/red', true);
    }else{
        xhr.open('PUT', serverAddr + '/red', true);
    }

    xhr.send();
    state['red'] = !state['red'];
    updateScreen();
}

/**
 * jQuery-style replacement for document.getElementById
 * @param {string} elementId Element ID to get
 * @returns {HTMLElement} The found element
 */
function $(elementId){
    return document.getElementById(elementId);
}

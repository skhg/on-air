const httpServerAddr = 'http://on-air.fritz.box/api';
const webSocketServerAddr = window.location.host;
const webRoot = 'https://jackhiggins.ie/on-air';

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
    document.body.innerHTML = `
  <div class="grid-container">
    <div class="row1col1 grid-box">
        <i class="fas fa-microphone-alt"></i>
    </div>
    <div id="blank-display" class="row1col2 grid-box">
        <i class="far fa-times-circle"></i>
    </div>
    <div id="random-pixels" class="row2col1 grid-box">
        <i class="fas fa-braille"></i>
    </div>
    <div class="row2col2 grid-box"></div>
    <div class="row3col1 grid-box"></div>
    <div class="row3col2 grid-box"></div>
    <div class="row4col1 grid-box"></div>
    <div class="row4col2 grid-box"></div>
  </div>
  `;

    const appleIcon = document.createElement('link');
    appleIcon.rel = 'apple-touch-icon';
    appleIcon.sizes = '180x180';
    appleIcon.href = webRoot + '/favicon_io/apple-touch-icon.png?version=fba70920bebe27d74ac92e4b67f7822a14f59f72';
    document.head.appendChild(appleIcon);

    const icon = document.createElement('link');
    icon.rel = 'icon';
    icon.type = 'image/png';
    icon.href = webRoot + '/favicon_io/favicon-32x32.png?version=fba70920bebe27d74ac92e4b67f7822a14f59f72';
    document.head.appendChild(icon);

    const stylesheet = document.createElement('link');
    stylesheet.rel = 'stylesheet';
    stylesheet.type = 'text/css';
    stylesheet.href = webRoot + '/style.css?version=fba70920bebe27d74ac92e4b67f7822a14f59f72';
    document.head.appendChild(stylesheet);

    document.title = "ON AIR";
}

/**
 * Entry point to application from HTML. Initialises global state, renders app, and creates event hooks.
 * @return {undefined}
 */
function app(){ // eslint-disable-line no-unused-vars
    state = {
        'mode': 'off',
        'zoom': {
            'alert-active': false,
            'call-in-progress': false
        }
    };

    renderSkeleton();

    const supportsTouch = 'ontouchstart' in window || navigator.msMaxTouchPoints;
    const eventName = supportsTouch ? 'touchend' : 'click';

    $('random-pixels').addEventListener(eventName, randomPixels);
    $('blank-display').addEventListener(eventName, blankDisplay);

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
    const randomPixelsDiv = $('random-pixels');
    const blankDisplayDiv = $('blank-display');

    if(state['mode'] === "random-pixels"){
        randomPixelsDiv.className = 'row2col1 grid-box active';
    } else {
        randomPixelsDiv.className = 'row2col1 grid-box';
    }

    if(state['mode'] === "off"){
        blankDisplayDiv.className = 'row1col2 grid-box active';
    } else {
        blankDisplayDiv.className = 'row1col2 grid-box';
    }
    

}

/**
 * Manually perform a GET request to the server to get a full state update
 * @return {undefined}
 */
function refreshState(){
    refreshQuery = new XMLHttpRequest();
    refreshQuery.open('GET', httpServerAddr + '/status', true);
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
 * control over the "Random pixels animation" mode
 * @return {undefined}
 */
 function randomPixels(){
    refreshQuery.abort();
    const xhr = new XMLHttpRequest();
    
    const newModeJson = {
        "name": "random-pixels"
    };

    xhr.open('PUT', httpServerAddr + '/mode');
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(newModeJson));

    state['mode'] = 'random-pixels';
    updateScreen();
}

/**
 * control over the "Blank display" mode
 * @return {undefined}
 */
 function blankDisplay(){
    refreshQuery.abort();
    const xhr = new XMLHttpRequest();
    
    const newModeJson = {
        "name": "off"
    };

    xhr.open('PUT', httpServerAddr + '/mode');
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(newModeJson));

    state['mode'] = 'off';
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

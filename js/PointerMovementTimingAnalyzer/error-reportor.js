
// Collect and show uncaught script error and unhandled promise rejection, like XHTML pageerror reporter
(() => {
    let config = {
      maxBufferItems: 10,
    };
    let htmlTemplate = document.createElement('template');
    htmlTemplate.innerHTML = /*html*/`<style>
  :host {
    position: relative;
    border: 2px solid #c77;
    padding: 8px 16px;
    margin: 16px;
    background-color: #fdd;
    color: #333;
    color-scheme: light;
    display: block;
  }
  :host(:not([open])){
    display: none;
  }
  .message{
    white-space: pre-wrap;
    word-wrap: break-word;
    font-family: monospace;
    font-size: 12px;
  }
  button[value="close"]{
    position: absolute;
    right: 1em;
    top: 0.5em;
  }
  </style>
  <h3>This page contains the following errors:</h3>
  <div class="message"></div>
  <h3>For more details, see DevTools Console</h3>
  <button value="close">Close</button>
  `;
    let weakMap = new WeakMap();
    class ScriptErrorElement extends HTMLElement {
      static get observedAttributes() {
        return ['open'];
      }
      constructor() {
        super();
        const shadowRoot = this.attachShadow({mode: 'closed'});
        shadowRoot.appendChild(htmlTemplate.content.cloneNode(true));
        shadowRoot.querySelector('button[value="close"]').addEventListener('click', () => {
          this.open = false;
          this.dispatchEvent(new Event('close'));
        });
        weakMap.set(this, shadowRoot);
        this.style.margin = '16px';
        this._onclose = null;
      }
      show() {
        this.open = true;
      }
      get open() {
        return this.getAttribute('open') !== null;
      }
      set open(v) {
        let willOpen = !!v;
        let isOpen = this.getAttribute('open') !== null;
        if (isOpen && !willOpen) {
          this.removeAttribute('open');
        } else if (!isOpen && willOpen) {
          this.setAttribute('open', '');
        }
      }
      set value(value) {
        let shadowRoot = weakMap.get(this);
        shadowRoot.querySelector('.message').textContent = value;
      }
      get value() {
        let shadowRoot = weakMap.get(this);
        return shadowRoot.querySelector('.message').textContent;
      }
      set onclose(handler) {
        let valid = typeof handler === 'function' || typeof handler === 'object' * handler.handleEvent;
        this.removeEventListener('close', this._onclose);
        if (valid) {
          this.addEventListener('close', handler);
          this._onclose = handler;
        } else {
          this._onclose = null;
        }
      }
      get onclose() {
        return this._onclose;
      }
      connectedCallback() {
        // TODO
      }
      disconnectedCallback() {
        // TODO
      }
    }
    Object.defineProperty(ScriptErrorElement, Symbol.toStringTag, {
      configurable: true,
      value: 'ScriptErrorElement'
    });
    customElements.define('script-error', ScriptErrorElement);
  
    let errorContainer;
    let errorBuffer = [];
    let ensureErrorContainer = () => {
      if (!errorContainer) {
        errorContainer = document.body.insertAdjacentElement('afterbegin', new ScriptErrorElement());
        errorContainer.onclose = () => {
          errorBuffer.splice(0, errorBuffer.length);
        };
      }
      return errorContainer;
    };
    /**
     * @param {Error} error
     * @param {Promise} [promise]
     */
    /**
     * @param {string} summary
     * @param {string} filename
     * @param {string} lineno
     * @param {string} colno
     * @param {Error} [error]
     */
    function showError(errorOrSummary/* , filename, lineno, colno, error*/) {
      let summary, filename, lineno, colno, error;
      if (typeof errorOrSummary === 'string' && arguments.length >= 5) {
        summary = (errorOrSummary.startsWith('Uncaught ') ? '' : 'Uncaught ') + errorOrSummary;
        filename = arguments[1];
        lineno = arguments[2];
        colno = arguments[3];
        error = arguments[4];
        if (filename === '' || lineno === 0 && colno === 0) {
          return false;
        }
        if (error && filename === document.documentURI && error.name === 'SyntaxError' && lineno < 10) {
          // Ignore errors caused by Console eager evaluation
          return false;
        }
      } else if (Object.prototype.toString.call(errorOrSummary) === '[object Error]') {
        error = errorOrSummary;
        summary = 'Uncaught ' + (arguments[1] instanceof Promise ? '(in promise) ' : '') + error.toString();
        if (error.sourceURL !== undefined) {
          filename = error.sourceURL;
          lineno = error.line;
          colno = error.column;
        } else if (error.fileName !== undefined) {
          filename = error.fileName;
          lineno = error.lineNumber;
          colno = error.columnNumber;
        } else {
          let lines = error.stack ? error.stack.split(/\n {2,4}/, 2) : [];
          if (lines.length === 2) {
            let cause = lines[1];
            cause = cause.endsWith(')') ? cause.replace(/^at \S+ \(/g, '').slice(0, -1) :
              cause.replace(/^at /, '');
            filename = cause.replace(/:(\d+):(\d+)$/, ($0, $1, $2) => {
              lineno = $1;
              colno = $2;
              return '';
            });
          } else {
            filename = '';
            lineno = 0;
            colno = 0;
          }
        }
      } else {
        return false;
      }
      let errorMessage = summary + '\n    at ' + (filename ? filename : '<anonymous>') + ':' + lineno + ':' + colno + '\n';
      errorBuffer.push(errorMessage);
      if (errorBuffer.length > config.maxBufferItems) {
        errorBuffer.shift();
      }
      let container = ensureErrorContainer();
      let host = container.shadowRoot || container;
      host.value = errorBuffer.join('');
      container.open = true;
      return true;
    }
    window.addEventListener('error', (e) => {
      let {message: summary, filename, lineno, colno, error} = e;
      showError(summary, filename, lineno, colno, error);
    });
    window.addEventListener('unhandledrejection', (e) => {
      showError(e.reason, e.promise);
    });
  })();
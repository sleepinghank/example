export class AnimationFrameControl extends EventTarget {
    #callback;
    #delay;
    #adaptive = false;
    #queue = [];
    #handle;
    #handle2;
    /**
     * @param {Function} callback
     * @param {Object} [options] like {delay: 16.666666, adaptive: true}
     */
    constructor(callback, options = null) {
      super();
      if (typeof callback === 'number') {
        this.#callback = () => {};
        this.delay = callback;
      } else if (typeof callback === 'function') {
        this.#callback = callback;
        let opts = Object(options);
        this.delay = opts.delay || 1000 / 60;
        if ('adaptive' in opts)
          this.adaptive = opts.adaptive;
      } else {
        throw new Error('Invalid parameter callback');
      }
    }
    get callback() {
      return this.#callback;
    }
    set callback(v) {
      if (typeof v !== 'function') {
        throw new TypeError('Invalid callback');
      }
      this.#callback = v;
    }
    get delay() {
      return this.#delay;
    }
    set delay(v) {
      if (!isFinite(v)) {
        throw new TypeError('Invalid option delay ' + v);
      } else if (v < 2) {
        throw new RangeError('option delay should be a number at least 2');
      }
      this.#delay = v;
    }
    get adaptive() {
      return this.#adaptive;
    }
    set adaptive(v) {
      this.#adaptive = !!v;
    }
    start() {
      if (this.#handle)
        return;
      let startTime;
      let lastFrameNum = 0;
      let tick = (currentTime) => {
        let frameNum;
        if (this.#adaptive || (frameNum = Math.round((currentTime - startTime + 2) / this.#delay)) !== lastFrameNum) { // supports up to 480Hz frame rate
          let queue = this.#queue;
          let len = queue.length;
          if (len > 0) {
            this.#queue = []; // use a new queue before invoking callbacks
            for (let i = 0; i < len; i++) {
              try {
                queue[i](currentTime);
              } catch (e) {
                queueMicrotask(() => { throw e; });
              }
            }
          }
          if (frameNum) {
            lastFrameNum = frameNum;
          }
          this.#handle = requestAnimationFrame(tick);
          this.#callback(currentTime);
        } else {
          this.#handle = requestAnimationFrame(tick);
        }
      };
      this.#handle = requestAnimationFrame((currentTime) => {
        startTime = currentTime;
        this.#handle = requestAnimationFrame(tick);
        this.dispatchEvent(new Event('start'));
      });
    }
    stop() {
      if (!this.#handle)
        return;
      this.#handle = void cancelAnimationFrame(this.#handle);
      this.#handle2 = void clearInterval(this.#handle2);
      this.dispatchEvent(new Event('stop'));
    }
    requestAnimationFrame(callback) {
      this.#queue.push(callback);
    }
  }
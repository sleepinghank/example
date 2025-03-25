export class EventStat {
    #callback;
    #delay;
    #handle;
    #queue;
    /**
     * @param {Function} callback like (tps) => {}
     * @param {Object} [options] like {delay: 1000}
     */
    constructor(callback, options = {delay: 1000}) {
      this.callback = callback;
      this.delay = options?.delay;
      this.#queue = [];
    }
    get callback() {
      return this.#callback;
    }
    set callback(v) {
      if (typeof v !== 'function') {
        throw new TypeError('Invalid parameter callback');
      }
      this.#callback = v;
    }
    get delay() {
      return this.#delay;
    }
    set delay(v) {
      let delay;
      if (v === void 0) {
        delay = 1000;
      } else if (!isFinite(v)) {
        throw new TypeError('Invalid option delay');
      } else if (v < 2) {
        throw new RangeError('option delay should be at least 2ms');
      } else {
        delay = v;
      }
      this.#delay = delay;
      if (this.#handle) {
        this.#handle = void clearInterval(this.#handle);
        this.start();
      }
    }
    get running() {
      return !!this.#handle;
    }
    start() {
      if (this.#handle)
        return;
      this.#handle = setInterval(() => {
        let queue = this.#queue;
        let length = queue.length;
        let tps;
        if (length < 2) {
          tps = length;
        } else {
          let duration = queue[length - 1] - queue[0];
          let count = length - 1;
          tps = duration <= 0 ? NaN : +(1000 / (duration / count)).toFixed(1);
        }
        this.#queue = [];
        this.#callback(tps);
      }, this.#delay);
    }
    stop() {
      if (!this.#handle)
        return;
      this.#handle = void clearInterval(this.#handle);
    }
    report(e) {
      this.#queue.push(e.timeStamp);
    }
    reset() {
      this.#queue.length = 0;
    }
  }
  
  /**
   * This class is used to monitor animation frame rate
   * @emits {CustomEvent} stabilitychange
   */
  export class FrameStat extends EventTarget {
    #callback;
    #delay;
    #handle;
    #history = [];
    #stable = false;
    /**
     * @param {Function} callback like (fps) => {}
     * @param {Object} [options] like {delay: 1000}
     */
    constructor(callback, options = {delay: 1000}) {
      super();
      this.callback = callback;
      this.delay = options?.delay;
    }
    get callback() {
      return this.#callback;
    }
    set callback(v) {
      if (typeof v !== 'function') {
        throw new TypeError('Invalid parameter callback');
      }
      this.#callback = v;
    }
    get delay() {
      return this.#delay;
    }
    set delay(v) {
      let delay;
      if (v === void 0) {
        delay = 1000;
      } else if (!isFinite(v)) {
        throw new TypeError('Invalid option delay');
      } else if (v < 2) {
        throw new RangeError('option delay should be at least 2ms, got ' + v);
      } else {
        delay = v;
      }
      this.#delay = delay;
      if (this.#handle) {
        this.#handle = void cancelAnimationFrame(this.#handle);
        this.start();
      }
    }
    get running() {
      return !!this.#handle;
    }
    get stable() {
      return this.#stable;
    }
    start() {
      if (this.#handle)
        return;
      let lastTime;
      let currentCount = 0;
      let lastCount = 0;
      let tick = (currentTime) => {
        currentCount++;
        if (currentTime - lastTime > this.#delay - 2) {
          let fps = (currentCount - lastCount) / ((currentTime - lastTime) / 1000);
          lastCount = currentCount;
          lastTime = currentTime;
          this.#handle = requestAnimationFrame(tick);
          this.#callback(fps);
          // check FPS stablility
          let history = this.#history;
          let roundedFps = Math.round(fps);
          let len = history.push(roundedFps);
          if (len > 3) {
            // check last 4 FPS records
            let latestValues = history.slice(-3);
            let set = new Set(latestValues);
            if (this.#stable) {
              if (set.size > 1) { // considerred unstable
                this.#stable = false;
                this.dispatchEvent(new CustomEvent('stabilitychange', {detail: fps}));
              }
            } else {
              if (set.size === 1) { // considerred stable
                this.#stable = true;
                let avgFps = latestValues.reduce((n, v) => n + v, 0) / latestValues.length;
                this.dispatchEvent(new CustomEvent('stabilitychange', {detail: avgFps}));
              }
            }
            if (len > 60) {
              history.length = 3;
            }
          }
        } else {
          this.#handle = requestAnimationFrame(tick);
        }
      };
      this.#handle = requestAnimationFrame((currentTime) => {
        lastTime = currentTime;
        this.#handle = requestAnimationFrame(tick);
      });
    }
    stop() {
      if (!this.#handle)
        return;
      this.#handle = void cancelAnimationFrame(this.#handle);
    }
  }
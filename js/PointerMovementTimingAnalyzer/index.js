import {EventStat, FrameStat} from './rate-stat.js';
import {AnimationFrameControl} from './rate-sync.js';

const $ = (s, c = document) => c.querySelector(s);
const FIREFOX = navigator.userAgent.indexOf('Firefox/') > 0;

const support = {
  pointerrawupdate: 'onpointerrawupdate' in HTMLElement.prototype,
  coalescedEvents: 'getCoalescedEvents' in PointerEvent.prototype,
  unadjustedMovement: !!(document.exitPointerLock && window.chrome), // FIXME
};

const MouseButton = {
  LEFT: 0,
  MIDDLE: 1,
  RIGHT: 2,
  BACK: 3,
  FORWARD: 4,
};
const MouseButtons = {
  LEFT: 1,
  MIDDLE: 4,
  RIGHT: 2,
  BACK: 8,
  FORWARD: 16,
};
// 添加一个函数来获取基础URL
function getBaseUrl() {
  // 获取当前主机地址和端口
  const host = window.location.hostname; // 获取主机名（不包含端口）
  return `http://${host}:5000`; // 返回完整的基础URL
}
async function send_data(moveRecords) {
  // 使用动态构建的URL
  const touchEndpoint = `${getBaseUrl()}/touch`;
  await fetch(touchEndpoint, {
    method: 'POST',
    headers: {
     'Accept': 'application/json',
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(moveRecords)
  })
  .then(response => {
    console.log('Success:', response);
  })
  .catch((error) => console.error('Error:', error));
  console.log("发送成功");
}

function main() {
  const settings = {
    frameRate: 60, // initial value 60 is used to draw x-axis scales
    showUncoalescedPoints: true,
    showMergedPoints: true,
    showPointGuideline: true,
  };
  let refs = {
    showUncoalescedPoints: $('#showUncoalescedPoints'),
    showMergedPoints: $('#showMergedPoints'),
    showPointGuideline: $('#showPointGuideline'),
  };
  settings.showUncoalescedPoints = refs.showUncoalescedPoints.checked;
  refs.showUncoalescedPoints.addEventListener('click', (e) => {
    settings.showUncoalescedPoints = e.target.checked;
  });
  settings.showMergedPoints = refs.showMergedPoints.checked;
  refs.showMergedPoints.addEventListener('click', (e) => {
    settings.showMergedPoints = e.target.checked;
  });
  settings.showPointGuideline = refs.showPointGuideline.checked;
  refs.showPointGuideline.addEventListener('click', (e) => {
    settings.showPointGuideline = e.target.checked;
  });
  let axesCanvas = $('#axesCanvas');
  let dataCanvas = $('#dataCanvas');
  if (window.innerWidth > 1280) {
    dataCanvas.width = axesCanvas.width = document.body.clientWidth;
  }

  let axesContext = axesCanvas.getContext('2d');
  let axesRect = new DOMRect(0, 0, axesCanvas.clientWidth, axesCanvas.clientHeight);
  let currentTps = 0;
  // report rate
  let reportRateRect = new DOMRect(axesRect.x + axesRect.width - 120, axesRect.y, 120, 30);
  let eventStat = new EventStat((tps) => {
    drawRate(axesContext, reportRateRect, 'Report rate: ' + tps + ' Hz');
    currentTps = tps;
    // auto reduce graphics drawing to avoid stuck
    if (tps > 249) {
      settings.showPointGuideline = refs.showPointGuideline.checked = false;
      if (tps > 499 && (settings.showUncoalescedPoints && settings.showMergedPoints)) {
        settings.showUncoalescedPoints = refs.showUncoalescedPoints.checked = false;
      }
    }
  });
  document.addEventListener('visibilitychange', () => {
    if (document.visibilityState === 'visible') {
      eventStat.start();
    } else {
      eventStat.stop();
    }
  });
  // stat refresh rate
  let frameRateRect = new DOMRect(axesRect.x + axesRect.width - 120, axesRect.y + axesRect.height - 30, 120, 30);
  let lastFrameRate;
  let frameStat = new FrameStat((fps) => {
    if (!lastFrameRate) {
      settings.frameRate = Math.round(fps);
      drawAxes(axesContext, axesRect, settings);
    }
    lastFrameRate = fps;
    drawRate(axesContext, frameRateRect, 'Refresh rate: ' + fps.toFixed(1) + ' Hz');
  });
  let isApproxmate = (a, b, epsilon = 1) => {
    return a > b - epsilon && a < b + epsilon;
  };
  let isApproxmatelyWhole = (a, epsilon = 0.1) => {
    return isApproxmate(a, Math.round(a), epsilon);
  };
  frameStat.addEventListener('stabilitychange', (e) => {
    if (e.target.stable) {
      settings.frameRate = Math.round(e.detail);
      drawAxes(axesContext, axesRect, settings);
      let v = e.detail;
      if (v < 62) {
        frc.delay = 1000 / v;
      } else if (isApproxmatelyWhole(v / 60, 2)) {
        frc.delay = 1000 / 60;
      } else if (isApproxmate(v, 144, 2)) {
        frc.delay = 1000 / 72;
      } else if (isApproxmate(v, 165, 2)) {
        frc.delay = 1000 / 72;
      } else {
        frc.delay = 1000 / (v / Math.ceil(v / 60));
      }
    }
  });
  frameStat.start();
  console.log("frameStat.start");
  let dataContext = dataCanvas.getContext('2d');
  let moveRecords = [];
  let moveRecords2 = [];
  let requestedPointerLock; // 是否请求指针锁定
  let startingEvent; // 开始事件
  let pointerdownHandler = (e) => {
    if (startingEvent)
      return;
    if (e.pointerType === 'mouse') {
      if (e.button !== MouseButton.LEFT)
        e.preventDefault();
        requestedPointerLock = e.ctrlKey && support.unadjustedMovement;
      if (requestedPointerLock) {
        dataCanvas.addEventListener('pointermove', pointermoveHandlerOnce, {once: true});// 添加指针移动事件
      } else {
        dataCanvas.setPointerCapture(e.pointerId); // 设置指针捕获
      }
    } else {
      requestedPointerLock = false; // 设置为false
    }
    startingEvent = e; // 设置开始事件
    dataCanvas.addEventListener('pointermove', pointermoveHandler);// 添加指针移动事件
    dataCanvas.addEventListener('pointerup', pointerupHandler);// 添加指针抬起事件
    dataCanvas.addEventListener('pointercancel', pointerupHandler);// 添加指针取消事件
  };
  let pointermoveHandlerOnce = (e) => { 
    if (support.unadjustedMovement) {
      dataCanvas.requestPointerLock({unadjustedMovement: true});
    }
  };
  let lastMove;
  let pointermoveHandler = support.coalescedEvents ? ((e) => {
    if (startingEvent && e.pointerId !== startingEvent.pointerId)
      return;
    let now = performance.now();
    let {movementX, movementY} = e;
    if (FIREFOX) {
      if (lastMove) {
        movementX = e.clientX - lastMove.clientX;
        movementY = e.clientY - lastMove.clientY;
      }
      lastMove = e;
    }
    let coalescedRecords = [];
    moveRecords.push({timeStamp: e.timeStamp, movementX, movementY, timeReceived: now, coalescedRecords});
    moveRecords2.push({timeStamp: e.timeStamp, movementX, movementY, timeReceived: now, coalescedRecords});
    e.getCoalescedEvents().forEach((e2) => {
      eventStat.report(e2);
      coalescedRecords.push(getMoveRecord(e2, now));
    });
  }) : ((e) => {
    if (startingEvent && e.pointerId !== startingEvent.pointerId)
      return;
    let now = performance.now();
    eventStat.report(e);
    var temp = getMoveRecord(e, now);
    moveRecords.push(temp);
    moveRecords2.push(temp);
  });
  let pointerupHandler = (e) => {
    // 发送数据到服务器
    if (moveRecords2.length > 0) {
      send_data(moveRecords2);
    }
    moveRecords2 = [];
    if (!startingEvent || e.pointerId !== startingEvent.pointerId)
      return;
    if (e.pointerType === 'mouse') {
      if (requestedPointerLock) {
        dataCanvas.removeEventListener('pointermove', pointermoveHandlerOnce, {once: true});
        if (document.pointerLockElement) {
          document.exitPointerLock();
        }
      } else {
        dataCanvas.releasePointerCapture(e.pointerId);
      }
    }
    startingEvent = null;
    dataCanvas.removeEventListener('pointermove', pointermoveHandler);
    dataCanvas.removeEventListener('pointerup', pointerupHandler);
    dataCanvas.removeEventListener('pointercancel', pointerupHandler);
  };
  dataCanvas.addEventListener('pointerdown', pointerdownHandler);

  let lastRawUpdate;
  let getMoveRecord = (e, time) => {
    let {movementX, movementY} = e;
    if (FIREFOX) {
      if (lastRawUpdate) {
        movementX = e.clientX - lastRawUpdate.clientX;
        movementY = e.clientY - lastRawUpdate.clientY;
      }
      lastRawUpdate = e;
    }
    return {timeStamp: e.timeStamp, movementX, movementY, timeReceived: time};
  };
  let tick = () => {
    if (moveRecords.length > 0) {
      let now = performance.now();
      truncateRecords(moveRecords, now, axesRect.width - 36);
      dataContext.clearRect(0, 0, axesRect.width, axesRect.height);
      plotMoveRecords(dataContext, axesRect, now, moveRecords, settings);
    }
    frc.requestAnimationFrame(tick);
  };
  let frc = new AnimationFrameControl(1000 / 60);
  frc.addEventListener('start', () => {
    frc.requestAnimationFrame(tick);
    eventStat.start();
  });
  frc.start();
}
document.readyState === 'loading' ? document.addEventListener('DOMContentLoaded', main) : queueMicrotask(main);

function truncateRecords(events, now, displayInterval) {
  let len = events.length;
  let end = -1;
  for (let i = 0, e; i < len; i++) {
    e = events[i];
    let elapsedTime = now - e.timeStamp;
    if (elapsedTime >= displayInterval) {
      end = i;
    } else {
      break;
    }
  }
  if (end > -1) {
    events.splice(0, end + 1);
  }
}
function plotMoveRecords(context, rect, now, records, settings) {
  let {showPointGuideline} = settings;
  let middleY = rect.y + rect.height / 2;
  let rightX = rect.x + rect.width;
  let drawRecord = (e) => {
    let vsyncX = rightX - (now - e.timeReceived);
    let {movementX, movementY} = e;
    let y1 = middleY + movementY;
    let y2 = middleY + movementX;
    if (settings.showMergedPoints && showPointGuideline) {
      context.fillStyle = '#333333';
      if (movementY < -1) {
        context.fillRect(vsyncX, y1 + 1, 1, -movementY - 1);
      } else if (movementY > 1) {
        context.fillRect(vsyncX, middleY, 1, movementY - 1);
      }
      if (movementX < -1) {
        context.fillRect(vsyncX, y2 + 1, 1, -movementX - 1);
      } else if (movementX > 1) {
        context.fillRect(vsyncX, middleY, 1, movementX - 1);
      }
    }
    if (settings.showUncoalescedPoints && e.coalescedRecords) {
      e.coalescedRecords.forEach((e2) => {
        drawUncoalescedMove(e2, vsyncX);
      });
    }
    if (settings.showMergedPoints) {
      context.fillStyle = '#00ff00';
      context.fillRect(vsyncX - 0.5, y1 - 0.5, 2, 2);
      context.fillStyle = '#ff0000';
      context.fillRect(vsyncX - 0.5, y2 - 0.5, 2, 2);
    }
  };
  let drawUncoalescedMove = (e, vsyncX) => {
    let x = rightX - (now - e.timeStamp);
    let {movementX, movementY} = e;
    let y1 = middleY + movementY;
    let y2 = middleY + movementX;
    if (showPointGuideline) {
      context.strokeStyle = '#333333';
      context.setLineDash([2, 1]);
      if (movementY !== 0) {
        context.beginPath();
        context.moveTo(x, y1);
        context.lineTo(vsyncX, middleY);
        context.stroke();
      }
      if (movementX !== 0) {
        context.beginPath();
        context.moveTo(x, y2);
        context.lineTo(vsyncX, middleY);
        context.stroke();
      }
    }
    context.fillStyle = '#00ff00';
    context.fillRect(x, y1, 1, 1);
    context.fillStyle = '#ff0000';
    context.fillRect(x, y2, 1, 1);
  };
  for (let i = 0, e; i < records.length; i++) {
    e = records[i];
    let vsyncX = rightX - (now - e.timeReceived);
    drawRecord(e);
    context.fillStyle = '#ffffff';
    context.fillRect(vsyncX, middleY, 1, 1);
  }
}
function drawRate(context, rect, text) {
  context.clearRect(rect.x, rect.y, rect.width, rect.height);
  context.font = '10px sans-serif';
  context.textBaseline = 'top';
  context.fillStyle = '#c0c0c0';
  context.fillText(text, rect.x, rect.y + 5);
}
function drawAxes(context, rect, settings) {
  context.clearRect(rect.x, rect.y, rect.width, rect.height);
  let {frameRate} = settings;
  const marginLeft = 36;
  let yLeft = rect.x + marginLeft;
  let xTop = rect.y + rect.height / 2;
  context.fillStyle = '#333333';
  // Y-axis
  context.fillRect(yLeft, rect.y, 1, rect.height);
  // X-axis
  context.fillRect(yLeft, xTop, rect.width, 1);
  // Y-ruler
  context.font = '10px monospace';
  context.textBaseline = 'middle';
  for (let y = -300, step = 100; y <= 300; y += step) {
    let top = xTop + y;
    context.fillStyle = '#333333';
    context.fillRect(yLeft - 5, top, 5, 1);
    context.fillStyle = '#808080';
    context.fillText(y.toString().padStart(4, ' '), rect.x + 5, top + 1, marginLeft);
  }
  // X-ruler
  for (let x = rect.right - 1, step = 1000 / frameRate, i = 0; x > yLeft; x -= step, i++) {
    context.fillStyle = '#333333';
    if (i % frameRate < 1) {
      context.fillRect(x, xTop - 20, 1, 20);
    } else {
      context.fillRect(x, xTop - 5, 1, 5);
    }
  }
}

# Tasks and Microtasks in JavaScript runtime environment

[1]. The event loop: https://developer.mozilla.org/en-US/docs/Web/JavaScript/EventLoop
  - https://www.youtube.com/watch?v=8aGhZQkoFbQ  

[2]. Tasks, microtasks, queues and schedules: https://jakearchibald.com/2015/tasks-microtasks-queues-and-schedules/

[3].  The Node.js Event Loop, Timers, and process.nextTick(): https://nodejs.org/en/docs/guides/event-loop-timers-and-nexttick/#what-is-the-event-loop 

[4]. Promise: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise

[5].Using Promises: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Using_promises

[6]. Using microtasks in JavaScript with queueMicrotask(): https://developer.mozilla.org/en-US/docs/Web/API/HTML_DOM_API/Microtask_guide

- A microtask is a short function which is executed after the function or program which created it exits and only if the JavaScript execution stack is empty, but before returning control to the event loop being used by the user agent to drive the script's execution environment.
- JavaScript promises and the Mutation Observer API both use the microtask queue to run their callbacks

[7]. queueMicrotask: https://developer.mozilla.org/en-US/docs/Web/API/queueMicrotask

[8]. In depth: Microtasks and the JavaScript runtime environment: https://developer.mozilla.org/en-US/docs/Web/API/HTML_DOM_API/Microtask_guide/In_depth#tasks_vs_microtasks

- To run JavaScript code, the runtime engine maintains a set of agents in which to execute JavaScript code.  Each agent is made up of ：
  - a set of execution contexts, 
  - the execution context stack,
  - a main thread
  - a set for any additional threads that may be created to handle workers,
  - a task queue
  - a microtask queue
  - event loop

- When executing tasks from the task queue, the runtime executes each task that is in the queue at the moment a new iteration of the event loop begins. Tasks added to the queue after the iteration begins will not run until the next iteration.

- Each time a task exits, and the execution context stack is empty, each microtask in the microtask queue is executed, one after another. The difference is that execution of microtasks continues until the queue is empty—even if new ones are scheduled in the interim. In other words, microtasks can enqueue new microtasks and those new microtasks will execute before the next task begins to run, and before the end of the current event loop iteration.
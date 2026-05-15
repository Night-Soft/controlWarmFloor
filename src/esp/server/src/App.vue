<script setup>
import Time from './Time.vue'

import { ref, onMounted } from 'vue'

//const items = ref([[34, 124, 13, 41], [3, 4, 3, 1], [34, 124, 13, 41]]);
const items = ref([]);
const notification = ref();

function onRemoveArr(arr) {
  console.log("onRemove", items);
  for (let i = 0; i < items.value.length; i++) {
    if (arr == items.value[i]) {
      items.value.splice(i, 1);
      break;
    }
  }
}

const isRelayOn = ref(false)

function addTime() { items.value.push([]); }

function getTime() {
  fetch("/getTime")
    .then(response => {
      if (!response.ok) {
        const msg = "Failed get time!"
        showInfo(msg, true);
        throw new Error(msg);
      }

      return response.json();
    }).then(data => {
      if (data.status) {
        showInfo(data.status, true);
        return;
      }
      items.value = [...data];
    }).catch(error => {
      showInfo("Failed set time, error parse data!", true);
    });

  fetch("/api")
    .then(response => {
      if (!response.ok) {
        const msg = "Failed get api!"
        showInfo(msg, true);
        throw new Error(msg);
      }

      return response.json();
    }).then(data => {
      isRelayOn.value = data.relay;
    }).catch(error => {
      showInfo("Failed get api, error parse data!", true);
    })
}

function saveTime() {
  items.value.forEach((arr) => {
    for (let i = 0; i < arr.length; i++) {
      arr[i] = Number(arr[i]);
    }
  });
  const saveData = JSON.stringify(items.value);

  console.log("saveData", saveData);
  fetch('/saveTime', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: saveData
  })
    .then(response => {
      if (!response.ok) {
        const msg = 'Network error while saving time!'
        showInfo(msg, true);
        throw new Error(msg);
      }
      return response.json();
    })
    .then(data => { // Log the response from the server
      showInfo(`${data.status}!`);
    })
    .catch(error => {
      showInfo(error, true);
      console.error('Error:', error);
    });
}

const isNotification = ref(false);
let timeoutId = 0;
function showInfo(msg, isError = false) {
  if (typeof msg != 'string') throw new Error(`Wrong info msg type: ${typeof msg}`);
  console.log(msg);

  clearTimeout(timeoutId);
  timeoutId = setTimeout(() => {
    isNotification.value = false;
  }, 10000);

  isNotification.value = true;
  notification.value.textContent = msg;

  if (isError) {
    notification.value.parentElement.style.background = "rgb(220 0 0 / 35%)";
  } else {
    notification.value.parentElement.style.background = "";
  }
}

const closeNotification = () => {
  clearTimeout(timeoutId);
  isNotification.value = false;
}

onMounted(() => {
  console.log("onMounted");
  getTime();
});

function toggleRelay() {
  const url = "/toggleRelay?enable=" + !isRelayOn.value;
  fetch(url)
    .then(response => {
      if (!response.ok) {
        const msg = "Failed get state relay!"
        showInfo(msg, true);
        throw new Error(msg);
      }

      return response.json();
    }).then(data => {
      if (data.status == "ok") {
        const msg = "Failed get state of relay!"
        showInfo(msg, true);
        return;
      }

      const value = Boolean(data.status);
      isRelayOn.value = value;

    }).catch(error => {
      showInfo("Error parse data!", true);
    })
}

</script>

<template>
  <div class="main-time-content">
    <Transition name="notification">
      <div v-show="isNotification" class="notification">
        <span ref="notification">Notification</span>
        <svg @click="closeNotification" class="close-notification" xmlns="http://www.w3.org/2000/svg"
          viewBox="0 0 72 72">
          <path
            d="M 19 15 C 17.977 15 16.951875 15.390875 16.171875 16.171875 C 14.609875 17.733875 14.609875 20.266125 16.171875 21.828125 L 30.34375 36 L 16.171875 50.171875 C 14.609875 51.733875 14.609875 54.266125 16.171875 55.828125 C 16.951875 56.608125 17.977 57 19 57 C 20.023 57 21.048125 56.609125 21.828125 55.828125 L 36 41.65625 L 50.171875 55.828125 C 51.731875 57.390125 54.267125 57.390125 55.828125 55.828125 C 57.391125 54.265125 57.391125 51.734875 55.828125 50.171875 L 41.65625 36 L 55.828125 21.828125 C 57.390125 20.266125 57.390125 17.733875 55.828125 16.171875 C 54.268125 14.610875 51.731875 14.609875 50.171875 16.171875 L 36 30.34375 L 21.828125 16.171875 C 21.048125 15.391875 20.023 15 19 15 z" />
        </svg>
      </div>
    </Transition>

    <button @click="toggleRelay" class="add relay" :class="[isRelayOn ? 'relay-on' : 'relay-off']">
      {{ isRelayOn ? 'Turn off relay' : 'Turn on relay' }}
    </button>

    <div class="time-container">
      <div ref="info" class="info"><span>Setup pump turn time!</span></div>
      <Time @removeArr="onRemoveArr" :timeArr="arr" v-for="arr in items"></Time>
      <div class="btn-content">
        <button @click="addTime" class="add">Add +</button>
        <button @click="saveTime" class="add save-time">Save</button>
      </div>
    </div>
  </div>
</template>

<style scoped>
.notification {
  height: 60px;
  background: rgb(0 220 130 / 50%);
  backdrop-filter: blur(4px);
  border-radius: 16px;
  color: white;
  font-size: 1.5rem;
  align-self: center;
  display: flex;
  justify-content: center;
  align-items: center;
  top: 0px;
  position: fixed;
  padding: 6px;
  width: 100%;
  z-index: +1;
  width: anchor-size(--my-target width);
  box-shadow: 0 4px 30px rgb(0 0 0 / 10%);
  backdrop-filter: blur(5px);
  border: 1px solid rgba(255, 255, 255, 0.3);
}

.close-notification {
  width: 32px;
  fill: white;
  position: absolute;
  top: -1px;
  right: -1px;
  cursor: pointer;
  background: rgb(127 127 127 / 25%);
  border-radius: 0px 16px 0px;
  transition: 350ms;
}

.close-notification:hover {
  background: rgba(170, 170, 170, 0.45);

}

.time-container {
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin: auto;
  width: fit-content;
  padding: 16px;
  background-color: hsl(177deg 76% 55% / 5%);
  border-radius: 16px;
}

.main-time-content {
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin: auto;
  width: fit-content;
  anchor-name: --my-target;
}

.info {
  background: #00dc8259;
  color: #464646;
  padding: 16px;
  border-radius: 16px;
  font-size: 1.5rem;
  text-align: center;
  transition: 0.5s;
}

.add {
  background: #1a73e8;
  color: white;
  border-radius: 20px;
  height: 40px;
  border: none;
  font-size: 1.5rem;
  width: 200px;
  cursor: pointer;
  margin-top: auto;
  margin-bottom: auto;
  transition: 0.35s;
  align-self: center;
}

.add:hover {
  background: #4993f3;
}

.add:active {
  background: #0450b4;
}

.relay {
  height: 60px;
  width: 100%;
  margin-bottom: 10px;
}

.relay-off {
  background-color: rgba(34, 211, 238, 1);
}

.relay-on {
  background-color: #fd8f3b;
}

.relay:hover {
  background-color: rgb(34, 231, 238);

}

.relay-on:hover {
  background-color: #ffb92c;
}

.save-time {
  background: #00dc82;
}

.save-time:hover {
  background: #03f791;
}

.save-time:active {
  background: #01ba6d;
}

.btn-content {
  display: flex;
  justify-content: space-evenly;
  gap: 5px;
}

.notification-enter-active {
  animation: scale-in-top 0.7s linear both;
}

.notification-leave-active {
  animation: scale-in-top 0.7s linear reverse both;
}

@keyframes scale-in-top {
  0% {
    transform: scale(0) translateY(-100%);
    transform-origin: 50% 0%;
    opacity: 0;
  }

  100% {
    transform: scale(1) translateY(0%);
    transform-origin: 50% 0%;
    opacity: 1;
  }
}
</style>

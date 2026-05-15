<script setup>
import { ref, reactive } from 'vue'
const props = defineProps(["timeArr"]);
const emit = defineEmits(["removeArr"]);
let timee = ref();
console.log(timee);
function btnRemove() {
  emit("removeArr", props.timeArr);
}

function onTimeInput(event) { // todo check max value
  let value = Number(event.target.value);
  if (value > event.target.max) value = event.target.max;
  if (value < event.target.min) value = event.target.min;
  if (value !== event.target.value) event.target.value = value;

  switch (event.target.name) {
    case "startHours":
      props.timeArr[0] = value;
      break;
    case "startMinutes":
      props.timeArr[1] = value;
      break;
    case "workingHours":
      props.timeArr[2] = value;
      break;
    case "workingMinutes":
      props.timeArr[3] = value;
      break;
  }
}

</script>

<template>
  <div class="time-content">
    <div class="input-content">
      <div ref="timee" class="time time-pos">
        <span class="span-txt">Start:</span>
        <div class="input-group">
          <input :value="timeArr[0]" @input="onTimeInput" autocomplete="off" class="input" required type="number"
            name="startHours" max="23" min="0">
          <label class="user-label">Hours</label>
        </div>
        <span>:</span>
        <div class="input-group">
          <input :value="timeArr[1]" @input="onTimeInput" autocomplete="off" required class="input" type="number" name="startMinutes"
            max="59" min="0">
          <label class="user-label">Minutes</label>
        </div>
      </div>
      <div ref="timee" class="time time-pos">
        <span class="span-txt">Working:</span>
        <div class="input-group">
          <input :value="timeArr[2]" @input="onTimeInput" autocomplete="off" class="input" required type="number" name="workingHours"
            max="23" min="0">
          <label class="user-label">Hours</label>
        </div>
        <span>:</span>
        <div class="input-group">
          <input :value="timeArr[3]" @input="onTimeInput" autocomplete="off" required class="input" type="number" name="workingMinutes"
            max="59" min="0">
          <label class="user-label">Minutes</label>
        </div>
      </div>
    </div>
    <button v-on:click="btnRemove" class="btn-remove">remove</button>
  </div>
</template>

<style scoped>
.time-content {
  background: color(srgb 0.1 0.45 0.91 / 0.07);
  padding: 10px;
  border-radius: 20px;
  gap: 20px;
  display: flex;
  flex-direction: row;
}

.input-content {
  display: flex;
  flex-direction: column;
  align-items: end;
  gap: 10px;
}

.btn-remove {
  background: #1a73e8;
  color: white;
  border-radius: 20px;
  height: 40px;
  border: none;
  font-size: 1rem;
  width: 100px;
  cursor: pointer;
  margin-top: auto;
  margin-bottom: auto;
  transition: 0.35s
}

.btn-remove:hover {
  background: #4993f3;
}

.btn-remove:active {
  background: #0450b4;
}

.time {
  display: flex;
  align-items: center;
  gap: 4px;
}

.span-txt {
  color: #5d5d5d;
}

.time span {
  font-size: 1.5rem;
  color: #8e8e8e;
}


/* From Uiverse.io by alexruix */
.input-group {
  position: relative;
}

.input {
  width: 5rem;
  border: solid 1.5px #8e8e8e;
  border-radius: 1rem;
  background: none;
  padding: 0.5rem;
  font-size: 1rem;
  color: #5d5d5d;
  transition: border 150ms cubic-bezier(0.4, 0, 0.2, 1);
}

.user-label {
  position: absolute;
  left: 15px;
  pointer-events: none;

  transition: 150ms cubic-bezier(0.4, 0, 0.2, 1);
  transform: translateY(-50%) scale(0.9);
  background-color: #eff5fd;
  padding: 0 .2em;
  color: #1a73e8;
  border: solid 1px;
  border-radius: 8px;
}

.input:focus {
  outline: none;
  border-color: #1a73e8;
}

/* For Chrome, Safari, Edge, and Opera */
input::-webkit-outer-spin-button,
input::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}

/* For Firefox */
input[type=number] {
  -moz-appearance: textfield;
}
</style>

document.getElementById("powerBtn").onclick = function () {
  fetch("/power").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

document.getElementById("tempUpBtn").onclick = function () {
  fetch("/tempup").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

document.getElementById("tempDownBtn").onclick = function () {
  fetch("/tempdown").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

document.getElementById("fanBtn").onclick = function () {
  fetch("/fan").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

document.getElementById("modeBtn").onclick = function () {
  fetch("/mode").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

document.getElementById("swingBtn").onclick = function () {
  fetch("/swing").then((response) => {
    if (response.ok) {
      updateStatus();
    }
  });
};

function updateStatus() {
  fetch("/status")
    .then((response) => response.json())
    .then((data) => {
      document.getElementById("currentTemp").innerText = data.temperature;
      document.getElementById("currentMode").innerText = data.mode;
      document.getElementById("currentFan").innerText = data.fanSpeed;
      document.getElementById("powerBtn").innerText =
        "Power: " + (data.power ? "ON" : "OFF");
      document.getElementById("fanBtn").innerText =
        "Fan Speed: " + data.fanSpeed;
      document.getElementById("modeBtn").innerText = "Mode: " + data.mode;
      document.getElementById("swingBtn").innerText =
        "Swing: " + (data.swing ? "ON" : "OFF");
    });
}

updateStatus();

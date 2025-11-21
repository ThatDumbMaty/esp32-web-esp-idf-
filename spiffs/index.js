const modeButton = document.getElementById('modeButton');

modeButton.addEventListener('click', function() {
    document.body.classList.toggle('darkMode');
});

const ledOnButton = document.getElementById('ledOn');
const ledOffButton = document.getElementById('ledOff');

ledOnButton.addEventListener('click', () => {
    fetch("/gpio/on", { method: "POST" })
        .then(() => console.log("LED ON"));
});

ledOffButton.addEventListener('click', () => {
    fetch("/gpio/off", { method: "POST" })
        .then(() => console.log("LED OFF"));
});
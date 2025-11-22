const modeButton = document.getElementById('modeButton');

modeButton.addEventListener('click', function() {
    document.body.classList.toggle('darkMode');
});

const ledOnButton = document.getElementById('ledOn');
const ledOffButton = document.getElementById('ledOff');
const sensorButton = document.getElementById('sensor');

ledOnButton.addEventListener('click', () => {
    fetch("/gpio/on", { method: "POST" })
        .then(() => console.log("LED ON"));
});

ledOffButton.addEventListener('click', () => {
    fetch("/gpio/off", { method: "POST" })
        .then(() => console.log("LED OFF"));
});

sensorButton.addEventListener('click', () => {
    getDistance();
});
async function getDistance() {
    const outputElement = document.getElementById('sensor-distance');
    outputElement.textContent = 'Fetching data...'; 

    try {
        const response = await fetch(`/gpio/sensor`, { method: "POST" });
        
        if (!response.ok) {
            throw new Error(`Chyba HTTP: ${response.status}`);
        }

        const data = await response.json();

        if (data.distance_cm !== undefined) {
            outputElement.textContent = `${data.distance_cm} cm`;
        } else if (data.error) {
            outputElement.textContent = `Chyba: ${data.error}`;
        }
        
    } catch (error) {
        console.error("Došlo k chybě při komunikaci s ESP32:", error);
        outputElement.textContent = 'CHYBA komunikace!';
    }
}
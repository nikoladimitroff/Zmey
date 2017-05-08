let actor = {
    x: 10,
    y: 20,
    z: 30,
    interval: console.error.bind(console, "kopr")
};

let nextFrame = function (delta) {
    actor.x += delta * 10;
    console.log(delta, actor.x, actor.y);
    console.error("Wazaa from nextFrame");
};

setTimeout(() => {
    console.warn("From timeout");
}, 5000);

setInterval(actor.interval, 1000);

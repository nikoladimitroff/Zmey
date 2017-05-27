let actor = {
    pos: new Vector3(1, 0, 0),
    interval: console.error.bind(console, "kopr")
};

let nextFrame = function (delta) {
    let transformManager = world.getManager(Manager.Transform);
    let transform = transformManager.lookup(world.getFirstEntity());
    let posy = new Vector3(0.1, 0, 0);
    //posy.x = 0.2;
    const pozz = transform.position();
    console.log(pozz.x, "|", JSON.stringify(posy.x));
    pozz.x = 0.1;
    //console.log(JSON.stringify(transform.position()));
    //actor.pos.x++;
    //console.log(actor.pos.x, actor.pos.length());
};

class TestScriptManager {
    constructor() {}
    simulate(delta) {
        console.log(world.getManager())
    }
}
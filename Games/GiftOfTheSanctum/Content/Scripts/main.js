let actor = {
    pos: new Vector3(1, 0, 0),
    interval: console.error.bind(console, "kopr")
};

let tagManager = world.getManager(Manager.Tag);
const entity = tagManager.findFirstByTag(new Name("Player1"));

let nextFrame = function (delta) {
    let transformManager = world.getManager(Manager.Transform);
    let transform = transformManager.lookup(entity);
    let posy = new Vector3(0.1, 0, 0);
    //posy.x = 0.2;
    let pozz = transform.position();
    //pozz.y += 0.01;
    //console.log(pozz.x, "|", JSON.stringify(posy.x));
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
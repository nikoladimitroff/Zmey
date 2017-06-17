let actor = {
    pos: new Vector3(1, 0, 0),
    interval: console.error.bind(console, "kopr")
};

let tagManager = world.getManager(Manager.Tag);
const entity = tagManager.findFirstByTag(new Name("Player1"));

const transformManager = world.getManager(Manager.Transform);

const walkAction = new Name("walk");
Modules.inputController.addListenerForAction(walkAction, 0, function (axisValue) {
    const playerTransform = transformManager.lookup(entity);
    playerTransform.position().x += 0.1;
})
let nextFrame = function (delta) {
};

import { Resizer } from "./resizer";
import { Scene } from "./scene";

async function fetchJSON(url: string): Promise<any> {
    let promise = new Promise(resolve => {
        let req = new XMLHttpRequest();
        req.overrideMimeType("application/json");
        req.open('GET', url, true);
        req.onload  = function() {
            resolve(eval("new Object(" + req.responseText + ")"));
        };
        req.send(null);
    });
    return await promise;
}

class GameLoop {
    private scene: Scene;
    private context: CanvasRenderingContext2D;
    constructor(context: CanvasRenderingContext2D) {
        this.context = context;
    }
    public async init(): Promise<any> {
        Resizer.installHandler(this.context.canvas);
        const sceneDescription = await fetchJSON("content/scene.json");
        this.scene = await Scene.parseSceneDescription(sceneDescription);
    }
    public run(): void {
        const runFrame = () => {
            this.scene.render(this.context);
            requestAnimationFrame(runFrame);
        }
        runFrame();
    }
}


async function main(): Promise<any> {
    const canvas = document.querySelector("canvas") as HTMLCanvasElement;
    console.log(canvas);

    const loop = new GameLoop(canvas.getContext("2d") as CanvasRenderingContext2D);
    await loop.init();
    loop.run();
}

main();
import { Mouser } from "./mouser"
import { Resizer } from "./resizer";
import { Scene } from "./scene";
import { Camera } from './camera';
import {PlayerBook} from './player';
import {UnitBook} from './unittypes';
import * as math from './math';
import { BrushManager } from './brushes';
import { EconomyManager } from './economy';

async function fetchJSON(url: string): Promise<any> {
    let promise = new Promise(resolve => {
        let req = new XMLHttpRequest();
        req.overrideMimeType("application/json");
        req.open('GET', url, true);
        req.onload  = function() {
            console.log( req.responseText);
            resolve(eval("new Object(" + req.responseText + ")"));
        };
        req.send(null);
    });
    return await promise;
}

class GameLoop {
    private scene: Scene;
    private playerBook: PlayerBook;
    private unitBook: UnitBook;
    private camera: Camera;
    private context: CanvasRenderingContext2D;
    private brushManager: BrushManager;
    private economy: EconomyManager;
    constructor(context: CanvasRenderingContext2D) {
        this.context = context;
        this.camera = new Camera(context);
        this.camera.setZoomLevels(0.5, 2);
        this.economy = new EconomyManager();
        this.brushManager = new BrushManager(this.economy);
    }
    public async init(): Promise<any> {
        Mouser.installHandler();
        Resizer.installHandler(this.context.canvas);
        // Install extra handler for mouse wheel
        window.addEventListener("wheel", (event) =>
            // Normalize by the window height; otherwise the values are in pixels scrolled
            this.camera.zoom(event.wheelDelta / window.innerHeight)
        , false);


        const unitsDescription = await fetchJSON("content/units.json");
        this.unitBook = await UnitBook.parseBook(unitsDescription);

        const playersDescription = await fetchJSON("content/player.json");
        this.playerBook = await PlayerBook.parseBook(playersDescription);

        const resourceDescription = await fetchJSON("content/resources.json");
        this.economy.parseResourceBook(resourceDescription);

        const sceneDescription = await fetchJSON("content/scene.json");
        this.scene = await Scene.parseSceneDescription(sceneDescription, this.playerBook, this.unitBook);
        this.economy.parseResourceNodes(sceneDescription.resources);
        this.economy.spawnNodes(this.scene);

        console.log(this.playerBook);
        console.log(this.unitBook);
    }
    private updateFrame(): void {
        // If the mouse is within the innermost X% of the window, move the camera around
        const mousePos = Mouser.state.screenPosition;
        const windowBorderCoeff = 0.025;
        const cameraMovementSpeed = 2.5;
        let cameraOffset = new math.Vector2(0, 0);
        if (mousePos.x < windowBorderCoeff * this.context.canvas.width) {
            cameraOffset.x = -cameraMovementSpeed;
        } else if (mousePos.x > (1 - windowBorderCoeff) * this.context.canvas.width) {
            cameraOffset.x = cameraMovementSpeed;
        }
        if (mousePos.y < windowBorderCoeff * this.context.canvas.height) {
            cameraOffset.y = -cameraMovementSpeed;
        } else if (mousePos.y > (1 - windowBorderCoeff) * this.context.canvas.height) {
            cameraOffset.y = cameraMovementSpeed;
        }
        if (!cameraOffset.isZero()) {
            this.camera.translate(cameraOffset);
        }

        this.brushManager.update(this.scene, this.camera);
    }
    private renderFrame(): void {
        this.scene.render(this.context, this.camera);
    }
    public run(): void {
        const runFrame = () => {
            if (this.camera.needsWorldSize()) {
                this.camera.setWorldSize(this.scene.worldSize);
            }
            this.updateFrame();
            this.renderFrame();
            requestAnimationFrame(runFrame);
        }
        runFrame();
    }
}

async function main(): Promise<any> {
    const canvas = document.querySelector("canvas") as HTMLCanvasElement;
    const loop = new GameLoop(canvas.getContext("2d") as CanvasRenderingContext2D);
    await loop.init();
    loop.run();
    // Expose as global for debugging
    (window as any).loop = loop;
}

main();
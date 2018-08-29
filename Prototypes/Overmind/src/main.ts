import { Mouser } from "./mouser"
import { Resizer } from "./resizer";
import { Scene } from "./scene";
import { Camera } from './camera';
import * as math from './math';
import { BrushManager } from './brushes';
import { EconomyManager } from './economy';
import { UIController } from './ui';
import { Terrain } from './terrain';
import { fetchJSON } from './utils'
import { GameLibrary } from './gamelibrary'
class GameLoop {
    private scene: Scene;

    private camera: Camera;
    private context: CanvasRenderingContext2D;
    private brushManager: BrushManager;
    private economy: EconomyManager;
    private terrain: Terrain;
    private library: GameLibrary;
    private ui: UIController;
    private tiles: HTMLImageElement;
    constructor(context: CanvasRenderingContext2D) {
        this.context = context;
        this.camera = new Camera(context);
        this.camera.setZoomLevels(0.1, 10);
        this.economy = new EconomyManager();
        this.brushManager = new BrushManager();
        this.ui = new UIController();
        this.terrain = new Terrain();
        this.tiles = new Image();
        this.library = new GameLibrary()
    }
    public async init(): Promise<any> {
        Mouser.installHandler();
        Resizer.installHandler(this.context.canvas);
        // Install extra handler for mouse wheel
        window.addEventListener("wheel", (event) =>
            // Normalize by the window height; otherwise the values are in pixels scrolled
            this.camera.zoom(event.wheelDelta / window.innerHeight)
        , false);

        await this.library.init("content/units.json",
         "content/player.json",
          "content/buildings.json")

        const resourceDescription = await fetchJSON("content/resources.json");
        this.economy.parseResourceBook(resourceDescription);

        const sceneDescription = await fetchJSON("content/scene.json");
        this.scene = await Scene.parseSceneDescription(sceneDescription, this.library);
        this.economy.parseResourceNodes(sceneDescription.resources);
        this.economy.spawnNodes(this.scene);
        // TODO: make this synchronously
        this.tiles.onload = function(): void {
            this.terrain.render(this.scene.terrain.getContext('2d'), this.tiles);
        }.bind(this);
        this.tiles.src = './content/tiles/terrain.png';

        this.brushManager.startTimers(this.economy, this.scene);

        this.ui.initialize(this.economy, this.library.unitBook, this.library.playerBook, this.scene, sceneDescription.humanPlayer, this.camera.displayOptions);
 
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
            window.requestAnimationFrame(runFrame);
        }
        runFrame();
    }

    public setCameraOption(option: string, value: any):void {
        (this.camera.displayOptions as any)[option] = value;
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
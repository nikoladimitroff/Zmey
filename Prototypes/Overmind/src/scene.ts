import { Camera } from './camera';
import * as math from './math';
import  { PlayerBook } from './player';
import  { UnitBook } from './unittypes';

export class GameObject {

    public position: math.Vector2;
    public color: string | null;
    public image: HTMLImageElement | null;

    constructor() {
        this.position = new math.Vector2(0, 0);
    }
    protected isValid(): boolean {
        return this.color !== null || this.image !== null;
    }
    public render(_0: CanvasRenderingContext2D, _1 : Camera): void {
        console.assert(false);
    }
}

export class RectangleGameObject extends GameObject {
    public width: number;
    public height: number;
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        console.assert(this.isValid());
        const w = camera.getZoom() * this.width;
        const h = camera.getZoom() * this.height;
        const x = (this.position.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.position.y - camera.getPosition().y) * camera.getZoom();
        if (this.color) {
            context.fillStyle = this.color;
            context.fillRect(x - w / 2, y -  h / 2, w, h);
        } else {
            context.drawImage(this.image as HTMLImageElement, x - w / 2, y - h / 2, w, h);
        }
    }
}

export class CircleGameObject extends GameObject {
    public radius: number;
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        console.assert(this.isValid());
        const r = camera.getZoom() * this.radius;
        const x = (this.position.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.position.y - camera.getPosition().y) * camera.getZoom();
        context.save();
        context.beginPath();
        context.arc(x, y, r, 0, Math.PI * 2, true);
        context.closePath();
        if (this.color) {
            context.fillStyle = this.color;
            context.fill();
        } else {
            context.clip();
            const diameter = 2 * r;
            context.drawImage(this.image as HTMLImageElement,
                x - r, y - r, diameter, diameter);
        }
        context.restore();
    }
}

export class Scene {
    public objects: Array<GameObject>;
    public worldSize: math.Vector2;
    private terrain: HTMLImageElement;
    constructor() {
        this.objects = [];
        this.worldSize = new math.Vector2(0, 0);
    }

    private static parseGameObject(obj: any, _0: PlayerBook, units: UnitBook): GameObject {
        let newGameObject: GameObject = new GameObject();

        newGameObject = new RectangleGameObject();
        (newGameObject as RectangleGameObject).width = 25;
        (newGameObject as RectangleGameObject).height = 25;


        newGameObject.position.x = obj.x;
        newGameObject.position.y = obj.y;
        newGameObject.image = document.createElement("img") as HTMLImageElement;
        newGameObject.image.src = units.getUnitByType(obj.type).image;
        console.log(newGameObject.image);
        return newGameObject;
    }
    public static parseSceneDescription(json: any, players: PlayerBook, units: UnitBook): Scene {
        let scene = new Scene();
        for (const obj of json.objects) {
            const newGameObject = Scene.parseGameObject(obj, players, units);
            scene.objects.push(newGameObject);
        }

        scene.terrain = document.createElement("img") as HTMLImageElement;
        scene.terrain.src = json.terrain;
        scene.terrain.onload = () => scene.worldSize.set(scene.terrain.naturalWidth, scene.terrain.naturalHeight);
        return scene;
    }
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        context.clearRect(0, 0, context.canvas.width, context.canvas.height);
        const cameraPosition = camera.getPosition();
        const viewport = camera.getVisibleViewport();
        context.drawImage(this.terrain,
            cameraPosition.x, cameraPosition.y, viewport.x, viewport.y,
            0, 0, context.canvas.width, context.canvas.height);

        for (const obj of this.objects) {
            obj.render(context, camera);
        }
    }
}
import { Camera } from './camera';
import * as math from './math';

export class GameObject {
    public x: number;
    public y: number;
    public color: string | null;
    public image: HTMLImageElement | null;
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
        const x = (this.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.y - camera.getPosition().y) * camera.getZoom();
        console.log(this.x);
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
        const x = (this.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.y - camera.getPosition().y) * camera.getZoom();
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
    private static parseGameObject(obj: any): GameObject {
        let newGameObject: GameObject = new GameObject();
        if (obj.type === "rect") {
            newGameObject = new RectangleGameObject();
            (newGameObject as RectangleGameObject).width = obj.width;
            (newGameObject as RectangleGameObject).height = obj.height;
        } else if (obj.type === "circle") {
            newGameObject = new CircleGameObject();
            (newGameObject as CircleGameObject).radius = obj.radius;
        } else {
            console.assert(false && "Parsing error: unknown game object");
        }

        newGameObject.x = obj.x;
        newGameObject.y = obj.y;
        if (obj.texture.startsWith("color:")) {
            newGameObject.color = obj.texture.replace("color:", "").trim()
        } else if (obj.texture.startsWith("url:")) {
            let texture = document.createElement("img") as HTMLImageElement;
            texture.src = obj.texture.replace("url:", "").trim();
            newGameObject.image = texture;
        }
        else {
            console.assert(false && "Parsing error: each gameobject.texture must be either a color or an image!");
        }
        return newGameObject;
    }
    public static parseSceneDescription(json: any): Scene {
        let scene = new Scene();
        for (const obj of json.objects) {
            const newGameObject = Scene.parseGameObject(obj);
            scene.objects.push(newGameObject);
        }
        scene.terrain = document.createElement("img") as HTMLImageElement;
        console.assert(json.terrain.startsWith("url:"));
        scene.terrain.src = json.terrain.replace("url:", "").trim();
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

        context.save();
        camera.applyTransform();
        for (const obj of this.objects) {
            obj.render(context, camera);
        }
        context.restore();
    }
}
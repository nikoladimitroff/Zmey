export class GameObject {
    public x: number;
    public y: number;
    public color: string | null;
    public image: HTMLImageElement | null;
    protected isValid(): boolean {
        return this.color !== null || this.image !== null;
    }
    public render(_: CanvasRenderingContext2D): void {
        console.assert(false);
    }
}

export class RectangleGameObject extends GameObject {
    public width: number;
    public height: number;
    public render(context: CanvasRenderingContext2D): void {
        console.assert(this.isValid());
        if (this.color) {
            context.fillStyle = this.color;
            context.fillRect(this.x, this.y, this.width, this.height);
        } else {
            context.drawImage(this.image as HTMLImageElement, this.x, this.y, this.width, this.height);
        }
    }
}

export class CircleGameObject extends GameObject {
    public radius: number;
    public render(context: CanvasRenderingContext2D): void {
        console.assert(this.isValid());

        context.save();
        context.beginPath();
        context.arc(this.x, this.y, this.radius, 0, Math.PI * 2, true);
        context.closePath();
        if (this.color) {
            context.fillStyle = this.color;
            context.fill();
        } else {
            context.clip();
            const diameter = 2 * this.radius;
            context.drawImage(this.image as HTMLImageElement,
                this.x - this.radius, this.y - this.radius, diameter, diameter);
        }
        context.restore();
    }
}

export class Scene {
    public objects: Array<GameObject>;
    constructor() {
        this.objects = [];
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
        return scene;
    }
    public render(context: CanvasRenderingContext2D): void {
        context.clearRect(0, 0, context.canvas.width, context.canvas.height);
        for (const obj of this.objects) {
            obj.render(context);
        }
    }
}
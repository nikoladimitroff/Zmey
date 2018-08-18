

import { Camera } from './camera';
import * as math from './math';

export class GameObject {

    public position: math.Vector2;
    public color: string | null;
    public image: HTMLImageElement | null;
    // NOTE: The following 2 properties aren't available on all game objects
    // so only use if you are sure they are units
    public owningPlayer: string | undefined;
    public unitType: string | undefined;

    constructor() {
        this.position = new math.Vector2(0, 0);
    }
    protected isValid(): boolean {
        return this.color !== null || this.image !== null;
    }
    public render(_0: CanvasRenderingContext2D, _1 : Camera): void {
        console.assert(false);
    }
    public renderBorder(_0: CanvasRenderingContext2D, _1 : Camera, _2: string): void {
        console.assert(false);
    }

    public rendeCount(_0: CanvasRenderingContext2D, _1: Camera, _2: string): void {
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

    public renderBorder(context: CanvasRenderingContext2D, camera: Camera, borderColor: string): void {
        console.assert(this.isValid());
        const w = camera.getZoom() * this.width;
        const h = camera.getZoom() * this.height;
        const x = (this.position.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.position.y - camera.getPosition().y) * camera.getZoom();
        context.strokeStyle = borderColor;
        const borderWidth = 2;
        context.lineWidth = borderWidth;
        context.strokeRect(x - w / 2 - borderWidth, y -  h / 2 - borderWidth, w + borderWidth, h + borderWidth);
    }

    public rendeCount(context: CanvasRenderingContext2D, camera: Camera, count: string): void {
        console.assert(this.isValid());
        const w = camera.getZoom() * this.width;
        const h = camera.getZoom() * this.height;
        const x = (this.position.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.position.y - camera.getPosition().y) * camera.getZoom();
        context.fillStyle = "rgb(255,255,255)";
        context.fillText(count , x - w / 2, y - h, 2000);
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

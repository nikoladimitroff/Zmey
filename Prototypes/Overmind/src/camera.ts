import * as math from "./math"

export class Camera {
    private context: CanvasRenderingContext2D;
    private zoomLevel: number;
    private position: math.Vector2;
    private worldSize: math.Vector2;
    constructor(context: CanvasRenderingContext2D) {
        this.context = context;

        this.zoomLevel = 1;
        this.position = math.Vector2.zero.clone();
        this.worldSize = math.Vector2.zero.clone();
    }
    public applyTransform(): void {
        this.context.translate(-this.position.x, -this.position.y);
        this.context.scale(this.zoomLevel, this.zoomLevel);
    }
    public transformVector(vec: math.Vector2): math.Vector2 {
        return vec.subtract(this.position).multiply(this.zoomLevel);
    }
    public translate(vec: math.Vector2): void {
        this.position = this.position.add(vec);
        const viewport = this.getVisibleViewport();
        this.position = math.Vector2.max(this.position, math.Vector2.zero);
        this.position = math.Vector2.min(this.position, this.worldSize.subtract(viewport));
    }
    public zoom(zoomDelta: number): void {
        this.zoomLevel += zoomDelta;
    }
    public getVisibleViewport(): math.Vector2 {
        return new math.Vector2(this.context.canvas.width, this.context.canvas.height).multiply(1 / this.zoomLevel);
    }
    public getPosition(): math.Vector2 {
        return this.position.clone();
    }
    public setWorldSize(size: math.Vector2): void {
        this.worldSize.set(size);
    }

}
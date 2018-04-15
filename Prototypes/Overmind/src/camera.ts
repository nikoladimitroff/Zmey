import * as math from "./math"

export class Camera {
    private context: CanvasRenderingContext2D;
    private zoomLevel: number;
    private minZoom: number;
    private maxZoom: number;
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
        this.position = this.position.clamp(math.Vector2.zero, this.worldSize.subtract(viewport));
    }
    public zoom(zoomDelta: number): void {
        // Can't zoom out more than the map size
        const canvasSize = new math.Vector2(this.context.canvas.width, this.context.canvas.height);
        const canvasToWorld = canvasSize.divide(this.worldSize).max();
        const minConstrainedZoom = Math.max(this.minZoom, canvasToWorld);
        const clamp = (number: number, min: number, max: number) => Math.max(min, Math.min(number, max));
        this.zoomLevel = clamp(this.zoomLevel + zoomDelta, minConstrainedZoom, this.maxZoom);
        // if the visible viewport now doesn't fill the screen, reverse translate so it does
        const newViewport = this.getVisibleViewport();
        this.position = this.position.clamp(math.Vector2.zero, this.worldSize.subtract(newViewport));
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
    public needsWorldSize(): boolean {
        return this.worldSize.isZero();
    }
    public setZoomLevels(min: number, max: number): void {
        this.minZoom = min;
        this.maxZoom = max;
    }

}
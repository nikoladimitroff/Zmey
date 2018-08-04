import { GameObject, Scene } from './scene';
import { Vector2 } from './vector';
import { Camera } from './camera';
import { Mouser } from './mouser';
import { MouseButton } from './mouser';
import { ResourceNode } from './economy';


enum BrushType {
    GatheringPoint = "red",
    MoveCorridor = "blue"
}

export class BrushGameObject extends GameObject {
    public path: Vector2[];
    public type: BrushType;
    private minPoint: Vector2;
    private maxPoint: Vector2;
    constructor(type: BrushType) {
        super();
        this.path = [];
        this.type = type;
        this.color = type;
    }
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        console.assert(this.isValid());
        context.beginPath();
        const mapPointToScreen = (p: Vector2) => camera.transformVector(p);
        const mappedPoints = this.path.map(mapPointToScreen);
        context.moveTo(mappedPoints[0].x, mappedPoints[0].y);
        for (const point of mappedPoints) {
            context.lineTo(point.x, point.y);
        }
        context.lineWidth = 10 * camera.getZoom();
        context.strokeStyle = this.color as string;
        context.stroke();

        // UNCOMMENT TO DISPLAY DEBUG AABB
        //if (this.minPoint) {
        //    context.strokeStyle = "purple";
        //    const min = camera.transformVector(this.minPoint);
        //    const max = camera.transformVector(this.maxPoint);
        //    context.strokeRect(min.x, min.y, max.x - min.x, max.y - min.y);
        //}
    }
    public computeAABB(): void {
        this.minPoint = this.path.reduce((min, x) => Vector2.min(min, x), this.path[0]);
        this.maxPoint = this.path.reduce((max, x) => Vector2.max(max, x), this.path[0]);
    }
    public liesOnPath(point: Vector2): boolean {
        return point.x <= this.maxPoint.x && point.y <= this.maxPoint.y &&
            point.x >= this.minPoint.x && point.y >= this.minPoint.y;
    }

    public generatePointInAABB(): Vector2 {
        return this.minPoint.lerpTo(this.maxPoint, Math.random());
    }
}

enum Keys {
    One = 49,
    Two = 50,
    Three = 51
}

export class BrushManager {
    constructor() {
        this.activeBrush = null;
        this.activeBrushType = BrushType.GatheringPoint;
        this.gatheringPoints = [];
        this.corridors = [];
        document.addEventListener('keydown', (event) => {
            if (event.keyCode == Keys.One) {
                this.activeBrushType = BrushType.GatheringPoint;
            }
            else if (event.keyCode == Keys.Two) {
                this.activeBrushType = BrushType.MoveCorridor;
            }
        });
    }
    public update(scene: Scene, camera: Camera): void {
        const worldPos = camera.untransformVector(Mouser.state.screenPosition);
        if (Mouser.state.buttonState[MouseButton.Left]) {
            if (this.activeBrush == null) {
                this.activeBrush = new BrushGameObject(this.activeBrushType);
                scene.objects.push(this.activeBrush);
            }
            if (this.activeBrush.path.length === 0 || this.activeBrush.path[this.activeBrush.path.length - 1] !== worldPos) {
                this.activeBrush.path.push(worldPos);
            }
        } else if (this.activeBrush !== null) {
            this.applyBrushEffect();
            this.activeBrush = null;
        }
        // move units
        for (const corridor of this.corridors) {
            const startingGatheringPoint = this.gatheringPoints.find(x => x.liesOnPath(corridor.path[0]));
            const endGatheringPoint = this.gatheringPoints.find(x => x.liesOnPath(corridor.path[corridor.path.length - 1]));
            if (!startingGatheringPoint || !endGatheringPoint) {
                continue;
            }
            corridor.color = 'yellow';
            const canUnitBeMoved = (x: GameObject) =>
                x.constructor !== ResourceNode && // Don't move resources
                startingGatheringPoint.liesOnPath(x.position) &&
                Math.random() < 0.01;
            const unitsToMove = scene.objects.filter(canUnitBeMoved);
            for(let unit of unitsToMove)
            {
                unit.position = endGatheringPoint.generatePointInAABB();
            }
        }
    }

    private applyBrushEffect(): void {
        if (!this.activeBrush) {
            return;
        }
        this.activeBrush.computeAABB();
        if (this.activeBrush.type == BrushType.GatheringPoint) {
            this.gatheringPoints.push(this.activeBrush);
        }
        else if (this.activeBrush.type == BrushType.MoveCorridor) {
            this.corridors.push(this.activeBrush);
        }
    }

    private activeBrush: BrushGameObject | null;
    private activeBrushType: BrushType;

    private gatheringPoints: BrushGameObject[];
    private corridors: BrushGameObject[];
}
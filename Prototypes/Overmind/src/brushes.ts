import { Scene } from './scene';
import { GameObject } from './gameobject';
import { Vector2 } from './vector';
import { Camera, DisplayOptions } from './camera';
import { Mouser } from './mouser';
import { MouseButton } from './mouser';
import { ResourceNode, EconomyManager } from './economy';
import { TimerManager } from './timers';
import { BattleSim } from './battlesim'
//import { Player } from './player';

class BrushType {

    public baseColor: string;
    public rectColor: string;
    constructor(baseColor: string, rectColor: string) {
        this.baseColor = baseColor;
        this.rectColor = rectColor;
    }
    public static GatheringPoint:BrushType = new BrushType('rgba(218, 68, 121, 1)', 'rgba(218, 68, 121 , 0.31)');
    public static MoveCorridor:BrushType = new BrushType('rgba(235, 219, 189, 1)' , 'rgba(235, 219, 189, 0.31)');
    public static MiningArea:BrushType = new BrushType('rgba(90, 91, 91, 1)' , 'rgba(90, 91, 91, 0.31)');
    public static UrbanAreas:BrushType = new BrushType('rgba(117,199,34, 1)' ,'rgba(117,199,34, 0.31)');
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
    }
    
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        if(!this.shouldShow(camera.displayOptions)) {
            return;
        }
        console.assert(this.isValid());
        context.beginPath();
        const mapPointToScreen = (p: Vector2) => camera.transformVector(p);
        const mappedPoints = this.path.map(mapPointToScreen);
        context.moveTo(mappedPoints[0].x, mappedPoints[0].y);
        for (const point of mappedPoints) {
            context.lineTo(point.x, point.y);
        }
        context.lineWidth = 10 * camera.getZoom();
        context.strokeStyle = this.type.baseColor;
        context.stroke();

        if (this.minPoint && camera.displayOptions.enableBrushesABBA) {
           context.fillStyle = this.type.rectColor;
           const min = camera.transformVector(this.minPoint);
           const max = camera.transformVector(this.maxPoint);
           context.fillRect(min.x, min.y, max.x - min.x, max.y - min.y);
        }
    }
    public computeAABB(): void {
        this.minPoint = this.path.reduce((min, x) => Vector2.min(min, x), this.path[0]);
        this.maxPoint = this.path.reduce((max, x) => Vector2.max(max, x), this.path[0]);
    }
    public liesWithinPath(point: Vector2): boolean {
        return point.x <= this.maxPoint.x && point.y <= this.maxPoint.y &&
            point.x >= this.minPoint.x && point.y >= this.minPoint.y;
    }

    public generatePointInAABB(): Vector2 {
        return this.minPoint.lerpTo(this.maxPoint, Math.random());
    }

    private shouldShow(opt: DisplayOptions): boolean {
        if(this.type == BrushType.GatheringPoint && !opt.hideGatheringPoints) {
            return true;
        }
        else if(this.type == BrushType.MoveCorridor && !opt.hideCorridors) {
            return true;
        }
        else if(this.type == BrushType.MiningArea && !opt.hideMiningAreas) {
            return true;
        }
        else if(this.type == BrushType.UrbanAreas && !opt.hideUrbanAreas) {
            return true;
        }
        return false;
    }
}

enum Keys {
    One = 49,
    Two = 50,
    Three = 51,
    Four = 52,
}

export class BrushManager {
    constructor() {
        this.activeBrush = null;
        this.activeBrushType = BrushType.GatheringPoint;
        this.gatheringPoints = [];
        this.corridors = [];
        this.miningAreas = [];
        this.urbanAreas = [];
        document.addEventListener('keydown', (event) => {
            if (event.keyCode == Keys.One) {
                this.activeBrushType = BrushType.GatheringPoint;
            }
            else if (event.keyCode == Keys.Two) {
                this.activeBrushType = BrushType.MoveCorridor;
            }
            else if (event.keyCode == Keys.Three) {
                this.activeBrushType = BrushType.MiningArea;
            } else if (event.keyCode == Keys.Four) {
                this.activeBrushType = BrushType.UrbanAreas;
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
    }

    public startTimers(economy: EconomyManager, scene: Scene): void {
        TimerManager.startTimer(1000, () => economy.gatherResourcesFrom(this.miningAreas), true);
        TimerManager.startTimer(1000, this.transportUnitsAcrossCorridors.bind(this, scene), true);
        TimerManager.startTimer(1000, BattleSim.simulateBattles.bind(null, scene, this.gatheringPoints), true);
    }



    private transportUnitsAcrossCorridors(scene: Scene): void {
        // move units
        for (const corridor of this.corridors) {
            const startingGatheringPoint = this.gatheringPoints.find(x => x.liesWithinPath(corridor.path[0]));
            const endGatheringPoint = this.gatheringPoints.find(x => x.liesWithinPath(corridor.path[corridor.path.length - 1]));
            if (!startingGatheringPoint || !endGatheringPoint) {
                continue;
            }
            corridor.color = 'yellow';
            const canUnitBeMoved = (x: GameObject) =>
                x.constructor !== ResourceNode && // Don't move resources
                startingGatheringPoint.liesWithinPath(x.position);
            const corridorBandwithPerSecond = 1;
            const unitsToMove = scene.objects.filter(canUnitBeMoved).slice(0, corridorBandwithPerSecond);
            for (let unit of unitsToMove)
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
        else if (this.activeBrush.type == BrushType.MiningArea) {
            this.miningAreas.push(this.activeBrush);
        } 
        else if (this.activeBrush.type == BrushType.UrbanAreas) {
            this.urbanAreas.push(this.activeBrush);
        }
    }

    private activeBrush: BrushGameObject | null;
    private activeBrushType: BrushType;

    private gatheringPoints: BrushGameObject[];
    private corridors: BrushGameObject[];
    private miningAreas: BrushGameObject[];
    private urbanAreas: BrushGameObject[];
}
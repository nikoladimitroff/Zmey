import {RectangleGameObject} from './gameobject';
import {checkAndSet} from './utils';
import { Camera } from './camera'


export class Building extends RectangleGameObject {
    public player: string;
    public type: BuildingPrototype;


    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        switch(this.type.name) {
            case 'MainTower':
                this.renderTower(context, camera);
                break;
            case 'Tent':
                RectangleGameObject.prototype.render.call(this, context, camera);
                break;
            default:
                RectangleGameObject.prototype.render.call(this, context, camera);
                break;
        }
    }

    private renderTower(context: CanvasRenderingContext2D, camera: Camera): void {
        RectangleGameObject.prototype.render.call(this, context, camera);
        context.fillStyle = 'rgba(155,48,255, 0.1)';
        context.strokeStyle = 'rgba(155,48,255, 0.8)';
        const x = (this.position.x - camera.getPosition().x) * camera.getZoom();
        const y = (this.position.y - camera.getPosition().y) * camera.getZoom();
        context.beginPath();
        context.arc(x, y,150,0,2*Math.PI);
        context.fill();
        context.stroke();
    }

}

export class BuildingPrototype {
    public name: string;
    public controlRange: number;
    public populationLimit: number;
}

export class BuildingBook {
    public prototypes: BuildingPrototype[];
    constructor() {
        this.prototypes = [];
    }

    private static parseUnitTypes(obj: any): BuildingPrototype {
        let newPrototype = new BuildingPrototype();
        checkAndSet(newPrototype, obj, "name");
        checkAndSet(newPrototype, obj, "controlRange");
        checkAndSet(newPrototype, obj, "populationLimit");
        return newPrototype;
    }

    public static parseBook(json: any): BuildingBook {
        let book = new BuildingBook();
        for (const obj of json.buildingTypes) {
            book.prototypes.push(BuildingBook.parseUnitTypes(obj));
        }
        return book;
    }
}

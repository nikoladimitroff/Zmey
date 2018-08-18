import {RectangleGameObject} from './gameobject';
import {checkAndSet} from './utils';

export class UnitDescription {
    public mentalToughness: number;
    public physicalToughness: number;
    public baseDamage: number;
    public armorFactor: number;
    public weaponFactor: number;
    public skillFactor: number;
}

export class Army extends RectangleGameObject {
    public player: string;
    public description: UnitDescription;
    public count: number;
}

export class UnitPrototype {
    public name: string;
    public description: UnitDescription;
    public image: string;
}

export class UnitBook {
    public prototypes: UnitPrototype[];
    constructor() {
        this.prototypes = [];
    }

    private static parseUnitTypes(obj: any): UnitPrototype {
        let newPrototype = new UnitPrototype();
        checkAndSet(newPrototype, obj, "name");
        checkAndSet(newPrototype, obj, "image");
        checkAndSet(newPrototype, obj, "description");
        return newPrototype;
    }

    public static parseBook(json: any): UnitBook {
        let book = new UnitBook();
        for (const obj of json.unitTypes) {
            book.prototypes.push(UnitBook.parseUnitTypes(obj));
        }
        return book;
    }
}
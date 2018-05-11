import {UnitDescription} from './battlesim';
import {RectangleGameObject} from './scene';
import {checkAndSet} from './utils';


export class Unit extends RectangleGameObject {
    public player: string;
    public description: UnitDescription;
}

export class UnitPrototype {
     public description: UnitDescription;
     public image: HTMLImageElement | null;
}

export class UnitBook {
    public prototypes: any;
    constructor() {
        this.prototypes = {};
    }

    public getUnitByType(type: string): UnitPrototype {
        return this.prototypes[type];
    }

    private static parseUnitTypes(obj: any): UnitPrototype {
        let newPrototype = new UnitPrototype();
        checkAndSet(newPrototype, obj, "name");
        checkAndSet(newPrototype, obj, "image");
        for(var prop in newPrototype.description) {
            checkAndSet(newPrototype.description, obj, prop);    
        }
        return newPrototype;
    }    

    public static parseBook(json: any): UnitBook {
        let book = new UnitBook();
        for (const obj of json.unitTypes) {
            book.prototypes[obj.name] = (UnitBook.parseUnitTypes(obj));
        }
        return book;
    }
}
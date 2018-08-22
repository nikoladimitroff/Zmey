import { Camera } from './camera';
import * as math from './math';
import  { PlayerBook} from './player';
import  { RectangleGameObject, GameObject } from './gameobject'
import  { UnitBook, Army } from './unittypes';
import  { Building } from './buildings'
import  { GameLibrary } from './gamelibrary'

export class Scene {
    public objects: Array<GameObject>;
    public worldSize: math.Vector2;
    public terrain: HTMLCanvasElement;
    public players: PlayerBook;
    public controlSuppliers: Array<GameObject>;
    constructor(players: PlayerBook) {
        this.players = players;
        this.objects = [];
        this.controlSuppliers = [];
        this.worldSize = new math.Vector2(0, 0);
        this.terrain = document.createElement("canvas");
    }

    private static parseGameObject(obj: any, _0: PlayerBook, units: UnitBook): GameObject {

        let newGameObject: GameObject = new RectangleGameObject();
        (newGameObject as RectangleGameObject).width = 25;
        (newGameObject as RectangleGameObject).height = 25;


        newGameObject.position.x = obj.x;
        newGameObject.position.y = obj.y;
        newGameObject.image = document.createElement("img") as HTMLImageElement;
        const unitType = units.prototypes.find(u => u.name === obj.type);
        if (!unitType) {
            throw new Error(`Unit type ${obj.type} is not recognized!`);
        }
        newGameObject.image.src = unitType.image;
        newGameObject.owningPlayer = obj.player;
        newGameObject.unitType = obj.type;
        return newGameObject;
    }

    private static parseArmy(obj: any, _0: PlayerBook, units: UnitBook): GameObject {

        let newGameObject: Army = new Army();
        (newGameObject as RectangleGameObject).width = 25;
        (newGameObject as RectangleGameObject).height = 25;


        newGameObject.position.x = obj.x;
        newGameObject.position.y = obj.y;
        newGameObject.image = document.createElement("img") as HTMLImageElement;
        const unitType = units.prototypes.find(u => u.name === obj.type);
        if (!unitType) {
            throw new Error(`Unit type ${obj.type} is not recognized!`);
        }
        newGameObject.image.src = unitType.image;
        newGameObject.description = unitType.description;
        newGameObject.owningPlayer = obj.player;
        newGameObject.unitType = obj.type;
        newGameObject.count = obj.count;
        return newGameObject;
    }

    private static parseStartingBuilding(json: any, library: GameLibrary) : GameObject {
        let startingBuilding = new Building();
        startingBuilding.width = 150;
        startingBuilding.height = 150;
        startingBuilding.position.x = json.startingPosition.x;
        startingBuilding.position.y = json.startingPosition.y;
        startingBuilding.image = document.createElement("img") as HTMLImageElement;
        startingBuilding.image.src = "./content/buildings/main-tower.png";
        let mainTowerProto = library.buildingBook.prototypes.find(u => u.name === 'MainTower');
        if(mainTowerProto) {
            startingBuilding.type = mainTowerProto;
        }
        return startingBuilding;
    }

    public static parseSceneDescription(json: any, library: GameLibrary): Scene {
        let scene = new Scene(library.playerBook);
        for (const obj of json.objects) {
            const unitType = library.unitBook.prototypes.find(u => u.name === obj.type);
            if(unitType)
            {
                const newGameObject = Scene.parseArmy(obj, library.playerBook, library.unitBook);
                scene.objects.push(newGameObject);
            }
            else
            {
                const newGameObject = Scene.parseGameObject(obj, library.playerBook, library.unitBook);
                scene.objects.push(newGameObject);
            }
        }

        let mainBuilding = Scene.parseStartingBuilding(json, library);
        scene.objects.push(mainBuilding);
        scene.players.at(0).buildings.push(mainBuilding as Building);

        scene.worldSize.x = scene.terrain.width = json.terrain.width * 32 /*tile size*/;
        scene.worldSize.y = scene.terrain.height = json.terrain.height * 32 /*tile size*/;

        return scene;
    }
    public render(context: CanvasRenderingContext2D, camera: Camera): void {
        context.clearRect(0, 0, context.canvas.width, context.canvas.height);
        const cameraPosition = camera.getPosition();
        const viewport = camera.getVisibleViewport();
        context.drawImage(this.terrain,
            cameraPosition.x, cameraPosition.y, viewport.x, viewport.y,
            0, 0, context.canvas.width, context.canvas.height);

        for (const obj of this.objects) {
            obj.render(context, camera);
            if (obj.owningPlayer) {
                const owner = this.players.players.find(p => p.name === obj.owningPlayer);
                if (owner === undefined) {
                    throw new Error(`Found an owned game object by invalid owner: ${obj.owningPlayer}`);
                }
                obj.renderBorder(context, camera, owner.flag);
                if(obj.constructor === Army) {
                    obj.rendeCount(context, camera, Math.ceil((obj as Army).count).toString());
                }
            }
        }
    }
}
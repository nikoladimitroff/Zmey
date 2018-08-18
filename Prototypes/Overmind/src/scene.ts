import { Camera } from './camera';
import * as math from './math';
import  { PlayerBook } from './player';
import  { RectangleGameObject, GameObject } from './gameobject'
import  { UnitBook, Army, UnitPrototype } from './unittypes';

export class Scene {
    public objects: Array<GameObject>;
    public worldSize: math.Vector2;
    private terrain: HTMLImageElement;
    public players: PlayerBook;
    constructor(players: PlayerBook) {
        this.players = players;
        this.objects = [];
        this.worldSize = new math.Vector2(0, 0);
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

    public static parseSceneDescription(json: any, players: PlayerBook, units: UnitBook): Scene {
        let scene = new Scene(players);
        for (const obj of json.objects) {
            const unitType = units.prototypes.find(u => u.name === obj.type);
            if(unitType)
            {
                const newGameObject = Scene.parseArmy(obj, players, units);
                scene.objects.push(newGameObject);
            }
            else
            {
                const newGameObject = Scene.parseGameObject(obj, players, units);
                scene.objects.push(newGameObject);
            }
        }

        scene.terrain = document.createElement("img") as HTMLImageElement;
        scene.terrain.src = json.terrain;
        scene.terrain.onload = () => scene.worldSize.set(scene.terrain.naturalWidth, scene.terrain.naturalHeight);
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
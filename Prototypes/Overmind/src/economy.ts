import { CircleGameObject } from './gameobject';
import { Scene } from './scene';
import { BrushGameObject } from './brushes';

export class Resource {
    public name: string;
    public image: string;
    constructor(name: string, imageUrl: string) {
        this.name = name;
        this.image = imageUrl;
    }
}

export class ResourceNode extends CircleGameObject {
    public type: string;
    public gainPerSecond: number;

    public static parseResourceDescription(obj: any, resourceList: Resource[]): ResourceNode {
        let node = new ResourceNode();
        node.type = obj.type;
        node.gainPerSecond = obj.gainPerSecond;
        node.radius = obj.size;
        node.position.x = obj.x;
        node.position.y = obj.y;
        node.image = document.createElement("img") as HTMLImageElement;
        const resource = resourceList.find(r => r.name === node.type);
        if (!resource) {
            throw new Error(`Resource identifier ${obj.type} is not recognized!`);
        }
        node.image.src = resource.image;
        return node;
    }
}
export class EconomyManager {
    public treasury: { [resourceType:string]: number };
    public resources: Resource[];

    private nodes: ResourceNode[];
    constructor() {
        this.resources = [];
        this.nodes = [];
        this.treasury = {};
    }

    public parseResourceBook(json: any): void {
        this.resources = json.resourceTypes.map((obj: any) => new Resource(obj.name, obj.image));
        this.resources.forEach(r => this.treasury[r.name] = 0);
    }

    public parseResourceNodes(json: any): void {
        this.nodes = json.map((n: any) => ResourceNode.parseResourceDescription(n, this.resources));
    }

    public spawnNodes(scene: Scene): void {
        scene.objects.push(...this.nodes);
    }

    public gatherResourcesFrom(miningAreas: BrushGameObject[]) {
        for (const area of miningAreas) {
            const nodesInArea = this.nodes.filter(n => area.liesWithinPath(n.position));
            for (const node of nodesInArea) {
                this.treasury[node.type] += node.gainPerSecond;
            }
        }
    }
}
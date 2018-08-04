import { Scene, CircleGameObject } from './scene';

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
    public static parseResourceDescription(obj: any, resourceList: Resource[]): ResourceNode {
        let node = new ResourceNode();
        node.type = obj.type;
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
    private resources: Resource[];
    private nodes: ResourceNode[];

    constructor() {
        this.resources = [];
        this.nodes = [];
    }

    public parseResourceBook(json: any): void {
        this.resources = json.resourceTypes.map((obj: any) => new Resource(obj.name, obj.image));
    }

    public parseResourceNodes(json: any): void {
        this.nodes = json.map((n: any) => ResourceNode.parseResourceDescription(n, this.resources));
    }

    public spawnNodes(scene: Scene): void {
        scene.objects.push(...this.nodes);
    }
}
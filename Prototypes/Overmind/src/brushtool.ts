import * as math from './math';
import { Scene, PathGameObject } from './scene';

export interface IBrushTool {
    onPathStarted(scene: Scene, position: math.Vector2): void;
    onPainting(scene: Scene, position: math.Vector2): void;
    onPathComplete(scene: Scene, path: Array<math.Vector2>): void;
}

export class DebugBrushTool implements IBrushTool {
    private gameObject: PathGameObject;
    private path: Array<math.Vector2>;

    onPathStarted(scene: Scene, position: math.Vector2): void {
        this.path = [position];
        this.gameObject = new PathGameObject(this.path, "blue");
        scene.objects.push(this.gameObject);
    }
    onPainting(_: Scene, position: math.Vector2): void {
        if (this.gameObject) {
            this.path.push(position);
        }
    }
    public onPathComplete(_: Scene, path: Array<math.Vector2>): void {
        console.log("Path is ", path.length, " versus ", this.path.length);
        this.gameObject = null as any;
    }
}

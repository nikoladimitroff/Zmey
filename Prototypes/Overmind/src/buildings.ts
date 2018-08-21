import {RectangleGameObject} from './gameobject';
import {checkAndSet} from './utils';
import { Camera } from './camera'


export class Building extends RectangleGameObject {
    public player: string;



    public render(context: CanvasRenderingContext2D, camera: Camera): void {
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

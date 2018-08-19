import { Vector2 } from './vector';
import { Camera } from './camera';

noise.seed(12);

export class Terrain {
    constructor(s: number) {
        this.size = s;
        this.elevationMap = new Array<Array<number>>(this.size);
        for(let i = 0; i < this.size; ++i) {
            this.elevationMap[i] = new Array<number>(this.size);
        }
    }
    public generateElevation(): void {
        for(let i = 0; i < this.elevationMap.length; ++i) {
            for(let j = 0; j < this.elevationMap.length; ++j) {
                this.elevationMap[j][i] = perlin(j, i);
            }
        }
    }

    public render(context: CanvasRenderingContext2D): void {
        let offsetX = Math.floor(context.canvas.width / this.size);
        let offsetY = Math.floor(context.canvas.height / this.size);

        for (let i = 0; i < this.size; ++i) {
            for (let j = 0; j < this.size; ++j) {
                const ele = noise.simplex2(j / 100, i / 100)  * 255;
                context.fillStyle = 'rgb(' + ele + ',' + ele + ',' + ele + ')' ;
                context.fillRect(j * offsetX, i * offsetY, offsetX, offsetY);
            }
        }

     }

    private elevationMap: number[][];
    private size: number;
}
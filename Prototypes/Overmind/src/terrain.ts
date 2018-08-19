import { Vector2 } from './vector';
import { Camera } from './camera';

noise.seed(12);

// each terrain has 3x6 tiles
// each tile is 32/32 pixels

//legend
// t - top
// b - bottom
// l - left
// r - right
// i - inner
// o - outer
// m - middle
// c - corner

// tile layout
// m tloc troc
// m bloc broc
// tlic ti tric
// mlic mid mric
// blic bi bric

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

    public render(context: CanvasRenderingContext2D, tiles: HTMLImageElement): void {
        let offsetX = Math.floor(context.canvas.width / this.size);
        let offsetY = Math.floor(context.canvas.height / this.size);


        for (let i = 0; i < this.size; ++i) {
            for (let j = 0; j < this.size; ++j) {
                const ele = noise.simplex2(j / 100, i / 100)  * 255;
                if(ele > 1)
                {
                    context.drawImage(tiles, 32, 3 * 32, 32, 32, j * offsetX, i * offsetY, offsetX, offsetY);
                }
                else
                {
                    context.drawImage(tiles, 3 * 9 * 32 + 1 * 32, 1 * 3 * 32, 32, 32, j * offsetX, i * offsetY, offsetX, offsetY);
                }
            }
        }

     }

    private elevationMap: number[][];
    private size: number;
}
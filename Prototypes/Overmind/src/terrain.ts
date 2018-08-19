import { Vector2 } from './vector';
import { Camera } from './camera';

function lerp(a0: number, a1: number, w: number): number{
    return (1.0 - w)*a0 + w*a1;
}

function dotGridGradient(ix: number, iy: number, x: number, y: number): number {

    // Precomputed (or otherwise) gradient vectors at each grid node
    //extern float Gradient[IYMAX][IXMAX][2];

    // Compute the distance vector
    let dx = x - ix;
    let dy = y - iy;

    // Compute the dot-product
    //return (dx*gradient[iy][ix][0] + dy*Gradient[iy][ix][1]);

    let v = new Vector2(Math.random(), Math.random());
    v = v.normalized();
    return (dx * v.x + dy * v.y);
}

// Compute Perlin noise at coordinates x, y
function perlin(x: number, y: number): number {

    // Determine grid cell coordinates
    let x0 = Math.floor(x);
    let x1 = x0 + 1;
    let y0 = Math.floor(y);
    let y1 = y0 + 1;

    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    let sx = x - x0;
    let sy = y - y0;

    // Interpolate between grid point gradients
    let n0, n1, ix0, ix1, value;
    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = lerp(n0, n1, sx);
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = lerp(n0, n1, sx);
    value = lerp(ix0, ix1, sy);

    return value;
}

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
                //const ele = perlin(j + j / this.size,i +  i / this.size) * 255;
                const ele = Math.random() * 255;
                //  console.log(ele);
                //const ele = Math.random()*255;
                context.fillStyle = 'rgb(' + ele + ',' + ele + ',' + ele + ')' ;
                context.fillRect(j * offsetX, i * offsetY, offsetX, offsetY);
            }
        }

     }

    private elevationMap: number[][];
    private size: number;
}
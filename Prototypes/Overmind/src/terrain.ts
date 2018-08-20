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

    public static drawTile(context: CanvasRenderingContext2D, tiles: HTMLImageElement, terrainType: string , tileType: string, target: any) {
        context.drawImage(tiles, Terrain.terrains[terrainType].x + Terrain.tilePositions[tileType].x,
                                             Terrain.terrains[terrainType].y + Terrain.tilePositions[tileType].y,
                                              32, 32, target.x , target.y , target.sizeX, target.sizeY);
    }

    public render(context: CanvasRenderingContext2D, tiles: HTMLImageElement): void {
        let sizeX = Math.floor(context.canvas.width / this.size);
        let sizeY = Math.floor(context.canvas.height / this.size);

        for (let i = 0; i < this.size; ++i) {
            for (let j = 0; j < this.size; ++j) {
                const ele = noise.simplex2(j / 50, i / 50);

                const target = {x: j * sizeX,
                                y: i * sizeY,
                                sizeX: sizeX,
                                sizeY: sizeY};
                if (ele > 0.8)
                {
                    Terrain.drawTile(context, tiles, "Snow", "mid", target);
                }
                else if(ele > 0.6)
                {
                    Terrain.drawTile(context, tiles, "BlackDirt", "mid", target);
                }
                else if(ele > 0.5)
                {
                    Terrain.drawTile(context, tiles, "LightDirt", "mid", target);
                }
                else if(ele > 0.0)
                {
                    Terrain.drawTile(context, tiles, "Grass", "mid", target);
                }
                else if (ele > -0.3)
                {
                    Terrain.drawTile(context, tiles, "Sand", "mid", target);
                }
                else
                {
                    Terrain.drawTile(context, tiles, "Water", "mid", target);
                }
            }
        }

     }

    private elevationMap: number[][];
    private size: number;
    private static tilePositions: any = { misk1: {x : 0, y: 0  }, tloc: {x : 32, y: 0  }, troc: {x : 64, y: 0  },
                                          misc2: {x : 0, y: 32 }, bloc: {x : 32, y: 32 }, broc: {x : 64, y: 32 },
                                          tlic:  {x : 0, y: 64 }, ti  : {x : 32, y: 64 }, tric: {x : 64, y: 64 },
                                          mlic:  {x : 0, y: 96 }, mid : {x : 32, y: 96 }, mric: {x : 64, y: 96 },
                                          blic:  {x : 0, y: 128}, bi  : {x : 32, y: 128}, bric: {x : 64, y: 128},};
    private static terrains: any = {
                                    LightDirt   : {x: 0  , y: 0  },
                                    DarkDirt    : {x: 96 , y: 0  },
                                    RedDirt     : {x: 192, y: 0  },
                                    BlackDirt   : {x: 288, y: 0  },
                                    GreyDirt    : {x: 384, y: 0  },
                                    Lava        : {x: 480, y: 0  },
                                    Hole        : {x: 576, y: 0  },
                                    RedHole     : {x: 672, y: 0  },
                                    BlackHole   : {x: 768, y: 0  },
                                    Water       : {x: 864, y: 0  },
                                    Grass       : {x: 0  , y: 192},
                                    Grass2      : {x: 96 , y: 192},
                                    DarkGrass   : {x: 192, y: 192},
                                    ShortGrass  : {x: 288, y: 192},
                                    LongGrass   : {x: 384, y: 192},
                                    Wheat       : {x: 480, y: 192},
                                    Sand        : {x: 576, y: 192},
                                    SandWater   : {x: 672, y: 192},
                                    Water2      : {x: 480, y: 384},
                                    Snow        : {x: 576, y: 384},
                                    SnowWater   : {x: 672, y: 384},
                                    IceWater    : {x: 672, y: 576},
                                    // BrickRoad   : {x: ,     y: 192}
                                    // Sewer       : {x: ,     y: 192}
                                    // SewerWater  : {x: ,     y: 192}
                                    }

}
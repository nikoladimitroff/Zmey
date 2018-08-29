import { Vector2 } from './vector';

noise.seed(125);
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


function isInRange(range: any, v: number): boolean {
    return range.max >= v && range.min <= v;
}

export class TerrainInfo {
    public position:Vector2;
    public biomeName: string;
    public latitude: number;
    public humidity: number;
    public temperature: number;
    public iron:number;
    public wood:number;
    public food:number;
    public radioactivity:number;
    public magicEnergy:number;
    public photoEnergy:number;


    private static mapScaleFactor = 50;
    private findBiome(): any {
        for(let i = 0; i < Terrain.biomes.length; ++i) {
            const current = Terrain.biomes[i];
            if (isInRange(current.latitude, this.latitude) &&
            isInRange(current.humidity, this.humidity) && true
            /*isInRange(current.temperature, tileDescr.temperature)*/) {
                return current;
            }
        }
        return null;
    }

    constructor(pos: Vector2) {
        this.position = pos;
        this.position.x /= TerrainInfo.mapScaleFactor;
        this.position.y /= TerrainInfo.mapScaleFactor;
        this.latitude =         noise.simplex2(pos.x + 10000, pos.y + 10000);
        this.humidity =         (noise.simplex2(pos.x + 20000, pos.y + 20000) + 1) / 2;
        this.temperature =      noise.simplex2(pos.x + 30000, pos.y + 30000);
        this.iron =             noise.simplex2(pos.x + 40000, pos.y + 40000);
        this.wood =             noise.simplex2(pos.x + 50000, pos.y + 50000);
        this.food =             noise.simplex2(pos.x + 60000, pos.y + 60000);
        this.radioactivity =    noise.simplex2(pos.x + 70000, pos.y + 70000);
        this.magicEnergy =      noise.simplex2(pos.x + 80000, pos.y + 80000);
        this.photoEnergy =      noise.simplex2(pos.x + 90000, pos.y + 90000);
        let biome = this.findBiome();
        if (biome) {
            this.biomeName = biome.tileName;
        }
        else {
            console.error(this.latitude + " " + this.humidity);
        }
    }
}

export class Terrain {

    constructor() {
    }

    public static drawTile(context: CanvasRenderingContext2D, tiles: HTMLImageElement, terrainType: string , tileType: string, target: any) {
        context.drawImage(tiles, Terrain.terrains[terrainType].x + Terrain.tilePositions[tileType].x,
                                             Terrain.terrains[terrainType].y + Terrain.tilePositions[tileType].y,
                                              32, 32, target.x , target.y , Terrain.tileSize, Terrain.tileSize);
    }

    public render(context: CanvasRenderingContext2D, tiles: HTMLImageElement): void {
        let sizeW = Math.floor(context.canvas.width / Terrain.tileSize);
        let sizeH = Math.floor(context.canvas.height / Terrain.tileSize);

        for (let i = 0; i < sizeH; ++i) {
            for (let j = 0; j < sizeW; ++j) {
                const target = {x: j * Terrain.tileSize,
                                y: i * Terrain.tileSize};
                let ti = new TerrainInfo(new Vector2(j, i));
                Terrain.drawTile(context, tiles, ti.biomeName, "mid", target);
            }
        }

     }


    private static tileSize = 32;

    private static biomes = [

        {
        name: 'sea',
        latitude: {min: -1.0, max: -0.3},
        humidity: {min: 0, max: 1.0},
        temperature: {min: 0.0, max: 1.0},
        tileName: "Water",
    },
        {
        name: 'beach',
        latitude: {min: -0.3, max: 0.1},
        humidity: {min: 0.0, max: 0.3},
        temperature: {min: 0.0, max: 1.0},
        tileName: "Sand",
    },
            {
        name: 'beach',
        latitude: {min: -0.3, max: -0.2},
        humidity: {min: 0.0, max: 1.0},
        temperature: {min: 0.0, max: 1.0},
        tileName: "Sand",
    },
            {
        name: 'grassland',
        latitude: {min: -0.3, max: 0.7},
        humidity: {min: 0.3, max: 0.6},
        temperature: {min: 0.0, max: 1.0},
        tileName: "Grass",
    },
            {
        name: 'forest',
        latitude: {min: -0.3, max: 0.7},
        humidity: {min: 0.6, max: 0.9},
        temperature: {min: 0.0, max: 1.0},
        tileName: "DarkGrass",
    },
            {
        name: 'tropicalRainForest',
        latitude: {min: -0.3, max: 0.7},
        humidity: {min: 0.9, max: 1.0},
        temperature: {min: 0.0, max: 1.0},
        tileName: "LongGrass",
    },
                {
        name: 'subtropicalDesert',
        latitude: {min: 0.1, max: 0.4},
        humidity: {min: 0.0, max: 0.3},
        temperature: {min: 0.0, max: 1.0},
        tileName: "Sand",
    },
                {
        name: 'temperedDesert',
        latitude: {min: 0.4, max: 0.8},
        humidity: {min: 0.0, max: 0.3},
        temperature: {min: 0.0, max: 1.0},
        tileName: "BlackDirt",
    },
                {
        name: 'scorched',
        latitude: {min: 0.8, max: 1.0},
        humidity: {min: 0.0, max: 0.3},
        temperature: {min: 0.0, max: 1.0},
        tileName: "GreyDirt",
    },
                {
        name: 'bare',
        latitude: {min: 0.7, max: 1.0},
        humidity: {min: 0.3, max: 0.5},
        temperature: {min: 0.0, max: 1.0},
        tileName: "GreyDirt",
    },
                {
        name: 'tundra',
        latitude: {min: 0.7, max: 1.0},
        humidity: {min: 0.5, max: 0.8},
        temperature: {min: 0.0, max: 1.0},
        tileName: "BlackDirt",
    },
        {
        name: 'icesheet',
        latitude: {min: 0.7, max: 1.0},
        humidity: {min: 0.8, max: 1.0},
        temperature: {min: -1.0, max: 1.0},
        tileName: "Snow",
    },
    ];
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
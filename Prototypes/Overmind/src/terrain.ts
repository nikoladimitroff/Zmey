
noise.seed(129);
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

export class Terrain {

    constructor() {
    }

    public static drawTile(context: CanvasRenderingContext2D, tiles: HTMLImageElement, terrainType: string , tileType: string, target: any) {
        context.drawImage(tiles, Terrain.terrains[terrainType].x + Terrain.tilePositions[tileType].x,
                                             Terrain.terrains[terrainType].y + Terrain.tilePositions[tileType].y,
                                              32, 32, target.x , target.y , Terrain.tileSize, Terrain.tileSize);
    }

    private static findBiome(tileDescr: any): any {
        for(let i = 0; i < Terrain.biomes.length; ++i) {
            const current = Terrain.biomes[i];
            if (isInRange(current.latitude, tileDescr.latitude) &&
            isInRange(current.humidity, tileDescr.humidity) && true
            /*isInRange(current.temperature, tileDescr.temperature)*/) {
                return current;
            }
        }
        return null;
    }
    public render(context: CanvasRenderingContext2D, tiles: HTMLImageElement): void {
        let sizeW = Math.floor(context.canvas.width / Terrain.tileSize);
        let sizeH = Math.floor(context.canvas.height / Terrain.tileSize);

        for (let i = 0; i < sizeH; ++i) {
            for (let j = 0; j < sizeW; ++j) {
                const ele = noise.simplex2(j / 50, i / 50);
                const humi = (noise.simplex2(1000+ (j / 50), 1000 + (i / 50)) + 1) / 2;
                const target = {x: j * Terrain.tileSize,
                                y: i * Terrain.tileSize};
                const biome = Terrain.findBiome({latitude: ele, humidity: humi, temperature: 0});
                if (biome) {
                    Terrain.drawTile(context, tiles, biome.tileName, "mid", target);
                }
                else {
                    console.error(ele + " " + humi);
                }
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
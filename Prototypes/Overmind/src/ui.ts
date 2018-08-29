import { EconomyManager } from './economy';
import { UnitBook } from './unittypes';
import { Scene } from './scene';
import { GameObject } from './gameobject';
import { PlayerBook } from './player';
import { DisplayOptions, Camera } from './camera';
import { Vector2 } from './vector';
import { TerrainInfo } from './terrain';
declare class Vue {
    constructor(obj: any);
}

export class UIController {
    private vueApp: any;
    private camera: Camera;
    public initialize(economy: EconomyManager, unitBook: UnitBook, playerBook: PlayerBook, scene: Scene, humanPlayerName: string, camera: Camera) {
        const countUnitByName = (unitName: string) => {
            const isUnitOfType = (o: GameObject) =>
                o.owningPlayer == humanPlayerName &&
                o.unitType == unitName;
            return scene.objects.filter(isUnitOfType).length;
        }
        this.vueApp = new Vue({
            el: "main#ui",
            data: {
                resources: economy.resources,
                treasury: economy.treasury,
                units: unitBook.prototypes,
                countUnitByName: countUnitByName,
                humanPlayer: playerBook.players.find(p => p.name == humanPlayerName),
                displayOption: camera.displayOptions,
                terrainInfo: new TerrainInfo(camera.transformVector(new Vector2(0, 0))),
                showTerrainInfo: false,
                terrainInfoClass: function() {
                    return this.showTerrainInfo ? "" :"terrain-info-display-hide";
                }.bind(this),
            }
        });
        this.camera = camera;
        // use here to silence warning about unused var
        this.vueApp;

            var bsDiv = document.getElementById("terrain-info-display");
            var x, y;
            window.addEventListener('mousemove', function(event){
                if (!this.vueApp.showTerrainInfo) {
                    return;
                }
                let pos:Vector2 = camera.untransformVector(new Vector2(event.clientX, event.clientY)).divide(32);
                pos.x = Math.floor(pos.x);
                pos.y = Math.floor(pos.y);
                 this.vueApp.terrainInfo = new TerrainInfo(pos);

                x = event.clientX;
                y = event.clientY;
                if ( typeof x !== 'undefined' ){
                    bsDiv.style.left = x + "px";
                    bsDiv.style.top = y + "px";
                }
        }.bind(this), false);
    }
}
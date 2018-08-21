import { EconomyManager } from './economy';
import { UnitBook } from './unittypes';
import { Scene } from './scene';
import { GameObject } from './gameobject';
import { PlayerBook } from './player';
import { DisplayOptions } from './camera';
declare class Vue {
    constructor(obj: any);
}
export class UIController {
    private vueApp: any;
    public initialize(economy: EconomyManager, unitBook: UnitBook, playerBook: PlayerBook, scene: Scene, humanPlayerName: string, displayOptions: DisplayOptions) {
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
                displayOption: displayOptions
            }
        });
        // use here to silence warning about unused var
        this.vueApp;
    }
}
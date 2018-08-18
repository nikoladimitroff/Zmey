import { EconomyManager } from './economy';
import { UnitBook } from './unittypes';
import { Scene } from './scene';
import { GameObject } from './gameobject';
import { PlayerBook } from './player';

declare class Vue {
    constructor(obj: any);
}
export class UIController {
    private vueApp: any;
    public initialize(economy: EconomyManager, unitBook: UnitBook, playerBook: PlayerBook, scene: Scene, humanPlayerName: string) {
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
                humanPlayer: playerBook.players.find(p => p.name == humanPlayerName)
            }
        });
        // use here to silence warning about unused var
        this.vueApp;
    }
}
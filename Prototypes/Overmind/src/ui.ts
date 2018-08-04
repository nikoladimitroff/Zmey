import { EconomyManager } from './economy';
import { UnitBook } from './unittypes';
import { Scene, GameObject } from './scene';

declare class Vue {
    constructor(obj: any);
}
export class UIController {
    private vueApp: any;
    public initialize(economy: EconomyManager, unitBook: UnitBook, scene: Scene, humanPlayerName: string) {
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
                countUnitByName: countUnitByName
            }
        });
    }
}
import { Army, UnitDescription } from './unittypes';
import { Scene } from './scene';
import { BrushGameObject } from './brushes';
import { GameObject } from './gameobject';
export class BattleReport {
    public redUnitsLost: number;
    public blueUnitsLost: number;
}

export class BattleSim {

    private static Simulate(ra: Army, ba: Army, dt: number): BattleReport {
        let rakillRate: number = BattleSim.CalculateKillRate(ra.description, ba.description);
        let bakillRate: number = BattleSim.CalculateKillRate(ba.description, ra.description);

        let result = new BattleReport;  

        result.redUnitsLost = ba.count * bakillRate * dt;
        result.blueUnitsLost = ra.count * rakillRate * dt;
        return result;
    }

    private static CalculateKillRate(attacker: UnitDescription, defender: UnitDescription): number {
        let result = 0;

        result = attacker.baseDamage * attacker.weaponFactor * attacker.skillFactor /
        defender.physicalToughness * defender.armorFactor * defender.skillFactor;
        console.log(result);
        return result;
    }

    public static simulateBattles(scene: Scene, gatheringPoints: BrushGameObject[] ): void {
        for (const gp of gatheringPoints) {
            const getUnitByPlayer = (playerName: String, u: GameObject) =>
            u.constructor === Army &&
            u.owningPlayer == playerName && gp.liesWithinPath(u.position);
            let u1: Army[] = scene.objects.filter(getUnitByPlayer.bind(this, scene.players.players[0].name)) as Army[];
            let u2: Army[] = scene.objects.filter(getUnitByPlayer.bind(this, scene.players.players[1].name)) as Army[];
            if(u1.length && u2.length) {

                let executeTroops = function(a: Army, cnt: number) {
                    if( a.count < cnt) {
                        scene.objects.splice( scene.objects.indexOf(a) , 1);
                    }
                    else {
                        a.count -= cnt;
                    }
                }
                let doBattle = function(longer: Army[], shorter: Army[]) {

                    for ( let i = 0; i < longer.length; ++i) {
                        const randomArmyIdx = Math.floor(Math.random() * shorter.length);
                        let result = BattleSim.Simulate(longer[i], shorter[randomArmyIdx], 1);
                        executeTroops(longer[i], result.redUnitsLost);
                        executeTroops(shorter[randomArmyIdx], result.blueUnitsLost);
                    }
                }
                if (u1.length >  u2.length) {
                    doBattle(u1, u2);
                }
                else {
                    doBattle(u2, u1);
                }
            }
        }
    }
}
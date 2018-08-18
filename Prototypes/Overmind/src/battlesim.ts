import { Army, UnitDescription } from './unittypes';

export class BattleReport {
    public redUnitsLost: number;
    public blueUnitsLost: number;
}

export class BattleSim {

    public static Simulate(ra: Army, ba: Army, dt: number): BattleReport {
        let rakillRate: number = BattleSim.CalculateKillRate(ra.description, ba.description);
        let bakillRate: number = BattleSim.CalculateKillRate(ba.description, ra.description);

        let result = new BattleReport;  

        result.redUnitsLost = ba.count * bakillRate * dt;
        result.blueUnitsLost = ra.count * rakillRate * dt;
        return result;
    }

    public static CalculateKillRate(attacker: UnitDescription, defender: UnitDescription): number {
        let result = 0;

        result = attacker.baseDamage * attacker.weaponFactor * attacker.skillFactor /
        defender.physicalToughness * defender.armorFactor * defender.skillFactor;
        console.log(result);
        return result;
    }
}
export class BattleReport {
    public redUnitsLost: number;
    public blueUnitsLost: number;
}

export class Army {
    // formed by army arrangement
    public operationalNumber: number;
    // formed by Unit vs Unit factors
    public killRate: number;
    // formed by terrain conditions, army satisfaction etc
    public mentalFactor: number;
}

export class UnitDescription {
    public mentalToughness: number;
    public physicalToughness: number;
    public baseDamage: number;
    public armorFactor: number;
    public weaponFactor: number;
    public skillFactor: number;
}

export class BattleSim {
    public static Simulate(ra: Army, ba: Army, dt: number): BattleReport {
        let result = new BattleReport;

        result.redUnitsLost = ba.operationalNumber * ba. killRate * ba.mentalFactor * dt;
        result.blueUnitsLost = ra.operationalNumber * ra.killRate * ra.mentalFactor * dt;
        return result;
    }

    public static CalculateKillRate(attacker: UnitDescription, defender: UnitDescription): number {
        let result = 0;

        result = defender.physicalToughness * defender.armorFactor * defender.skillFactor 
        / attacker.baseDamage * attacker.weaponFactor * attacker.skillFactor;
        return result;
    }
}
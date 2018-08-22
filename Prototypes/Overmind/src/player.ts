import {checkAndSet} from './utils';
import { Building } from './buildings'
export class Player {
    public flag: string;
    public name: string;

    public buildings: Building[];
    constructor() {
        this.buildings = [];
    }
}


export class PlayerBook {
    public players: Player[];
    constructor() {
        this.players = [];
    }

    public at(idx: number): Player {
        return this.players[idx];
    }

    private static parsePlayer(obj: any): Player {
        let newPlayer = new Player();
        for(var prop in obj) {
            checkAndSet(newPlayer, obj, prop);
        }
        return newPlayer;
    }

    public static parseBook(json: any): PlayerBook {
        let book = new PlayerBook();
        for (const obj of json.players) {
            book.players.push(PlayerBook.parsePlayer(obj));
        }
        return book;
    }
}
import {checkAndSet} from './utils';

export class Player {
    public flag: string;
    public name: string;
}


export class PlayerBook {
    public players: Player[];
    constructor() {
        this.players = [];
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
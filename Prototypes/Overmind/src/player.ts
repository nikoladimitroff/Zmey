import {checkAndSet} from './utils';

export class Player {
    public flag: string;
}


export class PlayerBook {
    public players: any;
    constructor() {
        this.players = {};
    }

    private static parsePlayer(obj: any): Player {
        let newPlayer = new Player();
        for(var prop in newPlayer) {
            checkAndSet(newPlayer, obj, prop);    
        }
        return newPlayer;
    }    

    public static parseBook(json: any): PlayerBook {
        let book = new PlayerBook();
        for (const obj of json.players) {
            book.players[obj.name] = (PlayerBook.parsePlayer(obj));
        }
        return book;
    }
}
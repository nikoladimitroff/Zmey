

import { PlayerBook } from './player';
import { UnitBook } from './unittypes';
import { BuildingBook } from './buildings';
import { fetchJSON } from './utils'


export class GameLibrary {
    public playerBook: PlayerBook;
    public unitBook: UnitBook;
    public buildingBook: BuildingBook;

    public async init(unitsPath: string,
     playersPath: string,
     buildingsPath: string): Promise<any>  {
        const unitsDescription = await fetchJSON(unitsPath);
        this.unitBook = await UnitBook.parseBook(unitsDescription);

        const playersDescription = await fetchJSON(playersPath);
        this.playerBook = await PlayerBook.parseBook(playersDescription);

        const buildingsDescription = await fetchJSON(buildingsPath);
        this.buildingBook = await BuildingBook.parseBook(buildingsDescription);
    }
}

export function checkAndSet(target: any, donor: any, prop: string): void {
    if(donor[prop] !== undefined) {
        target[prop] = donor[prop];
    }
    else {
        console.assert(false);
    }
}


export async function fetchJSON(url: string): Promise<any> {
    let promise = new Promise(resolve => {
        let req = new XMLHttpRequest();
        req.overrideMimeType("application/json");
        req.open('GET', url, true);
        req.onload  = function() {
            //console.log( req.responseText);
            resolve(eval("new Object(" + req.responseText + ")"));
        };
        req.send(null);
    });
    return await promise;
}


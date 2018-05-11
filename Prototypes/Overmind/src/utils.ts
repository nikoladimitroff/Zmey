
export function checkAndSet(target: any, donor: any, prop: string): void {
    if(donor[prop]) {
        target[prop] = donor[prop];
    }
    else {
        console.assert(false);
    }
}
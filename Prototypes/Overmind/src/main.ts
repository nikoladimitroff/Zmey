import * as math from "./math";

function main(): void {
    const canvas = document.querySelector("canvas");
    console.log(canvas);
    let x = math.Vector2.zero.add(math.Vector2.one);
    console.log(x);
}

main();
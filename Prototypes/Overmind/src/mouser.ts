import * as math from "./math"

// Designed to have the same values as MouseEvent.button
export const enum MouseButton {
    Left,
    Middle,
    Right
}

export class MouseState {
    public screenPosition: math.Vector2;
    public buttonState: { [button: number]: boolean};
    public _wheel: number;
    public get wheel(): number {
        return this._wheel / window.innerHeight;
    }
    constructor() {
        this.screenPosition = new math.Vector2(window.innerWidth / 2, window.innerHeight / 2);
        this.buttonState = [false, false, false];
        this._wheel = 0;
    }
}

export class Mouser {
    public static state: MouseState;
    public static installHandler(): void {
        Mouser.state = new MouseState();

        window.addEventListener("mousemove", function (event) {
            if (event.clientX > window.innerWidth || event.clientY > window.innerHeight) {
                return;
            }
            Mouser.state.screenPosition.set(event.clientX, event.clientY);
        }, false);
        window.addEventListener("mousedown", function (event) {
            Mouser.state.buttonState[event.button] = true;
        }, false);
        window.addEventListener("mouseup", function (event) {
            Mouser.state.buttonState[event.button] = false;
        }, false);
        window.addEventListener("wheel", function (event) {
            Mouser.state._wheel += event.wheelDelta;
        }, false);
    }
}
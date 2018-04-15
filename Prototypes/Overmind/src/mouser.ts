import * as math from "./math"

// Designed to have the same values as MouseEvent.button
export const enum MouseButton {
    Left,
    Middle,
    Right
}

export class MouseState {
    public position: math.Vector2;
    public buttonState: { [button: number]: boolean};
    constructor() {
        this.position = math.Vector2.zero;
        this.buttonState = [false, false, false];
    }
}

export class Mouser {
    public static state: MouseState;
    public static installHandler(): void {
        Mouser.state = new MouseState();

        window.addEventListener("mousemove", function (event) {
            if (event.clientX > window.outerWidth || event.clientY > window.outerHeight) {
                return;
            }
            Mouser.state.position.set(event.clientX, event.clientY);
        }, false);
        window.addEventListener("mousedown", function (event) {
            Mouser.state.buttonState[event.button] = true;
        }, false);
        window.addEventListener("mouseup", function (event) {
            Mouser.state.buttonState[event.button] = false;
        }, false);
    }
}
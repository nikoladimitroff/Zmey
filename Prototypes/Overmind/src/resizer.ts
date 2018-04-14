export class Resizer {
    public static installHandler(canvas: HTMLCanvasElement): void {
        function onresize() {
                canvas.width = window.innerWidth;
                canvas.height = window.innerHeight;
        };
        onresize();
        window.onresize = onresize;
    }
}

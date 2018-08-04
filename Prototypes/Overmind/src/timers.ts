
export class TimerManager {
    public static startTimer(period: number, callback: () => void, shouldLoop: boolean) {
        const timerFunc = shouldLoop ? setInterval : setTimeout;
        timerFunc(callback, period);
    }
}
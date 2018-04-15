"use strict";

export class Vector2 {
    public x: number;
    public y: number;

    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }

    public static zero = new Vector2(0, 0);
    public static right = new Vector2(1, 0);
    public static up = new Vector2(0, 1);
    public static left = new Vector2(-1, 0);
    public static down = new Vector2(0, -1);
    public static one = new Vector2(1, 1);

    public negated(): Vector2 {
        return new Vector2(-this.x, -this.y);
    }
    public add(v: Vector2 | number): Vector2 {
        if (v instanceof Vector2) return new Vector2(this.x + v.x, this.y + v.y);
        else return new Vector2(this.x + v, this.y + v);
    }
    public subtract(v: Vector2 | number): Vector2 {
        if (v instanceof Vector2) return new Vector2(this.x - v.x, this.y - v.y);
        else return new Vector2(this.x - v, this.y - v);
    }
    public multiply(v: Vector2 | number): Vector2 {
        if (v instanceof Vector2) return new Vector2(this.x * v.x, this.y * v.y);
        else return new Vector2(this.x * v, this.y * v);
    }
    public divide(v: Vector2 | number): Vector2 {
        if (v instanceof Vector2) return new Vector2(this.x / v.x, this.y / v.y);
        else return new Vector2(this.x / v, this.y / v);
    }
    public dot(v: Vector2): number {
        return this.x * v.x + this.y * v.y;
    }
    public length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }
    public lengthSquared(): number {
        return this.x * this.x + this.y * this.y;
    }
    public distanceTo(v: Vector2): number {
        var dx = this.x - v.x;
        var dy = this.y - v.y;
        return Math.sqrt(dx * dx + dy * dy);
    }
    public distanceToSquared(v: Vector2): number {
        var dx = this.x - v.x;
        var dy = this.y - v.y;
        return dx * dx + dy * dy;
    }
    public normalized(): Vector2 {
        return this.divide(this.length());
    }
    public rotate(angle: number): Vector2 {
        var cos = Math.cos(angle),
            sin = Math.sin(angle);
        return new Vector2(this.x * cos - this.y * sin,
                          this.x * sin + this.y * cos);
    }
    public angleTo(a: Vector2): number {
        return Math.acos(this.dot(a) / (this.length() * a.length()));
    }
    public angleTo360(a: Vector2): number {
        var n1 = this.clone(),
            n2 = a.clone();
        n1 = n1.normalized();
        n2 = n2.normalized();
        var cos = n1.dot(n2);
        var sin = ((n2.x + n2.y) - (n1.x + n1.y) * cos) / (n1.x - n1.y);
        var angle = Math.acos(cos);

        if (sin <= 0)
            angle = -angle;

        angle += Math.PI / 2;
        return angle;
    }
    public lerpTo(v: Vector2, alpha: number): Vector2 {
        return v.subtract(this).multiply(alpha).add(this);
    }
    public clamp(min: Vector2, max: Vector2): Vector2 {
        return Vector2.max(min, Vector2.min(this, max));
    }
    public min(): number {
        return Math.min(this.x, this.y);
    }
    public max(): number {
        return Math.max(this.x, this.y);
    }
    public equals(v: Vector2): boolean {
        return this.x == v.x && this.y == v.y;
    }
    public isZero(): boolean {
        return this.x === 0 && this.y === 0;
    }
    public toArray(): Array<number> {
        return [this.x, this.y];
    }
    public clone(): Vector2 {
        return new Vector2(this.x, this.y);
    }
    public set(x: Vector2 | number, y?: number): Vector2 {
        if (y === undefined) {
            this.x = (x as Vector2).x;
            this.y = (x as Vector2).y;
            return this;
        }
        this.x = x as number; this.y = y;
        return this;
    }
    public toString(): string {
        return "(" + this.x + ", " + this.y + ")";
    }
    public static max(v: Vector2, u: Vector2): Vector2 {
        return new Vector2(Math.max(v.x, u.x), Math.max(v.y, u.y));
    }
    public static min(v: Vector2, u: Vector2): Vector2 {
        return new Vector2(Math.min(v.x, u.x), Math.min(v.y, u.y));
    }
}

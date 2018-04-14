# Overmind prototype

This is a HTML5-based prototype for the Overmind game.
It's written in plain Typescript, without external canvas libraries.

## To set it up

Run:

```
npm install -g http-server
npm install .
```

* Open up terminal 1 and run:

`npm run-script watch`

* Open up terminal 2 and run:

`http-server -p 3000`

* Go to [http://localhost:3000/](http://localhost:3000/)

## To make changes

*content/scene.json* contains the description of the basic scene. Currently we support
* rectangles and circles as shapes
* solid colours and images as fills

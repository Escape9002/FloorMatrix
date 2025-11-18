# FloorMatrix
Just a matrix to hang up in the floor showing fun messages!

Currently only the Pixel Setting thingy is online, no access to set messages directly

The matrix has 32x8 pixels.

## API for pixels
All messages have a start and stop sign: ```<``` and ```>``` 
### Setting a single pixel:
```p:x,y,r,g,b```

So a complete package would be:
```<p:x,y,r,g,b>```

### Clearing the display:
```<CLEAR>```


# Poetry
To run this project, you need these commands:
```poetry install```
```poetry run floormatrix rainbow```
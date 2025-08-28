
# OkLab Dithering
OkLab Dithering

## JSON
Comments in settings.json not supported

### Settings

```json
{
  "ditherType": "ordered",
  "distanceMode": "srgb",
  "mathMode": "srgb",
  "hideSemiTransparent": true,
  "hideThreshold": 127,
  "mono": false,
  "grayscale": false
}
```

#### `ditherType`
`ordered` or `bayer` for standard ordered dithering  
`floyd` or `floyd-steinberg` or `steinberg` or `fs` for Floyd-Steinberg dithering  
`yliluoma` for Yliluoma ordered dithering  
`none` for no dithering

#### `distanceMode` and `mathMode`
`srgb` or `oklab`

#### `hideSemiTransparent`
`true` or `false`

#### `hideThreshold`
An unsigned integer between `0` and `255`  

#### `mono`
`true` or `false`  
Treats selected palette like it's a monochromatic palette  
Will override `grayscale` setting

#### `grayscale`
`true` or `false`  
Uses `distanceMode` to determine conversion to grayscale

# Credits
[JSON for Modern C++ version 3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)  
[stb_image](https://github.com/nothings/stb)  
[Joel Yliluoma's arbitrary-palette positional dithering algorithm](https://bisqwit.iki.fi/story/howto/dither/jy/)  

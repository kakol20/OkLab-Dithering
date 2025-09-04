
# OkLab Dithering
OkLab Dithering

## JSON
Comments in settings.json not supported

### Settings

```json
{
	"ditherType": "ordered",
	"distanceMode": "oklab",
	"mathMode": "srgb",
	"hideSemiTransparent": false,
	"hideThreshold": 127,
	"mono": false,
	"grayscale": false,
	"matrixType": "bayer"
}
```

#### `ditherType`
`ordered` for ordered dithering  
`floyd` or `floyd-steinberg` or `steinberg` or `fs` for Floyd-Steinberg dithering  
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

#### `matrixType`
Matrix used for ordered dithering  
`bayer` Bayer 16x16 matrix  
`blueNoise16` Blue Noise 16x16 matrix  

# Credits
[JSON for Modern C++ version 3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)  
[stb_image](https://github.com/nothings/stb) 

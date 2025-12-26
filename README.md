
# OkLab Dithering
OkLab Dithering

## JSON
Comments in settings.json not supported

### Settings

```json
{
	"ditherType": "fs",
	"distanceMode": "oklab",
	"mathMode": "lrgb",
	"hideSemiTransparent": false,
	"hideThreshold": 127,
	"mono": false,
	"grayscale": false,
	"matrixType": "bayer",
	"ditherAlpha": true,
	"ditherAlphaFactor": 1,
	"ditherAlphaType": "ordered"
}
```

#### `ditherType`
- `ordered` for ordered dithering  
- `floyd` or `floyd-steinberg` or `steinberg` or `fs` for Floyd-Steinberg dithering  
- `none` for no dithering

#### `distanceMode` and `mathMode`
- `srgb` for sRGB
- `oklab` for all values
- `oklab_l` for lightness only
- `lrgb` for Linear RGB

#### `hideSemiTransparent`
- `true` or `false`

#### `hideThreshold`
- An unsigned integer between `0` and `255`  

#### `mono`
- `true` or `false`  
	- Treats selected palette like it's a monochromatic palette  
	- Will override `grayscale` setting
	- NOTE: Due to the math - math mode `oklab` and `oklab_l` will produce the same result

#### `grayscale`
- `true` or `false`  
	- Uses `distanceMode` to determine conversion to grayscale
	- NOTE: Due to the math, math mode `oklab` and `oklab_l` will produce the same result

#### `matrixType`
Matrix used for ordered dithering  
- `bayer` Bayer 16x16 matrix  
- `blueNoise16` Blue Noise 16x16 matrix

### `ditherAlpha`
- `true` or `false`  
	- Enables dithering of alpha channel
	- This setting is overridden if `hideSemiTransparent` is `true`

### `ditherAlphaFactor`
- An unsigned integer between `0` and `255`  
	- Factor for alpha dithering

### `ditherAlphaType`
- Same options as `ditherType`  
	- Dithering algorithm for alpha channel
	- Uses `matrixType` for the threshold matrix if `ditherType` is `ordered`

# Credits
[JSON for Modern C++ version 3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)  
[stb_image](https://github.com/nothings/stb) 

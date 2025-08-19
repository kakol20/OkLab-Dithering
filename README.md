
# OkLab Dithering
OkLab Dithering

## JSON
Comments in settings.json not supported

### Settings

```json
{
  "grayscale": false,
  "dist_lightness": false,
  "ditherType": "ordered",
  "distanceMode": "srgb",
  "mathMode": "srgb"
}
```
#### `grayscale`
`true` or `false`

#### `dist_lightness`
`true` or `false`

### `ditherType`
`ordered` or `bayer` for ordered dithering  
`floyd` or `floyd-steinberg` or `steinberg` or `fs` for Floyd-Steinberg dithering  
`none` for no dithering

### `distanceMode` and `mathMode`
`srgb` or `oklab`

# Credits
[JSON for Modern C++ version 3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)  
[stb_image](https://github.com/nothings/stb) 


# Credits
[JSON for Modern C++ version 3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)  
[stb_image](https://github.com/nothings/stb) 

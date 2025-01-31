# hslkeyer

Apply a matte to the image based on its HSL components

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_hslkeyer](../assets/img_out/dog_hslkeyer.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_hslkeyer](../assets/img_out/female_hslkeyer.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_hslkeyer](../assets/img_out/male_hslkeyer.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_hslkeyer](../assets/img_out/tree_hslkeyer.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.hslkeyer
bypass=false
hueinterpolationmode=75
hueinverted=1
hueknots=75
lightnessinterpolationmode=75
lightnessinverted=1
lightnessknots=75
outputmode=normal
preblurradius=7500.0
saturationinterpolationmode=75
saturationinverted=1
saturationknots=75

[info]
description=Apply a matte to the image based on its HSL components
fileType=ibp.imagefilterlist
nFilters=1
name=HSL Keyer


```

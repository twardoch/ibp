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
hueinterpolationmode=1
hueinverted=1
hueknots=3
lightnessinterpolationmode=2
lightnessinverted=1
lightnessknots=75
outputmode=normal
preblurradius=40
saturationinterpolationmode=1
saturationinverted=1
saturationknots=3

[info]
description=Apply a matte to the image based on its HSL components
fileType=ibp.imagefilterlist
nFilters=1
name=HSL Keyer


```
